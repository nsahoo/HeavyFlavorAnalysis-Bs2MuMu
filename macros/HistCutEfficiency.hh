#ifndef HISTCUTEFFICIENCY
#define HISTCUTEFFICIENCY

#include "TH1.h"

#include <iostream>
#include <fstream>




class HistCutEfficiency: public TObject {

public:

  HistCutEfficiency(TH1 *h); 
  HistCutEfficiency(TH1 *h1, double cut, int includeOverflow = 1);
  ~HistCutEfficiency(); 

  void eff(double cut);
  void eff(TH1* h1, double cut);
  void eff(TH1* h1, double cut, int includeOverFlow);

  TH1 *fH; 
  int fIncludeOverflow; 

  double loEff, loErr; // efficiency of integral BELOW the cut
  double hiEff, hiErr; // efficiency of integral ABOVE the cut

  int fVerbose; 

  ClassDef(HistCutEfficiency,1) //Testing HistCutEfficiency

}; 

#endif
