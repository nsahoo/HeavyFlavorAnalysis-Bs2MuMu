/*
 *  ul_main.cpp
 *  final_calculator
 *
 *  Created by Christoph Nägeli <christoph.naegeli@psi.ch> on 17.03.11.
 *  Copyright 2011 Christoph Nägeli. All rights reserved.
 *
 */

// Standard Headers
#include <iostream>
#include <string>
#include <vector>

// ROOT headers
#include <TMath.h>
#include <RooRealVar.h>
#include <RooWorkspace.h>
#include <RooStats/ConfInterval.h>

#include "external_constants.h"
#include "ul_estimate.h"

using namespace std;

enum algo_t {
	kAlgo_Bayesian = 1,
	kAlgo_FeldmanCousins
};

static const char *workspace_path = NULL;
static const char *configfile_path = NULL;
static double gCLLevel = 0.9;
static uint32_t gDefaultPoissonLimit = 4;
static pair<double,double> gMuSRange(-1,-1);
static uint32_t gFCSteps = 20;
static double gFCErr = 0.0; // additional error in acceptance region
static algo_t gAlgorithm = kAlgo_Bayesian;
static bool gDisableErrors = false;

#ifdef __linux__
/* Linux does not have that nice function fgetln
 * Use a very simple approach in that case */
static char *fgetln(FILE *f, size_t *len)
{
  static char buffer[2048];
  char *result;

  result = fgets(buffer,sizeof(buffer),f);
  if (result) *len = strlen(buffer);
  else *len = 0;

  return result;
} // fgetln()
#endif

static const char *algo_name(algo_t a)
{
	const char *result;
	
	switch (a) {
		case kAlgo_Bayesian:
			result = "Bayesian";
			break;
		case kAlgo_FeldmanCousins:
			result = "Feldman-Cousins";
			break;
		default:
			cerr << "Unknown algorithm selected: " << a << endl;
			result = NULL;
			abort();
			break;
	}
	
	return result;
} // algo_name()

static bool validate_bmm(map<bmm_param,measurement_t> *bmm, int ch)
{
	bool valid;
	
	// check normalization decay info
	valid = bmm->count(make_pair(kTot_bplus, ch)) > 0;
	valid = valid || ( bmm->count(make_pair(kAcc_bplus, ch)) > 0 && bmm->count(make_pair(kEff_mu_bplus, ch)) > 0
					&& bmm->count(make_pair(kEff_trig_bplus, ch)) > 0 && bmm->count(make_pair(kEff_cand_bplus, ch)) > 0
					&& bmm->count(make_pair(kEff_ana_bplus, ch)) > 0 );
	if (!valid) {
		cerr << "Insufficient information in config file for channel " << ch << ": Supply either 'TOT_BPLUS' or ('ACC_BPLUS' and 'EFF_MU_BPLUS' and 'EFF_TRIG_BPLUS' and 'EFF_CAND_BPLUS' and 'EFF_ANA_BPLUS')" << endl;
		goto bail;
	}
	
	valid = bmm->count(make_pair(kObsBkg_bmm, ch)) > 0;
	if (!valid) {
		cerr << "Insufficient information in config file. Please specify 'OBS_BKG' for channel " << ch << endl;
		goto bail;
	}
	
	// FIXME: complete this list...
	
bail:
	return valid;
} // validate_bmm()

static bool process_line(string name, vector<double> *values, map<bmm_param,measurement_t> *bsmm, map<bmm_param,measurement_t> *bdmm)
{
	bmm_param p;
	bool bsparam;
	bool success = false;
	
	if (values->size() <= 1)
		goto bail;
	
	p.first = find_bmm_param_by_name(name, &bsparam);
	if (p.first == kUnknownParam) goto bail;
	
	// Channel Index
	p.second = (int)((*values)[0]);
	
	// Value
	if (bsparam)	(*bsmm)[p].setVal((*values)[1]);
	else			(*bdmm)[p].setVal((*values)[1]);
	
	// Error of the variable
	if (values->size() >= 3) {
		if (bsparam)	(*bsmm)[p].setErr((*values)[2]);
		else			(*bdmm)[p].setErr((*values)[2]);
	}
	
	success = true;
bail:
	return success;
} // process_line()

