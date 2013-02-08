#ifndef _HFDUMPMUONS_h_
#define _HFDUMPMUONS_h_

#include <memory>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"

#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"

#include "MuonAnalysis/MuonAssociators/interface/PropagateToMuon.h"

#include "AnalysisDataFormats/HeavyFlavorObjects/rootio/TAnaMuon.hh"


class TFile;
class TTree;
class TAna01Event;

// ----------------------------------------------------------------------
class HFDumpMuons : public edm::EDAnalyzer {
 public:
  explicit HFDumpMuons(const edm::ParameterSet&);
  ~HFDumpMuons();
  
 private:
  virtual void              beginJob();
  virtual void              beginRun(const edm::Run&, const edm::EventSetup&);
  virtual void              analyze(const edm::Event&, const edm::EventSetup&);
  virtual void              endJob();
  void                      fillMuon(const reco::Muon& tr, int type);
  void                      fillCaloMuon(const reco::CaloMuon& tr, int type);
  void                      extrapolateTracks(); 
  std::vector<unsigned int> muonStatHits(const reco::Track& tr);
  edm::InputTag             fTracksLabel;
  edm::InputTag             fMuonsLabel;
  edm::InputTag             fCaloMuonsLabel;
  
  edm::Handle<edm::View<reco::Track> > *fhTracks;
  edm::ESHandle<TransientTrackBuilder> fTTB;
  
  double                    fMaxTrackDistToStore;

  int                       fVerbose, fDoTruthMatching; 
  bool                      fRunOnAOD;

  PropagateToMuon           fpropM1, fpropM2;
  std::vector<xpTrack>     fXpTracks;
};

#endif
