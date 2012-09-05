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
  if (!input || !method || !channel) help();
  
  /// MC shapes
  if (SM && bd_const) {cout << "please select SM OR bd_const, not both" << endl; return EXIT_FAILURE;}
  if (pee && strcmp(input_name.c_str(), "new")) {cout << "per event error only with new trees, select -i new" << endl; return EXIT_FAILURE;}

  pdf_analysis ana1(print, meth, ch_s, "all", SM, bd_const);
  if (pee) ana1.pee = true;
  if (no_legend) ana1.no_legend = true;

  if (strcmp(input_name.c_str(), "new")) ana1.old_tree = true;
  ana1.initialize();
  RooWorkspace *ws = ana1.get_ws();

   /// INPUTS
  RooAbsData* rad_bs;
  RooAbsData* rad_bd;
  RooAbsData* rad_signals;
  RooAbsData* rad_peak;
  RooAbsData* rad_semi;
  RooAbsData* rad_rare;
  RooAbsData* rad_signalsrare;
  RooAbsData* rad_uniform;
  RooAbsData* rad_total;

  if (strcmp(input_name.c_str(), "new")) { // old way
    TFile* input_f = new TFile(input_name.c_str());
    TH1D* Bs_h = (TH1D*)input_f->Get(Form("Bs_%s_chan%s", meth.c_str(), ch_s.c_str()));
    TH1D* Bd_h = (TH1D*)input_f->Get(Form("Bd_%s_chan%s", meth.c_str(), ch_s.c_str()));

    double SM_ratio = Bd_h->Integral() / Bs_h->Integral();
    ana1.set_SMratio(SM_ratio);

    TH1D* Rare_h;
    if (ch_s=="0") Rare_h = (TH1D*)input_f->Get(Form("bRare_%s", meth.c_str()));
    if (ch_s=="1") Rare_h = (TH1D*)input_f->Get(Form("eRare_%s", meth.c_str()));
    ///histo SB real data
    TH1D* data_h = (TH1D*)input_f->Get(Form("hMassWithAllCuts_%s_5_chan%s", meth.c_str(), ch_s.c_str()));
    for (int i = 1; i < Bs_h->GetNbinsX(); i++){
      if (Bs_h->GetBinCenter(i) < 4.90 || Bs_h->GetBinCenter(i) > 5.90) {
        Bs_h->SetBinContent(i, 0.);
        Bs_h->SetBinError(i, 0.);
        Bd_h->SetBinContent(i, 0.);
        Bd_h->SetBinError(i, 0.);
        Rare_h->SetBinContent(i, 0.);
        Rare_h->SetBinError(i, 0.);
        data_h->SetBinContent(i, 0.);
        data_h->SetBinError(i, 0.);
      }
    }
    //// FIXMEEEEE ERRORS!!!
    for (int i = 1; i < Bs_h->GetNbinsX(); i++) {
      Bs_h->SetBinError(i, Bs_h->GetBinContent(i)*0.15);
      Bd_h->SetBinError(i, Bd_h->GetBinContent(i)*0.15);
      Rare_h->SetBinError(i, Rare_h->GetBinContent(i)*0.20);
    }
    TH1D* signals_h = (TH1D*)Bs_h->Clone("signals");
    signals_h->Add(Bd_h, 1);
    TH1D* signalsrare_h = (TH1D*)signals_h->Clone("signalsrare_h");
    signalsrare_h->Add(Rare_h, 1);
    TH1D* bkg_sub_h = (TH1D*)data_h->Clone("bkg_sub_h");
    bkg_sub_h->Add(Rare_h, -1.);
    for (int i = 1; i < bkg_sub_h->GetNbinsX(); i++){
      if ( (bkg_sub_h->GetBinCenter(i) >= 5.20 && bkg_sub_h->GetBinCenter(i) <= 5.45) || bkg_sub_h->GetBinCenter(i) < 4.90 || bkg_sub_h->GetBinCenter(i) > 5.90) {
        bkg_sub_h->SetBinContent(i, 0.);
        bkg_sub_h->SetBinError(i, 0.);
      }
    }
    /// combinatorial estimation
    double X_comb_estimated = bkg_sub_h->Integral() * (1. / 0.75); // 1 GeV / 0.75 GeV
    int N_comb_estimated = (int)X_comb_estimated;
    if (X_comb_estimated - N_comb_estimated > 0.5) N_comb_estimated++;
    cout << "estimated comb = " << N_comb_estimated << endl;
    TH1D* uniform_h = new TH1D("uniform_h", "uniform_h", signals_h->GetNbinsX(), signals_h->GetXaxis()->GetXmin(), signals_h->GetXaxis()->GetXmax());
    TRandom3 rand0(0);
    double N = rand0.Poisson(N_comb_estimated);
    for (int i = 1; i <= N; i++) {
      TRandom3 rand1(0);
      double n = rand1.Uniform(4.9, 5.9);
      uniform_h->Fill(n);
    }
    TH1D* total_h = (TH1D*)signalsrare_h->Clone("total_h");
    total_h->Add(uniform_h);

    RooDataHist* rdh_bs = new RooDataHist("rdh_bs", "dataset template for Bs", *ws->var("Mass"), Bs_h);
    RooDataHist* rdh_bd = new RooDataHist("rdh_bd", "dataset template for Bd", *ws->var("Mass"), Bd_h);
    RooDataHist* rdh_signals = new RooDataHist("rdh_signals", "dataset template for signals", *ws->var("Mass"), signals_h);
    RooDataHist* rdh_rare = new RooDataHist("rdh_rare", "dataset template for rare bkg", *ws->var("Mass"), Rare_h);
    RooDataHist* rdh_signalsrare = new RooDataHist("rdh_signalsrare", "dataset template for signals + rare bkg", *ws->var("Mass"), signalsrare_h);
    RooDataHist* rdh_uniform = new RooDataHist("rdh_uniform", "rdh_uniform", *ws->var("Mass"), uniform_h);
    RooDataHist* rdh_total = new RooDataHist("rdh_total", "rdh_total", *ws->var("Mass"), total_h);

    rad_bs = rdh_bs;
    rad_bd = rdh_bd;
    rad_signals = rdh_signals;
    rad_rare = rdh_rare;
    rad_signalsrare = rdh_signalsrare;
    rad_uniform = rdh_uniform;
    rad_total = rdh_total;
  }
  else { //new small trees way

    string cut = get_cut(ch_i);

    string decays[] = {"SgMc", "BdMc", "bgBd2KK", "bgBd2KPi", "bgBd2PiPi", "bgBs2KK", "bgBs2KPi", "bgBs2PiPi", "bgLb2KP", "bgLb2PiP", "bgBd2PiMuNu", "bgBs2KMuNu", "bgLb2PMuNu"};
    double weight_i[] = { 1,      1,      1./27.6,   1./1.1,     1./1.6,      1./1.0,    1./2.5,     1./2.5,      1./1.3,    1./2.2,     1./1.0,        1./1.1,       1./1.1};
    // misid:
    double pion_misid    = 0.0015;
    double kaons_misid   = 0.0017;
    double protons_misid = 0.0005;
    weight_i[2]  *= kaons_misid*kaons_misid;
    weight_i[3]  *= kaons_misid*pion_misid;
    weight_i[4]  *= pion_misid*pion_misid;
    weight_i[5]  *= kaons_misid*kaons_misid;
    weight_i[6]  *= kaons_misid*pion_misid;
    weight_i[7]  *= pion_misid*pion_misid;
    weight_i[8]  *= kaons_misid*protons_misid;
    weight_i[9]  *= pion_misid*protons_misid;
    weight_i[10] *= pion_misid;
    weight_i[11] *= kaons_misid;
    weight_i[12] *= protons_misid;

    int decays_n = sizeof(decays)/sizeof(string);

    vector <string> decays_filename(decays_n);
    vector <string> decays_treename(decays_n);
    vector <string> decays_rdsname(decays_n);

    vector <TFile*> smalltree_f(decays_n);
    vector <TTree*> smalltree(decays_n);
    vector <RooDataSet*> rds_smalltree(decays_n);

    RooRealVar* m = ws->var("Mass");
    RooRealVar* eta = ws->var("eta");
    RooRealVar* weight = new RooRealVar("weight", "weight", 0., 100000., "");
    m->SetName("m");
    for (int i = 0; i < decays_n; i++) {
      decays_filename[i] = "input/small-" + decays[i] + ".root";
      decays_treename[i] = decays[i] + "_bdt";
      decays_rdsname[i] = decays[i] + "_rds";

      smalltree_f[i] = new TFile(decays_filename[i].c_str(), "UPDATE");
      smalltree[i] = (TTree*)smalltree_f[i]->Get(decays_treename[i].c_str());
      TTree* reduced_tree = smalltree[i]->CopyTree(cut.c_str());
      reduced_tree->Branch("weight", &weight_i[i], "weight/D");
      rds_smalltree[i] = new RooDataSet(decays_rdsname[i].c_str(), decays_rdsname[i].c_str(), RooArgSet(*m, *eta, *weight), Import(*reduced_tree), WeightVar("weight"));
      rds_smalltree[i]->changeObservableName("m", "Mass");
      cout << rds_smalltree[i]->GetName() << " done: " << reduced_tree->GetEntries() << " / " << smalltree[i]->GetEntries() << endl;
    }
    m->SetName("Mass");

    rad_bs = rds_smalltree[0];
    ana1.define_etapdf(rds_smalltree[0], "bs");

    rad_bd = rds_smalltree[1];
    ana1.define_etapdf(rds_smalltree[1], "bd");

    RooDataSet* rds_signals = (RooDataSet*)rds_smalltree[0]->Clone("rds_signals");
    rds_signals->append(*rds_smalltree[1]);
    rad_signals = rds_signals;
    ana1.define_etapdf(rds_signals, "signals");

    RooDataSet* rds_semi = (RooDataSet*)rds_smalltree[10]->Clone("rds_semi");
    rds_semi->append(*rds_smalltree[11]);
    rds_semi->append(*rds_smalltree[12]);
    rad_semi = rds_semi;
    ana1.define_etapdf(rds_semi, "semi");

    RooDataSet* rds_peak = (RooDataSet*)rds_smalltree[2]->Clone("rds_peak");
    rds_peak->append(*rds_smalltree[3]);
    rds_peak->append(*rds_smalltree[4]);
    rds_peak->append(*rds_smalltree[5]);
    rds_peak->append(*rds_smalltree[6]);
    rds_peak->append(*rds_smalltree[7]);
    rds_peak->append(*rds_smalltree[8]);
    rds_peak->append(*rds_smalltree[9]);
    rad_peak = rds_peak;
    ana1.define_etapdf(rds_peak, "peak");

    RooDataSet* rds_rare = (RooDataSet*)rds_peak->Clone("rds_rare");
    rds_rare->append(*rds_semi);
    rad_rare = rds_rare;
    ana1.define_etapdf(rds_rare, "rare");

    RooDataSet* rds_signalsrare = (RooDataSet*)rds_signals->Clone("rds_signalsrare");
    rds_signalsrare->append(*rds_rare);
    rad_signalsrare = rds_signalsrare;
    ana1.define_etapdf(rds_rare, "signalsrare");

    //no uniform without estimation

    //    rad_uniform = rdh_uniform;
    //    rad_total = rdh_total;
  }
  ana1.simul_ = false;
  ana1.define_pdfs();
  ws->Print();
  /// FITS
  /// bs
  ana1.fit_pdf("bs", rad_bs, false);

  /// bd
  ana1.fit_pdf("bd", rad_bd, false);

  /// signals
  ana1.fit_pdf("signals", rad_signals, false);

 if (!strcmp(input_name.c_str(), "new")) {
   /// peak
   ana1.fit_pdf("peaking", rad_peak, false);

   /// semi
   ana1.fit_pdf("nonpeaking", rad_semi, false);
 }

 /// rare
 // ana1.define_rare2(rad_rare);
 ana1.fit_pdf("rare", rad_rare, false);
