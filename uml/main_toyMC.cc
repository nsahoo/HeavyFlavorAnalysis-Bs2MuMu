/* 
 * File:   main_toyMC.cc
 * Author: lucamartini
 *
 * Created on 24 aprile 2012, 18.25
 */
#include "pdf_analysis.h"
#include "pdf_toyMC.h"

#include <stdio.h>
#include <stdlib.h>

#include "CommonFun.h"

#include "TFile.h"

#include "RooWorkspace.h"

/*
 * 
 */

int main(int argc, char** argv) {

  parse_options(argc, argv);
  if (!estimate) help();
  if (!simul) {
    if (!input || !pdf) help();
    parse_input(input_name);
  }
  if (!(!bias_s.compare("no") || !bias_s.compare("c+") || !bias_s.compare("c-") || !bias_s.compare("tau+") || !bias_s.compare("tau-") || !bias_s.compare("sig+") || !bias_s.compare("sig-"))) { cout << "I don't understand what to bias: please enter -bias c+, c-, tau+, tau-, sig+, sig-" << endl; exit(EXIT_FAILURE);}
  if (!pdf_test_b) pdf_test = pdf_toy;

  pdf_toyMC toy1(print, inputs, inputs_bdt, input_estimates, meth, "all", SM, bd_const, 0, simul, simul_bdt, pee, bdt_fit, ch_s, sig_meth, bias_s);
  toy1.initialize();
  TFile* input_f = new TFile(input_name.c_str());
  RooWorkspace* ws = (RooWorkspace*)input_f->Get("ws");
  toy1.set_ws(ws);
  ws->Print();

  if (roomcs) toy1.mcstudy(NExp, pdf_toy, pdf_test);
  if (!roomcs) toy1.generate(NExp, pdf_toy, pdf_test);
  return EXIT_SUCCESS;
}

