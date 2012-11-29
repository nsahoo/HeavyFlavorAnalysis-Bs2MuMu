#ifndef PDF_FITDATA_H
#define PDF_FITDATA_H

#include <sstream>
#include <iostream>
#include <iomanip>

#include "pdf_analysis.h"

#include "TTree.h"
#include "TH2D.h"

#include "RooCategory.h"
#include "RooSimultaneous.h"
#include "RooRandom.h"
#include "RooGamma.h"
#include "RooMinuit.h"

#include "RooStats/ModelConfig.h"
#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/HypoTestResult.h"
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/ToyMCSampler.h"
#include "RooStats/FrequentistCalculator.h"
#include "RooStats/HybridCalculator.h"
#include "RooStats/HypoTestPlot.h"
#include "RooStats/RatioOfProfiledLikelihoodsTestStat.h"

using namespace RooStats;

class pdf_fitData : public pdf_analysis {
  public:
    pdf_fitData(bool print, string input_estimates = "", string range = "all", int BF = 0, bool SM = false, bool bd_constr = false, int simul = 1, int simulbdt = 1, int simulall = 1, bool pee_ = false , bool bdt_fit = false , string ch_s = "0", int sig = -1, bool asimov = false, bool syste = false, bool randomsyste = false, int nexp = 10, bool bd = false);
    ~pdf_fitData();
    void print();
    void print_each_channel(string var = "Mass", string output = "", RooWorkspace *ws = 0, RooDataSet *rds_ = 0);

    void define_dataset();
    void make_dataset(bool cut_b, vector<double> cut_, string cuts, TTree *tree, int offset = 0);
    void make_pdf_input(string root_s);
    void make_pdf();
    void hack_ws(string frozen_ws_address);

    RooDataSet* global_data;
    RooDataHist* global_datahist;
    RooSimultaneous* simul_pdf;

    void fit_pdf(bool do_not_import = false);
    void significance();
    void save();

    double lumi;
    bool random;
    void setnewlumi();
    void setsyst();

    int proof;

    void extract_N_inRanges();
    void profile_NLL();

    bool SMIsNull;

  protected:

    void randomize_constraints(RooWorkspace* ws);

    string input_estimates_;
    vector <double> estimate_bs;
    vector <double> estimate_bd;
    vector <double> estimate_semi;
    vector <double> estimate_comb;

    vector <vector <double> > estimate2D_bs;
    vector <vector <double> > estimate2D_bd;
    vector <vector <double> > estimate2D_semi;
    vector <vector <double> > estimate2D_comb;
    vector <vector <double> > estimate2D_channel;

    void parse_estimate();
    bool parse(char *cutName, float cut);
    string input_cuts_;
    bool asimov_;
    int sign;

    string input_systematics_;
    vector <double> systematics_bs;
    vector <double> systematics_bd;
    vector <double> systematics_semi;
    vector <double> systematics_comb;
    vector <double> systematics_channel;

    vector <vector <double> > systematics2D_bs;
    vector <vector <double> > systematics2D_bd;
    vector <vector <double> > systematics2D_semi;
    vector <vector <double> > systematics2D_comb;
    vector <vector <double> > systematics2D_channel;

    string pdfname;
    int NExp;
    bool Bd;

  private:

    TFile* ws_file_input;
    RooWorkspace* ws_input;

    void FillRooDataSet(RooDataSet* dataset, bool cut_b, vector<double> cut_, string cuts, TTree *tree, int offset);
    void changeName(RooWorkspace *ws, int str);

    Double_t sig_hand();
    void sig_plhc();
    void sig_plhts();
    void sig_hybrid_plhts();
    void sig_hybrid_roplhts();
    void make_prior();
    void make_models();
};

#endif // PDF_FITDATA_H
