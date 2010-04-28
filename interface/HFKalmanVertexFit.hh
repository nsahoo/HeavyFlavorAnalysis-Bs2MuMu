#include <vector>

#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"

#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"

using namespace std;
using namespace reco;
using namespace edm;

// ----------------------------------------------------------------------
class HFKalmanVertexFit {
  
public:
  
  HFKalmanVertexFit(const TransientTrackBuilder *TTB, Vertex &PV, int type = 0, int verbose = 0);
  ~HFKalmanVertexFit();

  // -- do a Kalman Vertex Fit
  // Note: ntracksToVertex:    how many tracks are used for vertexing
  //       trackMasses[i] < 0: a second invariant mass is computed without the tracks whose mass < 0
  void doFit(vector<Track> &trackList, vector<int> &trackIndices, vector<double> &trackMasses, 
	     int type = 0, int ntracksToVertex = -1);



  // -- Just combine the 4-vectors
  void doNotFit(vector<Track> &trackList, vector<int> &trackIndices, vector<double> &trackMasses, int type = 0);
  
  int                   fType, fVerbose; 
  Vertex                fPV;   
  const TransientTrackBuilder *fpTTB;   

  double                fMaxDoca; 
};
