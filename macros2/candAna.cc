#include "candAna.hh"
#include <cmath>
#include <string>

#include "../interface/HFMasses.hh"

using namespace std;

// ----------------------------------------------------------------------
candAna::candAna(bmm2Reader *pReader, string name, string cutsFile) {
  cout << "======================================================================" << endl;
  cout << "==> candAna: name = " << name << ", reading cutsfile " << cutsFile << endl;
  fpReader = pReader; 
  fName = name; 

  MASSMIN = 4.5;
  MASSMAX = 6.5; 
  BLIND = 0;

  fGenBTmi = fGenM1Tmi = fGenM2Tmi = fNGenPhotons = fRecM1Tmi = fRecM2Tmi = fCandTmi = -1; 

  fHistDir = gFile->mkdir(fName.c_str());

  fRegion.insert(make_pair("A", 0)); // all
  fRegion.insert(make_pair("B", 1)); // barrel
  fRegion.insert(make_pair("E", 2)); // endcap

  fRegion.insert(make_pair("AR0", 3)); // run range 0: HLT_DoubleMu3_Jpsi_v1 
                                       //              HLT_DoubleMu3_Jpsi_v2
  fRegion.insert(make_pair("AR1", 4)); // run range 1: HLT_Dimuon6p5_Jpsi_Displaced_v1
  fRegion.insert(make_pair("AR2", 5)); // run range 2: HLT_Dimuon7_Jpsi_Displaced_v1 
                                       //              HLT_Dimuon7_Jpsi_Displaced_v3
  fRegion.insert(make_pair("AR3", 6)); // run range 3: HLT_DoubleMu3p5_Jpsi_Displaced_v2
  fRegion.insert(make_pair("AR4", 7)); // run range 4: HLT_DoubleMu4_Jpsi_Displaced_v1 
                                       //              HLT_DoubleMu4_Jpsi_Displaced_v2
  fRegion.insert(make_pair("AR5", 8)); // run range 5: HLT_DoubleMu4_Jpsi_Displaced_v4/5

  fRegion.insert(make_pair("APV0", 9));  // low-NPV events
  fRegion.insert(make_pair("APV1", 10)); // large-NPV events

  fRegion.insert(make_pair("near", 11)); // another PV is closeby 
  fRegion.insert(make_pair("away", 12)); // the closest other PV is far away


}


// ----------------------------------------------------------------------
candAna::~candAna() {
  cout << "==> candAna: destructor..." << endl;
}



// ----------------------------------------------------------------------
void candAna::evtAnalysis(TAna01Event *evt) {

  fpEvt = evt; 
  //   cout << fEvt << "  " << (fEvt>0? " >0" : "<0") << endl;
  //   return;

  //cout << "----------------------------------------------------------------------" << endl;

  if (fIsMC) {
    genMatch(); 
    recoMatch(); 
    candMatch(); 
    efficiencyCalculation();
  } 

  triggerSelection();
  runRange(); 


  TAnaCand *pCand(0);
  for (int iC = 0; iC < fpEvt->nCands(); ++iC) {
    pCand = fpEvt->getCand(iC);
    if (fVerbose > 29) cout << "candidate at " << iC << " which is of type " << pCand->fType << endl;
    if (TYPE != pCand->fType) {
      if (fVerbose > 19) cout << "Skipping candidate at " << iC << " which is of type " << pCand->fType << endl;
      continue;
    }

    if (fVerbose > 10) {
      
      int gen1(-1), gen2(-1), gen0(-1);

      if (fIsMC) {
	gen1 = fpEvt->getGenCand(fpEvt->getRecTrack(fpEvt->getSigTrack(pCand->fSig1)->fIndex)->fGenIndex)->fID;
	gen2 = fpEvt->getGenCand(fpEvt->getRecTrack(fpEvt->getSigTrack(pCand->fSig2)->fIndex)->fGenIndex)->fID;
	gen0 = fpEvt->getGenCand(fpEvt->getGenCand(fpEvt->getRecTrack(fpEvt->getSigTrack(pCand->fSig1)->fIndex)->fGenIndex)->fMom1)->fID;
      }

      cout << "Analyzing candidate at " << iC << " which is of type " << TYPE 
	   << " with sig tracks: " << pCand->fSig1 << " .. " << pCand->fSig2
	   << " and rec tracks: " 
	   << fpEvt->getSigTrack(pCand->fSig1)->fIndex
	   << " .. " 
	   << fpEvt->getSigTrack(pCand->fSig2)->fIndex
	   << " gen IDs =  " << gen1 << " " << gen2 << " from " << gen0
	   << " tm = " << fGenM1Tmi << " " << fGenM2Tmi 
	   << " ctm " << fCandTmi 
	   << endl;
      if (TMath::Abs(gen1) != 13 || TMath::Abs(gen2) != 13) fpEvt->dumpGenBlock();

    }
    fpCand = pCand;
    fCandIdx = iC; 

    //cout<<" call analysis "<<fpCand<<" "<<iC<<endl;
    // -- call derived functions
    candAnalysis();

    if (0 && fCandM > 4.99 && fCandM < 5.02 && fCandFLS3d > 13 && fCandA < 0.05 && fMu1Pt > 4.5 && fMu2Pt > 4.5) {
      cout << "----------------------------------------------------------------------" << endl;
      int gen1(-1), gen2(-1), gen0(-1);
      gen1 = fpEvt->getGenCand(fpEvt->getRecTrack(fpEvt->getSigTrack(pCand->fSig1)->fIndex)->fGenIndex)->fID;
      gen2 = fpEvt->getGenCand(fpEvt->getRecTrack(fpEvt->getSigTrack(pCand->fSig2)->fIndex)->fGenIndex)->fID;
      gen0 = fpEvt->getGenCand(fpEvt->getGenCand(fpEvt->getRecTrack(fpEvt->getSigTrack(pCand->fSig1)->fIndex)->fGenIndex)->fMom1)->fID;

      cout << "Analyzing candidate at " << iC << " which is of type " << TYPE 
	   << " with sig tracks: " << pCand->fSig1 << " .. " << pCand->fSig2
	   << " and rec tracks: " 
	   << fpEvt->getSigTrack(pCand->fSig1)->fIndex
	   << " .. " 
	   << fpEvt->getSigTrack(pCand->fSig2)->fIndex << endl
	   << " gen IDs =  " << gen1 << " " << gen2 << " from " << gen0
	   << " tm = " << fGenM1Tmi << " " << fGenM2Tmi 
	   << " ctm " << fCandTmi 
	   << endl;
      cout << fpCand->fMass << endl;

      TLorentzVector gm1 = fpEvt->getGenCand(fpEvt->getRecTrack(fpEvt->getSigTrack(pCand->fSig1)->fIndex)->fGenIndex)->fP;
      TLorentzVector gm2 = fpEvt->getGenCand(fpEvt->getRecTrack(fpEvt->getSigTrack(pCand->fSig2)->fIndex)->fGenIndex)->fP;
      cout << "gen level mass: " << (gm1+gm2).M() << endl;
      cout << "----------------------------------------------------------------------" << endl;
      fpEvt->dumpGenBlock();
    }

    
    if (fIsMC) {
      fTree->Fill(); 
    } else {
      if (BLIND && fpCand->fMass > SIGBOXMIN && fpCand->fMass < SIGBOXMAX  && fCandIso > 0.7) {
	// do nothing
	//cout<<" blinded "<<BLIND<<" "<<fpCand->fMass<<" "<<fCandM<<" "<<SIGBOXMIN<<" "<<SIGBOXMAX<<" "<<fCandIso<<" "<<fPreselection<<endl;;
      } else {
	if (fPreselection) {
	  ((TH1D*)fHistDir->Get("../monEvents"))->Fill(12); 
	  fTree->Fill(); 
	}         
      }
    }


    // -- fill histograms
    fillCandidateHistograms(fRegion["A"]);
    if (fBarrel) {
      fillCandidateHistograms(fRegion["B"]);
    } else {
      fillCandidateHistograms(fRegion["E"]);
    }

    if (fPvN <  6) fillCandidateHistograms(fRegion["APV0"]);
    if (fPvN > 10) fillCandidateHistograms(fRegion["APV1"]);
    
    fillCandidateHistograms(fRegion[Form("AR%i", fRunRange)]);

    // -- special studies
    //    fillIsoPlots(); // FIXISOPLOTS

  }

}

// ----------------------------------------------------------------------
void candAna::candAnalysis() {

  //cout<<" cand "<<fpCand<<endl;

  if (0 == fpCand) return;

  ((TH1D*)fHistDir->Get("../monEvents"))->Fill(1); 

  int goodSV(0); 
  TAnaVertex *pVtx; 
  fPvN = 0; 
  for (int i = 0; i < fpEvt->nPV(); ++i) {
    pVtx = fpEvt->getPV(i); 
    if (0 == pVtx->fStatus) {
      ++fPvN;
    } else {
      //      cout << "skipping  fake vertex" << endl;
    }
  }

  //cout << "fPvN = " << fPvN << " "<<fpCand->fPvIdx <<" "<<fpCand->fPvIdx2 <<" "<<fpEvt->nPV()<<endl;

  if (fpCand->fPvIdx > -1 && fpCand->fPvIdx < fpEvt->nPV()) {
    goodSV = 1; 
    TAnaVertex *pv = fpEvt->getPV(fpCand->fPvIdx); 
    fPvX = pv->fPoint.X(); 
    fPvY = pv->fPoint.Y(); 
    fPvZ = pv->fPoint.Z(); 
    fPvNtrk = pv->getNtracks();
    fPvNdof = pv->fNdof;
    fPvAveW8 = ((fPvNdof+2.)/2.)/fPvNtrk;
  } else {
    fPvX = -99.;
    fPvY = -99.;
    fPvZ = -99.;
    fPvNtrk = -99;
  }

  if (fIsMC) {
    if (fCandTmi == fCandIdx) {
      if (fNGenPhotons) {
	fCandTM = 2; 
      } else {
	fCandTM = 1; 
      }
    } else {
      fCandTM = 0; 
    }
  } else {
    fCandTM = 0; 
  }

  fCandType     = fpCand->fType;
  fCandPt       = fpCand->fPlab.Perp();
  fCandP        = fpCand->fPlab.Mag();
  fCandEta      = fpCand->fPlab.Eta();
  fCandPhi      = fpCand->fPlab.Phi();
  fCandM        = fpCand->fMass;
  fCandDoca     = fpCand->fMaxDoca;

  // -- values of cand wrt PV
  fCandPvTip    = fpCand->fPvTip;
  fCandPvTipE   = fpCand->fPvTipE;
  fCandPvTipS   = fCandPvTip/fCandPvTipE;
  fCandPvLip    = fpCand->fPvLip;
  fCandPvLipE   = fpCand->fPvLipE;
  fCandPvLipS   = fCandPvLip/fCandPvLipE;
  fCandPvLip2   = fpCand->fPvLip2;
  fCandPvLipS2  = fpCand->fPvLip2/fpCand->fPvLipE2;
  if (fpCand->fPvLip2 > 999) fCandPvLipS2 = 999.; // in this case no 2nd best PV was found, reset to 'good' state
  fCandPvLip12  = fCandPvLip/fpCand->fPvLip2;
  fCandPvLipE12 = fCandPvLipE/fpCand->fPvLipE2;
  fCandPvLipS12 = fCandPvLipS/(fpCand->fPvLip2/fpCand->fPvLipE2);

  // -- 3d impact parameter wrt PV
  fCandPvIp     = TMath::Sqrt(fCandPvLip*fCandPvLip + fCandPvTip*fCandPvTip); 
  fCandPvIpE    = (fCandPvLip*fCandPvLip*fCandPvLipE*fCandPvLipE + fCandPvTip*fCandPvTip*fCandPvTipE*fCandPvTipE)/(fCandPvIp*fCandPvIp); 
  fCandPvIpE    = TMath::Sqrt(fCandPvIpE); 
  fCandPvIpS    = fCandPvIp/fCandPvIpE;
  if (TMath::IsNaN(fCandPvIpS)) fCandPvIpS = -1.;

  fCandM2 = constrainedMass();
  
  TAnaTrack *p0; 
  TAnaTrack *p1(0), *ps1(0);
  TAnaTrack *p2(0), *ps2(0); 
  
  fCandQ    = 0;

  for (int it = fpCand->fSig1; it <= fpCand->fSig2; ++it) {
    p0 = fpEvt->getSigTrack(it);     
    fCandQ += fpEvt->getRecTrack(p0->fIndex)->fQ;
    if (TMath::Abs(p0->fMCID) != 13) continue;
    if (0 == p1) {
      p1 = p0; 
    } else {
      p2 = p0; 
    }
  }

  // -- for rare backgrounds there are no "true" muons
  if (fpCand->fType > 1000000 && fpCand->fType < 2000000) {
    p1 = fpEvt->getSigTrack(fpCand->fSig1); 
    p2 = fpEvt->getSigTrack(fpCand->fSig2); 
  }

  if (fpCand->fType == 300030) {
    if (fpCand->fDau1 < 0 || fpCand->fDau1 > fpEvt->nCands()) return;
    TAnaCand *pD = fpEvt->getCand(fpCand->fDau1); 
    pD = fpEvt->getCand(pD->fDau1); 
    p1 = fpEvt->getSigTrack(pD->fSig1); 
    p2 = fpEvt->getSigTrack(pD->fSig2); 
  }
  
  // -- switch to RecTracks!
  ps1= p1; 
  p1 = fpEvt->getRecTrack(ps1->fIndex);
  ps2= p2; 
  p2 = fpEvt->getRecTrack(ps2->fIndex);

  if (1301 == fpCand->fType ||1302 == fpCand->fType || 1313 == fpCand->fType) {
    // do nothing, these types are for efficiency/TNP studies
  } else {
    if (p1->fPlab.Perp() < p2->fPlab.Perp()) {
      p0 = p1; 
      p1 = p2; 
      p2 = p0; 
      
      p0  = ps1; 
      ps1 = ps2; 
      ps2 = p0;
    }
  }

  fpMuon1 = p1; 
  fpMuon2 = p2; 

  //  fMu1Id        = goodMuon(p1); 
  fMu1Id        = tightMuon(p1); 
  fMu1Pt        = p1->fPlab.Perp(); 
  fMu1Eta       = p1->fPlab.Eta(); 
  fMu1Phi       = p1->fPlab.Phi(); 
  fMu1PtNrf     = ps1->fPlab.Perp();
  fMu1EtaNrf    = ps1->fPlab.Eta();
  fMu1TkQuality = p1->fTrackQuality & TRACKQUALITY;
  fMu1Q         = p1->fQ;
  fMu1Pix       = fpReader->numberOfPixLayers(p1);
  fMu1BPix      = fpReader->numberOfBPixLayers(p1);
  fMu1BPixL1    = fpReader->numberOfBPixLayer1Hits(p1);
  fMu1PV        = p1->fPvIdx;
  fMu1IP        = p1->fBsTip;
  fMu1IPE       = p1->fBsTipE;

  if (p1->fMuIndex > -1) {
    fMu1Chi2      = fpEvt->getMuon(p1->fMuIndex)->fMuonChi2;
  } else {
    fMu1Chi2 = -98.;
  }

  if (fCandTM && fGenM1Tmi < 0) fpEvt->dump();
  
  if (fCandTmi > -1) {
    TGenCand *pg1 = fpEvt->getGenCand(p1->fGenIndex);
    fMu1PtGen     = pg1->fP.Perp();
    fMu1EtaGen    = pg1->fP.Eta();
  } else {
    fMu1PtGen     = -99.;
    fMu1EtaGen    = -99.;
  }
  
  //  fMu2Id        = goodMuon(p2); 
  fMu2Id        = tightMuon(p2); 
  fMu2Pt        = p2->fPlab.Perp(); 
  fMu2Eta       = p2->fPlab.Eta(); 
  fMu2Phi       = p2->fPlab.Phi(); 
  fMu2PtNrf     = ps2->fPlab.Perp();
  fMu2EtaNrf    = ps2->fPlab.Eta();
  fMu2TkQuality = p2->fTrackQuality & TRACKQUALITY;
  fMu2Q         = p2->fQ;
  fMu2Pix       = fpReader->numberOfPixLayers(p2);
  fMu2BPix      = fpReader->numberOfBPixLayers(p2);
  fMu2BPixL1    = fpReader->numberOfBPixLayer1Hits(p2);
  fMu2PV        = p2->fPvIdx;
  fMu2IP        = p2->fBsTip;
  fMu2IPE       = p2->fBsTipE;

  // -- cut on fMuIndex so that fake muons (from rare backgrounds) can be treated above as real muons
  if (p1->fMuIndex > -1 && p2->fMuIndex > -1) {
    TVector3 rm1  = fpEvt->getMuon(p1->fMuIndex)->fPositionAtM2;
    TVector3 rm2  = fpEvt->getMuon(p2->fMuIndex)->fPositionAtM2;

    if (rm1.Mag() > 0.1 && rm2.Mag() > 0.1) {
      TVector3 rD   = rm2-rm1; 
      fMuDist   = rD.Mag(); 
      fMuDeltaR =  rm1.DeltaR(rm2); 
    } else {
      fMuDist   = -99.; 
      fMuDeltaR = -99.; 
    }

    if (fVerbose > 10) cout << "dist: " << fMuDist << " dr = " << fMuDeltaR << endl;
  } else {
    fMuDist   = -99.; 
    fMuDeltaR = -99.; 
  }

  if (p2->fMuIndex > -1) {
    fMu2Chi2      = fpEvt->getMuon(p2->fMuIndex)->fMuonChi2;
  } else {
    fMu2Chi2 = -98.;
  }

  if ((TMath::Abs(fMu1Eta) < 1.4) && (TMath::Abs(fMu2Eta) < 1.4)) {
    fBarrel = true; 
  } else {
    fBarrel = false; 
  }    

  fChan = detChan(fMu1Eta, fMu2Eta); 

  double dphi = p1->fPlab.DeltaPhi(p2->fPlab);
  fCowboy = (p1->fQ*dphi > 0); 

  // -- Muon weights
  PidTable *pT, *pT1, *pT2; 
  if (fCowboy) {
    pT = fpReader->ptCbMUID; 
  } else {
    pT = fpReader->ptSgMUID; 
  }
  fMu1W8Mu      = pT->effD(fMu1Pt, TMath::Abs(fMu1Eta), fMu1Phi);
  fMu2W8Mu      = pT->effD(fMu2Pt, TMath::Abs(fMu2Eta), fMu2Phi);
  
  if (fCowboy) {
    pT1 = fpReader->ptCbMUT1;
    pT2 = fpReader->ptCbMUT2;
  } else {
    pT1 = fpReader->ptSgMUT1;
    pT2 = fpReader->ptSgMUT2;
  }
  fMu1W8Tr      = pT1->effD(fMu1Pt, TMath::Abs(fMu1Eta), fMu1Phi)*pT2->effD(fMu1Pt, TMath::Abs(fMu1Eta), fMu1Phi);
  fMu2W8Tr      = pT1->effD(fMu2Pt, TMath::Abs(fMu2Eta), fMu2Phi)*pT2->effD(fMu2Pt, TMath::Abs(fMu2Eta), fMu2Phi);

  if (fCandTmi > -1) {
    TGenCand *pg2 = fpEvt->getGenCand(p2->fGenIndex);
    fMu2PtGen     = pg2->fP.Perp();
    fMu2EtaGen    = pg2->fP.Eta();
  } else {
    fMu2PtGen     = -99.;
    fMu2EtaGen    = -99.;
  }

  fCandW8Mu     = fMu1W8Mu*fMu2W8Mu;
  if (TMath::Abs(fCandW8Mu) > 1.) fCandW8Mu = 0.2; // FIXME correction for missing entries at low pT
  fCandW8Tr     = fMu1W8Tr*fMu2W8Tr;
  if (TMath::Abs(fCandW8Tr) > 1.) fCandW8Tr = 0.2; // FIXME correction for missing entries at low pT 

  // -- FIXME ????
  int pvidx = (fpCand->fPvIdx > -1? fpCand->fPvIdx : 0); 
  // -- this is from the full candidate
  TAnaVertex sv = fpCand->fVtx;
  // -- this is from the dimuon vertex
  TAnaCand *pD; 
  TAnaVertex sv2m;
  bool good2m(false); 
  //  cout << "looking at daughters " << fpCand->fDau1  << " .. " << fpCand->fDau2 << endl;
  for (int id = fpCand->fDau1; id <= fpCand->fDau2; ++id) {
    if (id < 0) break;
    pD = fpEvt->getCand(id); 
    //    cout << "looking at daughter " <<  id << " with type = " << pD->fType << endl;
    if (300443 == pD->fType) {
      sv2m = pD->fVtx;
      good2m = true; 
      //      cout << "  Found J/psi vertex" << endl;
      break;
    }
  }

  // -- go back to original!
  sv = fpCand->fVtx;

  TVector3 svpv(sv.fPoint - fpEvt->getPV(pvidx)->fPoint); 
  double alpha = svpv.Angle(fpCand->fPlab);
  fCandCosA   = TMath::Cos(alpha);
  fCandA      = alpha; 

  //  virtual double      isoClassicWithDOCA(TAnaCand*, double dca, double r = 1.0, double ptmin = 0.9); 
  double iso = isoClassicWithDOCA(fpCand, 0.05, 0.7, 0.9); // 500um DOCA cut
  fCandIso      = iso; 
  fCandPvTrk    = fCandI0trk;
  fCandIsoTrk   = fCandI2trk;
  fCandCloseTrk = nCloseTracks(fpCand, 0.03, 0.5);

  fCandChi2  = sv.fChi2;
  fCandDof   = sv.fNdof;
  fCandProb  = sv.fProb;
  fCandFL3d  = sv.fD3d;
  fCandFL3dE = sv.fD3dE;
  fCandFLS3d = sv.fD3d/sv.fD3dE; 
  if (TMath::IsNaN(fCandFLS3d)) fCandFLS3d = -1.;
  fCandFLxy  = sv.fDxy;
  fCandFLSxy = sv.fDxy/sv.fDxyE; 
  if (TMath::IsNaN(fCandFLSxy)) fCandFLSxy = -1.;

  fCandTau   = fCandFL3d*MBS/fCandP/TMath::Ccgs();

  // -- variables for production mechanism studies
  fpOsCand      = osCand(fpCand);
  fOsIso        = osIsolation(fpCand, 1.0, 0.9); 
  fOsRelIso     = fOsIso/fCandPt; 
  int osm       = osMuon(fpCand, 0.);
  fOsMuonPt     = (osm > -1? fpEvt->getRecTrack(osm)->fPlab.Perp():-1.);
  fOsMuonPtRel  = (osm > -1? fpEvt->getRecTrack(osm)->fPlab.Perp(fpCand->fPlab):-1);
  fOsMuonDeltaR =  (osm > -1? fpCand->fPlab.DeltaR(fpEvt->getRecTrack(osm)->fPlab):-1.);

//   if (301313 == fCandType) 
//     cout << fRun << " " << fEvt 
// 	 << " from PV = " << fpCand->fPvIdx << " with ntrk = " << fPvNtrk << " av w8 = " << fPvAveW8
// 	 << " cand tracks mindoca = " << fpCand->fMinDoca << " maxdoca = " << fpCand->fMaxDoca << " and chi2/dof = " << fCandChi2/fCandDof 
// 	 << endl;
  
  // -- dimuon vertex version
  if (good2m) { 
    f2MChi2  = sv2m.fChi2;
    f2MDof   = sv2m.fNdof;
    f2MProb  = sv2m.fProb;
    f2MFL3d  = sv2m.fD3d;
    f2MFL3dE = sv2m.fD3dE;
    f2MFLS3d = sv2m.fD3d/sv2m.fD3dE; 
    if (TMath::IsNaN(f2MFLS3d)) f2MFLS3d = -1.;
    f2MFLSxy = sv2m.fDxy/sv2m.fDxyE; 
    if (TMath::IsNaN(f2MFLSxy)) f2MFLSxy = -1.;
  } else {
    f2MChi2  = -1.;
    f2MDof   = -1;
    f2MProb  = -1.;
    f2MFL3d  = -1.;
    f2MFL3dE = -1.;
    f2MFLS3d = -1.; 
    f2MFLSxy = -1.; 
  }

  if (fpCand->fNstTracks.size() == 0) {
    //    cout << "HHHHEEEELLLLPPPP" << endl;
    fCandDocaTrk = 99.;
    fCandDocaTrkBdt = 99.;
  } else {
    fCandDocaTrk    = fpCand->fNstTracks[0].second.first;
    fCandDocaTrkBdt = fpCand->fNstTracks[0].second.first;
    
    int nsize(fpCand->fNstTracks.size());
    for (int i = 0; i<nsize; ++i) {
      int trkId = fpCand->fNstTracks[i].first;
      p0 = fpEvt->getRecTrack(trkId);
      // -- check that any track associated with a definitive vertex is from the same or the closest (compatible) other PV
      if ((p0->fPvIdx > -1) && (p0->fPvIdx != pvidx)) continue;
      
      fCandDocaTrk = fpCand->fNstTracks[i].second.first;
      break;
    }
    
  }
  
  if (BLIND && fpCand->fMass > SIGBOXMIN && fpCand->fMass < SIGBOXMAX  && fCandIso < 0.7) {
    calcBDT(true);
  } else {
    calcBDT(false);
  }  

  fWideMass       = ((fpCand->fMass > MASSMIN) && (fpCand->fMass < MASSMAX)); 

  fGoodMuonsID    = (fMu1Id && fMu2Id);
  fGoodMuonsPt    = ((fMu1Pt > MUPTLO) && (fMu1Pt < MUPTHI) && (fMu2Pt > MUPTLO) && (fMu2Pt < MUPTHI));
  fGoodMuonsEta   = ((fMu1Eta > MUETALO) && (fMu1Eta < MUETAHI) && (fMu2Eta > MUETALO) && (fMu2Eta < MUETAHI));
  fGoodTracks     = (goodTrack(p1) && goodTrack(p2));
  fGoodTracksPt   = ((fMu1Pt > TRACKPTLO) && (fMu1Pt < TRACKPTHI) && (fMu2Pt > TRACKPTLO) && (fMu2Pt < TRACKPTHI));
  fGoodTracksEta  = ((fMu1Eta > TRACKETALO) && (fMu1Eta < TRACKETAHI) && (fMu2Eta > TRACKETALO) && (fMu2Eta < TRACKETAHI));

  fGoodQ          = (fMu1Q*fMu2Q < 0); 
  fGoodPvAveW8    = (fPvAveW8 > PVAVEW8);
  fGoodPvLip      = (TMath::Abs(fCandPvLip) < CANDLIP); 
  fGoodPvLipS     = (TMath::Abs(fCandPvLipS) < CANDLIPS); 
  fGoodPvLip2     = (TMath::Abs(fCandPvLip2) > CANDLIP2); 
  fGoodPvLipS2    = (TMath::Abs(fCandPvLipS2) > CANDLIPS2); 
  fGoodMaxDoca    = (TMath::Abs(fCandDoca) < CANDDOCA); 
  fGoodIp         = (TMath::Abs(fCandPvIp) < CANDIP); 
  fGoodIpS        = (TMath::Abs(fCandPvIpS) < CANDIPS); 
    
  fGoodPt         = (fCandPt > CANDPTLO);
  fGoodEta        = ((fCandEta > CANDETALO) && (fCandEta < CANDETAHI)); 
  fGoodAlpha      = (fCandA < CANDALPHA); 
  fGoodChi2       = (fCandChi2/fCandDof < CANDVTXCHI2);
  fGoodFLS        =  ((fCandFLS3d > CANDFLS3D) && (fCandFLSxy > CANDFLSXY)); 
  if (TMath::IsNaN(fCandFLS3d)) fGoodFLS = false;

  fGoodCloseTrack = (fCandCloseTrk < CANDCLOSETRK); 
  fGoodIso        = (fCandIso > CANDISOLATION); 
  fGoodDocaTrk    = (fCandDocaTrk > CANDDOCATRK);
  fGoodLastCut    = true; 

  fAnaCuts.update(); 

  //fPreselection = fWideMass && fGoodTracks && fGoodTracksPt && fGoodTracksEta && fGoodMuonsPt && fGoodMuonsEta; 
  fPreselection = true; 
  fPreselection = fPreselection && fGoodPvLip && fGoodPvLipS && fGoodQ; 
  fPreselection = fPreselection && (fCandPt > 5) && (fCandA < 0.2) && (fCandFLS3d > 5) && (fCandChi2/fCandDof < 5); 

}