static void parse_input(const char *path, map<bmm_param,measurement_t> *bsmm, map<bmm_param,measurement_t> *bdmm)
{
	FILE *file = fopen(path, "r");
	char* input_line;
	size_t line_len;
	string line;
	string delim(" \t:");
	vector<double> values;
	string name;
	string::iterator it,next;
	
	while ( (input_line = fgetln(file, &line_len)) ) {
		
		// if comment line, read next!
		if (input_line[0] == '#')
			continue;
		
		// remove endline at end of line (if any)
		if (input_line[line_len-1] == '\n' || input_line[line_len-1] == '\r')
			line_len--;
		
		if (line_len == 0) // empty line
			continue;
		
		// create a c++ string out of input line
		line = string(input_line,line_len);
		
		// tokenize by delim
		it = find_first_of(line.begin(), line.end(), delim.begin(), delim.end());
		name = string(line,0,it-line.begin());
		
		// now, search for all tokens
		values.clear();
		
		do {
			next = find_first_of(it + 1, line.end(), delim.begin(), delim.end());
			if ((next - it) > 1) {
				values.push_back(atof( string(line, it-line.begin(), next - it).c_str() ));
			}
			it = next;
		} while (next != line.end());
		
		if (!process_line(name,&values,bsmm,bdmm)) {
			cerr << "Unable to process line: '" << line << "'." << endl;
			abort();
		}
	}
	
	fclose(file);
} // parse_input()

static void usage()
{
	cerr << "ulcalc [--disable-errors][[-n <FC steps>] -r x,y ] [-e fc_err] [-l cl] [-w workspace_outfile.root] [-p <nbr poisson avg>] [-a <\"bayes\"|\"fc\">] <configfile>" << endl;
} // usage()

static bool parse_arguments(const char **first, const char **last)
{
	bool ok = true;
	const char *arg;
	string s;
	string::iterator col;
	
	while (first != last) {
		
		arg = *first++;
		if (arg[0] == '-') {
			if (strcmp(arg, "-l") == 0) {
				if (first == last) {
					usage();
					abort();
				}
				gCLLevel = atof(*first++); // read the confidence level
			} else if (strcmp(arg, "-w") == 0) {
				if (first == last) {
					usage();
					abort();
				}
				workspace_path = *first++; // set the workspace output path
			} else if (strcmp(arg, "-p") == 0) {
				if (first == last) {
					usage();
					abort();
				}
				gDefaultPoissonLimit = (uint32_t)atoi(*first++);
			} else if (strcmp(arg, "-n") == 0) {
				if (first == last) {
					usage();
					abort();
				}
				gFCSteps = (uint32_t)atoi(*first++);
			} else if (strcmp(arg, "-r") == 0) {
				if (first == last) {
					usage();
					abort();
				}
				s = *first++;
				if ( (col = find(s.begin(), s.end(), ',')) == s.end() ) {
					usage();
					abort();
				}
				gMuSRange.first = atof(string("").append(s.begin(),col).c_str());
				gMuSRange.second = atof(string("").append(col+1,s.end()).c_str());
			} else if (strcmp(arg, "-a") == 0) {
				if (first == last) {
					usage();
					abort();
				}
				s = *first++;
				if (s.compare("bayes") == 0)	gAlgorithm = kAlgo_Bayesian;
				else if (s.compare("fc") == 0)	gAlgorithm = kAlgo_FeldmanCousins;
				else {
					usage();
					abort();
				}
			} else if (strcmp(arg, "--disable-errors") == 0) {
				gDisableErrors = true;
			} else if (strcmp(arg, "-e") == 0) {
				if (first == last) {
					usage();
					abort();
				}
				gFCErr = atof(*first++);
			}
			else {
				cerr << "Unknown option '" << arg << "'." << endl;
				usage();
				abort();
			}
		} else
			configfile_path = arg;
	}
	
	if (!configfile_path) {
		ok = false;
		goto bail;
	}
	
	// Output current configuration
	cout << "Reading configuration file: " << configfile_path << endl;
	cout << "Confidence Level: " << gCLLevel << endl;
	if (workspace_path)
		cout << "Output Workspace into: " << workspace_path << endl;
	if (gAlgorithm == kAlgo_FeldmanCousins) {
		cout << "mu_s range: (" << gMuSRange.first << ", " << gMuSRange.second << ")." << endl;
		cout << "Number of Steps " << gFCSteps << endl;
		cout << "Additional error in acceptance region " << gFCErr << endl;
	}
	cout << "Algorithm is " << algo_name(gAlgorithm) << endl;
	cout << "Errors on Variables are " << (gDisableErrors ? "disabled" : "enabled") << endl;
	
bail:
	return ok;
} // parse_arguments()

