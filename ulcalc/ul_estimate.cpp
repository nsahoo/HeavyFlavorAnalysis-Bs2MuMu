/*
 *  ul_estimate.cpp
 *  final_calculator
 *
 *  Created by Christoph Nägeli <christoph.naegeli@psi.ch> on 17.03.11.
 *  Copyright 2011 Christoph Nägeli. All rights reserved.
 *
 */

#include "ul_estimate.h"

// ROOT headers
#include <TStopwatch.h>

// RooFit headers
#include <RooProdPdf.h>
#include <RooRealVar.h>
#include <RooPoisson.h>

// RooStats headers
#include <RooStats/ModelConfig.h>
#include <RooStats/FeldmanCousins.h>
#include <RooStats/BayesianCalculator.h>
#include <RooStats/RooStatsUtils.h>
#include <RooStats/FrequentistCalculator.h>
#include <RooStats/HybridCalculator.h>
#include <RooStats/HypoTestInverter.h>
#include <RooStats/ProfileLikelihoodTestStat.h>
#include <RooStats/RatioOfProfiledLikelihoodsTestStat.h>

using namespace std;

/* Add all the channels present in bmm to channels */
void add_channels(std::map<bmm_param,measurement_t> *bmm, std::set<int> *channels)
{
	std::map<bmm_param,measurement_t>::const_iterator it;
	for (it = bmm->begin(); it != bmm->end(); ++it)
		channels->insert(it->first.second);
} // add_channels()

