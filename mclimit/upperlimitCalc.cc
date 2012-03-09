#include <stddef.h>
#include <iostream>
#include <string>
#include <fstream>
#include "TFile.h"
#include "TROOT.h"
#include "mclimit_csm.hh"
#include "TH1.h"
#include "TF1.h"
#include "TRandom3.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TArrow.h"
#include "TStopwatch.h"
#include "TString.h"

#define N_MASS_BINS 6
#define N_BDT_BINS 4

using namespace std;

int main(int argc, char **argv)
{
	TStopwatch t;
	t.Start();
	if (argc<3||argc>3) {
		cout << endl;
		cout << "Usage: bin/multibin_cls option1 option2" << endl;
		cout << "Option1 can be:" << endl;
		cout << "---> cms11bs or cmb11bd for summer 11 Bs or Bd" << endl; 
		cout << "---> cms12bs or cms12bd for Spring 12 Bs or Bd" << endl; 
		cout << "---> lhcb the combined 10 and 11 lhcb results" << endl;
		cout << "---> lhcb10 or lhcb11 for either data set" << endl;
		cout << "---> comb11 or comb12 (not available yet) for CMS-LHCb combined result" << endl;
		cout << "Option2 is a number (int) that is used to set the random seed." << endl;
		cout << "NOTE: In TRandom3, SetSeed(0) is always random so the results are NOT reproducible." << endl;
		cout << endl;
		return 0;		
	}
	
	delete gRandom;
	gRandom = (TRandom*) new TRandom3;
	
	char cms11bschan[] = "cms11bs"; char cms11bdchan[] = "cms11bd";
	char cms12bschan[] = "cms12bs"; char cms12bdchan[] = "cms12bd"; 
	char lhcbchan[] = "lhcb"; char lhcb10chan[] = "lhcb10"; char lhcb11chan[] = "lhcb11";
	char combned11[] = "comb11"; char combned12[] = "comb12";
	bool combined11 = false; bool combined12 = false; 
	bool cms11bs = false; bool cms11bd = false;
	bool cms12bs = false; bool cms12bd = false;
	bool lhcbs = false; bool lhcbs10 = false; bool lhcbs11 = false;
	bool barrel = false; bool endcap = false; 
	int i_numb = -1;

	if (!strcmp(cms11bschan,argv[1])) {
		cms11bs = true;
		barrel = true;
		endcap = true;
		i_numb = atoi(argv[2]);
	}
	else if (!strcmp(cms11bdchan,argv[1])) {
		cms11bd = true;
		barrel = true;
		endcap = true;
		i_numb = atoi(argv[2]);
	}
	else if (!strcmp(cms12bschan,argv[1])) {
		cms12bs = true;
		barrel = true;
		endcap = true;
		i_numb = atoi(argv[2]);
	}	
	else if (!strcmp(cms12bdchan,argv[1])) {
		cms12bd = true;
		barrel = true;
		endcap = true;
		i_numb = atoi(argv[2]);
	}	
	else if (!strcmp(lhcbchan,argv[1])) {
		lhcbs = true;
		lhcbs10 = true;
		lhcbs11 = true;
		i_numb = atoi(argv[2]);
	}
	else if	(!strcmp(combned11,argv[1])) {
		combined11 = true;
		cms11bs = true;
		barrel = true;
		endcap = true;
		
		lhcbs = true;
		lhcbs10 = true;
		lhcbs11 = true;

		i_numb = atoi(argv[2]);
	}
	else if	(!strcmp(combned12,argv[1])) {
		combined12 = true;
		cms12bs = true;
		barrel = true;
		endcap = true;
		
		lhcbs = true;
		lhcbs10 = true;
		lhcbs11 = true;
		
		i_numb = atoi(argv[2]);
	}
	else if (!strcmp(lhcb10chan,argv[1])) {
		lhcbs10 = true;
		
		i_numb = atoi(argv[2]);
	}
	else if (!strcmp(lhcb11chan,argv[1])) {
		lhcbs11 = true;
		
		i_numb = atoi(argv[2]);
	}	

		
	gRandom->SetSeed(i_numb);

	TString s_outputfile;
	if (combined11) s_outputfile = Form("CMS_LHCb_S11_Comb_Bs_Results%i.txt",i_numb);
	else if (combined12) s_outputfile = Form("CMS_LHCb_W12_Comb_Bs_Results%i.txt",i_numb);
	else if(cms11bs) s_outputfile = Form("CMS_S11_Bs_Results%i.txt",i_numb);
	else if(cms11bd) s_outputfile = Form("CMS_S11_Bd_Results%i.txt",i_numb);
	else if(cms12bs) s_outputfile = Form("CMS_W12_Bs_Results%i.txt",i_numb);
	else if(cms12bd) s_outputfile = Form("CMS_W12_Bd_Results%i.txt",i_numb);
	else if(lhcbs) s_outputfile = Form("LHCb_10_11_Comb_Bs_Results%i.txt",i_numb);
	else if(lhcbs10) s_outputfile = Form("LHCb_10_Bs_Results%i.txt",i_numb);
	else if(lhcbs11) s_outputfile = Form("LHCb_11_Bs_Results%i.txt",i_numb);
	
	ofstream f_outputfile;
	f_outputfile.open(s_outputfile);
	
	f_outputfile << "Setting the random seed to = " << i_numb << endl;
	
#include "prepUprLimCalc.h"
  
	cout << "Back from the prepare header file" << endl;
	
	//========================================
	// Have a visualization of how fitting looks like
	// It has nothing to do with limit calculation
	TCanvas *mycanvas = (TCanvas *) new TCanvas("Canvas1","Canvas1",0,0,600,600);
	if ((cms11bs || cms11bd || cms12bs || cms12bd) && !(combined11 || combined12)) {
		mycanvas->Divide(2,2);
		mycanvas->cd(1);
	}
		
	cout << ">>>>>>> Begin calculating!!! <<<<<<<<" << endl;
	csm* mycsm = new csm();
	if (cms11bs || cms11bd || cms12bs || cms12bd) {
		mycsm->set_htofit(h_dataB,channameB);
		mycsm->set_htofit(h_dataE,channameE);
	}
	if (lhcbs11) {
		for (int j = 0; j<N_MASS_BINS; j++) {
			for ( int l = 0; l<N_BDT_BINS; l++) {
				char * c_lhcbchan = new char[s_lhcbchan[j][l].size() + 1];
				copy(s_lhcbchan[j][l].begin(), s_lhcbchan[j][l].end(), c_lhcbchan);
				c_lhcbchan[s_lhcbchan[j][l].size()] = '\0'; 
				mycsm->set_htofit(h_lhcbdat[j][l],c_lhcbchan);
				
				
				delete[] c_lhcbchan;
			}
		}
	}
	if (lhcbs10) {
		for (int j = 0; j<N_MASS_BINS; j++) {
			for ( int l = 0; l<N_BDT_BINS; l++) {				
				char * c_lhcbchan1 = new char[s_lhcbchan1[j][l].size() + 1];
				copy(s_lhcbchan1[j][l].begin(), s_lhcbchan1[j][l].end(), c_lhcbchan1);
				c_lhcbchan1[s_lhcbchan1[j][l].size()] = '\0'; 			
				mycsm->set_htofit(h_lhcbdat1[j][l],c_lhcbchan1);
				
				delete[] c_lhcbchan1;
			}
		}
	}

	mycsm->set_modeltofit(testhyp);
	double chisq = mycsm->chisquared();
	
	csm_model* bestnullfit = mycsm->getbestmodel();
	if (cms11bs || cms11bd || cms12bs || cms12bd) { //////////////
		bestnullfit->plotwithdata(channameB,h_dataB);
		if ((cms11bs || cms11bd || cms12bs || cms12bd) && !(combined11 || combined12)) mycanvas->cd(2);/////////
		bestnullfit->plotwithdata(channameE,h_dataE);
	}
	if (lhcbs11) {
		for (int j = 0; j<N_MASS_BINS; j++) {
			for ( int l = 0; l<N_BDT_BINS; l++) {
				char * c_lhcbchan = new char[s_lhcbchan[j][l].size() + 1];
				copy(s_lhcbchan[j][l].begin(), s_lhcbchan[j][l].end(), c_lhcbchan);
				c_lhcbchan[s_lhcbchan[j][l].size()] = '\0'; 
				bestnullfit->plotwithdata(c_lhcbchan,h_lhcbdat[j][l]);
								
				delete[] c_lhcbchan;
			}
		}
	}
	if (lhcbs10) {
		for (int j = 0; j<N_MASS_BINS; j++) {
			for ( int l = 0; l<N_BDT_BINS; l++) {
				char * c_lhcbchan1 = new char[s_lhcbchan1[j][l].size() + 1];
				copy(s_lhcbchan1[j][l].begin(), s_lhcbchan1[j][l].end(), c_lhcbchan1);
				c_lhcbchan1[s_lhcbchan1[j][l].size()] = '\0'; 			
				bestnullfit->plotwithdata(c_lhcbchan1,h_lhcbdat1[j][l]);
				
				delete[] c_lhcbchan1;
			}
		}
	}

	cout << "chisq from fitter " << chisq << endl; 
	delete mycsm;
	
	
	//======================================
	//Construct limit calculator - mclimit_csm
	//  Sensitivity, Significance calculation
	
	mclimit_csm* mymclimit = (mclimit_csm*) new mclimit_csm();		
	//print out pseudo-experiments details 
	//mymclimit->setpxprintflag(1);	
	cout << ">>>>>> setting up hypotheses <<<<<<<" << endl;
	mymclimit->set_null_hypothesis(nullhyp);
	mymclimit->set_test_hypothesis(testhyp);
	mymclimit->set_null_hypothesis_pe(nullhyp_pe);
	mymclimit->set_test_hypothesis_pe(testhyp_pe);
	if (cms11bs || cms11bd || cms12bs || cms12bd) { ////////////////
		mymclimit->set_datahist(h_dataB,channameB);
		mymclimit->set_datahist(h_dataE,channameE);
	}
	if (lhcbs11) {
		for (int j = 0; j<N_MASS_BINS; j++) {
			for ( int l = 0; l<N_BDT_BINS; l++) {
				char * c_lhcbchan = new char[s_lhcbchan[j][l].size() + 1];
				copy(s_lhcbchan[j][l].begin(), s_lhcbchan[j][l].end(), c_lhcbchan);
				c_lhcbchan[s_lhcbchan[j][l].size()] = '\0'; 
				mymclimit->set_datahist(h_lhcbdat[j][l],c_lhcbchan);
								
				delete[] c_lhcbchan;
			}
		}
	}
	if (lhcbs10) {
		for (int j = 0; j<N_MASS_BINS; j++) {
			for ( int l = 0; l<N_BDT_BINS; l++) {
				char * c_lhcbchan1 = new char[s_lhcbchan1[j][l].size() + 1];
				copy(s_lhcbchan1[j][l].begin(), s_lhcbchan1[j][l].end(), c_lhcbchan1);
				c_lhcbchan1[s_lhcbchan1[j][l].size()] = '\0'; 			
				mymclimit->set_datahist(h_lhcbdat1[j][l],c_lhcbchan1);
				
				delete[] c_lhcbchan1;
			}
		}
	}
	cout << ">>>>>>> PRINTING Pseudo-exp (debug purposes) <<<<<<<<" << endl;
	testhyp_pe->print();
	mymclimit->set_npe(10000);
	mymclimit->run_pseudoexperiments();
	
	
	cout << "<<<<<<<< Getting Results >>>>>>>>" << endl;
	Double_t tsobs = mymclimit->ts();// test statistics of observed data
	cout << "Test statistics of observed data --> " << "ts: " << tsobs << endl;
	cout << "Test statistic in null hypothesis (H0), 2sig low edge --> " << "tsbm2: "   << mymclimit->tsbm2() << endl;
	cout << "Test statistic in null hypothesis (H0), 1sig low edge --> " << "tsbm1: "   << mymclimit->tsbm1() << endl;
	cout << "Test statistic in null hypothesis (H0), medium --> "        << "tsbmed: "  << mymclimit->tsbmed() << endl;
	cout << "Test statistic in null hypothesis (H0), 1sig upper edge --> " << "tsbp1: " << mymclimit->tsbp1() << endl;
	cout << "Test statistic in null hypothesis (H0), 2sig upper edge --> " << "tsbp2: " << mymclimit->tsbp2() << endl;
	cout << "Test statistic in null hypothesis (H1), 2sig low edge --> "   << "tssm2: " << mymclimit->tssm2() << endl;
	cout << "Test statistic in null hypothesis (H1), 1sig low edge --> "   << "tssm1: " << mymclimit->tssm1() << endl;
	cout << "Test statistic in null hypothesis (H0), medium --> "         << "tssmed: " << mymclimit->tssmed() << endl;
	cout << "Test statistic in null hypothesis (H1), 1sig upper edge --> " << "tssp1: " << mymclimit->tssp1() << endl;
	cout << "Test statistic in null hypothesis (H1), 2sig upper edge --> " << "tssp2: " << mymclimit->tssp2() << endl;
	
	f_outputfile << "CLb = " << mymclimit->clb() << endl;
	f_outputfile << "CLsb = " << mymclimit->clsb() << endl;
	f_outputfile << "CLs (CLsb/CLb) = " << mymclimit->cls() << endl;

	f_outputfile << "<<<<<<<< Getting Expected CLs Results for Null Hyp >>>>>>>>" << endl;
	f_outputfile << "CLs -2sigma (bkg): " << mymclimit->clsexpbm2() << endl;
	f_outputfile << "CLs -1sigma (bkg): " << mymclimit->clsexpbm1() << endl;
	f_outputfile << "CLs median  (bkg): " << mymclimit->clsexpbmed() << endl;
	f_outputfile << "CLs +1sigma (bkg): " << mymclimit->clsexpbp1() << endl;
	f_outputfile << "CLs +2sigma (bkg): " << mymclimit->clsexpbp2() << endl;
	f_outputfile << "<<<<<<<< Could background only explain the observed data? >>>>>>>>" << endl;
	f_outputfile << "1-CLb -2sigma (sig): " << mymclimit->omclbexpsm2() << endl;
	f_outputfile << "1-CLb -1sigma (sig): " << mymclimit->omclbexpsm1() << endl;
	f_outputfile << "1-CLb median  (sig): " << mymclimit->omclbexpsmed() << endl;
	f_outputfile << "1-CLb +1sigma (sig): " << mymclimit->omclbexpsp1() << endl;
	f_outputfile << "1-CLb +2sigma (sig): " << mymclimit->omclbexpsp2() << endl;
	
	f_outputfile << "<<<<<<<< Getting Probabilities assuming test Hyp is TRUE >>>>>>>>" << endl;
	// Sensitivity of test hypothesis. Probability of a x-sigma evidence assuming test hyp. is true
	f_outputfile << "2-sigma prob (H1 is true): " << mymclimit->p2sigmat() << endl; 
	f_outputfile << "3-sigma prob (H1 is true): " << mymclimit->p3sigmat() << endl; 
	f_outputfile << "5-sigma prob (H1 is true): " << mymclimit->p5sigmat() << endl; 

	f_outputfile << "<<<<<<<< Getting results from Rate calculations >>>>>>>>" << endl;
	double d_sf95 = mymclimit->s95();
	f_outputfile << "Scale factor of 95% CL excluded signal: s95 = " << d_sf95 << endl;
	if (combined11) f_outputfile << "95% CL upper limit (CERN combined summer 11): Br(Bs->MuMu) = " << d_sf95*(3.2e-9) << endl;
	if (combined12) f_outputfile << "95% CL upper limit (CERN combined winter 11-12): Br(Bs->MuMu) = " << d_sf95*(3.2e-9) << endl;
	else if (cms11bs) f_outputfile << "95% CL upper limit (CMS summer 11): Br(Bs->MuMu) = " << d_sf95*(3.2e-9) << endl;
	else if (cms11bd) f_outputfile << "95% CL upper limit (CMS summer 11): Br(Bd->MuMu) = " << d_sf95*(1.0e-10) << endl;
	else if (cms12bs) f_outputfile << "95% CL upper limit (CMS winter 11-12): Br(Bs->MuMu) = " << d_sf95*(3.2e-9) << endl;
	else if (cms12bd) f_outputfile << "95% CL upper limit (CMS winter 11-12): Br(Bd->MuMu) = " << d_sf95*(1.0e-10) << endl;
	else if (lhcbs) f_outputfile << "95% CL upper limit (LHCb): Br(Bs->MuMu) = " << d_sf95*(3.2e-9) << endl;
	else if (lhcbs10) f_outputfile << "95% CL upper limit (LHCb 10): Br(Bs->MuMu) = " << d_sf95*(3.2e-9) << endl;
	else if (lhcbs11) f_outputfile << "95% CL upper limit (LHCb 11): Br(Bs->MuMu) = " << d_sf95*(3.2e-9) << endl;
	
	f_outputfile << "s95 -2sigma (sig): " << mymclimit->s95m2() << endl;
	f_outputfile << "s95 -1sigma (sig): " << mymclimit->s95m1() << endl;
	f_outputfile << "s95 median  (sig): " << mymclimit->s95med() << endl;
	f_outputfile << "s95 +1sigma (sig): " << mymclimit->s95p1() << endl;
	f_outputfile << "s95 +2sigma (sig): " << mymclimit->s95p2() << endl;
		
	//Null hypothesis 
	f_outputfile << "Null Hypothesis:"
				 << " Median of test statistics = " << mymclimit->tsbmed() 
				 << ", p-value = " << mymclimit->omclbexpbmed()
				 << ", significance = "<< TMath::ErfcInverse(mymclimit->omclbexpbmed()*2)*sqrt(2)
				 << endl;
	
	
	//Expected significance
	f_outputfile << "Test Hypothesis:"
				 << " Median of test statistics = " << mymclimit->tssmed() 
				 << ", p-value = " << mymclimit->omclbexpsmed() 
				 << ", significance = " << TMath::ErfcInverse(mymclimit->omclbexpsmed()*2)*sqrt(2)
				 << endl;
	
	// Significance of input data 
	double ts_data = mymclimit->ts();
	double pval = mymclimit->omclb();
	double significance = TMath::ErfcInverse(pval*2)*sqrt(2); //convert pval to one-side gaussian significance
	
	f_outputfile << "Input Data: test statistics = " << ts_data << ", p-val = " << pval 
				 << ", significance = " << significance << endl;
	
	
	//==============================================
	//Draw the test statistics distribution 
	//test hypothesis - we can calculate probability of observering more than N sigma 
	//null hypothesis - we can calculate p-value of a measured test statistics  
	TH1F* ts_test = new TH1F("ts_test","",100,-50,50);
	TH1F* ts_null = new TH1F("ts_null","",100,-50,50);
	mymclimit->tshists(ts_test,ts_null); 
	if ((cms11bs || cms11bd || cms12bs || cms12bd) && !(combined11 || combined12)) mycanvas->cd(3);//////
	ts_null->SetLineColor(2);//red
	ts_test->SetLineColor(4);//blue
	ts_null->Draw();
	ts_test->Draw("same");
	
	TArrow* arrow = new TArrow(ts_data,0.3*ts_null->GetMaximum(),ts_data,0);
	arrow->Draw();
	TString s_pdffilename;
	if (combined11) s_pdffilename = Form("CMS_LHCb_S11_Comb_Bs_Results%i.pdf",i_numb);
	else if (combined12) s_pdffilename = Form("CMS_LHCb_W12_Comb_Bs_Results%i.pdf",i_numb);
	else if (cms11bs) s_pdffilename = Form("CMS_S11_Bs_Results%i.pdf",i_numb);
	else if (cms11bd) s_pdffilename = Form("CMS_S11_Bd_Results%i.pdf",i_numb);
	else if (cms12bs) s_pdffilename = Form("CMS_W12_Bs_Results%i.pdf",i_numb);
	else if (cms12bd) s_pdffilename = Form("CMS_W12_Bd_Results%i.pdf",i_numb);
	else if (lhcbs) s_pdffilename = Form("LHCb_10_11_Comb_Bs_Results%i.pdf",i_numb);
	else if (lhcbs10) s_pdffilename = Form("LHCb_10_Bs_Results%i.pdf",i_numb);
	else if (lhcbs11) s_pdffilename = Form("LHCb_11_Bs_Results%i.pdf",i_numb);
	mycanvas->Print(s_pdffilename);
	
	
	delete mymclimit;
	f_outfile.Write();
	f_outfile.Close();
	f_outputfile.close();

	t.Stop();
	t.Print();
}