static void dump_params(pair<bmm_param,measurement_t> p)
{
	cout << '\t' << find_bmm_name(p.first.first) << '[' << p.first.second << "]: " << p.second.getVal();
	if (p.second.getErr() > 0)
		cout << " +/- " << p.second.getErr();
	cout << endl;
} // dump_params()

static void recursive_calc(RooWorkspace *wspace, RooArgSet *obs, set<int> *channels, set<int>::iterator a,set<int>::iterator b, map<bmm_param,measurement_t> *bsmm, map<bmm_param,measurement_t> *bdmm, double *avgUL, double *avgWeight)
{
	RooDataSet *data = NULL;
	RooStats::ConfInterval *inter = NULL;
	double bkg,weight,ul;
	uint32_t obsBsMin,obsBsMax;
	uint32_t obsBdMin,obsBdMax;
	uint32_t obsBs,obsBd;
	set<int>::const_iterator it;
	RooAbsReal *bs_mean;
	RooAbsReal *bd_mean;
	int ch;
	
	if (a == b) { // end of recursion over channels
		
		// compute the weight of this configuration...
		wspace->var("mu_s")->setVal(1); // SM configuration
		wspace->var("mu_d")->setVal(1); // SM configuration
		weight = 1;
		for (it = channels->begin(); it != channels->end(); ++it) {
			
			// Bs window weight
			bs_mean = wspace->function(Form("bs_mean_%d",*it));
			weight *= TMath::Poisson(((RooRealVar&)(*obs)[Form("NsObs_%d",*it)]).getVal(),bs_mean->getVal());
			
			// Bd window weight
			bd_mean = wspace->function(Form("bd_mean_%d",*it));
			weight *= TMath::Poisson(((RooRealVar&)(*obs)[Form("NdObs_%d",*it)]).getVal(),bd_mean->getVal());
		}
		
		data = new RooDataSet("data","",*obs);
		data->add(*obs);
		data->Print("v");
		
		switch (gAlgorithm) {
			case kAlgo_Bayesian:
				inter = est_ul_bc(wspace, data, channels, gCLLevel, &ul, NULL);
				break;
			case kAlgo_FeldmanCousins:
				inter = est_ul_fc(wspace, data, channels, gCLLevel, gFCSteps, ((gMuSRange.first >= 0) ? &gMuSRange : NULL), gFCErr, &ul, NULL);
				break;
			default:
				cerr << "Unknown algorithm selected to determine the upper limit..." << endl;
				abort();
				break;
		}
		
		*avgWeight = *avgWeight + weight;
		*avgUL = *avgUL + ul * weight;
		
		delete inter;
		delete data;
		goto bail;
	}
	
	// save current channel and advance iterator
	ch = *a++;
	
	// Set current value and do recursive...
	bkg = ((*bsmm)[make_pair(kObsBkg_bmm, ch)]).getVal();
	((RooRealVar&)((*obs)[Form("NbObs_%d",ch)])).setVal(bkg);
	
	if (bsmm->count(make_pair(kObsB_bmm, ch)) > 0) {
		obsBsMin = obsBsMax = (uint32_t)((*bsmm)[make_pair(kObsB_bmm, ch)]).getVal();
	} else {
		obsBsMin = 0;
		obsBsMax = gDefaultPoissonLimit;
	}
	
	if (bdmm->count(make_pair(kObsB_bmm, ch)) > 0) {
		obsBdMin = obsBdMax = (uint32_t)((*bdmm)[make_pair(kObsB_bmm, ch)]).getVal();
	} else {
		obsBdMin = 0;
		obsBdMax = gDefaultPoissonLimit;
	}
	
	// iterate through these variables
	for (obsBs = obsBsMin; obsBs <= obsBsMax; obsBs++) {
		((RooRealVar&)((*obs)[Form("NsObs_%d",ch)])).setVal(obsBs);
		for (obsBd = obsBdMin; obsBd <= obsBdMax; obsBd++) {
			((RooRealVar&)((*obs)[Form("NdObs_%d",ch)])).setVal(obsBd);
			recursive_calc(wspace,obs,channels,a,b,bsmm,bdmm,avgUL,avgWeight);
		}
	}
bail:
	return;
} // recursive_calc()

