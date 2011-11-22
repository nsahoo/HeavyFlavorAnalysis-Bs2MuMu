#include "plotPU.hh"

#include "../macros/AnalysisDistribution.hh"
#include "TMath.h"

using namespace std; 
using std::string; 

ClassImp(plotPU)

// ----------------------------------------------------------------------
plotPU::plotPU(const char *files, const char *cuts, const char *dir, int mode) : plotClass(files, cuts, dir, mode) { 

}

// ----------------------------------------------------------------------
plotPU::~plotPU() {
  fHistFile->Write();
  fHistFile->Close();
}


// ----------------------------------------------------------------------
void plotPU::makeAll(int channels) {
  
  fDoPrint = true; 
  string candName("candAnaBu2JpsiK"); 
  fFile = "NoData"; 
  cd(fFile.c_str());  
  effVsNpv("iso",      0.75, "#epsilon(I>0.75)",           "A", candName.c_str(), "Ao"); 
  effVsNpv("chi2dof", -1.60, "#epsilon(#chi^{2}/dof<1.6)", "A", candName.c_str(), "Ao"); 
  effVsNpv("pchi2dof", 0.10, "#epsilon(prob>0.10)",        "A", candName.c_str(), "Ao"); 
  effVsNpv("fls3d",    15.0, "#epsilon(l_{3D}/#sigma>15)", "A", candName.c_str(), "Ao"); 
  effVsNpv("flsxy",    15.0, "#epsilon(l_{xy}/#sigma>15)", "A", candName.c_str(), "Ao"); 
  effVsNpv("docatrk", 0.015, "#epsilon(d^{0}_{ca}>0.015)", "A", candName.c_str(), "Ao"); 

  candName = "candAnaBs2JpsiPhi"; 
  fFile = "CsData"; 
  cd(fFile.c_str());  
  effVsNpv("iso",      0.75, "#epsilon(I>0.75)",           "A", candName.c_str(), "Ao"); 
  effVsNpv("chi2dof", -1.60, "#epsilon(#chi^{2}/dof<1.6)", "A", candName.c_str(), "Ao"); 
  effVsNpv("pchi2dof", 0.10, "#epsilon(prob>0.10)",        "A", candName.c_str(), "Ao"); 
  effVsNpv("fls3d",    15.0, "#epsilon(l_{3D}/#sigma>15)", "A", candName.c_str(), "Ao"); 
  effVsNpv("flsxy",    15.0, "#epsilon(l_{xy}/#sigma>15)", "A", candName.c_str(), "Ao"); 
  effVsNpv("docatrk", 0.015, "#epsilon(d^{0}_{ca}>0.015)", "A", candName.c_str(), "Ao"); 
  

}




// ----------------------------------------------------------------------
void plotPU::effVsNpv(const char *var, double cut, const char *ylabel, const char *chan, const char *dir, const char *selection) {

  TH1D *h[15];
  
  bool larger(true); 
  if (cut < 0) {
    larger = false; 
    cut = -cut; 
  }

  gFile->cd(dir); 
  AnalysisDistribution a("A_pvz"); 
  cout << "==> gDirectory = "; gDirectory->pwd(); 
  c0->Clear();
  c0->Divide(4,4);
  for (int i = 0; i < 15; ++i) {
    c0->cd(i+1);
    // A_Npv0/A_npv0_iso
    cout << "   directory: " << Form("%s/Npv%d", dir, i) << endl;
    gFile->cd(Form("%s/%s_Npv%d", dir, chan, i));
    cout << "     sbsDistribution(" << Form("%s_npv%d_%s", chan, i, var) << ", " << selection << ") " 
	 << var << (larger?" > ":" < ") << cut 
	 << endl;
    h[i] = a.sbsDistribution(Form("%s_npv%d_%s", chan, i, var), selection);
    // -- check that the bins are not negative. If they are, reset to zero
    for (int ix = 1; ix <= h[i]->GetNbinsX(); ++ix) {
      if (h[i]->GetBinContent(ix) < 0) {
	h[i]->SetBinContent(ix, 0.); 
	h[i]->SetBinError(ix, 0.); 
      }	
    }

    h[i]->Draw("hist");
  }

  if (fDoPrint)  
    c0->SaveAs(Form("%s/effVsNpv0-%s-%s-%s-%s-0_%d.pdf", fDirectory.c_str(), fFile.c_str(), dir, chan, var, static_cast<int>(100.*cut)));


  TH1D *heff = new TH1D("heff", "", 15, 0., 30.);
  for (int i = 0; i < 15; ++i) {
    double ntot = h[i]->GetSumOfWeights();
    double ncut(0.);
    if (larger) {
      ncut = h[i]->Integral(h[i]->FindBin(cut)+1, h[i]->GetNbinsX());
    } else {
      ncut = h[i]->Integral(1, h[i]->FindBin(cut));
    }
    double eff  = ncut/ntot; 
    double stat = dEff(static_cast<int>(ncut), static_cast<int>(ntot));
    double effE = TMath::Sqrt(stat*stat + 0.02*eff*0.02*eff);
    // -- can go above 1, if single bins with too large negative entries are present
    if (eff > 1) {
      eff = 0.; 
      effE = 0.;
    }
    cout << "h[i]->FindBin(cut) = " << h[i]->FindBin(cut) << endl;
    cout << "h[i]->GetNbinsX()  = " << h[i]->GetNbinsX() << endl;
    cout << "ntot = " << ntot << endl;
    cout << "ncut = " << ncut << endl;
    cout << "eff = " << eff << endl;
    cout << "effE = " << effE << endl;
    heff->SetBinContent(i+1, eff); 
    heff->SetBinError(i+1, effE); 
  }

  c0->Clear();
  gStyle->SetOptStat(0); 
  gStyle->SetOptFit(0); 
  setTitles(heff, "N_{PV}", Form("%s", ylabel)); 
  heff->SetMaximum(1.2);
  if (heff->GetMinimum() > 0.5) {
    heff->SetMinimum(0.5);
  } else {
    heff->SetMinimum(0.);
  }
  heff->SetMarkerSize(2); 
  heff->Fit("pol0", "FM");
  heff->GetFunction("pol0")->SetLineWidth(3);
  tl->SetTextSize(0.05); 
  tl->DrawLatex(0.55, 0.91, Form("#epsilon = %4.3f#pm%4.3f", 
				 heff->GetFunction("pol0")->GetParameter(0), 
				 heff->GetFunction("pol0")->GetParError(0))); 

  tl->DrawLatex(0.25, 0.82, Form("#chi^{2}/dof = %3.1f/%i", 
				 heff->GetFunction("pol0")->GetChisquare(), 
				 heff->GetFunction("pol0")->GetNDF())); 

  //  stamp(0.2, "CMS, 1.14 fb^{-1}", 0.7, "#sqrt{s} = 7 TeV"); 
  
  if (fDoPrint)  
    c0->SaveAs(Form("%s/effVsNpv-%s-%s-%s-%s-0_%d.pdf", fDirectory.c_str(), fFile.c_str(), dir, chan, var, static_cast<int>(100.*cut)));

}