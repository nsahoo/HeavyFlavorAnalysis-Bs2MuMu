/*
 *  HFSequentialVertexFit.h
 *
 *  Created by Christoph on 29.4.10.
 *
 */

#ifndef GUARD_HFSEQUENTIALVERTEXFIT_H
#define GUARD_HFSEQUENTIALVERTEXFIT_H

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/View.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"

#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"

#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicTree.h" 
#include "DataFormats/VertexReco/interface/Vertex.h"

#include "HeavyFlavorAnalysis/Bs2MuMu/interface/HFDecayTree.h"

class HFSequentialVertexFit
{
 public:
  
  HFSequentialVertexFit(edm::Handle<edm::View<reco::Track> > hTracks, const TransientTrackBuilder *TTB, edm::Handle<reco::VertexCollection> pvCollection, const MagneticField *field, int verbose = 0, bool removeCandTracksFromVtx = true);
  virtual ~HFSequentialVertexFit();
  
  void doFit(HFDecayTree *tree);
  
 private:
  bool fitTree(HFDecayTree *tree);
  void saveTree(HFDecayTree *tree);
  
  double getMaxDoca(std::vector<RefCountedKinematicParticle> &kinParticles);
  double getMinDoca(std::vector<RefCountedKinematicParticle> &kinParticles);

  // wrapper for the template routine bellow
  TAnaCand *addCandidate(HFDecayTree *tree, VertexState *wrtVertexState = NULL);
  float getParticleMass(int particleID, float *mass_sigma);
  void computeDaughterDistance(HFDecayTree *tree);

  // to create a covariance matrix for error propagation
  typedef ROOT::Math::SMatrix<double,3,3,ROOT::Math::MatRepSym<double,3> > cov33_t;
  typedef ROOT::Math::SMatrix<double,6,6,ROOT::Math::MatRepSym<double,6> > cov66_t;
  typedef ROOT::Math::SMatrix<double,7,7,ROOT::Math::MatRepSym<double,7> > cov77_t;
  typedef ROOT::Math::SMatrix<double,9,9,ROOT::Math::MatRepSym<double,9> > cov99_t;
  cov99_t makeCovarianceMatrix(cov33_t cov_vtx1, cov77_t cov_vtx2);
  cov33_t GlobalError2SMatrix_33(GlobalError);
  typedef ROOT::Math::SVector<double,9> jac9_t;

  jac9_t makeJacobianVector3d(const AlgebraicVector3 &vtx1, const AlgebraicVector3 &vtx2, const AlgebraicVector3 &momentum);
  jac9_t makeJacobianVector3d(const ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<double>, ROOT::Math::DefaultCoordinateSystemTag> &vtx1,
			    const GlobalPoint &vtx2, const TVector3 &tv3momentum);
  jac9_t makeJacobianVector3d(const GlobalPoint &vtx1, const GlobalPoint &vtx2, const TVector3 &tv3momentum);

  jac9_t makeJacobianVector2d(const AlgebraicVector3 &vtx1, const AlgebraicVector3 &vtx2, const AlgebraicVector3 &momentum);
  jac9_t makeJacobianVector2d(const ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<double>, ROOT::Math::DefaultCoordinateSystemTag> &vtx1,
			    const GlobalPoint &vtx2, const TVector3 &tv3momentum);
  jac9_t makeJacobianVector2d(const GlobalPoint &vtx1, const GlobalPoint &vtx2, const TVector3 &tv3momentum);
  
 private: // instance variables
	int fVerbose;
	const TransientTrackBuilder* fpTTB;
	edm::Handle<edm::View<reco::Track> > fhTracks;
	edm::Handle<reco::VertexCollection> fPVCollection;
	const MagneticField* magneticField;
	bool removeCandTracksFromVtx_;
	RefCountedHFNodeCut nodeCut;

};

#endif