RooWorkspace *build_model_nchannel(map<bmm_param,measurement_t> *bsmm, map<bmm_param,measurement_t> *bdmm, bool no_errors, int verbosity, bool compute_bd_ul, bool fixed_bkg, bool floatPoissonians)
{
	RooStats::ModelConfig *splusbModel = NULL;
	RooStats::ModelConfig *smModel = NULL;
	RooStats::ModelConfig *bModel = NULL;
	RooWorkspace *wspace = new RooWorkspace("wspace");
	RooProdPdf *totalPdf = NULL;
	RooProdPdf *priorPdf = NULL;
	RooArgList priorList;
	RooArgList poissonList;
	RooArgSet observables;
	RooArgSet nuisanceParams;
	set<int> channels;
	set<int>::const_iterator chan;
	measurement_t m;
	
	add_channels(bsmm,&channels);
	add_channels(bdmm,&channels);
	
	// make sure we cover the entire physical range
	wspace->factory("mu_s[1,0,20]");	// initialize to standard model
	wspace->factory("mu_d[1,0,200]");	// initialize to standard model
	
	// global correlated variables of all channels
	m = f_ratio();
	if (m.getErr() > 0 && !no_errors) {
		wspace->factory(Form("fratio0[%f]",m.getVal()));
		wspace->factory(Form("fratioErr[%f]",m.getErr()));
		wspace->factory(Form("fratio[%f,0,10]",m.getVal()));
		wspace->factory("Gaussian::fratio_Gauss(fratio,fratio0,fratioErr)");
	} else {
		wspace->factory(Form("fratio[%f,0,1]",m.getVal()));
		wspace->var("fratio")->setConstant(kTRUE);
	}
	
	// Create channel specific variables
	for (chan = channels.begin(); chan != channels.end(); ++chan) {
		
		// Observables
		wspace->factory(Form("NsObs_%d[0,1000]",*chan)); // Observed Evts in Bs window
		wspace->factory(Form("NdObs_%d[0,1000]",*chan)); // Observed Evts in Bd window
		wspace->factory(Form("NbObs_%d[0,1000]",*chan)); // Observed Background
		observables.add(*wspace->var(Form("NsObs_%d",*chan)));
		observables.add(*wspace->var(Form("NdObs_%d",*chan)));
		if (fixed_bkg)
			wspace->var(Form("NbObs_%d",*chan))->setConstant(kTRUE);
		else
			observables.add(*wspace->var(Form("NbObs_%d",*chan)));
		
		// nuisance parameter
		wspace->factory(Form("nu_b_%d[1,0,1000]",*chan)); // background strength
		
		///////////////////////////////////
		// build the background ratio Bs //
		///////////////////////////////////
		if ( ((*bsmm)[make_pair(kTau_bmm, *chan)]).getErr() > 0 && !no_errors ) {
			m = (*bsmm)[make_pair(kTau_bmm, *chan)];
			wspace->factory(Form("TauS0_%d[%f]",*chan,m.getVal()));
			wspace->factory(Form("TauSErr_%d[%f]",*chan,m.getErr()));
			wspace->factory(Form("TauS_%d[%f,%f,%f]",*chan,m.getVal(),0.0,1.0));
			wspace->factory(Form("Gaussian::TauS_Gauss_%d(TauS_%d,TauS0_%d,TauSErr_%d)",*chan,*chan,*chan,*chan));
		}
		else {
			wspace->factory(Form("TauS_%d[%f,0,10]",*chan, ((*bsmm)[make_pair(kTau_bmm, *chan)]).getVal()));
			wspace->var(Form("TauS_%d",*chan))->setConstant(kTRUE);
		}
		
		////////////////////////////////////
		// build the background ration Bd //
		////////////////////////////////////
		if ( ((*bdmm)[make_pair(kTau_bmm, *chan)]).getErr() > 0 && !no_errors ) {
			m = (*bdmm)[make_pair(kTau_bmm, *chan)];
			wspace->factory(Form("TauD0_%d[%f]",*chan,m.getVal()));
			wspace->factory(Form("TauDErr_%d[%f]",*chan,m.getErr()));
			wspace->factory(Form("TauD_%d[%f,%f,%f]",*chan,m.getVal(),0.0,1.0));
			wspace->factory(Form("Gaussian::TauD_Gauss_%d(TauD_%d,TauD0_%d,TauDErr_%d)",*chan,*chan,*chan,*chan));
		} else {
			wspace->factory(Form("TauD_%d[%f,0,10]",*chan,((*bdmm)[make_pair(kTau_bmm, *chan)]).getVal()));
			wspace->var(Form("TauD_%d",*chan))->setConstant(kTRUE);
		}
		
		/////////////////////////
		// Construction of NuS //
		/////////////////////////
		if ( ((*bsmm)[make_pair(kExpUncor_bmm, *chan)]).getErr() > 0 && !no_errors) {
			m = (*bsmm)[make_pair(kExpUncor_bmm, *chan)];
			wspace->factory(Form("NuSUncor0_%d[%f]", *chan, m.getVal())); // fixed mean variable
			wspace->factory(Form("NuSUncorErr_%d[%f]", *chan, m.getErr())); // fixed error variable
			wspace->factory(Form("NuSUncor_%d[%f,%f,%f]", *chan, m.getVal(), 0.0, m.getVal() + 100*m.getErr()));
			wspace->factory(Form("Gaussian::NuSUncor_Gauss_%d(NuSUncor_%d,NuSUncor0_%d,NuSUncorErr_%d)",*chan,*chan,*chan,*chan)); // error gaussian
		} else { // no error associated to this variable
			wspace->factory(Form("NuSUncor_%d[%f,0,100]", *chan, ((*bsmm)[make_pair(kExpUncor_bmm, *chan)]).getVal()));
			wspace->var(Form("NuSUncor_%d", *chan))->setConstant(kTRUE);
		}
		
		/////////////////////////
		// Construction of NuD //
		/////////////////////////
		if ( ((*bdmm)[make_pair(kExp_bmm, *chan)]).getErr() > 0 && !no_errors ) {
			m = (*bdmm)[make_pair(kExp_bmm, *chan)];
			wspace->factory(Form("NuD0_%d[%f]", *chan, m.getVal())); // fixed mean variable
			wspace->factory(Form("NuDErr_%d[%f]", *chan, m.getErr())); // fixed error variable
			wspace->factory(Form("NuD_%d[%f,%f,%f]", *chan, m.getVal(), 0.0, m.getVal() + 100*m.getErr()));
			wspace->factory(Form("Gaussian::NuD_Gauss_%d(NuD_%d,NuD0_%d,NuDErr_%d)",*chan,*chan,*chan,*chan)); // error gaussian
		} else {
			// no error assigned, just make a constant
			wspace->factory(Form("NuD_%d[%f,0,100]", *chan, ((*bdmm)[make_pair(kExp_bmm, *chan)]).getVal()));
			wspace->var(Form("NuD_%d",*chan))->setConstant(kTRUE);
		}
		
		/////////////////////////
		// Construction of Pss //
		/////////////////////////
		if ( ((*bsmm)[make_pair(kProb_swind_bmm, *chan)]).getErr() > 0 && !no_errors ) {
			m = (*bsmm)[make_pair(kProb_swind_bmm, *chan)];
			wspace->factory(Form("Pss0_%d[%f]", *chan, m.getVal()));
			wspace->factory(Form("PssErr_%d[%f]", *chan, m.getErr()));
			wspace->factory(Form("Pss_%d[%f,%f,%f]", *chan, m.getVal(), 0.0, 1.0));
			wspace->factory(Form("Gaussian::Pss_Gauss_%d(Pss_%d,Pss0_%d,PssErr_%d)",*chan,*chan,*chan,*chan));
		} else {
			// no error assigned, just make a constant.
			wspace->factory(Form("Pss_%d[%f,0,1]", *chan, ((*bsmm)[make_pair(kProb_swind_bmm, *chan)]).getVal()));
			wspace->var(Form("Pss_%d",*chan))->setConstant(kTRUE);
		}
		
		/////////////////////////
		// Construction of Psd //
		/////////////////////////
		if ( ((*bdmm)[make_pair(kProb_swind_bmm, *chan)]).getErr() > 0 && !no_errors ) {
			m = (*bdmm)[make_pair(kProb_swind_bmm, *chan)];
			wspace->factory(Form("Psd0_%d[%f]",*chan,m.getVal()));
			wspace->factory(Form("PsdErr_%d[%f]",*chan,m.getErr()));
			wspace->factory(Form("Psd_%d[%f,%f,%f]", *chan, m.getVal(), 0.0, 1.0));
			wspace->factory(Form("Gaussian::Psd_Gauss_%d(Psd_%d,Psd0_%d,PsdErr_%d)",*chan,*chan,*chan,*chan));
		} else {
			// no error assigned, just make a constant.
			wspace->factory(Form("Psd_%d[%f,0,1]", *chan, ((*bdmm)[make_pair(kProb_swind_bmm, *chan)]).getVal()));
			wspace->var(Form("Psd_%d",*chan))->setConstant(kTRUE);
		}
		
		/////////////////////////
		// Construction of Pds //
		/////////////////////////
		if ( ((*bsmm)[make_pair(kProb_dwind_bmm, *chan)]).getErr() > 0 && !no_errors ) {
			m = (*bsmm)[make_pair(kProb_dwind_bmm, *chan)];
			wspace->factory(Form("Pds0_%d[%f]",*chan,m.getVal()));
			wspace->factory(Form("PdsErr_%d[%f]",*chan,m.getErr()));
			wspace->factory(Form("Pds_%d[%f,%f,%f]",*chan,m.getVal(),0.0,1.0));
			wspace->factory(Form("Gaussian::Pds_Gauss_%d(Pds_%d,Pds0_%d,PdsErr_%d)",*chan,*chan,*chan,*chan));
		} else {
			// no error assigned, just make a constant.
			wspace->factory(Form("Pds_%d[%f,0,1]", *chan, ((*bsmm)[make_pair(kProb_dwind_bmm, *chan)]).getVal()));
			wspace->var(Form("Pds_%d",*chan))->setConstant(kTRUE);
		}
		
		/////////////////////////
		// Construction of Pdd //
		/////////////////////////
		if ( ((*bdmm)[make_pair(kProb_dwind_bmm, *chan)]).getErr() > 0 && !no_errors ) {
			m = (*bdmm)[make_pair(kProb_dwind_bmm, *chan)];
			wspace->factory(Form("Pdd0_%d[%f]",*chan,m.getVal()));
			wspace->factory(Form("PddErr_%d[%f]",*chan,m.getErr()));
			wspace->factory(Form("Pdd_%d[%f,%f,%f]",*chan,m.getVal(),0.0,1.0));
			wspace->factory(Form("Gaussian::Pdd_Gauss_%d(Pdd_%d,Pdd0_%d,PddErr_%d)",*chan,*chan,*chan,*chan));
		} else {
			wspace->factory(Form("Pdd_%d[%f,0,1]", *chan, ((*bdmm)[make_pair(kProb_dwind_bmm, *chan)]).getVal()));
			wspace->var(Form("Pdd_%d",*chan))->setConstant(kTRUE);
		}
		
		//////////////////////////////////////
		// Construction of rare backgrounds //
		//////////////////////////////////////
		if ( ((*bsmm)[make_pair(kPeakBkgOff_bmm, *chan)]).getErr() > 0 && !no_errors ) {
			m = (*bsmm)[make_pair(kPeakBkgOff_bmm, *chan)];
			wspace->factory(Form("PeakBkgSB0_%d[%f]",*chan,m.getVal()));
			wspace->factory(Form("PeakBkgSBErr_%d[%f]",*chan,m.getErr()));
			wspace->factory(Form("PeakBkgSB_%d[%f,%f,%f]",*chan,m.getVal(),0.0,m.getVal() + 10*m.getErr()));
			wspace->factory(Form("Gaussian::PeakBkgSB_Gauss_%d(PeakBkgSB_%d,PeakBkgSB0_%d,PeakBkgSBErr_%d)",*chan,*chan,*chan,*chan));
		} else {
			wspace->factory(Form("PeakBkgSB_%d[%f,0,50]", *chan, ((*bsmm)[make_pair(kPeakBkgOff_bmm, *chan)]).getVal()));
			wspace->var(Form("PeakBkgSB_%d",*chan))->setConstant(kTRUE);
		}
		
		if ( ((*bsmm)[make_pair(kPeakBkgOn_bmm, *chan)]).getErr() > 0 && !no_errors ) {
			m = (*bsmm)[make_pair(kPeakBkgOn_bmm, *chan)];
			wspace->factory(Form("PeakBkgBs0_%d[%f]",*chan,m.getVal()));
			wspace->factory(Form("PeakBkgBsErr_%d[%f]",*chan,m.getErr()));
			wspace->factory(Form("PeakBkgBs_%d[%f,%f,%f]",*chan,m.getVal(),0.0,m.getVal() + 10*m.getErr()));
			wspace->factory(Form("Gaussian::PeakBkgBs_Gauss_%d(PeakBkgBs_%d,PeakBkgBs0_%d,PeakBkgBsErr_%d)",*chan,*chan,*chan,*chan));
		} else {
			wspace->factory(Form("PeakBkgBs_%d[%f,0,50]",*chan,((*bsmm)[make_pair(kPeakBkgOn_bmm, *chan)]).getVal()));
			wspace->var(Form("PeakBkgBs_%d",*chan))->setConstant(kTRUE);
		}
		
		if ( ((*bdmm)[make_pair(kPeakBkgOn_bmm, *chan)]).getErr() > 0 && !no_errors ) {
			m = (*bdmm)[make_pair(kPeakBkgOn_bmm, *chan)];
			wspace->factory(Form("PeakBkgBd0_%d[%f]",*chan,m.getVal()));
			wspace->factory(Form("PeakBkgBdErr_%d[%f]",*chan,m.getErr()));
			wspace->factory(Form("PeakBkgBd_%d[%f,%f,%f]",*chan, m.getVal(), 0.0 ,m.getVal() + 10*m.getErr()));
			wspace->factory(Form("Gaussian::PeakBkgBd_Gauss_%d(PeakBkgBd_%d,PeakBkgBd0_%d,PeakBkgBdErr_%d)",*chan,*chan,*chan,*chan));
		} else {
			wspace->factory(Form("PeakBkgBd_%d[%f,0,50]",*chan,((*bdmm)[make_pair(kPeakBkgOn_bmm, *chan)]).getVal()));
			wspace->var(Form("PeakBkgBd_%d",*chan))->setConstant(kTRUE);
		}
		
		//////////////////////////////
		// Construction of formulas //
		//////////////////////////////
		wspace->factory(Form("FormulaVar::NuS_%d(\"fratio*NuSUncor_%d\",{fratio,NuSUncor_%d})",*chan,*chan,*chan));
		wspace->factory(Form("FormulaVar::bkg_mean_%d(\"nu_b_%d + PeakBkgSB_%d\",{nu_b_%d,PeakBkgSB_%d})",*chan,*chan,*chan,*chan,*chan));
		wspace->factory(Form("FormulaVar::bs_mean_%d(\"TauS_%d*nu_b_%d + PeakBkgBs_%d + Pss_%d*NuS_%d*mu_s + Psd_%d*NuD_%d*mu_d\",{TauS_%d,nu_b_%d,PeakBkgBs_%d,Pss_%d,NuS_%d,mu_s,Psd_%d,NuD_%d,mu_d})",*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan));
		wspace->factory(Form("FormulaVar::bd_mean_%d(\"TauD_%d*nu_b_%d + PeakBkgBd_%d + Pds_%d*NuS_%d*mu_s + Pdd_%d*NuD_%d*mu_d\",{TauD_%d,nu_b_%d,PeakBkgBd_%d,Pds_%d,NuS_%d,mu_s,Pdd_%d,NuD_%d,mu_d})",*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan,*chan));
		
		//////////////////////
		// Main Poissonians //
		//////////////////////		
		wspace->factory(Form("Poisson::bs_window_%d(NsObs_%d,bs_mean_%d)",*chan,*chan,*chan));
		wspace->factory(Form("Poisson::bd_window_%d(NdObs_%d,bd_mean_%d)",*chan,*chan,*chan));
		wspace->factory(Form("Poisson::bkg_window_%d(NbObs_%d,bkg_mean_%d)",*chan,*chan,*chan));
		
		if (floatPoissonians) {
			((RooPoisson*)wspace->pdf(Form("bs_window_%d",*chan)))->setNoRounding();
			((RooPoisson*)wspace->pdf(Form("bd_window_%d",*chan)))->setNoRounding();
			((RooPoisson*)wspace->pdf(Form("bkg_window_%d",*chan)))->setNoRounding();
		}
		
		// add the poissonians of this channel
		poissonList.add(*wspace->pdf(Form("bs_window_%d",*chan)));
		poissonList.add(*wspace->pdf(Form("bd_window_%d",*chan)));
		if (!fixed_bkg) poissonList.add(*wspace->pdf(Form("bkg_window_%d",*chan)));
	}
	
	// define the sets
	wspace->defineSet("obs", observables);
	nuisanceParams.addClone(wspace->allVars());
	if (compute_bd_ul)
		wspace->defineSet("poi", "mu_d");
	else
		wspace->defineSet("poi", "mu_s");
	nuisanceParams.remove(*wspace->set("obs"), kTRUE, kTRUE);
	nuisanceParams.remove(*wspace->set("poi"), kTRUE, kTRUE);
	RooStats::RemoveConstantParameters(&nuisanceParams);
	wspace->defineSet("nui", nuisanceParams);
	
	wspace->factory("beta[1]");
	wspace->factory("mu[0]");
	
	// build the prior
	priorList.add(wspace->allPdfs());
	priorList.remove(poissonList, kTRUE, kTRUE);
	if (priorList.getSize() > 0) {
		priorPdf = new RooProdPdf("prior_pdf","Total Prior PDF",priorList);
		poissonList.add(*priorPdf);
	}
	
	totalPdf = new RooProdPdf("total_pdf","Total Model PDF",poissonList);
	wspace->import(*totalPdf, ((verbosity > 0) ? RooCmdArg::none() : RooFit::Silence(kTRUE)) );	
	
	// uniform prior in case of bayesian code
	wspace->factory(Form("Uniform::prior_mu(mu_%c)",(compute_bd_ul ? 'd' : 's')));

	// Model configuration
	splusbModel = new RooStats::ModelConfig("splusbConfig");
	splusbModel->SetWorkspace(*wspace); // set the workspace
	splusbModel->SetPdf(*wspace->pdf("total_pdf"));
	splusbModel->SetParametersOfInterest(*wspace->set("poi"));
	splusbModel->SetObservables(*wspace->set("obs"));
	splusbModel->SetNuisanceParameters(*wspace->set("nui"));
	splusbModel->SetPriorPdf(*wspace->pdf("prior_mu"));
	wspace->import(*splusbModel);
	
	smModel = new RooStats::ModelConfig("smConfig");
	smModel->SetWorkspace(*wspace); // set the workspace
	smModel->SetPdf(*wspace->pdf("total_pdf"));
	smModel->SetParametersOfInterest(*wspace->set("poi"));
	smModel->SetObservables(*wspace->set("obs"));
	smModel->SetNuisanceParameters(*wspace->set("nui"));
	smModel->SetPriorPdf(*wspace->pdf("prior_mu"));
	wspace->import(*smModel);
	
	bModel = new RooStats::ModelConfig("bConfig");
	bModel->SetWorkspace(*wspace); // set the workspace
	bModel->SetPdf(*wspace->pdf("total_pdf"));
	bModel->SetParametersOfInterest(*wspace->set("poi"));
	bModel->SetObservables(*wspace->set("obs"));
	bModel->SetNuisanceParameters(*wspace->set("nui"));
	bModel->SetPriorPdf(*wspace->pdf("prior_mu"));
	wspace->import(*bModel);
	
	if (verbosity > 0) {
		cout << "-------------------------------------" << endl;
		cout << "Workspace Configuration:" << endl;
		wspace->Print();
		cout << "-------------------------------------" << endl;
		cout << "S+B ModelConfig configuration:" << endl;
		splusbModel->Print();
		cout << "-------------------------------------" << endl;
		cout << "SM ModelConfig configuration:" << endl;
		smModel->Print();
		cout << "-------------------------------------" << endl;
		cout << "B ModelConfig configuration:" << endl;
		bModel->Print();
		cout << "-------------------------------------" << endl;
		cout << "Variables:" << endl;
		wspace->allVars().Print("v");
		cout << "-------------------------------------" << endl;
	}
	
	delete bModel;
	delete splusbModel;
	delete priorPdf;
	delete totalPdf;
	
	return wspace;
} // build_model_nchannel()

