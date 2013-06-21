#include "CommonFun.h"
#include "pdf_analysis.h"

#include <string>
#include <vector>

#include "RooWorkspace.h"
#include "RooExtendPdf.h"
#include "RooAbsPdf.h"
#include "RooRealVar.h"

#include "TRandom3.h"
#include "TH1D.h"
#include "TTree.h"
#include "TCanvas.h"

#include "RooSimWSTool.h"
#include "RooCategory.h"

int main(int argc, char* argv[]) {

  parse_options(argc, argv);
  if (simul && channel) help();

  /// MC shapes
  if (SM && bd_const) {cout << "please select SM OR bd_const, not both" << endl; return EXIT_FAILURE;}

  pdf_analysis ana1(print, ch_s, "all", BF, SM, bd_const, inputs, (!simul_all) ? inputs_bdt : 1, inputs_all, pee, bdt_fit);
  ana1.set_SMratio(0.09);
  if (no_legend) ana1.no_legend = true;
  RooWorkspace *ws = ana1.get_ws();

  /// INPUTS

  static string decays[] = {"SgMc", "BdMc",
  													"bgBs2KK", "bgBs2KPi", "bgBs2PiPi", "bgBd2KK", "bgBd2KPi", "bgBd2PiPi", "bgLb2PiP", "bgLb2KP",
  													"bgBs2KMuNu", "bgBd2PiMuNu", "bgLb2PMuNu",
  													"bgBu2PiMuMu", "bgBu2KMuMu", "bgBd2Pi0MuMu", "bgBd2K0MuMu", "bgBd2MuMuGamma", "bgBs2MuMuGamma",
  													"SgData"};
  int decays_n = sizeof(decays)/sizeof(string);

  string year_s[2] = {"2011", "2012"};
  int years, years_i;
  if (years_opt == "all") years = 2;
  else {
    years = 1;
    years_i = atoi(years_opt.c_str());
  }

  vector <double> cuts_v(2*years, -1);
  if (cuts_f_b) cuts_v = cut_bdt_file();

  vector <string> decays_filename(decays_n);
  vector <string> decays_treename(decays_n);
  vector <string> decays_rdsname(decays_n);

  vector <RooDataSet*> rds_smalltree(decays_n);

  RooRealVar* m = ws->var("Mass");
  RooRealVar* bdt = ws->var("bdt");
  RooRealVar* eta = ws->var("eta");
  RooRealVar* m1eta = ws->var("m1eta");
  RooRealVar* m2eta = ws->var("m2eta");
  RooRealVar* weight = ws->var("weight");
  RooRealVar* MassRes = ws->var("MassRes");
  RooCategory* channel_cat = ws->cat("etacat");
  RooCategory* bdt_cat = ws->cat("bdtcat");
  RooCategory* all_cat = ws->cat("allcat");

  cout << "2011 barrel";
  vector < double > exp_v_0(get_singlerare_normalization("../uml/input/2011/anaBmm.plotResults.2011.tex", 0, decays_n));
  cout << "2011 endcap";
  vector < double > exp_v_1(get_singlerare_normalization("../uml/input/2011/anaBmm.plotResults.2011.tex", 1, decays_n));
  cout << "2012 barrel";
  vector < double > exp_v_2(get_singlerare_normalization("../uml/input/2012/anaBmm.plotResults.2012.tex", 0, decays_n));
  cout << "2012 endcap";
  vector < double > exp_v_3(get_singlerare_normalization("../uml/input/2012/anaBmm.plotResults.2012.tex", 1, decays_n));

  for (int i = 0; i < decays_n; i++) {

    decays_treename[i] = decays[i] + "_bdt";
    decays_rdsname[i] = decays[i] + "_rds";
    RooArgList varlist(*m, *MassRes, *channel_cat);
    if (bdt_fit) varlist.add(*bdt);
    if (simul_bdt || simul_all) varlist.add(*bdt_cat);
    if (simul_all) varlist.add(*all_cat);
    varlist.add(*weight);
    rds_smalltree[i] = new RooDataSet(decays_rdsname[i].c_str(), decays_rdsname[i].c_str(), varlist, "weight");

    for (int yy = 0; yy < years; yy++) {
      int y = (years == 2) ? yy : years_i;
      decays_filename[i] = "../uml/input/" + year_s[y] + "/small-" + decays[i] + ".root";
      cout << endl << decays_filename[i] << endl;
      TFile smalltree_f(decays_filename[i].c_str());
      TTree* reduced_tree = (TTree*)smalltree_f.Get(decays_treename[i].c_str());
      Double_t m_t, eta_t, m1eta_t, m2eta_t, bdt_t, me_t;
      Bool_t muid_t;
      reduced_tree->SetBranchAddress("m",     &m_t);
      reduced_tree->SetBranchAddress("bdt",   &bdt_t);
      reduced_tree->SetBranchAddress("eta",   &eta_t);
      reduced_tree->SetBranchAddress("m1eta", &m1eta_t);
      reduced_tree->SetBranchAddress("m2eta", &m2eta_t);
      reduced_tree->SetBranchAddress("me",    &me_t);
      reduced_tree->SetBranchAddress("muid",  &muid_t);
      double entries = reduced_tree->GetEntries();
      double events_0 = 0, events_1 = 0, events_2 = 0, events_3 = 0;
      for (int j = 0; j < entries && (j < 100000 || i == decays_n - 1 || make_bdt_binning_inputs); j++) {
        reduced_tree->GetEntry(j);
        Bool_t barrel = fabs(m1eta_t) < 1.4 && fabs(m2eta_t) < 1.4;
        if (m_t > 5.20 && m_t < 5.45 && i == decays_n-1) continue; // skip signal windows for comb bkg
        if (m_t < 4.90 || m_t > 5.90 || !muid_t) continue; // skip outside range
        if (me_t < 0.0 || me_t > 0.2) continue; //skip wrong mass error
        if (bdt_t < bdt_cut) continue;
        if (y == 0) {
          if (barrel) {
          	if (cuts_f_b && bdt_t < cuts_v[0]) continue;
          	events_0++;
          }
          else {
          	if (cuts_f_b && bdt_t < cuts_v[1]) continue;
          	events_1++;
          }
        }
        else {
          if (barrel) {
          	if (cuts_f_b && bdt_t < cuts_v[2]) continue;
          	events_2++;
          }
          else {
          	if (cuts_f_b && bdt_t < cuts_v[3]) continue;
          	events_3++;
          }
        }
      }
      for (int j = 0; j < entries && (j < 100000 || i == decays_n - 1 || make_bdt_binning_inputs); j++) {
        reduced_tree->GetEntry(j);
        Bool_t barrel = fabs(m1eta_t) < 1.4 && fabs(m2eta_t) < 1.4;
        m->setVal(m_t);
        eta->setVal(eta_t);
        m1eta->setVal(m1eta_t);
        m2eta->setVal(m2eta_t);
        bdt->setVal(bdt_t);
//        if (m_t > 5.20 && m_t < 5.45 && i == decays_n-1) continue; // skip signal windows for comb bkg
        if (m_t < 5.45 && i == decays_n-1) continue; // for comb bkg, take right SB only
        if (m_t < 4.90 || m_t > 5.90 || !muid_t) continue; // skip outside range
        if (me_t < 0.0 || me_t > 0.2) continue; //skip wrong mass error
        if (bdt_t < bdt_cut) continue;
        MassRes->setVal(me_t);
        /// eta channels
        int eta_channel = -1;
        if (barrel) {
          eta_channel = 0 + 2*yy;
          channel_cat->setIndex(eta_channel);
          if (i != decays_n-1) {
            if (cuts_f_b && bdt_t < cuts_v[0 + 2*yy]) continue;
          }
          else {
            if (cuts_f_b && bdt_t < 0.1) continue; // give some statistics to comb!
          }
        }
        else {
          eta_channel = 1 + 2*yy;
          channel_cat->setIndex(eta_channel);
          if (i != decays_n-1) {
            if (cuts_f_b && bdt_t < cuts_v[1 + 2*yy]) continue;
          }
          else {
            if (cuts_f_b && bdt_t < 0.1) continue; // give some statistics to comb!
          }
        }
        /// bdt channels
        int bdt_channel = ana1.bdt_index(eta_channel, bdt_t);
        if (simul_bdt || simul_all) {
          if (bdt_channel == -1) continue; /// bdt < ###
          bdt_cat->setIndex(bdt_channel);
        }
        if (simul_all) all_cat->setIndex(ana1.super_index(eta_channel, bdt_channel));

        double weight = 1;
        if (i > 1 && i < decays_n-1) {
          if (y == 0) {
            if (barrel) weight = exp_v_0[i] / events_0;
            else weight = exp_v_1[i] / events_1;
          }
          else {
            if (barrel) weight = exp_v_2[i] / events_2;
            else weight = exp_v_3[i] / events_3;
          }
        }
        RooArgList varlist_tmp(*m, *MassRes, *channel_cat);
        if (bdt_fit) varlist_tmp.add(*bdt);
        if (simul_bdt || simul_all) varlist_tmp.add(*bdt_cat);
        if (simul_all) varlist_tmp.add(*all_cat);
        rds_smalltree[i]->add(varlist_tmp, weight);
      }
      cout << rds_smalltree[i]->GetName() << " done: " << rds_smalltree[i]->numEntries() << "(" << rds_smalltree[i]->sumEntries() << " weighted) from tree " << reduced_tree->GetEntries() << endl;
      smalltree_f.Close();
    }
  }

  /// datasets
  RooAbsData* rad_bs = rds_smalltree[0];

  RooAbsData* rad_bd = rds_smalltree[1];

  RooDataSet* rds_peak = (RooDataSet*)rds_smalltree[2]->Clone("rds_peak");
  for (int i = 3; i <= 9; i++) rds_peak->append(*rds_smalltree[i]);
  RooAbsData* rad_peak = rds_peak;

  RooDataSet* rds_semi = (RooDataSet*)rds_smalltree[10]->Clone("rds_semi");
  for (int i = 11; i <= 18; i++) rds_semi->append(*rds_smalltree[i]);
  RooAbsData* rad_semi = rds_semi;

  RooAbsData* rad_comb = rds_smalltree[decays_n-1];

  TH1D * bs_bdt_histo[inputs];
  TH1D * bd_bdt_histo[inputs];
  TH1D * peak_bdt_histo[inputs];
  TH1D * semi_bdt_histo[inputs];
  TH1D * comb_bdt_histo[inputs];
  TH1D * bu_bdt_histo[inputs];
  for (int j = 0; j < inputs; j++) {
    ana1.channel = simul ? j : ch_i;
    /// 1D
    for (unsigned int k = 0; k < ana1.bdt_index_max(j); k++) {
      ana1.channel_bdt = (simul_bdt || simul_all) ? k : ch_bdt_i;
      ana1.define_massRes_pdf(rds_smalltree[0], "bs", rkeys);
      ana1.define_massRes_pdf(rds_smalltree[1], "bd", rkeys);
      ana1.define_massRes_pdf(rds_semi, "semi", rkeys);
      ana1.define_massRes_pdf(rds_peak, "peak", rkeys);
      ana1.define_massRes_pdf(rds_smalltree[decays_n-1], "comb", rkeys);
    }
    /// 2D
    if (bdt_fit) {
    	TFile *bdt_syst_f = new TFile("input/div_bdt_jpsi_bin0.root");
    	bs_bdt_histo[ana1.channel] = ana1.define_bdt_pdf(rds_smalltree[0], "bs", bdt_syst_f, rkeys);
    	bd_bdt_histo[ana1.channel] = ana1.define_bdt_pdf(rds_smalltree[1], "bd", bdt_syst_f, rkeys);
    	peak_bdt_histo[ana1.channel] = ana1.define_bdt_pdf(rds_semi, "semi", bdt_syst_f, rkeys);
    	semi_bdt_histo[ana1.channel] = ana1.define_bdt_pdf(rds_peak, "peak", bdt_syst_f, rkeys);
      TFile *bdt_syst_comb_f = new TFile("input/div_bdt.root");
      comb_bdt_histo[ana1.channel] = ana1.define_bdt_pdf(rds_smalltree[decays_n-1], "comb", bdt_syst_comb_f, rkeys);
    }
  }

  if (make_bdt_binning_inputs) { /// Normalization
  	if (!bdt_fit) {
  		cout << "make_bdt_binning_inputs works only with bdt_fit" << endl;
  		return EXIT_FAILURE;
  	}
  	for (int j = 0; j < inputs; j++) {
  		bu_bdt_histo[j] = new TH1D(Form("bdt_bu_%d_h", j), "bdt_bu", 1000, -1, 1);
  	}
  	for (int yy = 0; yy < years; yy++) {
  		int y = (years == 2) ? yy : years_i;
  		string filename = "../uml/input/" + year_s[y] + "/small-NoMc.root";
  		cout << endl << filename << endl;
  		TFile smalltree_f(filename.c_str());
  		TTree* reduced_tree = (TTree*)smalltree_f.Get("NoMc_bdt");
  		Double_t m_t, eta_t, m1eta_t, m2eta_t, bdt_t, me_t;
  		Bool_t muid_t;
  		reduced_tree->SetBranchAddress("m",     &m_t);
  		reduced_tree->SetBranchAddress("bdt",   &bdt_t);
  		reduced_tree->SetBranchAddress("eta",   &eta_t);
  		reduced_tree->SetBranchAddress("m1eta", &m1eta_t);
  		reduced_tree->SetBranchAddress("m2eta", &m2eta_t);
  		reduced_tree->SetBranchAddress("me",    &me_t);
  		reduced_tree->SetBranchAddress("muid",  &muid_t);
  		double entries = reduced_tree->GetEntries();
  		for (int j = 0; j < entries; j++) {
  			reduced_tree->GetEntry(j);
  			Bool_t barrel = fabs(m1eta_t) < 1.4 && fabs(m2eta_t) < 1.4;
  			if (m_t < 4.90 || m_t > 5.90 || !muid_t) continue; // skip outside range
  			if (me_t < 0.0 || me_t > 0.2) continue; //skip wrong mass error
  			if (y == 0) {
  				if (barrel)	bu_bdt_histo[0]->Fill(bdt_t);
  				else	bu_bdt_histo[1]->Fill(bdt_t);
  			}
  			else {
  				if (barrel) bu_bdt_histo[2]->Fill(bdt_t);
  				else bu_bdt_histo[3]->Fill(bdt_t);
  			}
  		}
  		smalltree_f.Close();
  	}

  	TFile * bdt_f = new TFile(bdtbinnings_s.c_str(), "RECREATE");
  	for (int j = 0; j < inputs; j++) {
  		bs_bdt_histo[j]->Write();
  		bd_bdt_histo[j]->Write();
  		peak_bdt_histo[j]->Write();
  		semi_bdt_histo[j]->Write();
  		comb_bdt_histo[j]->Write();
  		bu_bdt_histo[j]->Write();
  	}
  	bdt_f->Close();
  	cout << "written bdt pdfs to " << bdtbinnings_s << endl;
  	return EXIT_SUCCESS;
  }

  ana1.define_N();
  ana1.get_bkg_from_tex();

  ana1.define_pdfs();

  /// FITS
  for (int j = 0; j < inputs; j++) {
    for (unsigned int k = 0; k < ana1.bdt_index_max(j); k++) {
      ana1.channel = simul ? j : ch_i;
      ana1.channel_bdt = (simul_bdt || simul_all) ? k : ch_bdt_i;
      /// bs
      ana1.fit_pdf(ana1.name("bs", j, k), rad_bs, false, true, false);

      /// bd
      ana1.fit_pdf(ana1.name("bd", j, k), rad_bd, false, true, false);

      /// peak
      ana1.fit_pdf(ana1.name("peak", j, k), rad_peak, false, true, false);

      /// semi
      ana1.fit_pdf(ana1.name("semi", j, k), rad_semi, false, true, false);

      /// comb
      ana1.setSBslope(ana1.name("comb", j, k), rad_comb);
//        ana1.fit_pdf(ana1.name("comb", j, k), rad_comb, false, true, false, false);

//      ana1.print_ = false;
//      ana1.define_rare3(j, k);
//      ana1.fit_pdf(ana1.name("expo3", j, k), rad_semi, false, true, false);
//      ana1.print_ = print;
    }
  }

  ostringstream inputs_oss; inputs_oss << inputs;
  string output_s = "output/ws_";
  if (simul) output_s += "simul" + inputs_oss.str() + "_" + meth;
  else output_s += "pdf_" + meth + "_" + ch_s;
  if (simul_bdt) output_s += "_simulBdt";
  if (simul_all) output_s += "_simulAll";
  if (BF > 0) output_s += Form("_BF%d", BF);
  if (SM) output_s += "_SM";
  if (bd_const) output_s += "_BdConst";
  if (bdt_fit) output_s += "_2D";
  if (pee) output_s += "_PEE";
  output_s += ".root";
  TFile * output_f = new TFile(output_s.c_str(), "RECREATE");
  ws->Write();
  output_f->Close();
  if (!simul && !simul_bdt) ws->pdf("pdf_ext_total")->graphVizTree(Form("ext_%s.dot", ch_s.c_str()));
  ws->Print();

//  if (!simul && !SM && !bd_const) {
//    ws->var("N_bs")->setVal(5);
//    ws->var("N_bd")->setVal(2);
//    ws->var("N_peak")->setVal(1);
//    ws->var("N_semi")->setVal(1);
//    ws->var("N_comb")->setVal(25);
//    ana1.gen_and_fit("pdf_ext_total");
//  }
  cout << "workspace saved to " << output_s << endl;
  return EXIT_SUCCESS;
}