int main(int argc, const char *argv [])
{
	RooWorkspace *wspace = NULL;
	RooArgSet observables;
	map<bmm_param,measurement_t> bsmm,bdmm;
	set<int> channels;
	double avgUL = 0;
	double avgWeight = 0;
	
	if(!parse_arguments(&argv[1], &argv[argc])) {
		usage();
		abort();
	}
	
	parse_input(configfile_path,&bsmm,&bdmm);
	
	// some variables have to be redundant for convenience
	add_channels(&bsmm, &channels);
	add_channels(&bdmm, &channels);
	for (set<int>::const_iterator c = channels.begin(); c!=channels.end();++c) {
		bdmm[make_pair(kObsBkg_bmm, *c)] = bsmm[make_pair(kObsBkg_bmm, *c)];
		
		if (bsmm.count(make_pair(kTot_bplus, *c)) > 0)
			bdmm[make_pair(kTot_bplus, *c)] = bsmm[make_pair(kTot_bplus, *c)];
		if (bsmm.count(make_pair(kAcc_bplus, *c)) > 0)
			bdmm[make_pair(kAcc_bplus, *c)] = bsmm[make_pair(kAcc_bplus, *c)];
		if (bsmm.count(make_pair(kEff_mu_bplus, *c)) > 0)
			bdmm[make_pair(kEff_mu_bplus, *c)] = bsmm[make_pair(kEff_mu_bplus, *c)];
		if (bsmm.count(make_pair(kEff_trig_bplus, *c)) > 0)
			bdmm[make_pair(kEff_trig_bplus, *c)] = bsmm[make_pair(kEff_trig_bplus, *c)];
		if (bsmm.count(make_pair(kEff_cand_bplus, *c)) > 0)
			bdmm[make_pair(kEff_cand_bplus, *c)] = bsmm[make_pair(kEff_cand_bplus, *c)];
		if (bsmm.count(make_pair(kEff_ana_bplus, *c)) > 0)
			bdmm[make_pair(kEff_ana_bplus, *c)] = bsmm[make_pair(kEff_ana_bplus, *c)];
		if (bsmm.count(make_pair(kObs_bplus, *c)) > 0)
			bdmm[make_pair(kObs_bplus, *c)] = bsmm[make_pair(kObs_bplus, *c)];
		
		// check given parameters
		if(!validate_bmm(&bsmm,*c) || !validate_bmm(&bdmm,*c)) {
			cerr << "Insufficient parameters in the parameter file!!" << endl;
			abort();
		}
	}
	
	// show the input parsed
	cout << "---------------------------" << endl;
	cout << "Bs -> mumu parameters read:" << endl;
	for_each(bsmm.begin(), bsmm.end(), dump_params);
	cout << "---------------------------" << endl;
	cout << "Bd -> mumu parameters read:" << endl;
	for_each(bdmm.begin(), bdmm.end(), dump_params);
	cout << "---------------------------" << endl;
	
	compute_vars(&bsmm,true);
	compute_vars(&bdmm,false);
	wspace = build_model_nchannel(&bsmm,&bdmm,gDisableErrors,false);
	
	// set the measured candidates...
	observables.addClone(*wspace->set("obs"));
	recursive_calc(wspace,&observables,&channels,channels.begin(),channels.end(),&bsmm,&bdmm,&avgUL,&avgWeight);
	
	// reweight the UL.
	avgUL /= avgWeight;
	cout << "Upper limit for config file using algorithm " << algo_name(gAlgorithm) << ": " << avgUL*bstomumu() << endl;
	
	if (workspace_path) wspace->writeToFile(workspace_path,kTRUE);
	delete wspace;
	
	return 0;
} // main()