// ----------------------------------------------------------------------
void candAna::fillCandidateHistograms(int offset) {
 
  //cout<<" 11 "<<fpCand<<" "<<offset<<endl;

  // -- only candidate histograms below
  if (0 == fpCand) return;

  // -- Fill distributions
  fpTracksPt[offset]->fill(fMu1Pt, fCandM);
  fpTracksPt[offset]->fill(fMu2Pt, fCandM);
  fpTracksEta[offset]->fill(fMu1Eta, fCandM);
  fpTracksEta[offset]->fill(fMu2Eta, fCandM);
  fpTracksQual[offset]->fill((fGoodTracks?1:0), fCandM);

  fpMuonsPt[offset]->fill(fMu1Pt, fCandM);
  fpMuonsPt[offset]->fill(fMu2Pt, fCandM);
  fpMuonsEta[offset]->fill(fMu1Eta, fCandM);
  fpMuonsEta[offset]->fill(fMu2Eta, fCandM);
  fpMuon1Eta[offset]->fill(fMu1Eta, fCandM);
  fpMuon2Eta[offset]->fill(fMu2Eta, fCandM);
  fpMuon1Pt[offset]->fill(fMu1Pt, fCandM);
  fpMuon2Pt[offset]->fill(fMu2Pt, fCandM);

  fpHLT[offset]->fill((fGoodHLT?1:0), fCandM); 
  fpMuonsID[offset]->fill((fGoodMuonsID?1:0), fCandM); 
  fpPvZ[offset]->fill(fPvZ, fCandM); 
  fpPvN[offset]->fill(fPvN, fCandM); 
  fpPvNtrk[offset]->fill(fPvNtrk, fCandM); 
  fpPvAveW8[offset]->fill(fPvAveW8, fCandM); 
  fpPt[offset]->fill(fCandPt, fCandM); 
  fpP[offset]->fill(fCandP, fCandM);
  fpEta[offset]->fill(fCandEta, fCandM); 
  fpAlpha[offset]->fill(fCandA, fCandM);
  fpCosA[offset]->fill(fCandCosA, fCandM);
  fpIsoTrk[offset]->fill(fCandIsoTrk, fCandM);

  fpPt[offset]->fill(fCandPt, fCandM); 
  if (40 == fProcessType) fpPtGGF[offset]->fill(fCandPt, fCandM); 
  if (41 == fProcessType) fpPtFEX[offset]->fill(fCandPt, fCandM); 
  if (42 == fProcessType) fpPtGSP[offset]->fill(fCandPt, fCandM); 

  fpIso[offset]->fill(fCandIso, fCandM);
  if (40 == fProcessType) fpIsoGGF[offset]->fill(fCandIso, fCandM); 
  if (41 == fProcessType) fpIsoFEX[offset]->fill(fCandIso, fCandM); 
  if (42 == fProcessType) fpIsoGSP[offset]->fill(fCandIso, fCandM); 

  fpCloseTrk[offset]->fill(fCandCloseTrk, fCandM); 
  if (40 == fProcessType) fpCloseTrkGGF[offset]->fill(fCandCloseTrk, fCandM); 
  if (41 == fProcessType) fpCloseTrkFEX[offset]->fill(fCandCloseTrk, fCandM); 
  if (42 == fProcessType) fpCloseTrkGSP[offset]->fill(fCandCloseTrk, fCandM); 

  fpOsIso[offset]->fill(fOsIso, fCandM); 
  if (40 == fProcessType) fpOsIsoGGF[offset]->fill(fOsIso, fCandM); 
  if (41 == fProcessType) fpOsIsoFEX[offset]->fill(fOsIso, fCandM); 
  if (42 == fProcessType) fpOsIsoGSP[offset]->fill(fOsIso, fCandM); 

  fpOsRelIso[offset]->fill(fOsRelIso, fCandM); 
  if (40 == fProcessType) fpOsRelIsoGGF[offset]->fill(fOsRelIso, fCandM); 
  if (41 == fProcessType) fpOsRelIsoFEX[offset]->fill(fOsRelIso, fCandM); 
  if (42 == fProcessType) fpOsRelIsoGSP[offset]->fill(fOsRelIso, fCandM); 

  fpOsMuonPt[offset]->fill(fOsMuonPt, fCandM);
  if (40 == fProcessType) fpOsMuonPtGGF[offset]->fill(fOsMuonPt, fCandM); 
  if (41 == fProcessType) fpOsMuonPtFEX[offset]->fill(fOsMuonPt, fCandM); 
  if (42 == fProcessType) fpOsMuonPtGSP[offset]->fill(fOsMuonPt, fCandM); 

  fpOsMuonPtRel[offset]->fill(fOsMuonPtRel, fCandM);
  if (40 == fProcessType) fpOsMuonPtRelGGF[offset]->fill(fOsMuonPtRel, fCandM); 
  if (41 == fProcessType) fpOsMuonPtRelFEX[offset]->fill(fOsMuonPtRel, fCandM); 
  if (42 == fProcessType) fpOsMuonPtRelGSP[offset]->fill(fOsMuonPtRel, fCandM); 

  fpOsMuonDeltaR[offset]->fill(fOsMuonDeltaR, fCandM); 
  if (40 == fProcessType) fpOsMuonDeltaRGGF[offset]->fill(fOsMuonDeltaR, fCandM); 
  if (41 == fProcessType) fpOsMuonDeltaRFEX[offset]->fill(fOsMuonDeltaR, fCandM); 
  if (42 == fProcessType) fpOsMuonDeltaRGSP[offset]->fill(fOsMuonDeltaR, fCandM); 

  fpDocaTrk[offset]->fill(fCandDocaTrk, fCandM); 
  if (40 == fProcessType) fpDocaTrkGGF[offset]->fill(fCandDocaTrk, fCandM); 
  if (41 == fProcessType) fpDocaTrkFEX[offset]->fill(fCandDocaTrk, fCandM); 
  if (42 == fProcessType) fpDocaTrkGSP[offset]->fill(fCandDocaTrk, fCandM); 

  fpChi2[offset]->fill(fCandChi2, fCandM);
  fpChi2Dof[offset]->fill(fCandChi2/fCandDof, fCandM); 
  fpProb[offset]->fill(fCandProb, fCandM);   
  fpFLS3d[offset]->fill(fCandFLS3d, fCandM); 
  fpFL3d[offset]->fill(fCandFL3d, fCandM); 
  fpFL3dE[offset]->fill(fCandFL3dE, fCandM); 
  fpFLSxy[offset]->fill(fCandFLSxy, fCandM); 
  fpBDT[offset]->fill(fBDT, fCandM);    

  fpLip[offset]->fill(fCandPvLip, fCandM); 
  fpLipE[offset]->fill(fCandPvLipE, fCandM); 
  fpLipS[offset]->fill(fCandPvLipS, fCandM); 

  fpTip[offset]->fill(fCandPvTip, fCandM); 
  fpTipE[offset]->fill(fCandPvTipE, fCandM); 
  fpTipS[offset]->fill(fCandPvTipS, fCandM); 

  fpLip12[offset]->fill(fCandPvLip12, fCandM); 
  fpLipE12[offset]->fill(fCandPvLipE12, fCandM); 
  fpLipS12[offset]->fill(fCandPvLipS12, fCandM); 

  fpLip2[offset]->fill(fCandPvLip2, fCandM); 
  fpLipS2[offset]->fill(fCandPvLipS2, fCandM); 

  fpMaxDoca[offset]->fill(fCandDoca, fCandM); 
  fpIp[offset]->fill(fCandPvIp, fCandM); 
  fpIpS[offset]->fill(fCandPvIpS, fCandM); 

  fp2MChi2[offset]->fill(f2MChi2, fCandM);
  fp2MChi2Dof[offset]->fill(f2MChi2/f2MDof, fCandM); 
  fp2MProb[offset]->fill(f2MProb, fCandM);   
  fp2MFLS3d[offset]->fill(f2MFLS3d, fCandM); 
  fp2MFL3d[offset]->fill(f2MFL3d, fCandM); 
  fp2MFL3dE[offset]->fill(f2MFL3dE, fCandM); 
  fp2MFLSxy[offset]->fill(f2MFLSxy, fCandM); 

  // -- eta profiles
  int ieta = ((fCandEta+2.4)/5.)*NADPV;
  if (ieta < 0) ieta = 0; 
  if (ieta > 14) ieta = 14; 

  if (fpEtaFLS3d[ieta][offset]) 
    fpEtaFLS3d[ieta][offset]->fill(fCandFLS3d, fCandM); 
//   else 
//     cout << "missing fpEtaFLS3d: " << ieta << "  " << offset << endl;

  // -- NPV analysis distributions
  int ipv = 0; 
  if (fPvN < 50) {
    ipv = fPvN/2; 
  } else {
    ipv = NADPV-1; 
  }

  if (fpNpvPvN[ipv][offset]) {
    if (fpNpvPvN[ipv][offset]) fpNpvPvN[ipv][offset]->fill(fPvN, fCandM);  else cout << "missing fpNpvPvN" << endl;
    if (fpNpvChi2Dof[ipv][offset])  fpNpvChi2Dof[ipv][offset]->fill(fCandChi2/fCandDof, fCandM);  else cout << "missing fpNpvChi2Dof" << endl;
    if (fpNpvProb[ipv][offset]) fpNpvProb[ipv][offset]->fill(fCandProb, fCandM); else cout << "missing fpNpvProb" << endl;
    if (fpNpvAlpha[ipv][offset]) fpNpvAlpha[ipv][offset]->fill(fCandA, fCandM); else cout << "missing fpNpvAlpha" << endl;
    if (fpNpvFLS3d[ipv][offset]) fpNpvFLS3d[ipv][offset]->fill(fCandFLS3d, fCandM); else cout << "missing fpNpvFLS3d" << endl;
    if (fpNpvFLSxy[ipv][offset]) fpNpvFLSxy[ipv][offset]->fill(fCandFLSxy, fCandM); else cout << "missing fpNpvFLSxy" << endl;
    if (fpNpvDocaTrk[ipv][offset]) fpNpvDocaTrk[ipv][offset]->fill(fCandDocaTrk, fCandM); else cout << "missing fpNpvDocaTrk" << endl;
    if (fpNpvIso[ipv][offset]) fpNpvIso[ipv][offset]->fill(fCandIso, fCandM); else cout << "missing fpNpvIso" << endl;
    if (fpNpvIsoTrk[ipv][offset]) fpNpvIsoTrk[ipv][offset]->fill(fCandIsoTrk, fCandM); else cout << "missing fpNpvIsoTrk" << endl;
    if (fpNpvLip[ipv][offset]) fpNpvLip[ipv][offset]->fill(fCandPvLip, fCandM); else cout << "missing fpNpvLip" << endl;
    if (fpNpvLipS[ipv][offset]) fpNpvLipS[ipv][offset]->fill(fCandPvLipS, fCandM); else cout << "missing fpNpvLipS" << endl;
    if (fpNpvLip2[ipv][offset]) fpNpvLip2[ipv][offset]->fill(fCandPvLip2, fCandM); else cout << "missing fpNpvLip2" << endl;
    if (fpNpvLipS2[ipv][offset]) fpNpvLipS2[ipv][offset]->fill(fCandPvLipS2, fCandM); else cout << "missing fpNpvLipS2" << endl;
    if (fpNpvMaxDoca[ipv][offset]) fpNpvLipS2[ipv][offset]->fill(fCandDoca, fCandM); else cout << "missing fpNpvMaxDoca" << endl;
    if (fpNpvIp[ipv][offset]) fpNpvIp[ipv][offset]->fill(fCandPvIp, fCandM); else cout << "missing fpNpvIp" << endl;
    if (fpNpvIpS[ipv][offset]) fpNpvIpS[ipv][offset]->fill(fCandPvIpS, fCandM); else cout << "missing fpNpvIpS" << endl;
    if (fpNpvAveW8[ipv][offset]) fpNpvAveW8[ipv][offset]->fill(fPvAveW8, fCandM); else  cout << " fpNpvAveW8 missing! " << endl;
    if (fpNpvCloseTrk[ipv][offset]) fpNpvCloseTrk[ipv][offset]->fill(fCandCloseTrk, fCandM); else cout << "missing fpNpvCloseTrk" << endl;


    //   //  virtual double      isoClassicWithDOCA(TAnaCand*, double dca, double r = 1.0, double ptmin = 0.9); 
    //   double iso = isoClassicWithDOCA(fpCand, 0.05, 0.9, 0.7); // 500um DOCA cut

    double iso = isoClassicWithDOCA(fpCand, 0.05, 1.0, 0.9);
    if (fpNpvIso0[ipv][offset]) fpNpvIso0[ipv][offset]->fill(iso, fCandM); else cout << "missing fpNpvIso0" << endl;
    iso = isoClassicWithDOCA(fpCand, 0.05, 0.5, 0.5);
    if (fpNpvIso1[ipv][offset]) fpNpvIso1[ipv][offset]->fill(iso, fCandM); else cout << "missing fpNpvIso1" << endl;
    iso = isoClassicWithDOCA(fpCand, 0.05, 0.7, 0.5);
    if (fpNpvIso2[ipv][offset]) fpNpvIso2[ipv][offset]->fill(iso, fCandM); else cout << "missing fpNpvIso2" << endl;
    iso = isoClassicWithDOCA(fpCand, 0.05, 0.7, 0.9);
    if (fpNpvIso3[ipv][offset]) fpNpvIso3[ipv][offset]->fill(iso, fCandM); else cout << "missing fpNpvIso3" << endl;
    iso = isoClassicWithDOCA(fpCand, 0.04, 0.7, 0.5);
    if (fpNpvIso4[ipv][offset]) fpNpvIso4[ipv][offset]->fill(iso, fCandM); else cout << "missing fpNpvIso4" << endl;
    iso = isoClassicWithDOCA(fpCand, 0.03, 0.7, 0.7);
    if (fpNpvIso5[ipv][offset]) fpNpvIso5[ipv][offset]->fill(iso, fCandM); else cout << "missing fpNpvIso5" << endl;

  }
} 

