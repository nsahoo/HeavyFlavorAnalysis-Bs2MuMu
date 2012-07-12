/*
 *  bplus_estimator.cpp
 *
 *  Created by Christoph Nägeli <christoph.naegeli@psi.ch> on 09.03.11.
 *
 */

#include "bplus_estimator.h"

// Helper Classes
#include "../rootutils/NCRootUtils.h"

// Standard headers
#include <iostream>
#include <cmath>

// ROOT headers
#include <TFile.h>
#include <TH1D.h>
#include <TStyle.h>
#include <TEfficiency.h>
#include <TEventList.h>

using namespace std;

void estimate_bplus(std::map<bmm_param,measurement_t> *bplus, TTree *dataTree, TTree *mcTree, TTree *accTree, double minEta, double maxEta, uint32_t channelIx, TCut anaCut, std::map<systematics_t,double> *systematics_table)
{
	int64_t nbr_gens;
	int64_t nbr_acc,nbr_acc2;
	int64_t nbr_reco;
	int64_t nbr_trig;
	int64_t nbr_ana;
	int64_t nbr_mu;
	fit_t fit;
	TCut cut;
	TCut channelCut = TCut(Form("TMath::Abs(eta_mu1) < %f && TMath::Abs(eta_mu2) < %f",maxEta,maxEta)) && TCut(Form("!(TMath::Abs(eta_mu1) < %f && TMath::Abs(eta_mu2) < %f)",minEta,minEta));
	TCut truthCut("true_decay == 19");
	TCut acceptanceCutMCSoft("TMath::Abs(eta_mu1_gen) < 2.5 && TMath::Abs(eta_mu2_gen) < 2.5 && pt_mu1_gen > 1. && pt_mu2_gen > 1. && pt_kp1_gen > .4 && TMath::Abs(eta_kp1_gen) < 2.5");
	TCut acceptanceCutMCHard("TMath::Abs(eta_mu1_gen) < 2.5 && TMath::Abs(eta_mu2_gen) < 2.5 && pt_mu1_gen > 3.5 && pt_mu2_gen > 3.5 && TMath::Abs(eta_mu1) < 2.4 && TMath::Abs(eta_mu2) < 2.4 && (track_qual_mu1 & 4) && (track_qual_mu2 & 4) && pt_kp1_gen > .4 && TMath::Abs(eta_kp1_gen) < 2.5 && pt_kp1 > .5 && TMath::Abs(eta_kp1) < 2.4");
	TCut acceptanceCutData("(q_mu1*q_mu2 < 0) && TMath::Abs(eta_mu1) < 2.4 && TMath::Abs(eta_mu2) < 2.4 && pt_mu1 > 1. && pt_mu2 > 1. && (track_qual_mu1 & 4) && (track_qual_mu2 & 4) && pt_kp1 > .5 && TMath::Abs(eta_kp1) < 2.4 && (track_qual_kp1 & 4)");
	TCut muonCut("tight_mu1 && tight_mu2");
	TCut triggerCut("triggered_jpsi");
	measurement_t mes;
	TEfficiency effCalc("eff","",1,0,1); // we only need one bin
	TH1D *mbplus = new TH1D("mbplus","",40,4.8,6.0);
	double obs;
	TCanvas *can = NULL;
	static int plot_nbr = 1;
	map<systematics_t,double>::iterator syst_it;
	TEventList *elist;
	TEventList treeList;
	
	anaCut = anaCut && TCut("3.0 < mass_dimuon && mass_dimuon < 3.2"); // j/psi cut
	
	cout << "===> Processing channel " << channelIx << ": B+ -> J/psi K+" << endl;
	
	// display statistics
	gStyle->SetOptStat(111111);
	gStyle->SetOptFit(111);
	
	// configure the efficiency calculation
	effCalc.SetConfidenceLevel(0.68);
	effCalc.SetStatisticOption(TEfficiency::kFCP);
	
	/************************************
	 * Working with the acceptance tree *
	 ************************************/
	accTree->GetDirectory()->cd();
	
	// generated
	cout << "\tCounting generator canidates..." << flush;
	cut = TCut("candidate == -68");
	accTree->Draw(">>elist",cut);
	elist = (TEventList*)gDirectory->Get("elist");
	nbr_gens = elist->GetN();
	cout << '\t' << nbr_gens << endl;
	elist->Write("normGens", TObject::kOverwrite);
	
	// acceptance
	cout << "\tCounting accepted candidates..." << flush;
	cut = TCut("candidate == 68") && acceptanceCutMCSoft && acceptanceCutData && channelCut; // channel cut also goes into acceptance
	accTree->Draw(">>elist", cut);
	elist = (TEventList*)gDirectory->Get("elist");
	nbr_acc = elist->GetN();
	cout << '\t' << nbr_acc << flush;
	elist->Write(Form("normGensAcc_%d",channelIx), TObject::kOverwrite);
	
	cut = TCut("candidate == 68") && acceptanceCutMCHard && acceptanceCutData && channelCut; // for high stat mc different generator cuts
	accTree->Draw(">>elist", cut);
	elist = (TEventList*)gDirectory->Get("elist");
	nbr_acc2 = elist->GetN();
	cout << " (" << nbr_acc2 << " / " << flush;
	elist->Write(Form("normGensAccHard_%d",channelIx), TObject::kOverwrite);
	
	/****************************
	 * Working with the MC tree *
	 ****************************/	
	mcTree->GetDirectory()->cd();
	
	// the same, but with enlarged statistics
	mcTree->Draw(">>elist", cut);
	elist = (TEventList*)gDirectory->Get("elist");
	nbr_reco = elist->GetN();
	cout << nbr_reco << ")" << endl;	
	elist->Write(Form("normGensAccHard_%d",channelIx), TObject::kOverwrite);
	
	// analysis efficiency
	cout << "\tCounting analysis candidates..." << flush;
	cut = TCut("candidate == 300521") && acceptanceCutMCHard && acceptanceCutData && channelCut && anaCut && truthCut;
	mcTree->Draw(">>elist",cut);
	elist = (TEventList*)gDirectory->Get("elist");
	nbr_ana = elist->GetN();
	treeList.Clear();
	treeList.Add(elist);
	mcTree->SetEventList(&treeList);
	cout << '\t' << nbr_ana << endl;
	elist->Write(Form("normAna_%d",channelIx), TObject::kOverwrite);
	
	// muon efficiency
	cout << "\tCounting muon candidates..." << flush;
	mcTree->Draw(">>elist",muonCut);
	elist = (TEventList*)gDirectory->Get("elist");
	nbr_mu = elist->GetN();
	treeList.Clear();
	treeList.Add(elist);
	mcTree->SetEventList(&treeList);
	cout << '\t' << nbr_mu << endl;
	elist->Write(Form("normMuon_%d",channelIx), TObject::kOverwrite);
	
	// trigger efficiency
	cout << "\tCounting trigger candidates..." << flush;
	mcTree->Draw(">>elist",triggerCut);
	elist = (TEventList*)gDirectory->Get("elist");
	nbr_trig = elist->GetN();
	treeList.Clear();
	treeList.Add(elist);
	mcTree->SetEventList(&treeList);
	cout << '\t' << nbr_trig << endl;
	elist->Write(Form("normTrig_%d",channelIx), TObject::kOverwrite);
	
	mcTree->SetEventList(NULL);
	
	cout << "	Computing efficiencies..." << flush;
	// acceptance
	effCalc.SetTotalEvents(0, nbr_gens);
	effCalc.SetPassedEvents(0, nbr_acc);
	(*bplus)[make_pair(kAcc_bplus, channelIx)] = measurement_t(effCalc.GetEfficiency(0),(effCalc.GetEfficiencyErrorUp(0) + effCalc.GetEfficiencyErrorLow(0))/2.);
	
	// systematics tracking efficiency
	if( (syst_it = systematics_table->find(g_sys_acc_efftrack)) != systematics_table->end() ) {
		mes = (*bplus)[make_pair(kAcc_bplus, channelIx)];
		mes = measurement_t(0, mes.getVal() * syst_it->second);
		(*bplus)[make_pair(kAcc_bplus, channelIx)] = mes + (*bplus)[make_pair(kAcc_bplus, channelIx)];
	}
	
	// historical reason
	(*bplus)[make_pair(kEff_cand_bplus, channelIx)] = measurement_t(1., 0.);
	
	// analysis efficiency
	effCalc.SetTotalEvents(0, nbr_reco*nbr_acc/nbr_acc2);
	effCalc.SetPassedEvents(0, nbr_ana);
	(*bplus)[make_pair(kEff_ana_bplus, channelIx)] = measurement_t(effCalc.GetEfficiency(0),(effCalc.GetEfficiencyErrorUp(0) + effCalc.GetEfficiencyErrorLow(0))/2.);
	
	// muon efficiency
	effCalc.SetTotalEvents(0, nbr_ana);
	effCalc.SetPassedEvents(0, nbr_mu);
	(*bplus)[make_pair(kEff_mu_bplus, channelIx)] = measurement_t(effCalc.GetEfficiency(0),(effCalc.GetEfficiencyErrorUp(0) + effCalc.GetEfficiencyErrorLow(0))/2.);
	
	// trigger efficiency
	effCalc.SetTotalEvents(0, nbr_mu);
	effCalc.SetPassedEvents(0, nbr_trig);
	(*bplus)[make_pair(kEff_trig_bplus, channelIx)] = measurement_t(effCalc.GetEfficiency(0),(effCalc.GetEfficiencyErrorUp(0) + effCalc.GetEfficiencyErrorLow(0))/2.);
	
	cout << "\tdone" << endl;
	
	// get the number of observed Bu -> J/psi Kp
	cout << "	Measuring number of B+ -> J/Psi K+ decays..." << endl;
	can = new TCanvas;
	cut = TCut("candidate == 300521") && acceptanceCutData && channelCut && anaCut && muonCut && triggerCut;
	dataTree->Draw("mass >> mbplus", cut);
	init_gauss_linear(&fit, 5.1, 5.5);
	adjust_parameter_gauss_linear(mbplus, fit.fit_fct);
	mbplus->Fit(fit.fit_fct,"R"); // for convergence, first chi2 fit
	mbplus->Fit(fit.fit_fct,"LR"); // Likelihood fit
	
	obs = signal_events_gauss_linear(fit.fit_fct, mbplus->GetBinWidth(1));
	fit.fit_fct->SetParameter(0,fit.fit_fct->GetParError(0));
	(*bplus)[make_pair(kObs_bplus, channelIx)] = measurement_t(obs,signal_events_gauss_linear(fit.fit_fct, mbplus->GetBinWidth(1)));
	
	// systematic uncertainty on number of observed bplus to jpsi kp
	if ( (syst_it = systematics_table->find(g_sys_normfit)) != systematics_table->end() ) {
		mes = (*bplus)[make_pair(kObs_bplus, channelIx)];
		mes = measurement_t(0, mes.getVal() * syst_it->second);
		(*bplus)[make_pair(kObs_bplus, channelIx)] = mes + (*bplus)[make_pair(kObs_bplus, channelIx)];
	}
	
	// save the histogram
	mbplus->Draw("E1");
	can->SaveAs(Form("obs_bplus_%d.eps",plot_nbr++));
	
	accTree->SetEventList(0);
	mcTree->SetEventList(0);
	
	delete can;
	delete fit.fit_fct;
	delete mbplus;
} // estimate_bplus()