// ana1.define_rare3();
// ana1.fit_pdf("expo", rad_rare, false);

  if (strcmp(input_name.c_str(), "new")) {

  /// signals + rare
    ana1.fit_pdf("signalsrare", rad_signalsrare, false);

  /// comb
    //ana1.fit_pdf("comb", rad_uniform, false);
  
  /// total
    ana1.fit_pdf("total", rad_total, true);
  }

  string output_s = "output/ws_pdf_" + meth + "_" + ch_s;
  if (SM) output_s += "_SM";
  if (bd_const) output_s += "_BdConst";
  if (pee) output_s += "_PEE";
  output_s += ".root";
  ws->SaveAs(output_s.c_str());
  
  cout << "starting Bs " << rad_bs->sumEntries() << " fit Bs " << ws->var("N_bs")->getVal() << endl;
  if (SM) cout << "starting Bd " << rad_bd->sumEntries() << " fit Bd constraint to SM" << endl;
  if (!SM && !bd_const) cout << "starting Bd " << rad_bd->sumEntries() << " fit Bd " << ws->var("N_bd")->getVal() << endl;
  if (bd_const) cout << "starting Bd " << rad_bd->sumEntries() << " fit Bd " << ws->function("N_bd_constr")->getVal() << endl;
  if (strcmp(input_name.c_str(), "new")) {
    cout << "starting rare " << rad_rare->sumEntries() << " fit rare " << ws->var("N_rare")->getVal() << endl;
    cout << "starting comb " << rad_uniform->sumEntries() << " fit comb " << ws->var("N_comb")->getVal() << endl;
  }
  ws->pdf("pdf_ext_total")->graphVizTree(Form("exttotal_%s.dot", ch_s.c_str()));
  return EXIT_SUCCESS;
}