// ----------------------------------------------------------------------
void candAna::basicCuts() {
  cout << "    candAna basic cuts" << endl;
  fAnaCuts.addCut("fWideMass", "m(B candidate) [GeV]", fWideMass); 
  fAnaCuts.addCut("fGoodHLT", "HLT", fGoodHLT); 
  fAnaCuts.addCut("fGoodMuonsID", "lepton ID", fGoodMuonsID); 
  fAnaCuts.addCut("fGoodMuonsPt", "p_{T,#mu} [GeV]", fGoodMuonsPt); 
  fAnaCuts.addCut("fGoodMuonsEta", "#eta_{#mu}", fGoodMuonsEta); 
  fAnaCuts.addCut("fGoodTracks", "good tracks", fGoodTracks); 
  fAnaCuts.addCut("fGoodTracksPt", "p_{T,trk} [GeV]", fGoodTracksPt); 
  fAnaCuts.addCut("fGoodTracksEta", "#eta_{trk} ", fGoodTracksEta); 
}


// ----------------------------------------------------------------------
void candAna::moreBasicCuts() {
  cout << "    candAna more basic cuts?" << endl;

}


// ----------------------------------------------------------------------
void candAna::candidateCuts() {
  cout << "    candAna candidate cuts" << endl;
  fAnaCuts.addCut("fGoodQ", "q_{1} 1_{2}", fGoodQ);   
  fAnaCuts.addCut("fGoodPvAveW8", "<w8>", fGoodPvAveW8); 
  fAnaCuts.addCut("fGoodPvLip", "LIP(PV)", fGoodPvLip); 
  fAnaCuts.addCut("fGoodPvLipS", "LIPS(PV)", fGoodPvLipS); 
  fAnaCuts.addCut("fGoodPvLip2", "LIP2(PV)", fGoodPvLip2); 
  fAnaCuts.addCut("fGoodPvLipS2", "LIPS2(PV)", fGoodPvLipS2); 
  fAnaCuts.addCut("fGoodIp", "IP", fGoodIp); 
  fAnaCuts.addCut("fGoodIpS", "IPS", fGoodIpS); 
  fAnaCuts.addCut("fGoodMaxDoca", "MAXDOCA", fGoodMaxDoca); 
  fAnaCuts.addCut("fGoodPt", "p_{T,B}", fGoodPt); 
  fAnaCuts.addCut("fGoodEta", "#eta_{B}", fGoodEta); 
  fAnaCuts.addCut("fGoodAlpha", "#alpha", fGoodAlpha); 
  fAnaCuts.addCut("fGoodFLS", "l/#sigma(l)", fGoodFLS); 
  fAnaCuts.addCut("fGoodChi2", "#chi^{2}", fGoodChi2); 
  fAnaCuts.addCut("fGoodIso", "I_{trk}", fGoodIso); 
  fAnaCuts.addCut("fGoodCloseTrack", "close track veto", fGoodCloseTrack); 
  fAnaCuts.addCut("fGoodDocaTrk", "d_{ca}(trk)", fGoodDocaTrk); 
  fAnaCuts.addCut("fGoodLastCut", "lastCut", fGoodLastCut); 
}


// ----------------------------------------------------------------------
void candAna::moreCandidateCuts() {
  cout << "    candAna more candidate cuts?" << endl;

}


// ---------------------------------------------------------------------- 
AnalysisDistribution* candAna::bookDistribution(const char *hn, const char *ht, const char *hc, int nbins, double lo, double hi) {
  AnalysisDistribution *p = new AnalysisDistribution(hn, ht, nbins, lo, hi); 
  p->setSigWindow(SIGBOXMIN, SIGBOXMAX); 
  p->setBg1Window(BGLBOXMIN, BGLBOXMAX); 
  p->setBg2Window(BGHBOXMIN, BGHBOXMAX); 
  p->setAnalysisCuts(&fAnaCuts, hc); 
  p->setPreselCut(&fPreselection); 

  TH1 *h = (TH1D*)fHistDir->Get("analysisDistributions"); 
  for (int i = 1; i < h->GetNbinsX(); ++i) {
    if (!strcmp(h->GetXaxis()->GetBinLabel(i), "")) {
      // cout << "adding at bin " << i << " the label " << hn << endl;
      h->GetXaxis()->SetBinLabel(i, hn);
      break;
    }
  }
  return p; 
}



// ----------------------------------------------------------------------
bool candAna::goodTrack(TAnaTrack *pt) {

  if (TRACKQUALITY > 0 && (0 == (pt->fTrackQuality & TRACKQUALITY))) {
    if (fVerbose > 5) cout << "track " << pt->fIndex << " failed track quality: " << pt->fTrackQuality << endl;
    return false; 
  }
  
  if (TMath::Abs(pt->fTip) > TRACKTIP) {
    if (fVerbose > 5) cout << "track " << pt->fIndex << " failed tip: " << pt->fTip
			   << " pointing to PV = "  << pt->fPvIdx  << endl;
    return false; 
  }
  
  if (TMath::Abs(pt->fLip) > TRACKLIP) { 
    if (fVerbose > 5) cout << "track " << pt->fIndex << " failed lip: " << pt->fLip 
			   << " pointing to PV = "  << pt->fPvIdx  << endl;          
    return false; 
  }

  return true; 
}


// ----------------------------------------------------------------------
void candAna::genMatch() {
  cout << "candAna::genMatch()  wrong function" << endl;
}


// ----------------------------------------------------------------------
void candAna::recoMatch() {
  cout << "candAna::recoMatch()  wrong function" << endl;
}


// ----------------------------------------------------------------------
void candAna::candMatch() {
  cout << "candAna::candMatch()  wrong function" << endl;
}


// ----------------------------------------------------------------------
void candAna::efficiencyCalculation() {
  cout << "candAna::efficiencyCalculation()  wrong function" << endl;
}

// ----------------------------------------------------------------------
void candAna::triggerSelection() {

  fGoodHLT = false; 
  TString a; 
  int ps(0); 
  bool result(false), wasRun(false), error(false); 
  //  cout << " ----------------------------------------------------------------------" << endl;

  if (1) {
    TTrgObj *pM1(0), *pM2(0), *pM(0); 
    TTrgObj *p; 
    
    for (int i = 0; i < fpEvt->nTrgObj(); ++i) {
      p = fpEvt->getTrgObj(i); 
      //      cout << p->fLabel << endl;
      //      if (!p->fLabel.CompareTo("hltL1sL1DoubleMu33HighQ:HLT::")) cout << "= " << p->fLabel << endl;
      if (!p->fLabel.CompareTo("hltL1sL1DoubleMuOpen:HLT::")) {
        if (0 == pM1) {
          pM1 = p; 
        } else {
          pM2 = p; 
        }
        if (0) cout << p->fLabel << " pT = " << p->fP.Perp() << " eta = " << p->fP.Eta() << " phi = " << p->fP.Phi() <<  endl;
      }
      
      if (1 && !p->fLabel.CompareTo("hltL1sL1DoubleMu33HighQ:HLT::")) {
        //cout << p->fLabel << " pT = " << p->fP.Perp() << " eta = " << p->fP.Eta() << " phi = " << p->fP.Phi() <<  endl;
        ((TH1D*)fHistDir->Get("L1_0"))->Fill(p->fP.Eta());
      }
      
      if (1 && !p->fLabel.CompareTo("hltL1sL1DoubleMu0or33HighQ:HLT::")) {
        //      cout << p->fLabel << " pT = " << p->fP.Perp() << " eta = " << p->fP.Eta() << " phi = " << p->fP.Phi() <<  endl;
        ((TH1D*)fHistDir->Get("L1_1"))->Fill(p->fP.Eta());
      }

      if (1 && !p->fLabel.CompareTo("hltDimuon33L1Filtered0:HLT::")) {
        //      cout << p->fLabel << " pT = " << p->fP.Perp() << " eta = " << p->fP.Eta() << " phi = " << p->fP.Phi() <<  endl;
        ((TH1D*)fHistDir->Get("L1_2"))->Fill(p->fP.Eta());
      }

      if (1 && !p->fLabel.CompareTo("hltDimuonL2PreFiltered0:HLT::")) {
        //      cout << p->fLabel << " pT = " << p->fP.Perp() << " eta = " << p->fP.Eta() << " phi = " << p->fP.Phi() <<  endl;
        ((TH1D*)fHistDir->Get("L1_3"))->Fill(p->fP.Eta());
      }

      if (1 && !p->fLabel.CompareTo("hltDimuonL2PreFiltered0:HLT::")) {
        //      cout << p->fLabel << " pT = " << p->fP.Perp() << " eta = " << p->fP.Eta() << " phi = " << p->fP.Phi() <<  endl;
        ((TH1D*)fHistDir->Get("L1_4"))->Fill(p->fP.Eta());
      }

      if (1 && !p->fLabel.CompareTo("hltDoubleDisplacedMu4L3PreFiltered:HLT::")) {
        //      cout << p->fLabel << " pT = " << p->fP.Perp() << " eta = " << p->fP.Eta() << " phi = " << p->fP.Phi() <<  endl;
        ((TH1D*)fHistDir->Get("L1_5"))->Fill(p->fP.Eta());
      }

      if (1 && !p->fLabel.CompareTo("hltDoubleMu3p5LowMassDisplacedL3Filtered:HLT::")) {
        //      cout << p->fLabel << " pT = " << p->fP.Perp() << " eta = " << p->fP.Eta() << " phi = " << p->fP.Phi() <<  endl;
        ((TH1D*)fHistDir->Get("L1_6"))->Fill(p->fP.Eta());
      }
    }
    
  }


  if (HLTRANGE.begin()->first == "NOTRIGGER") {
    //    cout << "NOTRIGGER requested... " << endl;
    fGoodHLT = true; 
    return;
  }

  if (fVerbose == -33) {
    cout << "--------------------" << endl;
    for (int i = 0; i < NL1T; ++i) {
      result = wasRun = error = false;
      a = fpEvt->fL1TNames[i]; 
      ps = fpEvt->fL1TPrescale[i]; 
      result = fpEvt->fL1TResult[i]; 
      error  = fpEvt->fL1TMask[i]; 
      if (a.Contains("Mu")) {
	cout << a <<  " mask: " << error << " result: " << result << " ps: " << ps << endl;
      }
    }
  }
  
  for (int i = 0; i < NHLT; ++i) {
    result = wasRun = error = false;
    a = fpEvt->fHLTNames[i]; 
    ps = fpEvt->fHLTPrescale[i]; 
    wasRun = fpEvt->fHLTWasRun[i]; 
    result = fpEvt->fHLTResult[i]; 
    error  = fpEvt->fHLTError[i]; 
    
    //    if (-32 == fVerbose) cout << "path " << i << ": " << a << endl;
    if (wasRun && result) {
      if (-32 == fVerbose) {
	if ((a != "digitisation_step") 
	    && (a != "L1simulation_step") 
	    && (a != "digi2raw_step") 
	    && (a != "HLTriggerFinalPath") 
	    && (a != "raw2digi_step") 
	    && (a != "reconstruction_step") 
	    ) {
	  cout << "run and fired: " << a << endl;
	}
      }

      string spath; 
      int rmin, rmax; 
      for (map<string, pair<int, int> >::iterator imap = HLTRANGE.begin(); imap != HLTRANGE.end(); ++imap) {  
	spath = imap->first; 
	rmin = imap->second.first; 
	rmax = imap->second.second; 
	// 	if ((a != "digitisation_step") 
	// 	    && (a != "L1simulation_step") 
	// 	    && (a != "digi2raw_step") 
	// 	    && (a != "HLTriggerFinalPath") 
	// 	    && (a != "raw2digi_step") 
	// 	    && (a != "reconstruction_step") 
	// 	    ) {
	// 	  cout << "path: " << a << ", comparing to " << spath << " for run range " << rmin << " .. " << rmax << endl;
	// 	}
	if (!a.CompareTo(imap->first.c_str())) {
 	  fGoodHLT = true; 
	  fHLTPath = a.Data();
	  if (fVerbose > 0) cout << "exact match: " << imap->first.c_str() << " HLT: " << a << " result: " << result << endl;
	}

 	if (a.Contains(spath.c_str()) && (rmin <= fRun) && (fRun <= rmax)) {
	  fHLTPath = a.Data();
 	  fGoodHLT = true; 
	  if (fVerbose > 0) cout << "close match: " << imap->first.c_str() << " HLT: " << a 
				 << " result: " << result 
				 << " in run " << fRun 
				 << endl;
 	}
      }
    }      
  }


}