/* Start values are set the following way:
 *	nu_b = NbObs
 *	( Pss	Psd ) (mu_s NuS) + nu_b	(TauS) = (NsObs)
 *	( Pds	Pdd ) (mu_d NuD)		(TauD) = (NdObs)
 */
void measure_params(RooWorkspace *wspace, RooDataSet *data, set<int> *channels, int verbosity)
{
	RooStats::ModelConfig *splusbConfig = dynamic_cast<RooStats::ModelConfig*> (wspace->obj("splusbConfig"));
	RooStats::ModelConfig *smConfig = dynamic_cast<RooStats::ModelConfig*> (wspace->obj("smConfig"));
	RooStats::ModelConfig *bConfig = dynamic_cast<RooStats::ModelConfig*> (wspace->obj("bConfig"));
	RooRealVar *poi;
	wspace->allVars() = *data->get(0);
	
	// conditional likelihood fit for background model
	poi = (RooRealVar*)wspace->set("poi")->first();
	poi->setVal(0.0);
	poi->setConstant(kTRUE);
	wspace->pdf("total_pdf")->fitTo(*data, RooFit::GlobalObservables(*wspace->set("nui")), ((verbosity > 0) ? RooCmdArg::none() : RooFit::PrintLevel(-1)));
	poi->setConstant(kFALSE);
	bConfig->SetSnapshot(*wspace->set("poi"));
	
	// conditional likelihood fit for SM
	poi->setVal(1.0);
	poi->setConstant(kTRUE);
	wspace->pdf("total_pdf")->fitTo(*data, RooFit::GlobalObservables(*wspace->set("nui")), ((verbosity > 0) ? RooCmdArg::none() : RooFit::PrintLevel(-1)));
	poi->setConstant(kFALSE);
	smConfig->SetSnapshot(*wspace->set("poi"));
	
	// do a likelihood fit to the data to get the real values...
	wspace->pdf("total_pdf")->fitTo(*data, RooFit::GlobalObservables(*wspace->set("nui")), ((verbosity > 0) ? RooCmdArg::none() : RooFit::PrintLevel(-1)));
	splusbConfig->SetSnapshot(*wspace->set("poi"));
} // measure_params()

