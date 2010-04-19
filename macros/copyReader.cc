/*
 *  copyReader.cc
 *  massReader
 *
 *  Created by Christoph on 31.3.10.
 *  Copyright 2010 Christoph Nägeli. All rights reserved.
 *
 */

#include "copyReader.hh"

copyReader::copyReader(TChain *tree, TString evtClassName) : treeReader01(tree, evtClassName), copy_tree(NULL)
{ } // copyReader()

void copyReader::bookHist()
{
	// create the copy tree...
  fpHistFile->cd();
  copy_tree = new TTree("T1","Events");
  
  copy_evt = new TAna01Event(0);
  copy_tree->Branch("TAna01Event","TAna01Event",&copy_evt, 256000/8, 1);
} // bookHist()

void copyReader::eventProcessing()
{

  bool doCopy(false); 
  TAnaCand *pCand(0); 
  int TYPE(10020); 
  for (int j = 0; j < fpEvt->nCands(); ++j) {
    pCand = fpEvt->getCand(j);
    if ( pCand->fType == TYPE) {
      doCopy = true; 
      break;
    }
  }

  if (doCopy) {
    cout << "Found a candidate with type " << TYPE << ", copying, among ncands = " << fpEvt->nCands() << endl;
    *copy_evt = *fpEvt;
    copy_tree->Fill();
  }

} // eventProcessing()

 
// ----------------------------------------------------------------------
void copyReader::closeHistFile() {
  cout << "==> copyReader: Writing " << fpHistFile->GetName() << endl;
  fpHistFile->cd();
  copy_tree->Write();
  fpHistFile->Close();
  delete fpHistFile;

}