// ----------------------------------------------------------------------
void candAna::bookHist() {

  fHistDir->cd();

  TH1D *h11 = new TH1D("L1_0", "hltL1sL1DoubleMu33HighQ", 50, -2.5, 2.5); 
  h11 = new TH1D("L1_1", "hltL1sL1DoubleMu0or33HighQ", 50, -2.5, 2.5); 
  h11 = new TH1D("L1_2", "hltDimuon33L1Filtered0", 50, -2.5, 2.5); 
  h11 = new TH1D("L1_3", "hltDimuon33L2PreFiltered0", 50, -2.5, 2.5); 
  h11 = new TH1D("L1_4", "hltDimuonL2PreFiltered0", 50, -2.5, 2.5); 
  h11 = new TH1D("L1_5", "hltDoubleDisplacedMu4L3PreFiltered", 50, -2.5, 2.5); 
  h11 = new TH1D("L1_6", "hltDoubleMu3p5LowMassDisplacedL3Filtered", 50, -2.5, 2.5); 

  // -- Reduced Tree
  fTree = new TTree("events", "events");
  fTree->Branch("run",     &fRun,               "run/L");
  fTree->Branch("json",    &fJSON,              "json/O");
  fTree->Branch("evt",     &fEvt,               "evt/L");
  fTree->Branch("ls",      &fLS,                "ls/I");
  fTree->Branch("tm",      &fCandTM,            "tm/I");
  fTree->Branch("pr",      &fGenBpartial,       "pr/I"); 
  fTree->Branch("procid",  &fProcessType,       "procid/I");
  fTree->Branch("hlt",     &fGoodHLT,           "hlt/O");
  fTree->Branch("pvn",     &fPvN,               "pvn/I");
  fTree->Branch("cb",      &fCowboy,            "cb/O");
  fTree->Branch("rr",      &fRunRange,          "rr/I");
  fTree->Branch("bdt",     &fBDT,               "bdt/D");
  fTree->Branch("npv",     &fPvN,               "npv/I");
  fTree->Branch("pvw8",    &fPvAveW8,           "pvw8/D");

  // -- global cuts and weights
  fTree->Branch("gmuid",   &fGoodMuonsID,       "gmuid/O");
  fTree->Branch("gmupt",   &fGoodMuonsPt,       "gmupt/O");
  fTree->Branch("gmueta",  &fGoodMuonsEta,      "gmueta/O");
  fTree->Branch("gtqual",  &fGoodTracks,        "gtqual/O");
  fTree->Branch("gtpt",    &fGoodTracksPt,      "gtpt/O");
  fTree->Branch("gteta",   &fGoodTracksEta,     "gteta/O");
  fTree->Branch("w8mu",    &fCandW8Mu,          "w8mu/D");
  fTree->Branch("w8tr",    &fCandW8Tr,          "w8tr/D");

  // -- PV
  fTree->Branch("pvlip",    &fCandPvLip,        "pvlip/D");
  fTree->Branch("pvlips",   &fCandPvLipS,       "pvlips/D");
  fTree->Branch("pvlip2",   &fCandPvLip2,       "pvlip2/D");
  fTree->Branch("pvlips2",  &fCandPvLipS2,      "pvlips2/D");
  fTree->Branch("pvip",     &fCandPvIp,         "pvip/D");
  fTree->Branch("pvips",    &fCandPvIpS,        "pvips/D");

  // -- cand
  fTree->Branch("q",       &fCandQ,             "q/I");
  fTree->Branch("type",    &fCandType,          "type/I");
  fTree->Branch("pt",      &fCandPt,            "pt/D");
  fTree->Branch("eta",     &fCandEta,           "eta/D");
  fTree->Branch("phi",     &fCandPhi,           "phi/D");
  fTree->Branch("tau",     &fCandTau,           "tau/D");
  fTree->Branch("m",       &fCandM,             "m/D");
  fTree->Branch("cm",      &fCandM2,            "cm/D");
  fTree->Branch("cosa",    &fCandCosA,          "cosa/D");
  fTree->Branch("alpha",   &fCandA,             "alpha/D");
  fTree->Branch("iso",     &fCandIso,           "iso/D");
  fTree->Branch("isotrk",  &fCandIsoTrk,        "isotrk/I");
  fTree->Branch("closetrk",&fCandCloseTrk,      "closetrk/I");
  fTree->Branch("chi2",    &fCandChi2,          "chi2/D");
  fTree->Branch("dof",     &fCandDof,           "dof/D");
  fTree->Branch("prob",    &fCandProb,          "prob/D");
  fTree->Branch("fls3d",   &fCandFLS3d,         "fls3d/D");
  fTree->Branch("fl3d",    &fCandFL3d,          "fl3d/D");
  fTree->Branch("flxy",    &fCandFLxy,          "flxy/D");
  fTree->Branch("fl3dE",   &fCandFL3dE,         "fl3dE/D");
  fTree->Branch("flsxy",   &fCandFLSxy,         "flsxy/D");
  fTree->Branch("docatrk", &fCandDocaTrk,       "docatrk/D");  
  fTree->Branch("docatrkbdt", &fCandDocaTrkBdt, "docatrkbdt/D");  
  fTree->Branch("maxdoca", &fCandDoca,          "maxdoca/D");
  fTree->Branch("lip",     &fCandPvLip,         "lip/D");
  fTree->Branch("lipE",    &fCandPvLipE,        "lipE/D");
  fTree->Branch("tip",     &fCandPvTip,         "tip/D");
  fTree->Branch("tipE",    &fCandPvTipE,        "tipE/D");
  fTree->Branch("osiso",   &fOsIso,             "osiso/D");
  fTree->Branch("osreliso",&fOsRelIso,          "osreliso/D");
  fTree->Branch("osmpt",   &fOsMuonPt,          "osmpt/D");
  fTree->Branch("osmptrel",&fOsMuonPtRel,       "osmptrel/D");
  fTree->Branch("osmdr",   &fOsMuonDeltaR,      "osmdr/D");

  // -- muons
  fTree->Branch("m1q",     &fMu1Q,              "m1q/I");
  fTree->Branch("m1id",    &fMu1Id,             "m1id/O");
  fTree->Branch("m1pt",    &fMu1Pt,             "m1pt/D");
  fTree->Branch("m1eta",   &fMu1Eta,            "m1eta/D");
  fTree->Branch("m1phi",   &fMu1Phi,            "m1phi/D");
  fTree->Branch("m1ip",    &fMu1IP,             "m1ip/D");
  fTree->Branch("m1gt",    &fMu1TkQuality,      "m1gt/I");
  fTree->Branch("m1pix",   &fMu1Pix,            "m1pix/I");
  fTree->Branch("m1bpix",  &fMu1BPix,           "m1bpix/I");
  fTree->Branch("m1bpixl1",&fMu1BPixL1,         "m1bpixl1/I");
  fTree->Branch("m1chi2",  &fMu1Chi2,           "m1chi2/D");
  fTree->Branch("m1pv",    &fMu1PV,             "m1pv/I");
  fTree->Branch("m2q",     &fMu2Q,              "m2q/I");
  fTree->Branch("m2id",    &fMu2Id,             "m2id/O");
  fTree->Branch("m2pt",    &fMu2Pt,             "m2pt/D");
  fTree->Branch("m2eta",   &fMu2Eta,            "m2eta/D");
  fTree->Branch("m2phi",   &fMu2Phi,            "m2phi/D");
  fTree->Branch("m2ip",    &fMu2IP,             "m2ip/D");
  fTree->Branch("m2gt",    &fMu2TkQuality,      "m2gt/I");
  fTree->Branch("m2pix",   &fMu2Pix,            "m2pix/I");
  fTree->Branch("m2bpix",  &fMu2BPix,           "m2bpix/I");
  fTree->Branch("m2bpixl1",&fMu2BPixL1,         "m2bpixl1/I");
  fTree->Branch("m2chi2",  &fMu2Chi2,           "m2chi2/D");
  fTree->Branch("m2pv",    &fMu2PV,             "m2pv/I");

  fTree->Branch("mudist",  &fMuDist,            "mudist/D");
  fTree->Branch("mudeltar",&fMuDeltaR,          "mudeltar/D");

  fTree->Branch("g1pt",    &fMu1PtGen,          "g1pt/D");
  fTree->Branch("g2pt",    &fMu2PtGen,          "g2pt/D");
  fTree->Branch("g1eta",   &fMu1EtaGen,         "g1eta/D");
  fTree->Branch("g2eta",   &fMu2EtaGen,         "g2eta/D");
  fTree->Branch("gtau",    &fGenLifeTime,       "gtau/D");

  fTree->Branch("t1pt",    &fMu1PtNrf,          "t1pt/D");
  fTree->Branch("t1eta",   &fMu1EtaNrf,         "t1eta/D");
  fTree->Branch("t2pt",    &fMu2PtNrf,          "t2pt/D");
  fTree->Branch("t2eta",   &fMu2EtaNrf,         "t2eta/D");

  fTree->Branch("hm1pt",  &fHltMu1Pt,  "hm1pt/D");    
  fTree->Branch("hm1eta", &fHltMu1Eta, "hm1eta/D");  
  fTree->Branch("hm1phi", &fHltMu1Phi, "hm1phi/D");  
  fTree->Branch("hm2pt",  &fHltMu2Pt,  "hm2pt/D");    
  fTree->Branch("hm2eta", &fHltMu2Eta, "hm2eta/D");  
  fTree->Branch("hm2phi", &fHltMu2Phi, "hm2phi/D");  

  // -- Efficiency/Acceptance Tree
  fEffTree = new TTree("effTree", "effTree");
  fEffTree->Branch("run",    &fRun,               "run/L");
  fEffTree->Branch("evt",    &fEvt,               "evt/L");
  fEffTree->Branch("hlt",    &fGoodHLT,           "hlt/O");
  fEffTree->Branch("procid", &fProcessType,       "procid/I");
  fEffTree->Branch("bidx",   &fGenBTmi,           "bidx/I");

  fEffTree->Branch("gpt",    &fETgpt,             "gpt/F");
  fEffTree->Branch("geta",   &fETgeta,            "geta/F");

  fEffTree->Branch("m1pt",   &fETm1pt,            "m1pt/F");
  fEffTree->Branch("g1pt",   &fETg1pt,            "g1pt/F");
  fEffTree->Branch("m1eta",  &fETm1eta,           "m1eta/F");
  fEffTree->Branch("g1eta",  &fETg1eta,           "g1eta/F");
  fEffTree->Branch("m1q",    &fETm1q,             "m1q/I");
  fEffTree->Branch("m1gt",   &fETm1gt,            "m1gt/O");
  fEffTree->Branch("m1id",   &fETm1id,            "m1id/O");

  fEffTree->Branch("m2pt",   &fETm2pt,            "m2pt/F");
  fEffTree->Branch("g2pt",   &fETg2pt,            "g2pt/F");
  fEffTree->Branch("m2eta",  &fETm2eta,           "m2eta/F");
  fEffTree->Branch("g2eta",  &fETg2eta,           "g2eta/F");
  fEffTree->Branch("m2q",    &fETm2q,             "m2q/I");
  fEffTree->Branch("m2gt",   &fETm2gt,            "m2gt/O");
  fEffTree->Branch("m2id",   &fETm2id,            "m2id/O");

  fEffTree->Branch("m",      &fETcandMass,        "m/F");


  // -- Analysis distributions
  TH1D *h = new TH1D("analysisDistributions", "analysisDistributions", 10000, 0., 10000.); 
  h = 0; 

  TDirectory *pD;
  string name, dname; 
  int i(0); 
  for (map<string, int>::iterator imap = fRegion.begin(); imap != fRegion.end(); ++imap) {  
    i    = imap->second; 
    name = imap->first + "_";

    //    cout << "  booking analysis distributions for " << name << " at offset i = " << i << endl;

    fpHLT[i]       = bookDistribution(Form("%shlt", name.c_str()), "hlt", "fGoodHLT", 10, 0., 10.);           
    fpPvZ[i]       = bookDistribution(Form("%spvz", name.c_str()), "z_{PV} [cm]", "fGoodHLT", 40, -20., 20.);           
    fpPvN[i]       = bookDistribution(Form("%spvn", name.c_str()), "N(PV) ", "fGoodHLT", 50, 0., 50.);           
    fpPvNtrk[i]    = bookDistribution(Form("%spvntrk", name.c_str()), "N_{trk}^{PV} ", "fGoodHLT", 20, 0., 200.);           
    fpPvAveW8[i]   = bookDistribution(Form("%spvavew8", name.c_str()), "<w^{PV}>", "fGoodPvAveW8", 25, 0.5, 1.);           

    fpTracksPt[i]  = bookDistribution(Form("%strackspt", name.c_str()), "p_{T} [GeV]", "fGoodTracksPt", 25, 0., 25.);
    fpTracksEta[i] = bookDistribution(Form("%strackseta", name.c_str()), "#eta_{T}", "fGoodTracksEta", 25, -2.5, 2.5);
    fpTracksQual[i]= bookDistribution(Form("%stracksqual", name.c_str()), "track quality", "fGoodTracks", 10, 0., 10.); 
    fpMuonsID[i]   = bookDistribution(Form("%smuonsid", name.c_str()), "muon ID", "fGoodMuonsID", 10, 0., 10.); 
    fpMuonsPt[i]   = bookDistribution(Form("%smuonspt", name.c_str()), "p_{T, #mu} [GeV]", "fGoodMuonsPt", 25, 0., 25.); 
    fpMuon1Pt[i]   = bookDistribution(Form("%smuon1pt", name.c_str()), "p_{T, #mu1} [GeV]", "fGoodMuonsPt", 30, 0., 30.); 
    fpMuon2Pt[i]   = bookDistribution(Form("%smuon2pt", name.c_str()), "p_{T, #mu2} [GeV]", "fGoodMuonsPt", 40, 0., 20.); 
    fpMuonsEta[i]  = bookDistribution(Form("%smuonseta", name.c_str()), "#eta_{#mu}", "fGoodMuonsEta", 20, -2.5, 2.5); 
    fpMuon1Eta[i]  = bookDistribution(Form("%smuon1eta", name.c_str()), "#eta_{#mu1}", "fGoodMuonsEta", 20, -2.5, 2.5); 
    fpMuon2Eta[i]  = bookDistribution(Form("%smuon2eta", name.c_str()), "#eta_{#mu2}", "fGoodMuonsEta", 20, -2.5, 2.5); 
    fpPt[i]        = bookDistribution(Form("%spt", name.c_str()), "p_{T}(B) [GeV]", "fGoodPt", 30, 0., 60.); 
    fpP[i]         = bookDistribution(Form("%sp", name.c_str()), "p(B) [GeV]", "fGoodPt", 50, 0., 100.); 
    fpEta[i]       = bookDistribution(Form("%seta", name.c_str()), "#eta(B)", "fGoodEta", 20, -2.5, 2.5); 
    fpCosA[i]      = bookDistribution(Form("%scosa", name.c_str()), "cos(#alpha_{3D})", "fGoodAlpha", 30, 0.97, 1.); 
    fpAlpha[i]     = bookDistribution(Form("%salpha", name.c_str()), "#alpha_{3D}", "fGoodAlpha", 40, 0., 0.1); 
    fpIso[i]       = bookDistribution(Form("%siso", name.c_str()),  "isolation", "fGoodIso", 52, 0., 1.04); 
    fpIsoTrk[i]    = bookDistribution(Form("%sisotrk", name.c_str()),  "N_{trk}^{I}", "fGoodIso", 20, 0., 20.); 
    fpCloseTrk[i]  = bookDistribution(Form("%sclosetrk", name.c_str()),  "N_{trk}^{close}", "fGoodCloseTrack", 10, 0., 10.); 
    
    fpOsIso[i]        = bookDistribution(Form("%sosiso", name.c_str()),  "F_{OS} [GeV]", "fGoodIso", 100, 0., 100.);              
    fpOsRelIso[i]     = bookDistribution(Form("%sosreliso", name.c_str()),  "I_{OS}^{rel}", "fGoodIso", 60, 0., 6.);              
    fpOsMuonPt[i]     = bookDistribution(Form("%sosmuonpt", name.c_str()),  "p_{T,#mu}^{OS} [GeV]", "fGoodMuonsPt", 40, 0., 20.);              
    fpOsMuonPtRel[i]  = bookDistribution(Form("%sosmuonptrel", name.c_str()),  "p_{T,#mu}^{OS, rel} [GeV]", "fGoodMuonsPt", 40, 0., 20.); 
    fpOsMuonDeltaR[i] = bookDistribution(Form("%sosmuondr", name.c_str()),  "#Delta R(C, #mu^{OS})", "fGoodMuonsPt", 60, 0., 6.);              

    // -- mode dependent plots
    fpPtGGF[i]        = bookDistribution(Form("%sptggf", name.c_str()), "p_{T}(B) [GeV]", "fGoodPt", 30, 0., 60.); 
    fpPtGSP[i]        = bookDistribution(Form("%sptgsp", name.c_str()), "p_{T}(B) [GeV]", "fGoodPt", 30, 0., 60.); 
    fpPtFEX[i]        = bookDistribution(Form("%sptfex", name.c_str()), "p_{T}(B) [GeV]", "fGoodPt", 30, 0., 60.); 

    fpIsoGGF[i]       = bookDistribution(Form("%sisoggf", name.c_str()),  "isolation", "fGoodIso", 52, 0., 1.04); 
    fpIsoGSP[i]       = bookDistribution(Form("%sisogsp", name.c_str()),  "isolation", "fGoodIso", 52, 0., 1.04); 
    fpIsoFEX[i]       = bookDistribution(Form("%sisofex", name.c_str()),  "isolation", "fGoodIso", 52, 0., 1.04); 

    fpCloseTrkGGF[i]  = bookDistribution(Form("%sclosetrkggf", name.c_str()),  "N_{trk}^{close}", "fGoodCloseTrack", 10, 0., 10.); 
    fpCloseTrkGSP[i]  = bookDistribution(Form("%sclosetrkgsp", name.c_str()),  "N_{trk}^{close}", "fGoodCloseTrack", 10, 0., 10.); 
    fpCloseTrkFEX[i]  = bookDistribution(Form("%sclosetrkfex", name.c_str()),  "N_{trk}^{close}", "fGoodCloseTrack", 10, 0., 10.); 

    fpDocaTrkGGF[i]   = bookDistribution(Form("%sdocatrkggf", name.c_str()), "d_{ca}^{0} [cm]", "fGoodDocaTrk", 40, 0., 0.20);   
    fpDocaTrkGSP[i]   = bookDistribution(Form("%sdocatrkgsp", name.c_str()), "d_{ca}^{0} [cm]", "fGoodDocaTrk", 40, 0., 0.20);   
    fpDocaTrkFEX[i]   = bookDistribution(Form("%sdocatrkfex", name.c_str()), "d_{ca}^{0} [cm]", "fGoodDocaTrk", 40, 0., 0.20);   

    fpOsIsoGGF[i]     = bookDistribution(Form("%sosisoggf", name.c_str()),  "F_{OS} [GeV]", "fGoodIso", 100, 0., 100.);              
    fpOsIsoGSP[i]     = bookDistribution(Form("%sosisogsp", name.c_str()),  "F_{OS} [GeV]", "fGoodIso", 100, 0., 100.);              
    fpOsIsoFEX[i]     = bookDistribution(Form("%sosisofex", name.c_str()),  "F_{OS} [GeV]", "fGoodIso", 100, 0., 100.);              
    
    fpOsRelIsoGGF[i]  = bookDistribution(Form("%sosrelisoggf", name.c_str()),  "I_{OS}^{rel}", "fGoodIso", 60, 0., 6.);              
    fpOsRelIsoGSP[i]  = bookDistribution(Form("%sosrelisogsp", name.c_str()),  "I_{OS}^{rel}", "fGoodIso", 60, 0., 6.);              
    fpOsRelIsoFEX[i]  = bookDistribution(Form("%sosrelisofex", name.c_str()),  "I_{OS}^{rel}", "fGoodIso", 60, 0., 6.);              

    fpOsMuonPtGGF[i]  = bookDistribution(Form("%sosmuonptggf", name.c_str()),  "p_{T,#mu}^{OS} [GeV]", "fGoodMuonsPt", 40, 0., 20.);
    fpOsMuonPtGSP[i]  = bookDistribution(Form("%sosmuonptgsp", name.c_str()),  "p_{T,#mu}^{OS} [GeV]", "fGoodMuonsPt", 40, 0., 20.);
    fpOsMuonPtFEX[i]  = bookDistribution(Form("%sosmuonptfex", name.c_str()),  "p_{T,#mu}^{OS} [GeV]", "fGoodMuonsPt", 40, 0., 20.);

    fpOsMuonPtRelGGF[i]  = bookDistribution(Form("%sosmuonptrelggf", name.c_str()),  "p_{T,#mu}^{OS, rel} [GeV]", "fGoodMuonsPt", 40, 0., 20.);
    fpOsMuonPtRelGSP[i]  = bookDistribution(Form("%sosmuonptrelgsp", name.c_str()),  "p_{T,#mu}^{OS, rel} [GeV]", "fGoodMuonsPt", 40, 0., 20.);
    fpOsMuonPtRelFEX[i]  = bookDistribution(Form("%sosmuonptrelfex", name.c_str()),  "p_{T,#mu}^{OS, rel} [GeV]", "fGoodMuonsPt", 40, 0., 20.);

    fpOsMuonDeltaRGGF[i] = bookDistribution(Form("%sosmuondrggf", name.c_str()),  "#Delta R(C, #mu^{OS})", "fGoodMuonsPt", 60, 0., 6.);
    fpOsMuonDeltaRGSP[i] = bookDistribution(Form("%sosmuondrgsp", name.c_str()),  "#Delta R(C, #mu^{OS})", "fGoodMuonsPt", 60, 0., 6.);
    fpOsMuonDeltaRFEX[i] = bookDistribution(Form("%sosmuondrfex", name.c_str()),  "#Delta R(C, #mu^{OS})", "fGoodMuonsPt", 60, 0., 6.);

    fpChi2[i]      = bookDistribution(Form("%schi2", name.c_str()),  "#chi^{2}", "fGoodChi2", 30, 0., 30.);              
    fpChi2Dof[i]   = bookDistribution(Form("%schi2dof", name.c_str()),  "#chi^{2}/dof", "fGoodChi2", 40, 0., 4.);
    fpProb[i]      = bookDistribution(Form("%spchi2dof", name.c_str()),  "P(#chi^{2},dof)", "fGoodChi2", 26, 0., 1.04);    
    fpFLS3d[i]     = bookDistribution(Form("%sfls3d", name.c_str()), "l_{3D}/#sigma(l_{3D})", "fGoodFLS", 30, 0., 120.);  
    fpFL3d[i]      = bookDistribution(Form("%sfl3d", name.c_str()),  "l_{3D} [cm]", "fGoodFLS", 30, 0., 3.);  
    fpFL3dE[i]     = bookDistribution(Form("%sfl3de", name.c_str()), "#sigma(l_{3D}) [cm]", "fGoodFLS", 25, 0., 0.1);  
    fpFLSxy[i]     = bookDistribution(Form("%sflsxy", name.c_str()), "l_{xy}/#sigma(l_{xy})", "fGoodFLS", 30, 0., 120.);  
    fpDocaTrk[i]   = bookDistribution(Form("%sdocatrk", name.c_str()), "d_{ca}^{0} [cm]", "fGoodDocaTrk", 40, 0., 0.20);   
    fpBDT[i]       = bookDistribution(Form("%sbdt", name.c_str()), "BDT", "fGoodHLT", 40, -1.0, 1.0);   

    fpLip[i]       = bookDistribution(Form("%slip", name.c_str()), "l_{z} [cm]", "fGoodPvLip", 32, -0.02, 0.02);   
    fpLipE[i]      = bookDistribution(Form("%slipe", name.c_str()), "#sigma(l_{z}) [cm]", "fGoodPvLip", 50, 0., 0.05);   
    fpLipS[i]      = bookDistribution(Form("%slips", name.c_str()), "l_{z}/#sigma(l_{z})", "fGoodPvLipS", 40, -5., 5.);   

    fpTip[i]       = bookDistribution(Form("%stip", name.c_str()), "l_{T} [cm]", "fGoodPvLip", 50, 0., 0.5);   
    fpTipE[i]      = bookDistribution(Form("%stipe", name.c_str()), "#sigma(l_{T}) [cm]", "fGoodPvLip", 50, 0., 0.05);   
    fpTipS[i]      = bookDistribution(Form("%stips", name.c_str()), "l_{T}/#sigma(l_{T})", "fGoodPvLipS", 25, 0., 20.);   

    fpLip12[i]     = bookDistribution(Form("%slip12", name.c_str()), "ratio l_{z} ", "fGoodPvLip", 50, -0.5, 0.5);   
    fpLipE12[i]    = bookDistribution(Form("%slipe12", name.c_str()), "ratio #sigma(l_{z})", "fGoodPvLip", 50, 0., 2.);   
    fpLipS12[i]    = bookDistribution(Form("%slips12", name.c_str()), "ratio l_{z}/#sigma(l_{z})", "fGoodPvLipS", 50, -1., 1.);   

    fpLip2[i]      = bookDistribution(Form("%slip2", name.c_str()), "l_{z}^{2nd} [cm]", "fGoodPvLip2", 100, -1.0, 1.0);   
    fpLipS2[i]     = bookDistribution(Form("%slips2", name.c_str()), "l_{z}^{2nd}/#sigma(l_{z}^{2nd})", "fGoodPvLipS2", 100, -25., 25.);   

    fpMaxDoca[i]   = bookDistribution(Form("%smaxdoca", name.c_str()), "d^{max}", "fGoodMaxDoca", 60, 0., 0.03);   
    fpIp[i]        = bookDistribution(Form("%sip", name.c_str()), "#delta_{3D}", "fGoodIp", 50, 0., 0.05);   
    fpIpS[i]       = bookDistribution(Form("%sips", name.c_str()), "#delta_{3D}/#sigma(#delta_{3D})", "fGoodIpS", 50, 0., 5);   

    fp2MChi2[i]    = bookDistribution(Form("%s2mchi2", name.c_str()),  "#chi^{2}", "fGoodChi2", 30, 0., 30.);              
    fp2MChi2Dof[i] = bookDistribution(Form("%s2mchi2dof", name.c_str()),  "#chi^{2}/dof", "fGoodChi2", 30, 0., 3.);       
    fp2MProb[i]    = bookDistribution(Form("%s2mpchi2dof", name.c_str()),  "P(#chi^{2},dof)", "fGoodChi2", 26, 0., 1.04);    
    fp2MFLS3d[i]   = bookDistribution(Form("%s2mfls3d", name.c_str()), "l_{3D}/#sigma(l_{3D})", "fGoodFLS", 30, 0., 120.);  
    fp2MFL3d[i]    = bookDistribution(Form("%s2mfl3d", name.c_str()),  "l_{3D} [cm]", "fGoodFLS", 25, 0., 5.);  
    fp2MFL3dE[i]   = bookDistribution(Form("%s2mfl3de", name.c_str()), "#sigma(l_{3D}) [cm]", "fGoodFLS", 25, 0., 0.5);  
    fp2MFLSxy[i]   = bookDistribution(Form("%s2mflsxy", name.c_str()), "l_{xy}/#sigma(l_{xy})", "fGoodFLS", 30, 0., 120.);  

    if (name == "A_") {
      //      cout << "  booking NPV distributions for " << name << endl;

      // -- eta 'profiles'
      for (int ipv = 0; ipv < NADPV; ++ipv) {
	pD = fHistDir->mkdir(Form("%sEta%i", name.c_str(), ipv));
	pD->cd();
	dname = Form("%seta%i_", name.c_str(), ipv);
	fpEtaFLS3d[ipv][i] =  bookDistribution(Form("%sfls3d", dname.c_str()), "l_{3d}/#sigma(l_{3d})", "fGoodFLS", 25, 0., 100.);  
      }

      // -- NPV analysis distributions
      for (int ipv = 0; ipv < NADPV; ++ipv) {
	pD = fHistDir->mkdir(Form("%sNpv%i", name.c_str(), ipv));
	pD->cd();
	dname = Form("%snpv%i_", name.c_str(), ipv);
	fpNpvPvN[ipv][i]     = bookDistribution(Form("%spvn", dname.c_str()), "N(PV) ", "fGoodHLT", 60, 0., 60.);           
        fpNpvAveW8[ipv][i]   = bookDistribution(Form("%spvavew8", dname.c_str()),  "<w^{PV}>", "fGoodPvAveW8", 25, 0.5, 1.0); 
	fpNpvChi2Dof[ipv][i] = bookDistribution(Form("%schi2dof", dname.c_str()), "#chi^{2}/dof", "fGoodChi2", 30, 0., 3.);       
	fpNpvProb[ipv][i]    = bookDistribution(Form("%spchi2dof", dname.c_str()), "P(#chi^{2},dof)", "fGoodChi2", 25, 0., 1.);    
	fpNpvAlpha[ipv][i]   = bookDistribution(Form("%salpha", dname.c_str()),  "#alpha_{3D}", "fGoodAlpha", 30, 0., 0.15); 
	fpNpvFLS3d[ipv][i]   = bookDistribution(Form("%sfls3d", dname.c_str()), "l_{3d}/#sigma(l_{3d})", "fGoodFLS", 25, 0., 100.);  
	fpNpvFLSxy[ipv][i]   = bookDistribution(Form("%sflsxy", dname.c_str()), "l_{xy}/#sigma(l_{xy})", "fGoodFLS", 25, 0., 100.);  
	//	fpNpvDocaTrk[ipv][i] = bookDistribution(Form("%sdocatrk", dname.c_str()), "d_{ca}^{0} [cm]", "fGoodDocaTrk", 35, 0., 0.14);   
	fpNpvDocaTrk[ipv][i] = bookDistribution(Form("%sdocatrk", dname.c_str()), "d_{ca}^{0} [cm]", "fGoodDocaTrk", 100, 0., 0.20);   
	fpNpvIso[ipv][i]     = bookDistribution(Form("%siso", dname.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
	fpNpvIsoTrk[ipv][i]  = bookDistribution(Form("%sisotrk", dname.c_str()),  "N_{trk}^{I}", "fGoodIso", 20, 0., 20.); 
	fpNpvCloseTrk[ipv][i]= bookDistribution(Form("%sclosetrk", dname.c_str()),  "N_{trk}^{close}", "fGoodCloseTrack", 20, 0., 20.); 
	fpNpvLip[ipv][i]     = bookDistribution(Form("%slip", dname.c_str()),  "l_{z}", "fGoodPvLip", 25, -0.05, 0.05); 
	fpNpvLipS[ipv][i]    = bookDistribution(Form("%slips", dname.c_str()),  "l_{z}/#sigma(l_{z})", "fGoodPvLipS", 25, -10., 10.); 

	fpNpvLip2[ipv][i]    = bookDistribution(Form("%slip2", dname.c_str()),  "l_{z,2}", "fGoodPvLip2", 100, -1.0, 1.0); 
	fpNpvLipS2[ipv][i]   = bookDistribution(Form("%slips2", dname.c_str()),  "l_{z,2}/#sigma(l_{z,2})", "fGoodPvLipS2", 100, -25., 25.); 

	fpNpvMaxDoca[ipv][i] = bookDistribution(Form("%smaxdoca", dname.c_str()), "d^{max}", "fGoodMaxDoca", 60, 0., 0.03);   
	fpNpvIp[ipv][i]      = bookDistribution(Form("%sip", dname.c_str()), "#delta_{3D}", "fGoodIp", 50, 0., 0.1);   
	fpNpvIpS[ipv][i]     = bookDistribution(Form("%sips", dname.c_str()), "#delta_{3D}/#sigma(#delta_{3D})", "fGoodIpS", 50, 0., 10);   

	fpNpvIso0[ipv][i]     = bookDistribution(Form("%siso0", dname.c_str()),  "I0", "fGoodIso", 22, 0., 1.1); 
	fpNpvIso1[ipv][i]     = bookDistribution(Form("%siso1", dname.c_str()),  "I0", "fGoodIso", 22, 0., 1.1); 
	fpNpvIso2[ipv][i]     = bookDistribution(Form("%siso2", dname.c_str()),  "I0", "fGoodIso", 22, 0., 1.1); 
	fpNpvIso3[ipv][i]     = bookDistribution(Form("%siso3", dname.c_str()),  "I0", "fGoodIso", 22, 0., 1.1); 
	fpNpvIso4[ipv][i]     = bookDistribution(Form("%siso4", dname.c_str()),  "I0", "fGoodIso", 22, 0., 1.1); 
	fpNpvIso5[ipv][i]     = bookDistribution(Form("%siso5", dname.c_str()),  "I0", "fGoodIso", 22, 0., 1.1); 

	fHistDir->cd();	
      }
    } else {
      for (int ipv = 0; ipv < NADPV; ++ipv) {
	fpNpvPvN[ipv][i]     = 0; 
      }
    }
  }

  //  bookIsoPlots(); //FIXISOPLOTS

}

// ----------------------------------------------------------------------
void candAna::fillIsoPlots() {

  double iso(0.), doca(0.); 

  // -- make sure this is the same as in bookIsoPlots!
  vector<string> vname; 
  vname.push_back("Doca05"); 
  vname.push_back("Doca04"); 
  vname.push_back("Doca03"); 
  vname.push_back("Doca02"); 
  
  string name; 
  for (unsigned int i = 0; i < vname.size(); ++i) {
    name = vname[i];

    if (0 == i) {doca = 0.05; }
    if (1 == i) {doca = 0.04; }
    if (2 == i) {doca = 0.03; }
    if (3 == i) {doca = 0.02; }

    // R < 0.3
    iso = isoClassicWithDOCA(fpCand, doca, 0.3, 0.3);
    fpIsoR03Pt03[i]->fill(iso, fCandM);
    fpTk0R03Pt03[i]->fill(fCandI0trk, fCandM);
    fpTk1R03Pt03[i]->fill(fCandI1trk, fCandM);
    fpTk2R03Pt03[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.3, 0.5);
    fpIsoR03Pt05[i]->fill(iso, fCandM);
    fpTk0R03Pt05[i]->fill(fCandI0trk, fCandM);
    fpTk1R03Pt05[i]->fill(fCandI1trk, fCandM);
    fpTk2R03Pt05[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.3, 0.7);
    fpIsoR03Pt07[i]->fill(iso, fCandM);
    fpTk0R03Pt07[i]->fill(fCandI0trk, fCandM);
    fpTk1R03Pt07[i]->fill(fCandI1trk, fCandM);
    fpTk2R03Pt07[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.3, 0.9);
    fpIsoR03Pt09[i]->fill(iso, fCandM);
    fpTk0R03Pt09[i]->fill(fCandI0trk, fCandM);
    fpTk1R03Pt09[i]->fill(fCandI1trk, fCandM);
    fpTk2R03Pt09[i]->fill(fCandI2trk, fCandM);
    
    iso = isoClassicWithDOCA(fpCand, doca, 0.3, 1.1);
    fpIsoR03Pt11[i]->fill(iso, fCandM);
    fpTk0R03Pt11[i]->fill(fCandI0trk, fCandM);
    fpTk1R03Pt11[i]->fill(fCandI1trk, fCandM);
    fpTk2R03Pt11[i]->fill(fCandI2trk, fCandM);

    // R < 0.5
    iso = isoClassicWithDOCA(fpCand, doca, 0.5, 0.3);
    fpIsoR05Pt03[i]->fill(iso, fCandM);
    fpTk0R05Pt03[i]->fill(fCandI0trk, fCandM);
    fpTk1R05Pt03[i]->fill(fCandI1trk, fCandM);
    fpTk2R05Pt03[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.5, 0.5);
    fpIsoR05Pt05[i]->fill(iso, fCandM);
    fpTk0R05Pt05[i]->fill(fCandI0trk, fCandM);
    fpTk1R05Pt05[i]->fill(fCandI1trk, fCandM);
    fpTk2R05Pt05[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.5, 0.7);
    fpIsoR05Pt07[i]->fill(iso, fCandM);
    fpTk0R05Pt07[i]->fill(fCandI0trk, fCandM);
    fpTk1R05Pt07[i]->fill(fCandI1trk, fCandM);
    fpTk2R05Pt07[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.5, 0.9);
    fpIsoR05Pt09[i]->fill(iso, fCandM);
    fpTk0R05Pt09[i]->fill(fCandI0trk, fCandM);
    fpTk1R05Pt09[i]->fill(fCandI1trk, fCandM);
    fpTk2R05Pt09[i]->fill(fCandI2trk, fCandM);
    
    iso = isoClassicWithDOCA(fpCand, doca, 0.5, 1.1);
    fpIsoR05Pt11[i]->fill(iso, fCandM);
    fpTk0R05Pt11[i]->fill(fCandI0trk, fCandM);
    fpTk1R05Pt11[i]->fill(fCandI1trk, fCandM);
    fpTk2R05Pt11[i]->fill(fCandI2trk, fCandM);


    // R < 0.7
    iso = isoClassicWithDOCA(fpCand, doca, 0.7, 0.3);
    fpIsoR07Pt03[i]->fill(iso, fCandM);
    fpTk0R07Pt03[i]->fill(fCandI0trk, fCandM);
    fpTk1R07Pt03[i]->fill(fCandI1trk, fCandM);
    fpTk2R07Pt03[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.7, 0.5);
    fpIsoR07Pt05[i]->fill(iso, fCandM);
    fpTk0R07Pt05[i]->fill(fCandI0trk, fCandM);
    fpTk1R07Pt05[i]->fill(fCandI1trk, fCandM);
    fpTk2R07Pt05[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.7, 0.7);
    fpIsoR07Pt07[i]->fill(iso, fCandM);
    fpTk0R07Pt07[i]->fill(fCandI0trk, fCandM);
    fpTk1R07Pt07[i]->fill(fCandI1trk, fCandM);
    fpTk2R07Pt07[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.7, 0.9);
    fpIsoR07Pt09[i]->fill(iso, fCandM);
    fpTk0R07Pt09[i]->fill(fCandI0trk, fCandM);
    fpTk1R07Pt09[i]->fill(fCandI1trk, fCandM);
    fpTk2R07Pt09[i]->fill(fCandI2trk, fCandM);
    
    iso = isoClassicWithDOCA(fpCand, doca, 0.7, 1.1);
    fpIsoR07Pt11[i]->fill(iso, fCandM);
    fpTk0R07Pt11[i]->fill(fCandI0trk, fCandM);
    fpTk1R07Pt11[i]->fill(fCandI1trk, fCandM);
    fpTk2R07Pt11[i]->fill(fCandI2trk, fCandM);


    // R < 0.9
    iso = isoClassicWithDOCA(fpCand, doca, 0.9, 0.3);
    fpIsoR09Pt03[i]->fill(iso, fCandM);
    fpTk0R09Pt03[i]->fill(fCandI0trk, fCandM);
    fpTk1R09Pt03[i]->fill(fCandI1trk, fCandM);
    fpTk2R09Pt03[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.9, 0.5);
    fpIsoR09Pt05[i]->fill(iso, fCandM);
    fpTk0R09Pt05[i]->fill(fCandI0trk, fCandM);
    fpTk1R09Pt05[i]->fill(fCandI1trk, fCandM);
    fpTk2R09Pt05[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.9, 0.7);
    fpIsoR09Pt07[i]->fill(iso, fCandM);
    fpTk0R09Pt07[i]->fill(fCandI0trk, fCandM);
    fpTk1R09Pt07[i]->fill(fCandI1trk, fCandM);
    fpTk2R09Pt07[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 0.9, 0.9);
    fpIsoR09Pt09[i]->fill(iso, fCandM);
    fpTk0R09Pt09[i]->fill(fCandI0trk, fCandM);
    fpTk1R09Pt09[i]->fill(fCandI1trk, fCandM);
    fpTk2R09Pt09[i]->fill(fCandI2trk, fCandM);
    
    iso = isoClassicWithDOCA(fpCand, doca, 0.9, 1.1);
    fpIsoR09Pt11[i]->fill(iso, fCandM);
    fpTk0R09Pt11[i]->fill(fCandI0trk, fCandM);
    fpTk1R09Pt11[i]->fill(fCandI1trk, fCandM);
    fpTk2R09Pt11[i]->fill(fCandI2trk, fCandM);


    // R < 1.0
    iso = isoClassicWithDOCA(fpCand, doca, 1.0, 0.3);
    fpIsoR10Pt03[i]->fill(iso, fCandM);
    fpTk0R10Pt03[i]->fill(fCandI0trk, fCandM);
    fpTk1R10Pt03[i]->fill(fCandI1trk, fCandM);
    fpTk2R10Pt03[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 1.0, 0.5);
    fpIsoR10Pt05[i]->fill(iso, fCandM);
    fpTk0R10Pt05[i]->fill(fCandI0trk, fCandM);
    fpTk1R10Pt05[i]->fill(fCandI1trk, fCandM);
    fpTk2R10Pt05[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 1.0, 0.7);
    fpIsoR10Pt07[i]->fill(iso, fCandM);
    fpTk0R10Pt07[i]->fill(fCandI0trk, fCandM);
    fpTk1R10Pt07[i]->fill(fCandI1trk, fCandM);
    fpTk2R10Pt07[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 1.0, 0.9);
    fpIsoR10Pt09[i]->fill(iso, fCandM);
    fpTk0R10Pt09[i]->fill(fCandI0trk, fCandM);
    fpTk1R10Pt09[i]->fill(fCandI1trk, fCandM);
    fpTk2R10Pt09[i]->fill(fCandI2trk, fCandM);
    
    iso = isoClassicWithDOCA(fpCand, doca, 1.0, 1.1);
    fpIsoR10Pt11[i]->fill(iso, fCandM);
    fpTk0R10Pt11[i]->fill(fCandI0trk, fCandM);
    fpTk1R10Pt11[i]->fill(fCandI1trk, fCandM);
    fpTk2R10Pt11[i]->fill(fCandI2trk, fCandM);


    // R < 1.1
    iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.3);
    fpIsoR11Pt03[i]->fill(iso, fCandM);
    fpTk0R11Pt03[i]->fill(fCandI0trk, fCandM);
    fpTk1R11Pt03[i]->fill(fCandI1trk, fCandM);
    fpTk2R11Pt03[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.5);
    fpIsoR11Pt05[i]->fill(iso, fCandM);
    fpTk0R11Pt05[i]->fill(fCandI0trk, fCandM);
    fpTk1R11Pt05[i]->fill(fCandI1trk, fCandM);
    fpTk2R11Pt05[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.7);
    fpIsoR11Pt07[i]->fill(iso, fCandM);
    fpTk0R11Pt07[i]->fill(fCandI0trk, fCandM);
    fpTk1R11Pt07[i]->fill(fCandI1trk, fCandM);
    fpTk2R11Pt07[i]->fill(fCandI2trk, fCandM);

    iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.9);
    fpIsoR11Pt09[i]->fill(iso, fCandM);
    fpTk0R11Pt09[i]->fill(fCandI0trk, fCandM);
    fpTk1R11Pt09[i]->fill(fCandI1trk, fCandM);
    fpTk2R11Pt09[i]->fill(fCandI2trk, fCandM);
    
    iso = isoClassicWithDOCA(fpCand, doca, 1.1, 1.1);
    fpIsoR11Pt11[i]->fill(iso, fCandM);
    fpTk0R11Pt11[i]->fill(fCandI0trk, fCandM);
    fpTk1R11Pt11[i]->fill(fCandI1trk, fCandM);
    fpTk2R11Pt11[i]->fill(fCandI2trk, fCandM);
  }
}


