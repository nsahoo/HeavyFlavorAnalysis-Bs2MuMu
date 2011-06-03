int chan, file, run; 
float mlo, mhi, pt, m1pt, m2pt, iso1, chi2dof, alpha, fls3d; 
float ul, nobs, nexp, eff; 

// ----------------------------------------------------------------------
void readOptimize(int nfiles = 50) {
  TFile f("optimize.root", "RECREATE"); 

  TTree *t = new TTree("t","t");
  

  t->Branch("chan", &chan ,"chan/I");
  t->Branch("file", &file ,"file/I");
  t->Branch("run", &run ,"run/I");
  t->Branch("ul", &ul, "ul/F");
  t->Branch("nobs", &nobs, "nobs/F");
  t->Branch("nexp", &nexp, "nexp/F");
  t->Branch("eff", &eff, "eff/F");
  t->Branch("mlo", &mlo, "mlo/F");
  t->Branch("mhi", &mhi, "mhi/F");
  t->Branch("pt", &pt, "pt/F");
  t->Branch("m1pt", &m1pt, "m1pt/F");
  t->Branch("m2pt", &m2pt, "m1pt/F");
  t->Branch("iso1", &iso1, "iso1/F");
  t->Branch("chi2dof", &chi2dof, "chi2dof/F");
  t->Branch("alpha", &alpha, "alpha/F");
  t->Branch("fls3d", &fls3d, "fls3d/F");
  
  for (int i = 0; i < nfiles; ++i) {
    file = i; readfile(Form("optimizeUL-%d.txt", i), t);
  }

  t->Write(); 
  f.Close();

}

// ----------------------------------------------------------------------
void readfile(const char *fname, TTree *t) {

  char  buffer[200];
  ifstream is(fname);
  string line;

  while (is.getline(buffer, 200, '\n')) {
    line = buffer; 
    //    cout << line << endl;
    if (string::npos != line.find("mBsLo")) {
      sscanf(buffer, "mBsLo %f", &mlo); 
      ul1  = -1.; // reset 
    }
    if (string::npos != line.find("mBsHi")) sscanf(buffer, "mBsHi %f", &mhi); 
    if (string::npos != line.find("pt")) sscanf(buffer, "pt %f", &pt); 
    if (string::npos != line.find("m1pt")) sscanf(buffer, "m1pt %f", &m1pt); 
    if (string::npos != line.find("m2pt")) sscanf(buffer, "m2pt %f", &m2pt); 
    if (string::npos != line.find("iso1")) sscanf(buffer, "iso1 %f", &iso1); 
    if (string::npos != line.find("chi2dof")) sscanf(buffer, "chi2dof %f", &chi2dof); 
    if (string::npos != line.find("alpha")) sscanf(buffer, "alpha %f", &alpha); 
    if (string::npos != line.find("fls3d")) sscanf(buffer, "fls3d %f", &fls3d); 

    if (string::npos != line.find("chan=0")) {
      sscanf(buffer, "==> effTot = %f chan=0 version=%d", &eff, &run); 
      sscanf(buffer, "==> nObs   = %f chan=0 version=%d", &nobs, &run); 
      sscanf(buffer, "==> nExp   = %f chan=0 version=%d", &nexp, &run); 
      sscanf(buffer, "==> UL     = %f chan=0 version=%d", &ul, &run); 
      if (ul > 0) {
	chan = 0; 
	cout << "eff: " << eff << " ul = " << ul << " version = " << run << endl;
	t->Fill();
	ul = -1;
      }
    }

    if (string::npos != line.find("chan=1")) {
      sscanf(buffer, "==> effTot = %f chan=1 version=%d", &eff, &run); 
      sscanf(buffer, "==> nObs   = %f chan=1 version=%d", &nobs, &run); 
      sscanf(buffer, "==> nExp   = %f chan=1 version=%d", &nexp, &run); 
      sscanf(buffer, "==> UL     = %f chan=1 version=%d", &ul, &run); 
      if (ul > 0) {
	chan = 1; 
	cout << "eff: " << eff << " ul = " << ul << " version = " << run << endl;
	t->Fill();
	ul = -1;
      }
    }
  }
  is.close();

}