void est_ul_fc(RooWorkspace *wspace, RooDataSet *data, std::set<int> *channels, double cLevel, int verbosity, double *ulLimit, double *loLimit, std::pair<double,double> *rg, uint32_t *inBins, double *cpuUsed, uint32_t nbrProof, int nToys)
{
	using namespace RooStats;
	ModelConfig *splusbConfig = dynamic_cast<ModelConfig*>(wspace->obj("splusbConfig"));
	FeldmanCousins fc(*data,*splusbConfig);
	ToyMCSampler *toySampler;
	PointSetInterval *psInterval = NULL;
	TStopwatch swatch;
	RooDataSet *custom_points;
	Double_t v;
	Int_t j;
	ProofConfig *pc = NULL;
	string proofString = Form("workers=%u",nbrProof);
	
	swatch.Start(kTRUE);
	
	// configure Feldman Cousins
	fc.CreateConfBelt(true);
	fc.SetTestSize(1.-cLevel);
	fc.SetNBins(*inBins);
	fc.FluctuateNumDataEntries(false);
	toySampler = dynamic_cast<ToyMCSampler*> (fc.GetTestStatSampler());
	toySampler->SetNToys(nToys);
	if (nbrProof > 1) {
		uint32_t nPackages = (((nToys + 999)/1000+(nbrProof-1))/nbrProof)*nbrProof;
		pc = new ProofConfig(*wspace, nPackages, proofString.c_str(), kFALSE);
		toySampler->SetProofConfig(pc);
	}
	if (rg) {
		RooArgSet poi;
		poi.addClone(*wspace->set("poi"));
		custom_points = new RooDataSet("","",poi);
		
		for (j = 0; j < (Int_t)*inBins; j++) {
			v = rg->first + (rg->second - rg->first) * j / (*inBins - 1);
			((RooRealVar*)poi.first())->setVal(v); // Currenlty only one parameter of interest supported
			custom_points->add(poi);
		}
		fc.SetPOIPointsToTest(*custom_points); // note, this is owned by Feldman-Cousins.
	}
	
	measure_params(wspace, data, channels, verbosity);
	splusbConfig->LoadSnapshot();
	psInterval = fc.GetInterval();
	
	if (ulLimit)
		*ulLimit = psInterval->UpperLimit((RooRealVar&)*wspace->set("poi")->first());
	if (loLimit)
		*loLimit = psInterval->LowerLimit((RooRealVar&)*wspace->set("poi")->first());
	
	swatch.Stop();
	if (cpuUsed) *cpuUsed = swatch.CpuTime();
	
	wspace->import(*psInterval);
	
	delete pc;
	delete psInterval;
} // est_ul()