// ----------------------------------------------------------------------
void candAna::bookIsoPlots() {

  TDirectory *pD = fHistDir->mkdir(Form("Isolation"));
  pD->cd();
  
  vector<string> vname; 
  vname.push_back("Doca05"); 
  vname.push_back("Doca04"); 
  vname.push_back("Doca03"); 
  vname.push_back("Doca02"); 

  string name; 
  for (unsigned int i = 0; i < vname.size(); ++i) {
    name = vname[i];
    cout << "booking i = " << i << " with name = " << name << endl;

    fpIsoR03Pt03[i] = bookDistribution(Form("%sisor03pt03", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR03Pt05[i] = bookDistribution(Form("%sisor03pt05", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR03Pt07[i] = bookDistribution(Form("%sisor03pt07", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR03Pt09[i] = bookDistribution(Form("%sisor03pt09", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR03Pt11[i] = bookDistribution(Form("%sisor03pt11", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    
    fpIsoR05Pt03[i] = bookDistribution(Form("%sisor05pt03", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR05Pt05[i] = bookDistribution(Form("%sisor05pt05", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR05Pt07[i] = bookDistribution(Form("%sisor05pt07", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR05Pt09[i] = bookDistribution(Form("%sisor05pt09", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR05Pt11[i] = bookDistribution(Form("%sisor05pt11", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    
    fpIsoR07Pt03[i] = bookDistribution(Form("%sisor07pt03", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR07Pt05[i] = bookDistribution(Form("%sisor07pt05", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR07Pt07[i] = bookDistribution(Form("%sisor07pt07", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR07Pt09[i] = bookDistribution(Form("%sisor07pt09", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR07Pt11[i] = bookDistribution(Form("%sisor07pt11", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    
    fpIsoR09Pt03[i] = bookDistribution(Form("%sisor09pt03", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR09Pt05[i] = bookDistribution(Form("%sisor09pt05", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR09Pt07[i] = bookDistribution(Form("%sisor09pt07", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR09Pt09[i] = bookDistribution(Form("%sisor09pt09", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR09Pt11[i] = bookDistribution(Form("%sisor09pt11", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    
    fpIsoR10Pt03[i] = bookDistribution(Form("%sisor10pt03", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR10Pt05[i] = bookDistribution(Form("%sisor10pt05", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR10Pt07[i] = bookDistribution(Form("%sisor10pt07", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR10Pt09[i] = bookDistribution(Form("%sisor10pt09", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR10Pt11[i] = bookDistribution(Form("%sisor10pt11", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    
    fpIsoR11Pt03[i] = bookDistribution(Form("%sisor11pt03", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR11Pt05[i] = bookDistribution(Form("%sisor11pt05", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR11Pt07[i] = bookDistribution(Form("%sisor11pt07", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR11Pt09[i] = bookDistribution(Form("%sisor11pt09", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    fpIsoR11Pt11[i] = bookDistribution(Form("%sisor11pt11", name.c_str()),  "I", "fGoodIso", 22, 0., 1.1); 
    
    
    // same PV track multiplicity
    fpTk0R03Pt03[i]  = bookDistribution(Form("%stk0r03pt03", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R03Pt05[i]  = bookDistribution(Form("%stk0r03pt05", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R03Pt07[i]  = bookDistribution(Form("%stk0r03pt07", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R03Pt09[i]  = bookDistribution(Form("%stk0r03pt09", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R03Pt11[i]  = bookDistribution(Form("%stk0r03pt11", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk0R05Pt03[i]  = bookDistribution(Form("%stk0r05pt03", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R05Pt05[i]  = bookDistribution(Form("%stk0r05pt05", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R05Pt07[i]  = bookDistribution(Form("%stk0r05pt07", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R05Pt09[i]  = bookDistribution(Form("%stk0r05pt09", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R05Pt11[i]  = bookDistribution(Form("%stk0r05pt11", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk0R07Pt03[i]  = bookDistribution(Form("%stk0r07pt03", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R07Pt05[i]  = bookDistribution(Form("%stk0r07pt05", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R07Pt07[i]  = bookDistribution(Form("%stk0r07pt07", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R07Pt09[i]  = bookDistribution(Form("%stk0r07pt09", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R07Pt11[i]  = bookDistribution(Form("%stk0r07pt11", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk0R09Pt03[i]  = bookDistribution(Form("%stk0r09pt03", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R09Pt05[i]  = bookDistribution(Form("%stk0r09pt05", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R09Pt07[i]  = bookDistribution(Form("%stk0r09pt07", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R09Pt09[i]  = bookDistribution(Form("%stk0r09pt09", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R09Pt11[i]  = bookDistribution(Form("%stk0r09pt11", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk0R10Pt03[i]  = bookDistribution(Form("%stk0r10pt03", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R10Pt05[i]  = bookDistribution(Form("%stk0r10pt05", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R10Pt07[i]  = bookDistribution(Form("%stk0r10pt07", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R10Pt09[i]  = bookDistribution(Form("%stk0r10pt09", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R10Pt11[i]  = bookDistribution(Form("%stk0r10pt11", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk0R11Pt03[i]  = bookDistribution(Form("%stk0r11pt03", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R11Pt05[i]  = bookDistribution(Form("%stk0r11pt05", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R11Pt07[i]  = bookDistribution(Form("%stk0r11pt07", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R11Pt09[i]  = bookDistribution(Form("%stk0r11pt09", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk0R11Pt11[i]  = bookDistribution(Form("%stk0r11pt11", name.c_str()),  "N_{tk0}^{I}", "fGoodIso", 20, 0., 20.); 
    
    // close track multiplicity
    fpTk1R03Pt03[i]  = bookDistribution(Form("%stk1r03pt03", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R03Pt05[i]  = bookDistribution(Form("%stk1r03pt05", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R03Pt07[i]  = bookDistribution(Form("%stk1r03pt07", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R03Pt09[i]  = bookDistribution(Form("%stk1r03pt09", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R03Pt11[i]  = bookDistribution(Form("%stk1r03pt11", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk1R05Pt03[i]  = bookDistribution(Form("%stk1r05pt03", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R05Pt05[i]  = bookDistribution(Form("%stk1r05pt05", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R05Pt07[i]  = bookDistribution(Form("%stk1r05pt07", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R05Pt09[i]  = bookDistribution(Form("%stk1r05pt09", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R05Pt11[i]  = bookDistribution(Form("%stk1r05pt11", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk1R07Pt03[i]  = bookDistribution(Form("%stk1r07pt03", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R07Pt05[i]  = bookDistribution(Form("%stk1r07pt05", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R07Pt07[i]  = bookDistribution(Form("%stk1r07pt07", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R07Pt09[i]  = bookDistribution(Form("%stk1r07pt09", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R07Pt11[i]  = bookDistribution(Form("%stk1r07pt11", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk1R09Pt03[i]  = bookDistribution(Form("%stk1r09pt03", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R09Pt05[i]  = bookDistribution(Form("%stk1r09pt05", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R09Pt07[i]  = bookDistribution(Form("%stk1r09pt07", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R09Pt09[i]  = bookDistribution(Form("%stk1r09pt09", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R09Pt11[i]  = bookDistribution(Form("%stk1r09pt11", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk1R10Pt03[i]  = bookDistribution(Form("%stk1r10pt03", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R10Pt05[i]  = bookDistribution(Form("%stk1r10pt05", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R10Pt07[i]  = bookDistribution(Form("%stk1r10pt07", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R10Pt09[i]  = bookDistribution(Form("%stk1r10pt09", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R10Pt11[i]  = bookDistribution(Form("%stk1r10pt11", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk1R11Pt03[i]  = bookDistribution(Form("%stk1r11pt03", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R11Pt05[i]  = bookDistribution(Form("%stk1r11pt05", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R11Pt07[i]  = bookDistribution(Form("%stk1r11pt07", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R11Pt09[i]  = bookDistribution(Form("%stk1r11pt09", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk1R11Pt11[i]  = bookDistribution(Form("%stk1r11pt11", name.c_str()),  "N_{tk1}^{I}", "fGoodIso", 20, 0., 20.); 


    // total track multiplicity
    fpTk2R03Pt03[i]  = bookDistribution(Form("%stk2r03pt03", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R03Pt05[i]  = bookDistribution(Form("%stk2r03pt05", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R03Pt07[i]  = bookDistribution(Form("%stk2r03pt07", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R03Pt09[i]  = bookDistribution(Form("%stk2r03pt09", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R03Pt11[i]  = bookDistribution(Form("%stk2r03pt11", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk2R05Pt03[i]  = bookDistribution(Form("%stk2r05pt03", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R05Pt05[i]  = bookDistribution(Form("%stk2r05pt05", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R05Pt07[i]  = bookDistribution(Form("%stk2r05pt07", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R05Pt09[i]  = bookDistribution(Form("%stk2r05pt09", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R05Pt11[i]  = bookDistribution(Form("%stk2r05pt11", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk2R07Pt03[i]  = bookDistribution(Form("%stk2r07pt03", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R07Pt05[i]  = bookDistribution(Form("%stk2r07pt05", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R07Pt07[i]  = bookDistribution(Form("%stk2r07pt07", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R07Pt09[i]  = bookDistribution(Form("%stk2r07pt09", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R07Pt11[i]  = bookDistribution(Form("%stk2r07pt11", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk2R09Pt03[i]  = bookDistribution(Form("%stk2r09pt03", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R09Pt05[i]  = bookDistribution(Form("%stk2r09pt05", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R09Pt07[i]  = bookDistribution(Form("%stk2r09pt07", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R09Pt09[i]  = bookDistribution(Form("%stk2r09pt09", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R09Pt11[i]  = bookDistribution(Form("%stk2r09pt11", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk2R10Pt03[i]  = bookDistribution(Form("%stk2r10pt03", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R10Pt05[i]  = bookDistribution(Form("%stk2r10pt05", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R10Pt07[i]  = bookDistribution(Form("%stk2r10pt07", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R10Pt09[i]  = bookDistribution(Form("%stk2r10pt09", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R10Pt11[i]  = bookDistribution(Form("%stk2r10pt11", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    
    fpTk2R11Pt03[i]  = bookDistribution(Form("%stk2r11pt03", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R11Pt05[i]  = bookDistribution(Form("%stk2r11pt05", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R11Pt07[i]  = bookDistribution(Form("%stk2r11pt07", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R11Pt09[i]  = bookDistribution(Form("%stk2r11pt09", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
    fpTk2R11Pt11[i]  = bookDistribution(Form("%stk2r11pt11", name.c_str()),  "N_{tk2}^{I}", "fGoodIso", 20, 0., 20.); 
  }
  
  fHistDir->cd();	

}


// ----------------------------------------------------------------------
void candAna::readCuts(string fileName, int dump) {

  // -- set up cut sequence for analysis
  basicCuts(); 
  moreBasicCuts(); 
  candidateCuts(); 
  moreCandidateCuts(); 

  fCutFile = fileName;

  if (dump) cout << "==> candAna: Reading " << fCutFile << " for cut settings" << endl;
  vector<string> cutLines; 
  readFile(fCutFile, cutLines);

  char CutName[100];
  float CutValue;
  int ok(0);

  char  buffer[1000], XmlName[1000];
  fHistDir->cd();
  if (dump) cout << "gDirectory: "; fHistDir->pwd();
  TH1D *hcuts = new TH1D("hcuts", "", 1000, 0., 1000.);
  hcuts->GetXaxis()->SetBinLabel(1, fCutFile.c_str());
  int ibin; 
  string cstring = "B cand"; 

  for (unsigned int i = 0; i < cutLines.size(); ++i) {
    sprintf(buffer, "%s", cutLines[i].c_str()); 
    
    ok = 0;
    if (buffer[0] == '#') {continue;}
    if (buffer[0] == '/') {continue;}
    sscanf(buffer, "%s %f", CutName, &CutValue);

    if (!strcmp(CutName, "TYPE")) {
      TYPE = int(CutValue); ok = 1;
      if (dump) cout << "TYPE:           " << TYPE << endl;
      if (1313 == TYPE) cstring = "#mu^{+}#mu^{-}";
      if (301313 == TYPE) cstring = "#mu^{+}#mu^{-}";
      if (200521 == TYPE) cstring = "#mu^{+}#mu^{-}K^{+}";
      if (300521 == TYPE) cstring = "#mu^{+}#mu^{-}K^{+}";
      ibin = 1;
      hcuts->SetBinContent(ibin, TYPE);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: Candidate type", CutName));
    }

    if (!strcmp(CutName, "SELMODE")) {
      SELMODE = int(CutValue); ok = 1;
      if (dump) cout << "SELMODE:           " << SELMODE << endl;
      ibin = 2;
      hcuts->SetBinContent(ibin, SELMODE);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: Selection Mode :: %i", CutName, SELMODE));
    }

    if (!strcmp(CutName, "TRIGRANGE")) {
      char triggerlist[1000]; ok = 1; 
      sscanf(buffer, "%s %s", CutName, triggerlist);
      string tl(triggerlist); 
      int r1(0), r2(0); 
      string hlt = splitTrigRange(tl, r1, r2); 
      HLTRANGE.insert(make_pair(hlt, make_pair(r1, r2))); 
      if (dump) {
	cout << "HLTRANGE:       " << hlt << " from " << r1 << " to " << r2 << endl; 
      }
      ibin = 3; 
      hcuts->SetBinContent(ibin, 1);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: %s", CutName, triggerlist));
    }


    if (!strcmp(CutName, "TRUTHCAND")) {
      TRUTHCAND = int(CutValue); ok = 1;
      if (dump) cout << "TRUTHCAND:           " << TRUTHCAND << endl;
      ibin = 4;
      hcuts->SetBinContent(ibin, TYPE);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: Candidate type", CutName));
    }

    if (!strcmp(CutName, "IGNORETRIGGER")) {
      IGNORETRIGGER = int(CutValue); ok = 1;
      if (dump) cout << "IGNORETRIGGER      " << IGNORETRIGGER << endl;
      ibin = 5;
      hcuts->SetBinContent(ibin, IGNORETRIGGER);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: Ignore trigger :: %i", CutName, IGNORETRIGGER));
    }

    if (!strcmp(CutName, "CANDPTLO")) {
      CANDPTLO = CutValue; ok = 1;
      if (dump) cout << "CANDPTLO:           " << CANDPTLO << " GeV" << endl;
      ibin = 11;
      hcuts->SetBinContent(ibin, CANDPTLO);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: p_{T}^{min}(%s) :: %3.1f", CutName, cstring.c_str(), CANDPTLO));
    }

    if (!strcmp(CutName, "CANDETALO")) {
      CANDETALO = CutValue; ok = 1;
      if (dump) cout << "CANDETALO:           " << CANDETALO << endl;
      ibin = 12;
      hcuts->SetBinContent(ibin, CANDETALO);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: #eta^{min}(%s) :: %3.1f", CutName, cstring.c_str(), CANDETALO));
    }

    if (!strcmp(CutName, "CANDETAHI")) {
      CANDETAHI = CutValue; ok = 1;
      if (dump) cout << "CANDETAHI:           " << CANDETAHI << endl;
      ibin = 13;
      hcuts->SetBinContent(ibin, CANDETAHI);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: #eta^{max}(%s) :: %3.1f", CutName, cstring.c_str(), CANDETAHI));
    }

    if (!strcmp(CutName, "CANDCOSALPHA")) {
      CANDCOSALPHA = CutValue; ok = 1;
      if (dump) cout << "CANDCOSALPHA:           " << CANDCOSALPHA << endl;
      ibin = 20;
      hcuts->SetBinContent(ibin, CANDCOSALPHA);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: cos#alpha :: %5.4f", CutName, CANDCOSALPHA));
    }

    if (!strcmp(CutName, "CANDALPHA")) {
      CANDALPHA = CutValue; ok = 1;
      if (dump) cout << "CANDALPHA:           " << CANDALPHA << endl;
      ibin = 21;
      hcuts->SetBinContent(ibin, CANDALPHA);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: #alpha :: %5.4f", CutName, CANDALPHA));
    }

    if (!strcmp(CutName, "CANDFLS3D")) {
      CANDFLS3D = CutValue; ok = 1;
      if (dump) cout << "CANDFLS3D:           " << CANDFLS3D << endl;
      ibin = 22;
      hcuts->SetBinContent(ibin, CANDFLS3D);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: l_{3d}/#sigma(l_{3d}) :: %3.1f", CutName, CANDFLS3D));
    }

    if (!strcmp(CutName, "CANDFLSXY")) {
      CANDFLSXY = CutValue; ok = 1;
      if (dump) cout << "CANDFLSXY:           " << CANDFLSXY << endl;
      ibin = 23;
      hcuts->SetBinContent(ibin, CANDFLSXY);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: l_{xy}/#sigma(l_{xy}) :: %3.1f", CutName, CANDFLSXY));
    }

    if (!strcmp(CutName, "CANDVTXCHI2")) {
      CANDVTXCHI2 = CutValue; ok = 1;
      if (dump) cout << "CANDVTXCHI2:           " << CANDVTXCHI2 << endl;
      ibin = 24;
      hcuts->SetBinContent(ibin, CANDVTXCHI2);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: #chi^{2} :: %3.1f", CutName, CANDVTXCHI2));
    }

    if (!strcmp(CutName, "CANDISOLATION")) {
      CANDISOLATION = CutValue; ok = 1;
      if (dump) cout << "CANDISOLATION:           " << CANDISOLATION << endl;
      ibin = 25;
      hcuts->SetBinContent(ibin, CANDISOLATION);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: I_{trk} :: %4.2f", CutName, CANDISOLATION));
    }

    if (!strcmp(CutName, "CANDDOCATRK")) {
      CANDDOCATRK = CutValue; ok = 1;
      if (dump) cout << "CANDDOCATRK:           " << CANDDOCATRK << endl;
      ibin = 26;
      hcuts->SetBinContent(ibin, CANDDOCATRK);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: doca_{trk} :: %4.3f", CutName, CANDDOCATRK));
    }

    if (!strcmp(CutName, "CANDCLOSETRK")) {
      CANDCLOSETRK = CutValue; ok = 1;
      if (dump) cout << "CANDCLOSETRK:           " << CANDCLOSETRK << endl;
      ibin = 27;
      hcuts->SetBinContent(ibin, CANDCLOSETRK);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: N_{close tracks} :: %4.2f", CutName, CANDCLOSETRK));
    }

    if (!strcmp(CutName, "PVAVEW8")) {
      PVAVEW8 = CutValue; ok = 1;
      if (dump) cout << "PVAVEW8:           " << PVAVEW8 << endl;
      ibin = 28;
      hcuts->SetBinContent(ibin, PVAVEW8);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: <w^{PV}_{trk}> :: %4.3f", CutName, PVAVEW8));
    }

    if (!strcmp(CutName, "CANDLIP")) {
      CANDLIP = CutValue; ok = 1;
      if (dump) cout << "CANDLIP:           " << CANDLIP << endl;
      ibin = 40;
      hcuts->SetBinContent(ibin, CANDLIP);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: l_{z} :: %4.3f", CutName, CANDLIP));
    }

    if (!strcmp(CutName, "CANDLIPS")) {
      CANDLIPS = CutValue; ok = 1;
      if (dump) cout << "CANDLIPS:          " << CANDLIPS << endl;
      ibin = 41;
      hcuts->SetBinContent(ibin, CANDLIPS);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: l_{z}/#sigma(l_{z}) :: %4.3f", CutName, CANDLIPS));
    }

    if (!strcmp(CutName, "CAND2LIP")) {
      CANDLIP2 = CutValue; ok = 1;
      if (dump) cout << "CAND2LIP:           " << CANDLIP2 << endl;
      ibin = 42;
      hcuts->SetBinContent(ibin, CANDLIP2);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: l_{z,2} :: %4.3f", CutName, CANDLIP2));
    }

    if (!strcmp(CutName, "CAND2LIPS")) {
      CANDLIPS2 = CutValue; ok = 1;
      if (dump) cout << "CAND2LIPS:          " << CANDLIPS2 << endl;
      ibin = 43;
      hcuts->SetBinContent(ibin, CANDLIPS2);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: l_{z,2}/#sigma(l_{z,2}) :: %4.3f", CutName, CANDLIPS2));
    }

    if (!strcmp(CutName, "CANDIP")) {
      CANDIP = CutValue; ok = 1;
      if (dump) cout << "CANDIP:          " << CANDIP << endl;
      ibin = 44;
      hcuts->SetBinContent(ibin, CANDIP);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: l_{3d} :: %4.3f", CutName, CANDIP));
    }

    if (!strcmp(CutName, "CANDIPS")) {
      CANDIPS = CutValue; ok = 1;
      if (dump) cout << "CANDIPS:          " << CANDIPS << endl;
      ibin = 45;
      hcuts->SetBinContent(ibin, CANDIPS);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: l_{3d}/#sigma(l_{3d}) :: %4.3f", CutName, CANDIPS));
    }

    if (!strcmp(CutName, "MAXDOCA")) {
      CANDDOCA = CutValue; ok = 1;
      if (dump) cout << "MAXDOCA:         " << CANDDOCA << endl;
      ibin = 46;
      hcuts->SetBinContent(ibin, CANDDOCA);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: d :: %4.3f", CutName, CANDDOCA));
    }


    if (!strcmp(CutName, "SIGBOXMIN")) {
      SIGBOXMIN = CutValue; ok = 1;
      if (dump) cout << "SIGBOXMIN:           " << SIGBOXMIN << endl;
      ibin = 90;
      hcuts->SetBinContent(ibin, SIGBOXMIN);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: SIGBOXMIN :: %6.3f", CutName, SIGBOXMIN));
    }

    if (!strcmp(CutName, "SIGBOXMAX")) {
      SIGBOXMAX = CutValue; ok = 1;
      if (dump) cout << "SIGBOXMAX:           " << SIGBOXMAX << endl;
      ibin = 91;
      hcuts->SetBinContent(ibin, SIGBOXMAX);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: SIGBOXMAX :: %6.3f", CutName, SIGBOXMAX));
    }

    if (!strcmp(CutName, "BGLBOXMIN")) {
      BGLBOXMIN = CutValue; ok = 1;
      if (dump) cout << "BGLBOXMIN:           " << BGLBOXMIN << endl;
      ibin = 92;
      hcuts->SetBinContent(ibin, BGLBOXMIN);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: BGLBOXMIN :: %6.3f", CutName, BGLBOXMIN));
    }

    if (!strcmp(CutName, "BGLBOXMAX")) {
      BGLBOXMAX = CutValue; ok = 1;
      if (dump) cout << "BGLBOXMAX:           " << BGLBOXMAX << endl;
      ibin = 93;
      hcuts->SetBinContent(ibin, BGLBOXMAX);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: BGLBOXMAX :: %6.3f", CutName, BGLBOXMAX));
    }

    if (!strcmp(CutName, "BGHBOXMIN")) {
      BGHBOXMIN = CutValue; ok = 1;
      if (dump) cout << "BGHBOXMIN:           " << BGHBOXMIN << endl;
      ibin = 94;
      hcuts->SetBinContent(ibin, BGHBOXMIN);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: BGHBOXMIN :: %6.3f", CutName, BGHBOXMIN));
    }

    if (!strcmp(CutName, "BGHBOXMAX")) {
      BGHBOXMAX = CutValue; ok = 1;
      if (dump) cout << "BGHBOXMAX:           " << BGHBOXMAX << endl;
      ibin = 95;
      hcuts->SetBinContent(ibin, BGHBOXMAX);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: BGHBOXMAX :: %6.3f", CutName, BGHBOXMAX));
    }

    // -- Tracks
    if (!strcmp(CutName, "TRACKQUALITY")) {
      TRACKQUALITY = CutValue; ok = 1;
      if (dump) cout << "TRACKQUALITY:           " << TRACKQUALITY << " " << endl;
      ibin = 100;
      hcuts->SetBinContent(ibin, TRACKQUALITY);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: track quality :: %d", CutName, TRACKQUALITY));
    }

    if (!strcmp(CutName, "TRACKPTLO")) {
      TRACKPTLO = CutValue; ok = 1;
      if (dump) cout << "TRACKPTLO:           " << TRACKPTLO << " GeV" << endl;
      ibin = 101;
      hcuts->SetBinContent(ibin, TRACKPTLO);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: p_{T}^{min}(track) :: %3.1f", CutName, TRACKPTLO));
    }

    if (!strcmp(CutName, "TRACKPTHI")) {
      TRACKPTHI = CutValue; ok = 1;
      if (dump) cout << "TRACKPTHI:           " << TRACKPTHI << " GeV" << endl;
      ibin = 102;
      hcuts->SetBinContent(ibin, TRACKPTHI);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: p_{T}^{max}(track) :: %3.1f", CutName, TRACKPTHI));
    }

    if (!strcmp(CutName, "TRACKTIP")) {
      TRACKTIP = CutValue; ok = 1;
      if (dump) cout << "TRACKTIP:           " << TRACKTIP << " cm" << endl;
      ibin = 103;
      hcuts->SetBinContent(ibin, TRACKTIP);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: doca_{xy}(track) :: %3.1f", CutName, TRACKTIP));
    }

    if (!strcmp(CutName, "TRACKLIP")) {
      TRACKLIP = CutValue; ok = 1;
      if (dump) cout << "TRACKLIP:           " << TRACKLIP << " cm" << endl;
      ibin = 104;
      hcuts->SetBinContent(ibin, TRACKLIP);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: doca_{z}(track) :: %3.1f", CutName, TRACKLIP));
    }

    if (!strcmp(CutName, "TRACKETALO")) {
      TRACKETALO = CutValue; ok = 1;
      if (dump) cout << "TRACKETALO:           " << TRACKETALO << " " << endl;
      ibin = 105;
      hcuts->SetBinContent(ibin, TRACKETALO);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: #eta_{min}(track) :: %3.1f", CutName, TRACKETALO));
    }

    if (!strcmp(CutName, "TRACKETAHI")) {
      TRACKETAHI = CutValue; ok = 1;
      if (dump) cout << "TRACKETAHI:           " << TRACKETAHI << " " << endl;
      ibin = 106;
      hcuts->SetBinContent(ibin, TRACKETAHI);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: #eta_{max}(track) :: %3.1f", CutName, TRACKETAHI));
    }

    // -- Muons
    if (!strcmp(CutName, "MUIDMASK")) {
      MUIDMASK = int(CutValue); ok = 1;
      if (dump) cout << "MUIDMASK:           " << MUIDMASK << endl;
      ibin = 200;
      hcuts->SetBinContent(ibin, MUIDMASK);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: MuIDMask :: %d", CutName, MUIDMASK));
    }

    if (!strcmp(CutName, "MUIDRESULT")) {
      // MUIDRESULT == 0: compare result of & with ">=0"
      // MUIDRESULT != 0: compare result of & with "==MUIDRESULT"
      MUIDRESULT = int(CutValue); ok = 1;
      if (dump) cout << "MUIDRESULT:           " << MUIDRESULT << endl;
      ibin = 201;
      hcuts->SetBinContent(ibin, MUIDRESULT);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: MuIDResult :: %d", CutName, MUIDRESULT));
    }

    if (!strcmp(CutName, "MUPTLO")) {
      MUPTLO = CutValue; ok = 1;
      if (dump) cout << "MUPTLO:           " << MUPTLO << " GeV" << endl;
      ibin = 202;
      hcuts->SetBinContent(ibin, MUPTLO);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: p_{T}^{min}(#mu) :: %3.1f", CutName, MUPTLO));
    }

    if (!strcmp(CutName, "MUPTHI")) {
      MUPTHI = CutValue; ok = 1;
      if (dump) cout << "MUPTHI:           " << MUPTHI << " GeV" << endl;
      ibin = 203;
      hcuts->SetBinContent(ibin, MUPTHI);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: p_{T}^{max}(#mu) :: %3.1f", CutName, MUPTHI));
    }

    if (!strcmp(CutName, "MUETALO")) {
      MUETALO = CutValue; ok = 1;
      if (dump) cout << "MUETALO:           " << MUETALO << endl;
      ibin = 204;
      hcuts->SetBinContent(ibin, MUETALO);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: #eta^{min}(#mu) :: %3.1f", CutName, MUETALO));
    }

    if (!strcmp(CutName, "MUETAHI")) {
      MUETAHI = CutValue; ok = 1;
      if (dump) cout << "MUETAHI:           " << MUETAHI << endl;
      ibin = 205;
      hcuts->SetBinContent(ibin, MUETAHI);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: #eta^{max}(#mu) :: %3.1f", CutName, MUETAHI));
    }

    if (!strcmp(CutName, "MUIP")) {
      MUIP = CutValue; ok = 1;
      if (dump) cout << "MUIP:           " << MUIP << endl;
      ibin = 206;
      hcuts->SetBinContent(ibin, MUIP);
      hcuts->GetXaxis()->SetBinLabel(ibin, Form("%s :: IP(#mu) :: %3.1f", CutName, MUIP));
    }


    sscanf(buffer, "%s %s", CutName, XmlName);
    string ctmp = CutName; 
    string sXmlName;
    replaceAll(ctmp, " ", ""); 
    // -- barrel
    if (!strcmp(ctmp.c_str(), "xml0")) {
      sXmlName = "weights/" + string(XmlName) + "-Events0_BDT.weights.xml"; 
      if (dump) cout << "xml:                   " << sXmlName << endl;
      fReaderEvents0.push_back(setupReader(sXmlName, frd)); 
      sXmlName = "weights/" + string(XmlName) + "-Events1_BDT.weights.xml"; 
      if (dump) cout << "xml:                   " << sXmlName << endl;
      fReaderEvents1.push_back(setupReader(sXmlName, frd)); 
      sXmlName = "weights/" + string(XmlName) + "-Events2_BDT.weights.xml"; 
      if (dump) cout << "xml:                   " << sXmlName << endl;
      fReaderEvents2.push_back(setupReader(sXmlName, frd)); 
    }

    // -- endcap
    if (!strcmp(ctmp.c_str(), "xml1")) {
      sXmlName = "weights/" + string(XmlName) + "-Events0_BDT.weights.xml"; 
      if (dump) cout << "xml:                   " << sXmlName << endl;
      fReaderEvents0.push_back(setupReader(sXmlName, frd)); 
      sXmlName = "weights/" + string(XmlName) + "-Events1_BDT.weights.xml"; 
      if (dump) cout << "xml:                   " << sXmlName << endl;
      fReaderEvents1.push_back(setupReader(sXmlName, frd)); 
      sXmlName = "weights/" + string(XmlName) + "-Events2_BDT.weights.xml"; 
      if (dump) cout << "xml:                   " << sXmlName << endl;
      fReaderEvents2.push_back(setupReader(sXmlName, frd)); 
    }


    //    if (!ok) cout << "==> candAna: error? nothing done with " << CutName << "!!" << endl;
  }

  if (dump)  cout << "------------------------------------" << endl;
}


// ----------------------------------------------------------------------
void candAna::readFile(string filename, vector<string> &lines) {
  cout << "    readFile " << filename << endl;
  char  buffer[200];
  ifstream is(filename.c_str());
  if (!is) {
    exit(1);
  }
  char input[1000]; 
  while (is.getline(buffer, 200, '\n')) {
    if (buffer[0] != '+') {
      lines.push_back(string(buffer));
    } else {
      sscanf(buffer, "+input %s", input);
      readFile(input, lines); 
    }
  }

}


// ----------------------------------------------------------------------
bool candAna::goodMuon(TAnaTrack *pT, int mask) {
  int result(0); 
  result = pT->fMuID & MUIDMASK;
  if (mask > 0) {
    result = pT->fMuID & mask;
  }

  if (HLTRANGE.begin()->first == "NOTRIGGER") {
    //    cout << "NOTRIGGER requested... " << endl;
    return true;
  }

  if (0 == MUIDRESULT ) {
    // -- result must be larger than zero for successful muon ID, i.e., the OR of the bits
    if (0 == result){
      if (fVerbose > 4) cout << "muon " << pT->fIndex << " failed MUID: " << hex << pT->fMuID << dec << endl;          
      return false; 
    } else {
      return true; 
    }
  } else {
    // -- result must be equal to the mask for successful muon ID, i.e., the AND of the bits
    if (MUIDRESULT != result){
      if (fVerbose > 4) cout << "muon " << pT->fIndex << " failed MUID: " << hex << pT->fMuID << dec << endl;          
      return false; 
    } else {
      return true;
    }
  }
}


// ----------------------------------------------------------------------
bool candAna::tightMuon(TAnaTrack *pT) {

  if (HLTRANGE.begin()->first == "NOTRIGGER") {
    //    cout << "NOTRIGGER requested... " << endl;
    return true;
  }

  bool muflag = ((pT->fMuID & 80) == 80);
  //  bool muflag = ((pT->fMuID & 16) == 16);

  bool mucuts(false); 
  if (pT->fMuIndex > -1) {
    TAnaMuon *pM = fpEvt->getMuon(pT->fMuIndex);
    if (pM->fTimeNdof > 1) mucuts = true; 
    //    if (!muflag) cout << "matched muon stations: " << pM->fTimeNdof << endl;
  }

  bool trackcuts(true); 

  if (TMath::Abs(pT->fBsTip) > 0.2) trackcuts = false;
  if (fpReader->numberOfPixLayers(pT) < 1) trackcuts = false;
  if (pT->fValidHits < 11) trackcuts = false; 

  if (muflag && mucuts && trackcuts) {
    return true; 
  } else {
    return false;
  }
}


// ----------------------------------------------------------------------
string candAna::splitTrigRange(string tl, int &r1, int &r2) {

  string::size_type id1 = tl.find_first_of("("); 
  string::size_type id2 = tl.find_first_of(":"); 
  string::size_type id3 = tl.find_first_of(")"); 

  //cout << "tl: " << tl << endl;
  string hlt = tl.substr(0, id1);
  //cout << "hlt: " << hlt << endl;
  string a   = tl.substr(id1+1, id2-id1-1);
  r1 = atoi(a.c_str());
  //cout << "1st a: " << a << " -> r1 = " << r1 << endl;
  a  = tl.substr(id2+1, id3-id2-1); 
  r2 = atoi(a.c_str());
  //cout << "2nd a: " << a << " -> r2 = " << r2 << endl;

  return hlt; 

}


// ----------------------------------------------------------------------
int candAna::nCloseTracks(TAnaCand *pC, double dcaCut, double ptCut) {
  int cnt(0); 
  int nsize = pC->fNstTracks.size(); 
  int pvIdx = pC->fPvIdx;
  int pvIdx2= nearestPV(pvIdx, 0.1);
  if (TMath::Abs(fCandPvLipS2) > 2) pvIdx2 = -1;
  
  bool sameOrCloseVertex(false);
  
  if (0) {
    if (TMath::Abs(pC->fPvLip2) < 3 || TMath::Abs(pC->fPvLip2/pC->fPvLipE2) < 3) {
      cout << "XXXXXXXX " << fEvt << " XXX this cand (" << pC->fType << ") from PVidx = " << pvIdx 
	   << ") LIP: " << pC->fPvLip << " E = " << pC->fPvLipE 
	   << " LIP2: " << pC->fPvLip2 << " 2E = " << pC->fPvLipE2 
	   << endl;
      if (pvIdx2 > -1) cout << "   z = " << fpEvt->getPV(pC->fPvIdx)->fPoint.Z() << " (" << fpEvt->getPV(pC->fPvIdx)->getNtracks() << ")"
			    << " -> PV2 z = " << fpEvt->getPV(pvIdx2)->fPoint.Z() << " (" << fpEvt->getPV(pvIdx2)->getNtracks() << ")" 
			    << " cand z = " << pC->fVtx.fPoint.Z()
			    << endl;
      cout << " muon 1 pT = " << fpMuon1->fPlab.Perp() << " from " << fpMuon1->fPvIdx << " IPz = " << fpMuon1->fLip << "+/-" << fpMuon1->fLipE 
	   << " IPt = " << fpMuon1->fTip << "+/-" << fpMuon1->fTipE 
	   << " IPbsl = " << fpMuon1->fBsLip << "+/-" << fpMuon1->fBsLipE 
	   << " IPbst = " << fpMuon1->fBsTip << "+/-" << fpMuon1->fBsTipE 
	   << endl;
      
      cout << " muon 2 pT = " << fpMuon2->fPlab.Perp() << " from " << fpMuon2->fPvIdx << " IPz = " << fpMuon2->fLip << "+/-" << fpMuon2->fLipE 
	   << " IP = " << fpMuon2->fTip << "+/-" << fpMuon2->fTipE 
	   << " IPbsl = " << fpMuon2->fBsLip << "+/-" << fpMuon2->fBsLipE 
	   << " IPbst = " << fpMuon2->fBsTip << "+/-" << fpMuon2->fBsTipE 
	   << endl;
    }
  }

  TAnaTrack *pT; 
  double pt(0.); 
  if (nsize > 0) {
    for (int i = 0; i<nsize; ++i) {
      int trkId = pC->fNstTracks[i].first;
      double doca = pC->fNstTracks[i].second.first;
      
      if (doca > dcaCut) continue; // check the doca cut
      
      pT = fpEvt->getRecTrack(trkId);
      // -- check that any track associated with a definitive vertex is from the same or the closest (compatible) other PV
      if ((pT->fPvIdx > -1) && (pT->fPvIdx != pvIdx)) continue;
//       if (pT->fPvIdx > -1) {
// 	sameOrCloseVertex = (pT->fPvIdx == pvIdx) || (pT->fPvIdx == pvIdx2);
// 	if (!sameOrCloseVertex) continue;
//       }

      //      if ((pT->fPvIdx > -1) && (pT->fPvIdx != pvIdx)) continue;
      
      pt = pT->fPlab.Perp();  
      if (pt < ptCut) continue;
      
      ++cnt;
    }
  }
  return cnt;
}


// ----------------------------------------------------------------------
TAnaCand* candAna::osCand(TAnaCand *pC) {
  TAnaCand *a = new TAnaCand(); 
  a->fPvIdx = pC->fPvIdx; 
  a->fType = pC->fType; 
  a->fSig1 = a->fSig2 = -1; 
  a->fPlab = TVector3(-pC->fPlab.X(), -pC->fPlab.Y(), -pC->fPlab.Z()); //???
  return a; 
}


// ----------------------------------------------------------------------
double candAna::osIsolation(TAnaCand *pC, double r, double ptmin) {
  double iso(0.); 
  TAnaTrack *pT(0); 
  int overlap(0), verbose(0); 

  // -- look at all tracks that are associated to the same vertex
  for (int i = 0; i < fpEvt->nRecTracks(); ++i) {
    pT = fpEvt->getRecTrack(i); 
    if (verbose) {
      cout << "   track " << i 
     	   << " with pT = " << pT->fPlab.Perp()
     	   << " eta = " << pT->fPlab.Eta()
     	   << " pointing at PV " << pT->fPvIdx;
    }

    // -- check against overlap to primary candidate
    overlap = 0; 
    for (int j = pC->fSig1; j <= pC->fSig2; ++j) {
      if (i == fpEvt->getSigTrack(j)->fIndex) {
	overlap = 1;
	break;
      }
    }
    if (1 == overlap) continue;
    
    
// ???
//     if ((pT->fPvIdx > -1) && (pT->fPvIdx != pvIdx)) { 	 
//       if (verbose) cout << " doca track " << trkId << " skipped because it is from a different PV " << pT->fPvIdx <<endl; 	 
//       continue; 	 
//     }


    if (pT->fPvIdx != pC->fPvIdx) { 	 
      if (verbose) cout << " skipped because of PV index mismatch" << endl; 	     //FIXME
      continue;
    }
    
    double pt = pT->fPlab.Perp(); 
    if (pt < ptmin) {
      if (verbose) cout << " skipped because of pt = " << pt << endl;
      continue;
    }

    if (pT->fPlab.DeltaR(pC->fPlab) > r) {
      iso += pt; 
      if (verbose) cout << endl;
    } 
    else {
      if (verbose) cout << " skipped because of deltaR = " << pT->fPlab.DeltaR(pC->fPlab) << endl;
    }
  }
  
  return iso;
 
}

// ----------------------------------------------------------------------
int candAna::osMuon(TAnaCand *pC, double r) {

  double mpt(-1.); 
  int idx (-1); 
  TAnaTrack *pT(0); 
  int overlap(0), verbose(0); 
  
  // -- look at all tracks that are associated to the same vertex
  for (int i = 0; i < fpEvt->nRecTracks(); ++i) {
    pT = fpEvt->getRecTrack(i); 
    if (!tightMuon(pT)) continue;

    // -- check against overlap to primary candidate
    overlap = 0; 
    for (int j = pC->fSig1; j <= pC->fSig2; ++j) {
      if (i == fpEvt->getSigTrack(j)->fIndex) {
	overlap = 1;
	break;
      }
    }
    if (1 == overlap) continue;

    if (verbose) {
      cout << "   track " << i 
     	   << " with pT = " << pT->fPlab.Perp()
     	   << " eta = " << pT->fPlab.Eta()
     	   << " pointing at PV " << pT->fPvIdx;
    }
    
    // ???
    //     if (pT->fPvIdx != pvIdx) { 	 
    //       if (verbose) cout << " skipped because of PV index mismatch" << endl; 	     //FIXME
    //       continue;
    //     }
    
    if ((pT->fPvIdx > -1) && (pT->fPvIdx != pC->fPvIdx)) { 	 
      if (verbose) cout << " track " << i << " skipped because it is from a different PV " << pT->fPvIdx <<endl; 	 
      continue; 	 
    }
    
    if (pT->fPlab.DeltaR(pC->fPlab) > r) {
      if (pT->fPlab.Perp() > mpt) {
	mpt = pT->fPlab.Perp(); 
	idx = i; 
      }
      if (verbose) cout << endl;
    } 
    else {
      if (verbose) cout << " skipped because of deltaR = " << pT->fPlab.DeltaR(pC->fPlab) << endl;
    }
  }
  
  return idx;


}




// ----------------------------------------------------------------------
double candAna::isoClassicWithDOCA(TAnaCand *pC, double docaCut, double r, double ptmin) {
  const double ptCut(ptmin), coneSize(r); 
  const bool verbose(false);

  double iso(-1.), pt(0.), sumPt(0.), candPt(0.), candPtScalar(0.); 
  TAnaTrack *pT; 
  vector<int> cIdx, pIdx; 
  int pvIdx = pC->fPvIdx;
  int pvIdx2= nearestPV(pvIdx, 0.1);
  if (TMath::Abs(fCandPvLipS2) > 2) pvIdx2 = -1;
  
  bool sameOrCloseVertex(false);

  fCandI0trk = 0; 
  fCandI1trk = 0; 
  fCandI2trk = 0; 

  getSigTracks(cIdx, pC); 
  for (unsigned int i = 0; i < cIdx.size(); ++i) {
    pT = fpEvt->getRecTrack(i); 
 
    if (verbose) cout << " track idx = " << pT->fIndex << " with ID = " << pT->fMCID << endl;
    candPtScalar += pT->fPlab.Perp(); 
    if (verbose) {
      int tIdx = fpEvt->getRecTrack(pT->fIndex)->fPvIdx;
      if (pvIdx != tIdx) {
    	cout << "Signal track pointing to PV " << tIdx << " instead of " << pvIdx << endl;
      }
    }
  }
  
  candPt = pC->fPlab.Perp(); 
  
  // -- look at all tracks that are associated to the same vertex
  for (int i = 0; i < fpEvt->nRecTracks(); ++i) {
    pT = fpEvt->getRecTrack(i); 
    if (verbose) {
      cout << "   track " << i 
     	   << " with pT = " << pT->fPlab.Perp()
     	   << " eta = " << pT->fPlab.Eta()
     	   << " pointing at PV " << pT->fPvIdx;
    }
    
//     // -- check that any track associated with a definitive vertex is from the same PV
//     if (pT->fPvIdx > -1) {
//       sameOrCloseVertex = (pT->fPvIdx == pvIdx);
//       //       sameOrCloseVertex = (pT->fPvIdx == pvIdx) || (pT->fPvIdx == pvIdx2);
//       if (!sameOrCloseVertex) continue;
//     }

    if (pT->fPvIdx != pvIdx) { 	 
      if (verbose) cout << " skipped because of PV index mismatch" << endl; 	     //FIXME
      continue;
    }
    

    pt = pT->fPlab.Perp(); 
    if (pt < ptCut) {
      if (verbose) cout << " skipped because of pt = " << pt << endl;
      continue;
    }
    if (cIdx.end() != find(cIdx.begin(), cIdx.end(), i))  {
      if (verbose) cout << " skipped because it is a sig track " << endl;
      continue;
    }
    if (pT->fPlab.DeltaR(pC->fPlab) < coneSize) {
      pIdx.push_back(i); 
      ++fCandI0trk;
      sumPt += pt; 
      if (verbose) cout << endl;
    } 
    else {
      if (verbose) cout << " skipped because of deltaR = " << pT->fPlab.DeltaR(pC->fPlab) << endl;
    }
  }

  // -- Now consider the DOCA tracks
  int nsize = pC->fNstTracks.size(); 
  if (nsize>0) {
    for(int i = 0; i<nsize; ++i) {
      int trkId = pC->fNstTracks[i].first;
      double doca = pC->fNstTracks[i].second.first;
      // double docaE = pC->fNstTracks[i].second.second;

      if(doca > docaCut) continue; // check the doca cut

      pT = fpEvt->getRecTrack(trkId);

//       // -- check that any track associated with a definitive vertex is from the same or the closest other PV
//       if (pT->fPvIdx > -1) {
// 	sameOrCloseVertex = (pT->fPvIdx == pvIdx);
// 	//	sameOrCloseVertex = (pT->fPvIdx == pvIdx) || (pT->fPvIdx == pvIdx2);
// 	if (!sameOrCloseVertex) continue;
//       }

      if ((pT->fPvIdx > -1) && (pT->fPvIdx != pvIdx)) { 	 
	if (verbose) cout << " doca track " << trkId << " skipped because it is from a different PV " << pT->fPvIdx <<endl; 	 
	continue; 	 
      }

      pt = pT->fPlab.Perp();  
      if (pt < ptCut) {
	if (verbose) cout << " doca track " << trkId << " skipped because of pt = " << pt << endl;
	continue;
      }

      if (pT->fPlab.DeltaR(pC->fPlab) > coneSize) {
	if (verbose) cout << " doca track " << trkId << " skipped because of deltaR = " << pT->fPlab.DeltaR(pC->fPlab) << endl;
	continue;
      }

      // -- Skip tracks already included above
      if (pIdx.end() != find(pIdx.begin(), pIdx.end(), trkId))  continue;
      if (cIdx.end() != find(cIdx.begin(), cIdx.end(), trkId))  continue;

      ++fCandI1trk;
      sumPt += pt; 
      if (verbose) cout << " doca track " << trkId << " included "<<doca<<" "<<pt<<endl;

    } // for loop over tracks
  } // end if 

  fCandI2trk = fCandI0trk + fCandI1trk;

  iso = candPt/(candPt + sumPt); 

  //   if (verbose) cout << "--> iso = " << candPt << " .. " << sumPt << " = " << iso << endl;
  //   if (verbose) cout << "--> iso = " << pC->fPlab.Perp() << " .. " << sumPt << " = " << pC->fPlab.Perp()/(pC->fPlab.Perp() + sumPt) << endl;

  return iso; 
}
 
// ----------------------------------------------------------------------
double candAna::constrainedMass() {
  vector<int> rectracks;
  int bla;
  for (int it = fpCand->fSig1; it <= fpCand->fSig2; ++it) {
    bla = fpEvt->getSigTrack(it)->fIndex; 
    rectracks.push_back(bla);
  } 
  
  TAnaCand *pC(0), *pDau(0), *pMcCand(0); 
  int mctype = TYPE + 100000; 
  unsigned int nmatch(0); 
  for (int iC = 0; iC < fpEvt->nCands(); ++iC) {
    pC = fpEvt->getCand(iC);
    if (pC->fType != mctype) continue;
    nmatch = 0; 
    for (int it = pC->fSig1; it <= pC->fSig2; ++it) {
      bla = fpEvt->getSigTrack(it)->fIndex; 
      if (rectracks.end() != find(rectracks.begin(), rectracks.end(), bla)) {
	++nmatch; 
      }
    }
    if (pC->fDau1 > 0) {
      pDau = fpEvt->getCand(pC->fDau1); 
      for (int it = pDau->fSig1; it <= pDau->fSig2; ++it) {
	bla = fpEvt->getSigTrack(it)->fIndex; 
	if (rectracks.end() != find(rectracks.begin(), rectracks.end(), bla)) {
	  ++nmatch; 
	}
      }
    }
    if (nmatch == rectracks.size()) {
      pMcCand = pC; 
      break;
    }
  }
  if (pMcCand) {
    return pMcCand->fMass;
  } else {
    return -2.;
  }
}


// ----------------------------------------------------------------------
void candAna::runRange() {
  fRunRange = 6;
  if ((fRun >= 160329) && (fRun <= 163261)) {
    fRunRange = 0; // the beginning
  } 
  if ((fRun >= 163269) && (fRun <= 163869)) {
    fRunRange = 1; // displaced J/psi
  } 
  if ((fRun >= 165088) && (fRun <= 167913)) {
    fRunRange = 2; // HLTDimuon7
  } 
  if ((fRun >= 170249) && (fRun <= 173198)) {
    fRunRange = 3; // 2e33
  } 
  if ((fRun >= 173236) && (fRun <= 178380)) {
    fRunRange = 4; // 3e33 WITH the ETA cut!
  } 
  if ((fRun >= 178420) && (fRun <= 999999)) {
    fRunRange = 5; // 5e33
  } 

  if (fIsMC) {
    if (string::npos != fHLTPath.find("HLT_Dimuon7_Jpsi_Displaced_v1")) {
      fRunRange = 2;
    }
    if (string::npos != fHLTPath.find("HLT_DoubleMu3p5_Jpsi_Displaced_v2_Bs")) {
      fRunRange = 3;
    }
    if (string::npos != fHLTPath.find("HLT_DoubleMu4_Jpsi_Displaced_v1")) {
      fRunRange = 4;
    }
  }

}


// ----------------------------------------------------------------------
void candAna::isolationStudy(double doca) {
  // r = 0.3
  double iso = isoClassicWithDOCA(fpCand, doca, 0.3, 0.3);
  fIsoR03Pt03.iso       = iso; 
  fIsoR03Pt03.pvTracks  = fCandI0trk; 
  fIsoR03Pt03.clTracks  = fCandI1trk; 
  fIsoR03Pt03.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.3, 0.5);
  fIsoR03Pt05.iso       = iso; 
  fIsoR03Pt05.pvTracks  = fCandI0trk; 
  fIsoR03Pt05.clTracks  = fCandI1trk; 
  fIsoR03Pt05.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.3, 0.7);
  fIsoR03Pt07.iso       = iso; 
  fIsoR03Pt07.pvTracks  = fCandI0trk; 
  fIsoR03Pt07.clTracks  = fCandI1trk; 
  fIsoR03Pt07.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.3, 0.9);
  fIsoR03Pt09.iso       = iso; 
  fIsoR03Pt09.pvTracks  = fCandI0trk; 
  fIsoR03Pt09.clTracks  = fCandI1trk; 
  fIsoR03Pt09.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.3, 1.1);
  fIsoR03Pt11.iso       = iso; 
  fIsoR03Pt11.pvTracks  = fCandI0trk; 
  fIsoR03Pt11.clTracks  = fCandI1trk; 
  fIsoR03Pt11.Tracks    = fCandI2trk; 

  // r = 0.5
  iso = isoClassicWithDOCA(fpCand, doca, 0.5, 0.3);
  fIsoR05Pt03.iso       = iso; 
  fIsoR05Pt03.pvTracks  = fCandI0trk; 
  fIsoR05Pt03.clTracks  = fCandI1trk; 
  fIsoR05Pt03.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.5, 0.5);
  fIsoR05Pt05.iso       = iso; 
  fIsoR05Pt05.pvTracks  = fCandI0trk; 
  fIsoR05Pt05.clTracks  = fCandI1trk; 
  fIsoR05Pt05.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.5, 0.7);
  fIsoR05Pt07.iso       = iso; 
  fIsoR05Pt07.pvTracks  = fCandI0trk; 
  fIsoR05Pt07.clTracks  = fCandI1trk; 
  fIsoR05Pt07.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.5, 0.9);
  fIsoR05Pt09.iso       = iso; 
  fIsoR05Pt09.pvTracks  = fCandI0trk; 
  fIsoR05Pt09.clTracks  = fCandI1trk; 
  fIsoR05Pt09.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.5, 1.1);
  fIsoR05Pt11.iso       = iso; 
  fIsoR05Pt11.pvTracks  = fCandI0trk; 
  fIsoR05Pt11.clTracks  = fCandI1trk; 
  fIsoR05Pt11.Tracks    = fCandI2trk; 


  // r = 0.7
  iso = isoClassicWithDOCA(fpCand, doca, 0.7, 0.3);
  fIsoR07Pt03.iso       = iso; 
  fIsoR07Pt03.pvTracks  = fCandI0trk; 
  fIsoR07Pt03.clTracks  = fCandI1trk; 
  fIsoR07Pt03.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.7, 0.5);
  fIsoR07Pt05.iso       = iso; 
  fIsoR07Pt05.pvTracks  = fCandI0trk; 
  fIsoR07Pt05.clTracks  = fCandI1trk; 
  fIsoR07Pt05.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.7, 0.7);
  fIsoR07Pt07.iso       = iso; 
  fIsoR07Pt07.pvTracks  = fCandI0trk; 
  fIsoR07Pt07.clTracks  = fCandI1trk; 
  fIsoR07Pt07.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.7, 0.9);
  fIsoR07Pt09.iso       = iso; 
  fIsoR07Pt09.pvTracks  = fCandI0trk; 
  fIsoR07Pt09.clTracks  = fCandI1trk; 
  fIsoR07Pt09.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.7, 1.1);
  fIsoR07Pt11.iso       = iso; 
  fIsoR07Pt11.pvTracks  = fCandI0trk; 
  fIsoR07Pt11.clTracks  = fCandI1trk; 
  fIsoR07Pt11.Tracks    = fCandI2trk; 

  // r = 0.9
  iso = isoClassicWithDOCA(fpCand, doca, 0.9, 0.3);
  fIsoR09Pt03.iso       = iso; 
  fIsoR09Pt03.pvTracks  = fCandI0trk; 
  fIsoR09Pt03.clTracks  = fCandI1trk; 
  fIsoR09Pt03.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.9, 0.5);
  fIsoR09Pt05.iso       = iso; 
  fIsoR09Pt05.pvTracks  = fCandI0trk; 
  fIsoR09Pt05.clTracks  = fCandI1trk; 
  fIsoR09Pt05.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.9, 0.7);
  fIsoR09Pt07.iso       = iso; 
  fIsoR09Pt07.pvTracks  = fCandI0trk; 
  fIsoR09Pt07.clTracks  = fCandI1trk; 
  fIsoR09Pt07.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.9, 0.9);
  fIsoR09Pt09.iso       = iso; 
  fIsoR09Pt09.pvTracks  = fCandI0trk; 
  fIsoR09Pt09.clTracks  = fCandI1trk; 
  fIsoR09Pt09.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 0.9, 1.1);
  fIsoR09Pt11.iso       = iso; 
  fIsoR09Pt11.pvTracks  = fCandI0trk; 
  fIsoR09Pt11.clTracks  = fCandI1trk; 
  fIsoR09Pt11.Tracks    = fCandI2trk; 


  // r = 1.0
  iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.3);
  fIsoR10Pt03.iso       = iso; 
  fIsoR10Pt03.pvTracks  = fCandI0trk; 
  fIsoR10Pt03.clTracks  = fCandI1trk; 
  fIsoR10Pt03.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.5);
  fIsoR10Pt05.iso       = iso; 
  fIsoR10Pt05.pvTracks  = fCandI0trk; 
  fIsoR10Pt05.clTracks  = fCandI1trk; 
  fIsoR10Pt05.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.7);
  fIsoR10Pt07.iso       = iso; 
  fIsoR10Pt07.pvTracks  = fCandI0trk; 
  fIsoR10Pt07.clTracks  = fCandI1trk; 
  fIsoR10Pt07.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.9);
  fIsoR10Pt09.iso       = iso; 
  fIsoR10Pt09.pvTracks  = fCandI0trk; 
  fIsoR10Pt09.clTracks  = fCandI1trk; 
  fIsoR10Pt09.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 1.1, 1.1);
  fIsoR10Pt11.iso       = iso; 
  fIsoR10Pt11.pvTracks  = fCandI0trk; 
  fIsoR10Pt11.clTracks  = fCandI1trk; 
  fIsoR10Pt11.Tracks    = fCandI2trk; 


  // r = 1.1
  iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.3);
  fIsoR11Pt03.iso       = iso; 
  fIsoR11Pt03.pvTracks  = fCandI0trk; 
  fIsoR11Pt03.clTracks  = fCandI1trk; 
  fIsoR11Pt03.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.5);
  fIsoR11Pt05.iso       = iso; 
  fIsoR11Pt05.pvTracks  = fCandI0trk; 
  fIsoR11Pt05.clTracks  = fCandI1trk; 
  fIsoR11Pt05.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.7);
  fIsoR11Pt07.iso       = iso; 
  fIsoR11Pt07.pvTracks  = fCandI0trk; 
  fIsoR11Pt07.clTracks  = fCandI1trk; 
  fIsoR11Pt07.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 1.1, 0.9);
  fIsoR11Pt09.iso       = iso; 
  fIsoR11Pt09.pvTracks  = fCandI0trk; 
  fIsoR11Pt09.clTracks  = fCandI1trk; 
  fIsoR11Pt09.Tracks    = fCandI2trk; 

  iso = isoClassicWithDOCA(fpCand, doca, 1.1, 1.1);
  fIsoR11Pt11.iso       = iso; 
  fIsoR11Pt11.pvTracks  = fCandI0trk; 
  fIsoR11Pt11.clTracks  = fCandI1trk; 
  fIsoR11Pt11.Tracks    = fCandI2trk; 


 }


// ----------------------------------------------------------------------
int candAna::nearestPV(int pvIdx, double maxDist) {

  if (pvIdx==-1) return -1;  // add protection d.k. 9/6/12

  TAnaVertex *v0 = fpEvt->getPV(pvIdx); 

  if (0 == v0) return -1;  // add protection

  double zV0 = v0->fPoint.Z(); 
  
  int idx(-1); 
  double z(0.), zabs(0.), zmin(99.);
  for (int i = 0; i < fpEvt->nPV(); ++i) {
    if (i == pvIdx) continue;
    z = fpEvt->getPV(i)->fPoint.Z();
    zabs = TMath::Abs(zV0 - z); 
    if (zabs < zmin) {
      idx = i; 
      zmin = zabs;
    }
  }
  
  if (zmin < maxDist) {
    //     cout << "pvIdx = " << pvIdx << " at = " << zV0 
    // 	 << ", nearest other PV with idx = " << idx << " at z = " << fpEvt->getPV(idx)->fPoint.Z() 
    // 	 << " and delta(z) = " << zmin << endl;
    return idx; 
  } else {
    return -1;
  }
}


// ----------------------------------------------------------------------
void candAna::getSigTracks(vector<int> &v, TAnaCand *pC) {
  TAnaCand *pD; 
  TAnaTrack *pT; 
  vector<int> bla; 

  // -- loop over daughters
  if (pC->fDau1 > -1) {
    for (int j = pC->fDau1; j <= pC->fDau2; ++j) {
      pD = fpEvt->getCand(j); 
      getSigTracks(bla, pD); 
    }

    for (unsigned j = 0; j < bla.size(); ++j) v.push_back(bla[j]);
  }

  // -- add direct sigtracks
  for (int i = pC->fSig1; i <= pC->fSig2; ++i) {
    pT = fpEvt->getSigTrack(i); 
    if (v.end() == find(v.begin(), v.end(), pT->fIndex)) {
      v.push_back(pT->fIndex); 
    }
  }

}


// ----------------------------------------------------------------------
void candAna::calcBDT(bool rejectInvIso) {
  fBDT = -99.;
  //??  if (5 == mode && 5.2 < mass && mass < 5.45 && fb.iso < 0.7) continue; 
  if (fChan < 0) return;
  if (0 == fReaderEvents0.size()) {
    cout << "no BDT defined" << endl;
    return;
  }
  if (rejectInvIso && 5.2 < fCandM && fCandM < 5.45 && fCandIso < 0.7) return;
  if (fCandPt > 100) return;
  if (fCandPt < 6) return;
  if (fMu1Pt < 4) return;
  if (fMu2Pt < 4) return;
  if (fCandFL3d > 1.5) return;
  if (fCandFL3d < 0.) return;
  if (fCandM > 5.9) return;
  if (fCandM < 4.9) return;
  
  //   if (!fb.hlt) return;
  //   if (!fb.gmuid) return;
  
  frd.pt = fCandPt; 
  frd.eta = fCandEta; 
  frd.m1eta = fMu1Eta; 
  frd.m2eta = fMu2Eta; 
  frd.m1pt = fMu1Pt; 
  frd.m2pt = fMu2Pt;
  frd.fls3d = fCandFLS3d; 
  frd.alpha = fCandA; 
  frd.maxdoca = fCandDoca;
  frd.pvip = fCandPvIp; 
  frd.pvips = fCandPvIpS; 
  frd.iso = fCandIso; 
  frd.docatrk = fCandDocaTrk; 
  frd.chi2dof = fCandChi2/fCandDof; 
  frd.closetrk = fCandCloseTrk; 
  
  frd.m  = fCandM; 
  //  cout << "Evt = " << fEvt << " %3 = " << fEvt%3 << " chan = " << fChan << " " << " etas = " << fMu1Eta << " " << fMu2Eta;
  if (0 == fEvt%3) {
    fBDT   = fReaderEvents0[fChan]->EvaluateMVA("BDT"); 
  } else if (1 == fEvt%3) {
    fBDT   = fReaderEvents1[fChan]->EvaluateMVA("BDT"); 
  } else if (2 == fEvt%3) {
    fBDT   = fReaderEvents2[fChan]->EvaluateMVA("BDT"); 
  } else {
    cout << "all hell break loose" << endl;
  }
  //  cout << " bdt = " << fBDT << endl;
}


// ----------------------------------------------------------------------
int candAna::detChan(double m1eta, double m2eta) {
  // -- simple two channel analysis: channel 0 if both muons in barrel, channel 1 else
  if (TMath::Abs(m1eta) < 1.4 && TMath::Abs(m2eta) < 1.4) return 0; 
  if (TMath::Abs(m1eta) < 2.4 && TMath::Abs(m2eta) < 2.4) return 1; 
  return -1; 
}


// ----------------------------------------------------------------------
TMVA::Reader* candAna::setupReader(string xmlFile, readerData &rd) {
  TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );
  
  TString dir    = "weights/";
  TString methodNameprefix = "BDT";
  //  TString methodName = TString(fBdt) + TString(" method");
  //  TString weightfile = dir + fBdt + "_" + methodNameprefix + TString(".weights.xml");
  TString weightfile = xmlFile;

  // -- read in variables from weight file
  vector<string> allLines; 
  char  buffer[2000];
  cout << "setupReader, open file " << weightfile << endl;
  ifstream is(weightfile); 
  while (is.getline(buffer, 2000, '\n')) allLines.push_back(string(buffer));
  int nvars(-1); 
  string::size_type m1, m2;
  string stype; 
  cout << "  read " << allLines.size() << " lines " << endl;
  for (unsigned int i = 0; i < allLines.size(); ++i) {
    // -- parse and add variables
    if (string::npos != allLines[i].find("Variables NVar")) {
      m1 = allLines[i].find("=\""); 
      stype = allLines[i].substr(m1+2, allLines[i].size()-m1-2-2); 
      cout << "  " << stype << " variables" << endl;
      nvars = atoi(stype.c_str());
      if (-1 == nvars) continue;
      for (unsigned int j = i+1; j < i+nvars+1; ++j) {
	m1 = allLines[j].find("Expression=\"")+10; 
	m2 = allLines[j].find("\" Label=\"");
	stype = allLines[j].substr(m1+2, m2-m1-2); 
	//	cout << "ivar " << j-i << " variable string: ->" << stype << "<-" << endl;
	if (stype == "m1pt") {
	  cout << "  adding m1pt" << endl;
	  reader->AddVariable( "m1pt", &rd.m1pt);
	}
	if (stype == "m2pt") {
	  cout << "  adding m2pt" << endl;
	  reader->AddVariable( "m2pt", &rd.m2pt);
	}
	if (stype == "m1eta") {
	  cout << "  adding m1eta" << endl;
	  reader->AddVariable( "m1eta", &rd.m1eta);
	}
	if (stype == "m2eta") {
	  reader->AddVariable( "m2eta", &rd.m2eta);
	  cout << "  adding m2eta" << endl;
	}
	if (stype == "pt") {
	  cout << "  adding pt" << endl;
	  reader->AddVariable( "pt", &rd.pt);
	}
	if (stype == "eta") {
	  cout << "  adding eta" << endl;
	  reader->AddVariable( "eta", &rd.eta);
	}
	if (stype == "fls3d") {
	  cout << "  adding fls3d" << endl;
	  reader->AddVariable( "fls3d", &rd.fls3d);
	}
	if (stype == "alpha") {
	  cout << "  adding alpha" << endl;
	  reader->AddVariable( "alpha", &rd.alpha);
	}
	if (stype == "maxdoca") {
	  cout << "  adding maxdoca" << endl;
	  reader->AddVariable( "maxdoca", &rd.maxdoca);
	}
	if (stype == "pvip") {
	  cout << "  adding pvip" << endl;
	  reader->AddVariable( "pvip", &rd.pvip);
	}
	if (stype == "pvips") {
	  cout << "  adding pvips" << endl;
	  reader->AddVariable( "pvips", &rd.pvips);
	}
	if (stype == "iso") {
	  cout << "  adding iso" << endl;
	  reader->AddVariable( "iso", &rd.iso);
	}
	if (stype == "docatrk") {
	  cout << "  adding docatrk" << endl;
	  reader->AddVariable( "docatrk", &rd.docatrk);
	}
	if (stype == "closetrk") {
	  cout << "  adding closetrk" << endl;
	  reader->AddVariable( "closetrk", &rd.closetrk);
	}
	if (stype == "chi2/dof") {
	  cout << "  adding chi2/dof" << endl;
	  reader->AddVariable( "chi2dof := chi2/dof", &rd.chi2dof);
	}
      }
      break;
    }
  }
  
  nvars = -1; 
  for (unsigned int i = 0; i < allLines.size(); ++i) {
    // -- parse and add spectators
    if (string::npos != allLines[i].find("Spectators NSpec")) {
      m1 = allLines[i].find("=\""); 
      stype = allLines[i].substr(m1+2, allLines[i].size()-m1-2-2); 
      //      cout << "==> " << stype << endl;
      nvars = atoi(stype.c_str());
      if (-1 == nvars) continue;
      for (unsigned int j = i+1; j < i+nvars+1; ++j) {
	m1 = allLines[j].find("Expression=\"")+10; 
	m2 = allLines[j].find("\" Label=\"");
	stype = allLines[j].substr(m1+2, m2-m1-2); 
	cout << "ivar " << j-i << " spectator string: ->" << stype << "<-" << endl;
	if (stype == "m") {
	  cout << "  adding m as spectator" << endl;
	  reader->AddSpectator( "m", &rd.m);  
	}
      }
      break;
    }
  }

  // --- Book the MVA methods
  reader->BookMVA("BDT", weightfile); 
  return reader; 
}


// ----------------------------------------------------------------------
void candAna::replaceAll(std::string &s, std::string a, std::string b) {
  
  TString ts(s.c_str()); 
  ts.ReplaceAll(a.c_str(), b.c_str()); 
  s = ts.Data(); 

}
