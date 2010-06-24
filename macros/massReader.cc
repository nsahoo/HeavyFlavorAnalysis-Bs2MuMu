#include "massReader.hh"
#include <cstdlib>

#define require_true(COND,LABEL) if( !(COND) ) goto LABEL

using namespace std;

// test a <= x < b
static inline int in_interval(int x, int a, int b)
{
	return (a <= x) && (x < b);
} // in_interval()

massReader::massReader(TChain *tree, TString evtClassName) : treeReader01(tree, evtClassName),reduced_tree(NULL)
{
  fMomentumPtr = &fMomentum;
} // massReader()

massReader::~massReader()
{ } // ~massReader()

void massReader::eventProcessing()
{
	int j,nc;
	
	// Fill a reduced tree
	nc = fpEvt->nCands();
	for (j = 0; j<nc; j++) {
		
		if(loadCandidateVariables(fpEvt->getCand(j)))
			reduced_tree->Fill();
	}
} // massReader::eventProcessing()

int massReader::loadCandidateVariables(TAnaCand *pCand)
{
	TAnaCand *momCand;
	
	// Save in the tree
	fCandidate = pCand->fType;
	fMomentum = pCand->fPlab;

	fMass = pCand->fMass;
	fTruth = checkTruth(pCand);
	fNbrMuons = countMuons(pCand);
	fD3 = pCand->fVtx.fD3d;
	fD3E = pCand->fVtx.fD3dE;
	fDxy = pCand->fVtx.fDxy;
	fDxyE = pCand->fVtx.fDxyE;
	fChi2 = pCand->fVtx.fChi2;
	fNdof = pCand->fVtx.fNdof;
	
	if (pCand->fMom >= 0) {
		momCand = fpEvt->getCand(pCand->fMom);
		fAlpha = pCand->fPlab.Angle(pCand->fVtx.fPoint - momCand->fVtx.fPoint);
	} else
		fAlpha = pCand->fPlab.Angle(pCand->fVtx.fPoint - fpEvt->bestPV()->fPoint);
	
	return 1;
} // loadCandidateVariables()

void massReader::bookHist()
{
	// create the tree
	reduced_tree = new TTree("T","Candidate Mass / Eta / Charge");
	
	// and add the branches
	reduced_tree->Branch("candidate",&fCandidate,"candidate/I");
	reduced_tree->Branch("p","TVector3",&fMomentumPtr);
	reduced_tree->Branch("mass",&fMass,"mass/D");
	reduced_tree->Branch("truth",&fTruth,"truth/I");
	reduced_tree->Branch("ident_muons",&fNbrMuons,"ident_muons/I");
	reduced_tree->Branch("d3",&fD3,"d3/D");
	reduced_tree->Branch("d3e",&fD3E,"d3e/D");
	reduced_tree->Branch("dxy",&fDxy,"dxy/D");
	reduced_tree->Branch("dxye",&fDxyE,"dxye/D");
	reduced_tree->Branch("alpha",&fAlpha,"alpha/D");
	reduced_tree->Branch("chi2",&fChi2,"chi2/D");
	reduced_tree->Branch("Ndof",&fNdof,"Ndof/D");
} // massReader::bookHist()

int massReader::checkTruth(TAnaCand *cand)
{
	int result = 0;
	TAnaTrack *pTrack;
	TGenCand *truthParticle;
	TGenCand *trackParticle;
	int nGens, truth_type, j;
	
	truth_type = cand->fType % 1000;
	nGens = fpEvt->nGenCands();
	
	pTrack = fpEvt->getSigTrack(cand->fSig1);
	pTrack = fpEvt->getRecTrack(pTrack->fIndex);
	
	if (pTrack->fGenIndex < 0 || pTrack->fGenIndex >= nGens) goto bail;
	truthParticle = fpEvt->getGenCand(pTrack->fGenIndex);
	
	while (abs(truthParticle->fID) != truth_type) {
		if (truthParticle->fMom1 < 0 || truthParticle->fMom1 >= nGens) goto bail;
		truthParticle = fpEvt->getGenCand(truthParticle->fMom1);
	}
	
	// check to see if the other tracks originate from the same track...
	for (j = cand->fSig1+1; j <= cand->fSig2; j++) {
		
		pTrack = fpEvt->getSigTrack(j);
		pTrack = fpEvt->getRecTrack(pTrack->fIndex);
		if (pTrack->fGenIndex < 0 || pTrack->fGenIndex >= nGens) goto bail;
		
		trackParticle = fpEvt->getGenCand(pTrack->fGenIndex);
		while (trackParticle->fNumber != truthParticle->fNumber) {
			if (trackParticle->fMom1 < 0 || trackParticle->fMom1 >= nGens) goto bail;
			trackParticle = fpEvt->getGenCand(trackParticle->fMom1);
		}
	}
	
	result = 1;
bail:
	return result;
} // checkTruth()

// count the number of identified muons in the candidate 'cand'
int massReader::countMuons(TAnaCand *cand)
{
	TAnaTrack *track;
	unsigned nbrMu = 0;
	int j;
	
	// check for invalid candidates
	if (cand->fSig1 < 0)
		return 0;
	
	for (j = cand->fSig1; j <= cand->fSig2; j++) {
		
		track = fpEvt->getSigTrack(j); // signal track
		// check if this is supposed to be a muon!
		if (abs(track->fMCID) == 13) {
			track = fpEvt->getRecTrack(track->fIndex); // rec track
			if (track->fMuID > 0 && (track->fMuID & 6))
				nbrMu++;
		}
	}
	
	return nbrMu;
} // countMuons()

void massReader::buildDecay(TGenCand *gen, multiset<int> *particles)
{
	particles->insert(abs(gen->fID));
	for (int j = gen->fDau1; j <= gen->fDau2 && j>= 0; j++)
		buildDecay(fpEvt->getGenCand(j),particles);
} // buildDecay()