void est_ul_bc(RooWorkspace *wspace, RooDataSet *data, std::set<int> *channels, double cLevel, int verbosity, double *ulLimit, double *cpuUsed)
{
	using namespace RooStats;
	BayesianCalculator bc(*data,*(dynamic_cast<ModelConfig*>(wspace->obj("splusbConfig"))));
	RooStats::ModelConfig *splusbConfig;
	SimpleInterval *simpleInt = NULL;
	TStopwatch swatch;
	
	swatch.Start(kTRUE);
	
	// configure BayesianCalculator
	bc.SetConfidenceLevel(cLevel);
	bc.SetLeftSideTailFraction(0.0); // compute upper limit
	
	measure_params(wspace, data, channels, verbosity);
	
	// Load the Snapshot of the s+b configuration
	splusbConfig = dynamic_cast<RooStats::ModelConfig*> (wspace->obj("splusbConfig"));
	splusbConfig->LoadSnapshot();
	simpleInt = bc.GetInterval();
	
	if (ulLimit)
		*ulLimit = simpleInt->UpperLimit();
	
	swatch.Stop();
	if (cpuUsed) *cpuUsed = swatch.CpuTime();
	
	wspace->import(*simpleInt);
	delete simpleInt;
} // est_ul_bc()

// hybrid approach and ratioofprofiled
void est_ul_hybrid(RooWorkspace *wspace, RooDataSet *data, std::set<int> *channels, double cLevel, int verbosity, double *ulLimit, std::pair<double,double> *rg, uint32_t* inBins, double *cpuUsed, uint32_t nbrProof, int nToys, bool bdmm, bool fixedBkg, bool smExpectation)
{
	using namespace RooStats;
	ModelConfig *bModel = dynamic_cast<ModelConfig*> (wspace->obj("bConfig"));
	ModelConfig *sbModel = dynamic_cast<ModelConfig*> (wspace->obj("splusbConfig"));
	ModelConfig *smModel = dynamic_cast<ModelConfig*> (wspace->obj("smConfig"));
	((RooRealVar*)wspace->set("poi")->first())->setVal(0); // for background
	RatioOfProfiledLikelihoodsTestStat testStat(*sbModel->GetPdf(),*bModel->GetPdf(),wspace->set("poi"));
	testStat.SetSubtractMLE(false);
	testStat.SetGlobalObservables(wspace->set("nui"));
	ToyMCSampler *mcSampler = new ToyMCSampler(testStat,nToys);
	HybridCalculator *hybCalc = new HybridCalculator(*data,*bModel,*sbModel,mcSampler);
	HypoTestInverter *hypoInv = NULL;
	HypoTestInverterResult *result = NULL;
	double limitErr = 0.1;
	double obs;
	TStopwatch swatch;
	bool ok;
	RooArgSet mu;
	uint32_t nBins = inBins ? *inBins : 10; // default 10 bins
	ProofConfig *pc =  NULL;
	string proofString = Form("workers=%u",nbrProof);
	double beta = 0;
	set<int>::const_iterator it;
	RooProdPdf *nui_sampling = NULL;
	RooArgList *nui_sampling_list = new RooArgList;
	
	if (nbrProof > 1) {
		uint32_t nPackages = (((nToys + 999)/1000+(nbrProof-1))/nbrProof)*nbrProof;
		pc = new ProofConfig(*wspace, nPackages, proofString.c_str(), kFALSE);
		mcSampler->SetProofConfig(pc);
	}
	
	swatch.Start(kTRUE);
	measure_params(wspace, data, channels, verbosity);
	sbModel->LoadSnapshot();
	
	mcSampler->SetNEventsPerToy(1);
	
	// if we have no bkg gammas include them in the integration prior...
	for (it = channels->begin(); !fixedBkg && it != channels->end(); ++it) {
		wspace->factory(Form("Gamma::bkg_prior_%d(bkg_mean_%d,gamma_%d[1],beta,mu)",*it,*it,*it));
		nui_sampling_list->add(*wspace->pdf(Form("bkg_prior_%d",*it)));
		wspace->var(Form("gamma_%d",*it))->setVal( wspace->var(Form("NbObs_%d",*it))->getVal()+1 );
	}
	
	if (bdmm) {
		for (it = channels->begin(); it != channels->end(); ++it)
			beta += wspace->var(Form("Pss_%d",*it))->getVal() * wspace->function(Form("NuS_%d",*it))->getVal();
		
		obs = beta * wspace->var("mu_s")->getVal();
		
		beta = 1./beta; // beta is actually the inverse thereof in RooFit
		wspace->factory(Form("Gamma::mu_prior_gamma(mu_s,gamma_b[%f],beta_b[%f],mu)",1.0 + obs,beta));
		nui_sampling_list->add(*wspace->pdf("mu_prior_gamma"));
	} else {
		for (it = channels->begin(); it != channels->end(); ++it)
			beta += wspace->var(Form("Pdd_%d",*it))->getVal() * wspace->var(Form("NuD_%d",*it))->getVal();
		
		obs = beta * wspace->var("mu_d")->getVal();
		beta = 1./beta; // beta is actually the inverse thereof in RooFit
		wspace->factory(Form("Gamma::mu_prior_gamma(mu_d,gamma_b[%f],beta_b[%f],mu)",1.0 + obs, beta));
		nui_sampling_list->add(*wspace->pdf("mu_prior_gamma"));
	}
	nui_sampling_list->add(((RooProdPdf*)wspace->pdf("prior_pdf"))->pdfList());
	nui_sampling = new RooProdPdf("nui_sampling","",*nui_sampling_list);
	wspace->import(*nui_sampling, RooFit::Silence(kTRUE));
	cout << "Nuisance parameter integration through" << endl;
	wspace->pdf("nui_sampling")->Print();
	hybCalc->ForcePriorNuisanceAlt(*wspace->pdf("nui_sampling"));
	hybCalc->ForcePriorNuisanceNull(*wspace->pdf("nui_sampling"));
	hypoInv = new HypoTestInverter(*hybCalc, (RooRealVar*)sbModel->GetParametersOfInterest()->first(), 1.0 - cLevel);
	hypoInv->SetAutoScan();
	hypoInv->UseCLs(true);
	hypoInv->SetTestSize(1.0 - cLevel);
	hypoInv->SetVerbose(verbosity);
	
	if (rg)	ok = hypoInv->RunFixedScan(nBins, rg->first, rg->second);
	else	ok = hypoInv->RunLimit(*ulLimit,limitErr);

	swatch.Stop();
	if (cpuUsed) *cpuUsed = swatch.CpuTime();
	
	result = hypoInv->GetInterval();
	*ulLimit = result->UpperLimit();
	wspace->import(*result);
	
	if (smExpectation) {
		delete hybCalc;
		delete hypoInv;
		delete result;
		
		// new nuisance-parameter sampling function for sm expectation
		wspace->factory("sm_mu[1]");
		wspace->factory("sm_sigma[1e-10]");
		if (bdmm)
			wspace->factory("Gaussian::mu_prior_gauss(mu_s,sm_mu,sm_sigma)");
		else
			wspace->factory("Gaussian::mu_prior_gauss(mu_d,sm_mu,sm_sigma)");
		
		delete nui_sampling_list; nui_sampling_list = new RooArgList;
		for (it = channels->begin(); !fixedBkg && it != channels->end(); ++it)
			nui_sampling_list->add(*wspace->pdf(Form("bkg_prior_%d",*it)));
		
		nui_sampling_list->add(*wspace->pdf("mu_prior_gauss"));
		nui_sampling_list->add( ((RooProdPdf*)wspace->pdf("prior_pdf"))->pdfList() );
		nui_sampling = new RooProdPdf("nui_sampling_SM","",*nui_sampling_list);
		wspace->import(*nui_sampling, RooFit::Silence(kTRUE));
		cout << "Nuisance parameter integration through" << endl;
		wspace->pdf("nui_sampling_SM")->Print();
		
		delete mcSampler; mcSampler = new ToyMCSampler(testStat,nToys);
		mcSampler->SetNEventsPerToy(1);
		if(pc) mcSampler->SetProofConfig(pc);
		
		hybCalc = new HybridCalculator(*data,*smModel,*sbModel,mcSampler); // now background = SM
		hybCalc->ForcePriorNuisanceAlt(*wspace->pdf("nui_sampling_SM"));
		hybCalc->ForcePriorNuisanceNull(*wspace->pdf("nui_sampling_SM"));
		
		hypoInv = new HypoTestInverter(*hybCalc, (RooRealVar*)sbModel->GetParametersOfInterest()->first(), 1.0 - cLevel);
		hypoInv->SetAutoScan();
		hypoInv->UseCLs(true);
		hypoInv->SetTestSize(1.0 - cLevel);
		hypoInv->SetVerbose(verbosity);

		if (rg)	ok = hypoInv->RunFixedScan(nBins, rg->first, rg->second);
		else	ok = hypoInv->RunLimit(*ulLimit,limitErr);
		
		result = hypoInv->GetInterval();
		string name(result->GetName());
		result->SetName(Form("%s_SM",name.c_str()));
		wspace->import(*result);
	}
	
	delete nui_sampling_list;
	delete hybCalc;
	delete nui_sampling;
	delete hypoInv;
	delete pc;
	delete result;
} // est_ul_hybrid()

void est_ul_cls(RooWorkspace *wspace, RooDataSet *data, std::set<int> *channels, double cLevel, int verbosity, double *ulLimit, std::pair<double,double> *rg, uint32_t *npts, double *cpuUsed, uint32_t nbrProof, int nToys)
{
	using namespace RooStats;
	ModelConfig *bModel = dynamic_cast<ModelConfig*> (wspace->obj("bConfig"));
	ModelConfig *sbModel = dynamic_cast<ModelConfig*> (wspace->obj("splusbConfig"));
	ProfileLikelihoodTestStat testStat(*wspace->pdf("total_pdf"));
	testStat.SetOneSided(true);
	testStat.SetGlobalObservables(wspace->set("nui"));
	ToyMCSampler *mcSampler = new ToyMCSampler(testStat,nToys);
	FrequentistCalculator frequCalc(*data,*bModel,*sbModel,mcSampler); // Note null = sb, alt = b
	HypoTestInverter *hypoInv = NULL;
	HypoTestInverterResult *result = NULL;
	double limitErr = 0.1;
	TStopwatch swatch;
	bool ok;
	int nbr = npts ? *npts : 10; // defaults to 10 points if not otherwise specified.
	ProofConfig *pc = NULL;
	string proofString = Form("workers=%u",nbrProof);
	
	if (nbrProof > 1) {
		uint32_t nPackages = (((nToys + 999)/1000+(nbrProof-1))/nbrProof)*nbrProof;
		pc = new ProofConfig(*wspace, nPackages, proofString.c_str(), kFALSE);
		mcSampler->SetProofConfig(pc);
	}
	
	swatch.Start(kTRUE);
	
	measure_params(wspace, data, channels, verbosity);
	sbModel->LoadSnapshot();
	
	mcSampler->SetNEventsPerToy(1);
	hypoInv = new HypoTestInverter(frequCalc, (RooRealVar*)sbModel->GetParametersOfInterest()->first(), 1.0 - cLevel);
	hypoInv->SetAutoScan();
	hypoInv->UseCLs();
	hypoInv->SetTestSize(1.0 - cLevel);
	hypoInv->SetVerbose(verbosity);
	
	if (rg)	ok = hypoInv->RunFixedScan(nbr, rg->first, rg->second);
	else	ok = hypoInv->RunLimit(*ulLimit,limitErr);
	
	swatch.Stop();
	if (cpuUsed) *cpuUsed = swatch.CpuTime();
	
	result = hypoInv->GetInterval();
	*ulLimit = result->UpperLimit();
	
	wspace->import(*result);
	
	delete pc;
	delete hypoInv;
	delete result;
} // est_ul_cls()

void est_ul_clb_hybrid(RooWorkspace *wspace, RooDataSet *data, std::set<int> *channels, int verbosity, double *pvalue, uint32_t nbrProof, int nToys, bool bdmm, bool fixedBkg, bool smExpectation)
{
	using namespace RooStats;
	ModelConfig *bModel = dynamic_cast<ModelConfig*> (wspace->obj("bConfig"));
	ModelConfig *sbModel = dynamic_cast<ModelConfig*> (wspace->obj("splusbConfig"));
	ModelConfig *smModel = dynamic_cast<ModelConfig*> (wspace->obj("smConfig"));
	ProfileLikelihoodTestStat testStat(*wspace->pdf("total_pdf"));
	testStat.SetGlobalObservables(wspace->set("nui"));
	testStat.SetOneSidedDiscovery(kTRUE);
	ToyMCSampler *mcSampler = new ToyMCSampler(testStat,nToys);
	HybridCalculator *hybCalc = new HybridCalculator(*data,*sbModel,*bModel,mcSampler); // null = bModel interpreted as signal, alt = s+b interpreted as bkg
	HypoTestResult *result;
	RooArgSet mu;
	ProofConfig *pc = NULL;
	string proofString = Form("workers=%u",nbrProof);
	double beta = 0;
	double obs;
	RooProdPdf *nui_sampling = NULL;
	RooArgList *nui_sampling_list = new RooArgList;
	set<int>::const_iterator it;
	
	if (nbrProof > 1) {
		uint32_t nPackages = (((nToys + 999)/1000+(nbrProof-1))/nbrProof)*nbrProof;
		pc = new ProofConfig(*wspace,nPackages,proofString.c_str(),kFALSE);
		mcSampler->SetProofConfig(pc);
	}
	
	mcSampler->SetNEventsPerToy(1);
	
	measure_params(wspace, data, channels, verbosity);
	bModel->LoadSnapshot();
	
	// if we have no bkg gammas, include them in the integration prior
	for (it = channels->begin(); !fixedBkg && it != channels->end(); ++it) {
		wspace->factory(Form("Gamma::bkg_prior_%d(bkg_mean_%d,gamma_%d[1],beta,mu)",*it,*it,*it));
		nui_sampling_list->add(*wspace->pdf(Form("bkg_prior_%d",*it)));
		wspace->var(Form("gamma_%d",*it))->setVal( wspace->var(Form("NbObs_%d",*it))->getVal()+1 );
	}
	
	// nuisance signal
	if (bdmm) {
		for (set<int>::const_iterator it = channels->begin(); it != channels->end(); ++it)
			beta += wspace->var(Form("Pss_%d",*it))->getVal() * wspace->function(Form("NuS_%d",*it))->getVal();
		
		obs = beta * wspace->var("mu_s")->getVal();
		
		beta = 1./beta; // beta is actually the inverse thereof in RooFit
		wspace->factory(Form("Gamma::mu_prior_gamma(mu_s,gamma_b[%f],beta_b[%f],mu)",1.0 + obs,beta));
		nui_sampling_list->add(*wspace->pdf("mu_prior_gamma"));
	} else {
		for (set<int>::const_iterator it = channels->begin(); it != channels->end(); ++it)
			beta += wspace->var(Form("Pdd_%d",*it))->getVal() * wspace->var(Form("NuD_%d",*it))->getVal();
		
		obs = beta * wspace->var("mu_d")->getVal();
		beta = 1./beta; // beta is actually the inverse thereof in RooFit
		wspace->factory(Form("Gamma::mu_prior_gamma(mu_d,gamma_b[%f],beta_b[%f],mu)",1.0 + obs, beta));
		nui_sampling_list->add(*wspace->pdf("mu_prior_gamma"));
	}
	
	nui_sampling_list->add( ((RooProdPdf*)wspace->pdf("prior_pdf"))->pdfList() );
	nui_sampling = new RooProdPdf("nui_sampling","",*nui_sampling_list);
	wspace->import(*nui_sampling, RooFit::Silence(kTRUE));
	cout << "Nuisance parameter integration through" << endl;
	wspace->pdf("nui_sampling")->Print();
	hybCalc->ForcePriorNuisanceAlt(*wspace->pdf("nui_sampling"));
	hybCalc->ForcePriorNuisanceNull(*wspace->pdf("nui_sampling"));
	hybCalc->SetToys(nToys, nToys);
	
	result = hybCalc->GetHypoTest();
	result->SetBackgroundAsAlt(kTRUE);
	
	*pvalue = result->CLsplusb();
	wspace->import(*result);
	
	if (smExpectation) {
		delete hybCalc;
		delete result;
		
		// new nuisance-parameter sampling functionfor sm expectation...
		wspace->factory("sm_mu[1]");
		wspace->factory("sm_sigma[1e-10]");
		if (bdmm)	wspace->factory("Gaussian::mu_prior_gauss(mu_s,sm_mu,sm_sigma)");
		else		wspace->factory("Gaussian::mu_prior_gauss(mu_d,sm_mu,sm_sigma)");
		
		delete nui_sampling_list; nui_sampling_list = new RooArgList;
		for (it = channels->begin(); !fixedBkg && it != channels->end(); ++it)
			nui_sampling_list->add(*wspace->pdf(Form("bkg_prior_%d",*it)));
		nui_sampling_list->add(*wspace->pdf("mu_prior_gauss"));
		nui_sampling_list->add( ((RooProdPdf*)wspace->pdf("prior_pdf"))->pdfList() );
		nui_sampling = new RooProdPdf("nui_sampling_SM","",*nui_sampling_list);
		wspace->import(*nui_sampling, RooFit::Silence(kTRUE));
		cout << "Nuisance parameter integration through" << endl;
		wspace->pdf("nui_sampling_SM")->Print();
		
		// setup new MC Sampler. This is due to a bug in RooStats. Fixed in dev trunk as of 20120920
		delete mcSampler; mcSampler = new ToyMCSampler(testStat,nToys);
		mcSampler->SetNEventsPerToy(1);
		if(pc) mcSampler->SetProofConfig(pc);
		
		hybCalc = new HybridCalculator(*data,*smModel,*bModel,mcSampler); // null = bModel
		hybCalc->ForcePriorNuisanceAlt(*wspace->pdf("nui_sampling_SM"));
		hybCalc->ForcePriorNuisanceNull(*wspace->pdf("nui_sampling_SM"));
		hybCalc->SetToys(nToys, nToys);
		
		result = hybCalc->GetHypoTest();
		string name(result->GetName());
		result->SetBackgroundAsAlt(kTRUE);
		result->SetName(Form("%s_SM",name.c_str()));
		wspace->import(*result);
	}
	
	delete nui_sampling_list;
	delete nui_sampling;
	delete pc;
	delete mcSampler;
	delete result;
	delete hybCalc;
} // est_ul_clb_hybrid()

void est_ul_clb(RooWorkspace *wspace, RooDataSet *data, std::set<int> *channels, int verbosity, double *pvalue, uint32_t nbrProof, int nToys)
{
	using namespace RooStats;
	ModelConfig *bModel = dynamic_cast<ModelConfig*> (wspace->obj("bConfig"));
	ModelConfig *sbModel = dynamic_cast<ModelConfig*> (wspace->obj("splusbConfig"));
	ProfileLikelihoodTestStat testStat(*wspace->pdf("total_pdf"));
	testStat.SetGlobalObservables(wspace->set("nui"));
	testStat.SetOneSidedDiscovery(kTRUE);
	ToyMCSampler *mcSampler = new ToyMCSampler(testStat,nToys);
	FrequentistCalculator frequCalc(*data,*sbModel,*bModel,mcSampler); // null = bModel interpreted as signal, alt = s+b interpreted as bkg
	HypoTestResult *result = NULL;
	ProofConfig *pc = NULL;
	string proofString = Form("workers=%u",nbrProof);
	RooArgList mu;
	
	if (nbrProof > 1) {
		uint32_t nPackages = (((nToys + 999)/1000+(nbrProof-1))/nbrProof)*nbrProof;
		pc = new ProofConfig(*wspace,nPackages,proofString.c_str(),kFALSE);
		mcSampler->SetProofConfig(pc);
	}
	
	// set background prior to measurement
	measure_params(wspace, data, channels, verbosity);
	bModel->LoadSnapshot();
	mcSampler->SetNEventsPerToy(1);
	frequCalc.SetToys(nToys, nToys);
	
	result = frequCalc.GetHypoTest();
	result->SetBackgroundAsAlt(kTRUE);
	*pvalue = result->CLsplusb();
	
	wspace->import(*result);
	
	delete mcSampler;
	delete pc;
	delete result;
} // est_ul_clb()

void est_ul_zbi(RooWorkspace *wspace, RooDataSet *data, set<int> *channels, double cLevel, bool bdlimit, double *ul)
{
	RooRealVar *poi = dynamic_cast<RooRealVar*>(wspace->set("poi")->first());
	RooFormulaVar *formula;
	set<int>::const_iterator it;
	double max = poi->getMax();
	double min = 0.;
	double mu = poi->getMax(),pvalue;
	
	double nu_off,nu_on,rho;
	double nbkg,nsig;
	
	wspace->allVars() = *data->get(0);
	poi->setConstant(kTRUE);
	
	nbkg = nsig = 0.0;
	for (it = channels->begin(); it != channels->end(); ++it) {
		nbkg += wspace->var(Form("NbObs_%d",*it))->getVal();
		if (bdlimit)	nsig += wspace->var(Form("NsObs_%d",*it))->getVal();
		else			nsig += wspace->var(Form("NsObs_%d",*it))->getVal();
	}
	
	while ( (max - min) > 1.e-2 ) {
		
		mu = (max + min) /2.;
		poi->setVal(mu); // estimate the values for 
		
		// estimate the values for the current mu
		wspace->pdf("total_pdf")->fitTo(*data, RooFit::PrintLevel(-2));
		
		// compute the rho = nu_on / nu_tot
		nu_off = nu_on = 0.0;
		for (it = channels->begin(); it != channels->end(); ++it) {
			
			formula = dynamic_cast<RooFormulaVar*> (wspace->function(Form("bkg_mean_%d",*it)));
			nu_off += formula->getVal();
			
			if (bdlimit)	formula = dynamic_cast<RooFormulaVar*> (wspace->function(Form("bd_mean_%d",*it)));
			else			formula = dynamic_cast<RooFormulaVar*> (wspace->function(Form("bs_mean_%d",*it)));
			
			nu_on += formula->getVal();
		}
		
		rho = nu_on / (nu_off + nu_on);
		pvalue = TMath::BetaIncomplete(1 - rho, nbkg, nsig + 1);
		
		if (pvalue > 1. - cLevel)	min = mu;
		else						max = mu;
	}
	
	poi->setConstant(kFALSE);
	
	*ul = mu;
} // est_ul_zbi()

void compute_vars(map<bmm_param,measurement_t> *bmm, bool bstomumu)
{
	set<int> channels;
	set<int>::const_iterator it;
	measurement_t tot_bplus,nu;
	measurement_t c = bstomumu ? c_s_theory() : c_d_theory();
	measurement_t f_rat = f_ratio();
	
	add_channels(bmm, &channels);
	
	for (it = channels.begin(); it != channels.end(); ++it) {
		
		if (bmm->count(make_pair(kTot_bplus, *it)) > 0)
			tot_bplus = (*bmm)[make_pair(kTot_bplus, *it)];
		else
			tot_bplus = (*bmm)[make_pair(kObs_bplus, *it)] / compute_efftot_bplus(bmm,*it);
		
		nu = c * compute_efftot_bmm(bmm,*it) * tot_bplus;
		(*bmm)[make_pair(kExpUncor_bmm, *it)] = nu; // uncorrelated error from measurements
		
		if (bstomumu) nu = nu * f_rat;
		(*bmm)[make_pair(kExp_bmm, *it)] = nu; // for convenience compute expectation as well.
	}
} // compute_vars()
