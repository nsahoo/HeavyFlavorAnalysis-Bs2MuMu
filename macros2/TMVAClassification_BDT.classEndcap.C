// Class: ReadBDT2
// Automatically generated by MethodBase::MakeClass
//

/* configuration options =====================================================

#GEN -*-*-*-*-*-*-*-*-*-*-*- general info -*-*-*-*-*-*-*-*-*-*-*-

Method         : BDT2::BDT2
TMVA Release   : 4.1.2         [262402]
ROOT Release   : 5.27/06       [334598]
Creator        : tinti_g
Date           : Wed Jan 18 16:01:16 2012
Host           : Linux lxbuild151.cern.ch 2.6.18-194.11.3.el5.cve20103081 #1 SMP Thu Sep 16 15:17:10 CEST 2010 x86_64 x86_64 x86_64 GNU/Linux
Dir            : /shome/tinti_g/TMVA/test
Training events: 15024
Analysis type  : [Classification]


#OPT -*-*-*-*-*-*-*-*-*-*-*-*- options -*-*-*-*-*-*-*-*-*-*-*-*-

# Set by User:
V: "False" [Verbose output (short form of "VerbosityLevel" below - overrides the latter one)]
H: "False" [Print method-specific help message]
NTrees: "50" [Number of trees in the forest]
BoostType: "AdaBoost" [Boosting type for the trees in the forest]
AdaBoostBeta: "5.000000e-01" [Parameter for AdaBoost algorithm]
SeparationType: "giniindex" [Separation criterion for node splitting]
nEventsMin: "150" [Minimum number of events required in a leaf node (default: Classification: max(40, N_train/(Nvar^2)/10), Regression: 10)]
nCuts: "40" [Number of steps during node cut optimisation]
PruneMethod: "nopruning" [Method used for pruning (removal) of statistically insignificant branches]
MaxDepth: "3" [Max depth of the decision tree allowed]
# Default:
VerbosityLevel: "Default" [Verbosity level]
VarTransform: "None" [List of variable transformations performed before training, e.g., "D_Background,P_Signal,G,N_AllClasses" for: "Decorrelation, PCA-transformation, Gaussianisation, Normalisation, each for the given class of events ('AllClasses' denotes all events of all classes, if no class indication is given, 'All' is assumed)"]
CreateMVAPdfs: "False" [Create PDFs for classifier outputs (signal and background)]
IgnoreNegWeightsInTraining: "False" [Events with negative weights are ignored in the training (but are included for testing and performance evaluation)]
RenormByClass: "False" [Individually re-normalize each event class to the original size after boosting]
AdaBoostR2Loss: "quadratic" [Type of Loss function in AdaBoostR2t (Linear,Quadratic or Exponential)]
UseBaggedGrad: "False" [Use only a random subsample of all events for growing the trees in each iteration. (Only valid for GradBoost)]
GradBaggingFraction: "6.000000e-01" [Defines the fraction of events to be used in each iteration when UseBaggedGrad=kTRUE. (Only valid for GradBoost)]
Shrinkage: "1.000000e+00" [Learning rate for GradBoost algorithm]
UseRandomisedTrees: "False" [Choose at each node splitting a random set of variables]
UseNvars: "3" [Number of variables used if randomised tree option is chosen]
UsePoissonNvars: "True" [Interpret "UseNvars" not as fixed number but as mean of a Possion distribution in each split]
UseNTrainEvents: "15024" [Number of randomly picked training events used in randomised (and bagged) trees]
UseWeightedTrees: "True" [Use weighted trees or simple average in classification from the forest]
UseYesNoLeaf: "True" [Use Sig or Bkg categories, or the purity=S/(S+B) as classification of the leaf node]
NodePurityLimit: "5.000000e-01" [In boosting/pruning, nodes with purity > NodePurityLimit are signal; background otherwise.]
UseFisherCuts: "False" [Use multivariate splits using the Fisher criterion]
MinLinCorrForFisher: "8.000000e-01" [The minimum linear correlation between two variables demanded for use in Fisher criterion in node splitting]
UseExclusiveVars: "False" [Variables already used in fisher criterion are not anymore analysed individually for node splitting]
PruneStrength: "0.000000e+00" [Pruning strength]
PruneBeforeBoost: "False" [Flag to prune the tree before applying boosting algorithm]
PruningValFraction: "5.000000e-01" [Fraction of events to use for optimizing automatic pruning.]
NNodesMax: "100000" [Max number of nodes in tree]
DoBoostMonitor: "False" [Create control plot with ROC integral vs tree number]
NegWeightTreatment: "inverseboostnegweights" [How to treat events with negative weights in the BDT2 training (particular the boosting) : Ignore;  Boost With inverse boostweight; Pair events with negative and positive weights in traning sample and *annihilate* them (experimental!); Randomly pair events with negative and positive weights in leaf node and do not boost them (experimental!) ]
##


#VAR -*-*-*-*-*-*-*-*-*-*-*-* variables *-*-*-*-*-*-*-*-*-*-*-*-

NVar 10
alpha                         alpha                         alpha                         pointing angle                                                  'F'    [5.66287781112e-05,0.199980735779]
fls3d                         fls3d                         fls3d                         3D flight length significance                                   'F'    [5.00079488754,193.958969116]
iso                           iso                           iso                           isolation                                                       'F'    [0.062537997961,1]
m1pt                          m1pt                          m1pt                          m1pt (GeV)                                                      'F'    [4.50148534775,73.1412506104]
m2pt                          m2pt                          m2pt                          m2pt (GeV)                                                      'F'    [4.00001955032,23.7721290588]
m1eta                         m1eta                         m1eta                         m1 #eta                                                         'F'    [-2.39940500259,2.39998412132]
m2eta                         m2eta                         m2eta                         m2 #eta                                                         'F'    [-2.3989815712,2.3987095356]
docatrkbdt                       docatrkbdt                       docatrkbdt                       doca (cm)                                                       'F'    [1.44957757584e-05,8.98750972748]
pvlips                        pvlips                        pvlips                        PV IP significance                                              'F'    [-2.49927377701,2.49796748161]
closetrk                      closetrk                      closetrk                      N^{#DeltaR}_{trk}                                               'F'    [0,20]
NSpec 0


============================================================================ */

#include <vector>
#include <cmath>
#include <string>
#include <iostream>

#define NN2 new BDT2Node
   
#ifndef BDT2Node__def
#define BDT2Node__def
   
class BDT2Node {
   
public:
   
   // constructor of an essentially "empty" node floating in space
   BDT2Node ( BDT2Node* left,BDT2Node* right,
                          int selector, double cutValue, bool cutType, 
                          int nodeType, double purity, double response ) :
   fLeft         ( left         ),
   fRight        ( right        ),
   fSelector     ( selector     ),
   fCutValue     ( cutValue     ),
   fCutType      ( cutType      ),
   fNodeType     ( nodeType     ),
   fPurity       ( purity       ),
   fResponse     ( response     ){
   }

   virtual ~BDT2Node();

   // test event if it decends the tree at this node to the right
   virtual bool GoesRight( const std::vector<double>& inputValues ) const;
   BDT2Node* GetRight( void )  {return fRight; };

   // test event if it decends the tree at this node to the left 
   virtual bool GoesLeft ( const std::vector<double>& inputValues ) const;
   BDT2Node* GetLeft( void ) { return fLeft; };   

   // return  S/(S+B) (purity) at this node (from  training)

   double GetPurity( void ) const { return fPurity; } 
   // return the node type
   int    GetNodeType( void ) const { return fNodeType; }
   double GetResponse(void) const {return fResponse;}

private:

   BDT2Node*   fLeft;     // pointer to the left daughter node
   BDT2Node*   fRight;    // pointer to the right daughter node
   int                     fSelector; // index of variable used in node selection (decision tree)   
   double                  fCutValue; // cut value appplied on this node to discriminate bkg against sig
   bool                    fCutType;  // true: if event variable > cutValue ==> signal , false otherwise
   int                     fNodeType; // Type of node: -1 == Bkg-leaf, 1 == Signal-leaf, 0 = internal 
   double                  fPurity;   // Purity of node from training
   double                  fResponse; // Regression response value of node
}; 
   
//_______________________________________________________________________
   BDT2Node::~BDT2Node()
{
   if (fLeft  != NULL) delete fLeft;
   if (fRight != NULL) delete fRight;
}; 
   
//_______________________________________________________________________
bool BDT2Node::GoesRight( const std::vector<double>& inputValues ) const
{
   // test event if it decends the tree at this node to the right
   bool result;
     result = (inputValues[fSelector] > fCutValue );
   if (fCutType == true) return result; //the cuts are selecting Signal ;
   else return !result;
}
   
//_______________________________________________________________________
bool BDT2Node::GoesLeft( const std::vector<double>& inputValues ) const
{
   // test event if it decends the tree at this node to the left
   if (!this->GoesRight(inputValues)) return true;
   else return false;
}
   
#endif
   
#ifndef IClassifierReader__def
#define IClassifierReader__def

class IClassifierReader {

 public:

   // constructor
   IClassifierReader() : fStatusIsClean( true ) {}
   virtual ~IClassifierReader() {}

   // return classifier response
   virtual double GetMvaValue( const std::vector<double>& inputValues ) const = 0;

   // returns classifier status
   bool IsStatusClean() const { return fStatusIsClean; }

 protected:

   bool fStatusIsClean;
};

#endif

class ReadBDT2 : public IClassifierReader {

 public:

   // constructor
   ReadBDT2( std::vector<std::string>& theInputVars ) 
      : IClassifierReader(),
        fClassName( "ReadBDT2" ),
        fNvars( 10 ),
        fIsNormalised( false )
   {      
      // the training input variables
      const char* inputVars[] = { "alpha", "fls3d", "iso", "m1pt", "m2pt", "m1eta", "m2eta", "docatrkbdt", "pvlips", "closetrk" };

      // sanity checks
      if (theInputVars.size() <= 0) {
         std::cout << "Problem in class \"" << fClassName << "\": empty input vector" << std::endl;
         fStatusIsClean = false;
      }

      if (theInputVars.size() != fNvars) {
         std::cout << "Problem in class \"" << fClassName << "\": mismatch in number of input values: "
                   << theInputVars.size() << " != " << fNvars << std::endl;
         fStatusIsClean = false;
      }

      // validate input variables
      for (size_t ivar = 0; ivar < theInputVars.size(); ivar++) {
         if (theInputVars[ivar] != inputVars[ivar]) {
            std::cout << "Problem in class \"" << fClassName << "\": mismatch in input variable names" << std::endl
                      << " for variable [" << ivar << "]: " << theInputVars[ivar].c_str() << " != " << inputVars[ivar] << std::endl;
            fStatusIsClean = false;
         }
      }

      // initialize min and max vectors (for normalisation)
      fVmin[0] = 5.66287781111896e-05;
      fVmax[0] = 0.199980735778809;
      fVmin[1] = 5.00079488754272;
      fVmax[1] = 193.958969116211;
      fVmin[2] = 0.0625379979610443;
      fVmax[2] = 1;
      fVmin[3] = 4.5014853477478;
      fVmax[3] = 73.1412506103516;
      fVmin[4] = 4.00001955032349;
      fVmax[4] = 23.7721290588379;
      fVmin[5] = -2.39940500259399;
      fVmax[5] = 2.39998412132263;
      fVmin[6] = -2.39898157119751;
      fVmax[6] = 2.39870953559875;
      fVmin[7] = 1.44957757584052e-05;
      fVmax[7] = 8.98750972747803;
      fVmin[8] = -2.49927377700806;
      fVmax[8] = 2.49796748161316;
      fVmin[9] = 0;
      fVmax[9] = 20;

      // initialize input variable types
      fType[0] = 'F';
      fType[1] = 'F';
      fType[2] = 'F';
      fType[3] = 'F';
      fType[4] = 'F';
      fType[5] = 'F';
      fType[6] = 'F';
      fType[7] = 'F';
      fType[8] = 'F';
      fType[9] = 'F';

      // initialize constants
      Initialize();

   }

   // destructor
   virtual ~ReadBDT2() {
      Clear(); // method-specific
   }

   // the classifier response
   // "inputValues" is a vector of input values in the same order as the 
   // variables given to the constructor
   double GetMvaValue( const std::vector<double>& inputValues ) const;

 private:

   // method-specific destructor
   void Clear();

   // common member variables
   const char* fClassName;

   const size_t fNvars;
   size_t GetNvar()           const { return fNvars; }
   char   GetType( int ivar ) const { return fType[ivar]; }

   // normalisation of input variables
   const bool fIsNormalised;
   bool IsNormalised() const { return fIsNormalised; }
   double fVmin[10];
   double fVmax[10];
   double NormVariable( double x, double xmin, double xmax ) const {
      // normalise to output range: [-1, 1]
      return 2*(x - xmin)/(xmax - xmin) - 1.0;
   }

   // type of input variable: 'F' or 'I'
   char   fType[10];

   // initialize internal variables
   void Initialize();
   double GetMvaValue__( const std::vector<double>& inputValues ) const;

   // private members (method specific)
   std::vector<BDT2Node*> fForest;       // i.e. root nodes of decision trees
   std::vector<double>                fBoostWeights; // the weights applied in the individual boosts
};

double ReadBDT2::GetMvaValue__( const std::vector<double>& inputValues ) const
{
   double myMVA = 0;
   double norm  = 0;
   for (unsigned int itree=0; itree<fForest.size(); itree++){
      BDT2Node *current = fForest[itree];
      while (current->GetNodeType() == 0) { //intermediate node
         if (current->GoesRight(inputValues)) current=(BDT2Node*)current->GetRight();
         else current=(BDT2Node*)current->GetLeft();
      }
      myMVA += fBoostWeights[itree] *  current->GetNodeType();
      norm  += fBoostWeights[itree];
   }
   return myMVA /= norm;
};

void ReadBDT2::Initialize()
{
  // itree = 0
  fBoostWeights.push_back(3.5947486051252);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 1,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.193335,-99) , 
1, 50.535, 1, 0, 0.414082,-99) , 
NN2(
0, 
0, 
-1, 25.2377, 0, -1, 0.0299059,-99) , 
7, 0.0479113, 0, 0, 0.049687,-99) , 
NN2(
0, 
0, 
-1, 0.0191014, 1, -1, 0.000557508,-99) , 
0, 0.00980902, 1, 0, 0.000832875,-99)    );
  // itree = 1
  fBoostWeights.push_back(1.98247);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.832784,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.441671,-99) , 
1, 14.2238, 0, 0, 0.721972,-99) , 
NN2(
0, 
0, 
-1, 0.839945, 0, -1, 0.140414,-99) , 
9, 1.17073, 1, 0, 0.43391,-99) , 
NN2(
0, 
0, 
-1, 0.0327708, 1, -1, 0.0124644,-99) , 
0, 0.0195614, 1, 0, 0.0268102,-99)    );
  // itree = 2
  fBoostWeights.push_back(1.25875);
  fForest.push_back( 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0.888464, 0, 1, 0.741167,-99) , 
NN2(
0, 
0, 
-1, 0.706276, 0, -1, 0.191008,-99) , 
2, 0.817081, 0, 0, 0.498711,-99) , 
NN2(
0, 
0, 
-1, 0.908636, 0, -1, 0.0460181,-99) , 
0, 0.0390662, 1, 0, 0.116851,-99)    );
  // itree = 3
  fBoostWeights.push_back(0.792872);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.674168,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.431386,-99) , 
2, 0.966552, 0, 0, 0.517763,-99) , 
NN2(
0, 
0, 
-1, 0.0317045, 1, -1, 0.139826,-99) , 
2, 0.657026, 0, 0, 0.439564,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.579227,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.209153,-99) , 
0, 0.0930914, 1, 0, 0.325172,-99) , 
NN2(
0, 
0, 
-1, 0.791352, 0, -1, 0.0417544,-99) , 
2, 0.934093, 0, 0, 0.0950757,-99) , 
0, 0.058571, 1, 0, 0.208654,-99)    );
  // itree = 4
  fBoostWeights.push_back(0.588836);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.660805,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.461323,-99) , 
1, 9.61041, 0, 0, 0.579301,-99) , 
NN2(
0, 
0, 
-1, 0.0279458, 1, -1, 0.189042,-99) , 
2, 0.588431, 0, 0, 0.53991,-99) , 
NN2(
0, 
0, 
-1, 0.862954, 0, -1, 0.161391,-99) , 
0, 0.0536948, 1, 0, 0.29652,-99)    );
  // itree = 5
  fBoostWeights.push_back(0.480739);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.611421,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.363716,-99) , 
1, 18.8278, 1, 0, 0.56423,-99) , 
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.51041,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.20582,-99) , 
2, 0.950946, 0, 0, 0.339373,-99) , 
0, 0.0829369, 1, 0, 0.476477,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.602938,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.151397,-99) , 
3, 12.67, 1, 0, 0.526637,-99) , 
NN2(
0, 
0, 
-1, 0.06717, 1, -1, 0.0402212,-99) , 
0, 0.0443906, 1, 0, 0.157441,-99) , 
2, 0.748486, 0, 0, 0.364824,-99)    );
  // itree = 6
  fBoostWeights.push_back(0.412937);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.539236,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.259759,-99) , 
5, 1.7828, 1, 0, 0.474254,-99) , 
NN2(
0, 
0, 
-1, -0.555372, 0, -1, 0.252902,-99) , 
5, -1.81286, 0, 0, 0.433324,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.685737,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.396173,-99) , 
2, 0.631288, 0, 0, 0.546438,-99) , 
NN2(
0, 
0, 
-1, 0.0671685, 1, -1, 0.0909575,-99) , 
0, 0.029804, 1, 0, 0.168037,-99) , 
2, 0.748486, 0, 0, 0.348332,-99)    );
  // itree = 7
  fBoostWeights.push_back(0.368125);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.524599,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.325564,-99) , 
2, 0.657026, 0, 0, 0.494154,-99) , 
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.645164,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.163307,-99) , 
0, 0.0116385, 1, 0, 0.251601,-99) , 
3, 14.5485, 1, 0, 0.465611,-99) , 
NN2(
0, 
0, 
-1, 0.780309, 0, -1, 0.223133,-99) , 
0, 0.0683234, 1, 0, 0.345548,-99)    );
  // itree = 8
  fBoostWeights.push_back(0.294838);
  fForest.push_back( 
NN2(
0, 
0, 
-1, 0.121962, 1, -1, 0.356709,-99)    );
  // itree = 9
  fBoostWeights.push_back(0.151387);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.501344,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.219533,-99) , 
8, 1.16629, 1, 0, 0.484583,-99) , 
NN2(
0, 
0, 
-1, 0.800271, 0, -1, 0.12259,-99) , 
8, -1.40232, 0, 0, 0.472888,-99) , 
NN2(
0, 
0, 
-1, 0.868185, 0, -1, 0.250085,-99) , 
0, 0.121962, 1, 0, 0.42682,-99)    );
  // itree = 10
  fBoostWeights.push_back(0.285388);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.542319,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.364948,-99) , 
5, -1.57886, 0, 0, 0.485239,-99) , 
NN2(
0, 
0, 
-1, 0.420153, 1, -1, 0.312009,-99) , 
6, 1.81363, 1, 0, 0.457172,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.742191,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.401317,-99) , 
0, 0.0155695, 1, 0, 0.467092,-99) , 
NN2(
0, 
0, 
-1, 0.110339, 1, -1, 0.0798912,-99) , 
0, 0.0687015, 1, 0, 0.276995,-99) , 
2, 0.748486, 0, 0, 0.409019,-99)    );
  // itree = 11
  fBoostWeights.push_back(0.240388);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.53091,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.401932,-99) , 
3, 7.76714, 1, 0, 0.476704,-99) , 
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.610599,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.241964,-99) , 
0, 0.0228845, 1, 0, 0.33674,-99) , 
5, 1.81469, 1, 0, 0.451608,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.503373,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0,-99) , 
5, -1.69791, 0, 0, 0.449116,-99) , 
NN2(
0, 
0, 
-1, 0.774364, 0, -1, 0.102287,-99) , 
2, 0.868185, 0, 0, 0.277016,-99) , 
0, 0.117085, 1, 0, 0.413746,-99)    );
  // itree = 12
  fBoostWeights.push_back(0.180987);
  fForest.push_back( 
NN2(
0, 
0, 
-1, 0.102457, 1, -1, 0.410482,-99)    );
  // itree = 13
  fBoostWeights.push_back(0.136779);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.516354,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.204896,-99) , 
8, -1.32117, 0, 0, 0.50789,-99) , 
NN2(
0, 
0, 
-1, 1.46576, 1, -1, 0.0809551,-99) , 
8, 1.52126, 1, 0, 0.500428,-99) , 
NN2(
0, 
0, 
-1, 0.75834, 0, -1, 0.331312,-99) , 
0, 0.102457, 1, 0, 0.454876,-99)    );
  // itree = 14
  fBoostWeights.push_back(0.123008);
  fForest.push_back( 
NN2(
NN2(
0, 
0, 
-1, 1.40101, 1, -1, 0.457211,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.546326,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.000580476,-99) , 
6, -1.4459, 0, 0, 0.42514,-99) , 
NN2(
0, 
0, 
-1, 1.93252, 0, -1, 0.0431309,-99) , 
0, 0.0592679, 1, 0, 0.204378,-99) , 
2, 0.542701, 0, 0, 0.440782,-99)    );
  // itree = 15
  fBoostWeights.push_back(0.128578);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.522152,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.384254,-99) , 
6, -1.8139, 0, 0, 0.499932,-99) , 
NN2(
0, 
0, 
-1, 0.879321, 1, -1, 0.237983,-99) , 
8, 1.40101, 1, 0, 0.487861,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.515703,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.000656407,-99) , 
6, -1.4459, 0, 0, 0.406377,-99) , 
NN2(
0, 
0, 
-1, 1.93252, 0, -1, 0.0485026,-99) , 
0, 0.0592679, 1, 0, 0.203024,-99) , 
2, 0.542701, 0, 0, 0.469958,-99)    );
  // itree = 16
  fBoostWeights.push_back(0.118852);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.507708,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.299785,-99) , 
0, 0.16095, 1, 0, 0.492009,-99) , 
NN2(
0, 
0, 
-1, 0.536593, 1, -1, 0.34065,-99) , 
5, 1.81488, 1, 0, 0.468394,-99) , 
NN2(
0, 
0, 
-1, 0.0592679, 1, -1, 0.201496,-99) , 
2, 0.542701, 0, 0, 0.452148,-99)    );
  // itree = 17
  fBoostWeights.push_back(0.129463);
  fForest.push_back( 
NN2(
NN2(
0, 
0, 
-1, 1.76666, 1, -1, 0.453171,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.544701,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.197891,-99) , 
1, 6.98338, 0, 0, 0.458285,-99) , 
NN2(
0, 
0, 
-1, 1.93252, 0, -1, 0.051554,-99) , 
0, 0.0592679, 1, 0, 0.241785,-99) , 
2, 0.565566, 0, 0, 0.437896,-99)    );
  // itree = 18
  fBoostWeights.push_back(0.0732335);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.502293,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.244825,-99) , 
8, -1.45894, 0, 0, 0.491722,-99) , 
NN2(
0, 
0, 
-1, 0.993789, 1, -1, 0.148846,-99) , 
8, 1.76666, 1, 0, 0.485403,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.516186,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.172999,-99) , 
6, -1.4459, 0, 0, 0.441245,-99) , 
NN2(
0, 
0, 
-1, 1.93252, 0, -1, 0.0582644,-99) , 
0, 0.0592679, 1, 0, 0.241736,-99) , 
2, 0.565566, 0, 0, 0.468313,-99)    );
  // itree = 19
  fBoostWeights.push_back(0.184817);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.575498,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.440053,-99) , 
6, -0.943487, 0, 0, 0.494857,-99) , 
NN2(
0, 
0, 
-1, 0.418788, 1, -1, 0.391887,-99) , 
6, 1.57959, 1, 0, 0.46921,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.559375,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.279161,-99) , 
0, 0.0322712, 1, 0, 0.430509,-99) , 
NN2(
0, 
0, 
-1, 1.93252, 0, -1, 0.0624152,-99) , 
0, 0.0592679, 1, 0, 0.241209,-99) , 
2, 0.565566, 0, 0, 0.453493,-99)    );
  // itree = 20
  fBoostWeights.push_back(0.168186);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.532839,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.148047,-99) , 
1, 51.0882, 1, 0, 0.52153,-99) , 
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.540463,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.245394,-99) , 
8, 0.179298, 1, 0, 0.405468,-99) , 
5, 1.46382, 1, 0, 0.487103,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.513534,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.209004,-99) , 
9, 4.39024, 0, 0, 0.424965,-99) , 
NN2(
0, 
0, 
-1, 1.93252, 0, -1, 0.0741459,-99) , 
0, 0.0592679, 1, 0, 0.249735,-99) , 
2, 0.565566, 0, 0, 0.471349,-99)    );
  // itree = 21
  fBoostWeights.push_back(0.313642);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.600984,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.40539,-99) , 
8, -0.67397, 0, 0, 0.561457,-99) , 
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.622003,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.326687,-99) , 
4, 6.8935, 0, 0, 0.356238,-99) , 
0, 0.0390934, 0, 0, 0.491501,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.666963,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.364824,-99) , 
2, 0.823684, 1, 0, 0.602543,-99) , 
NN2(
0, 
0, 
-1, -1.93126, 0, -1, 0.303824,-99) , 
0, 0.0293138, 1, 0, 0.38892,-99) , 
2, 0.86281, 0, 0, 0.444466,-99)    );
  // itree = 22
  fBoostWeights.push_back(0.0860997);
  fForest.push_back( 
NN2(
0, 
0, 
-1, 0.146343, 1, -1, 0.457056,-99)    );
  // itree = 23
  fBoostWeights.push_back(0.0961585);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.515931,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.329981,-99) , 
5, 2.16595, 1, 0, 0.506459,-99) , 
NN2(
0, 
0, 
-1, -1.33047, 0, -1, 0.322726,-99) , 
2, 0.565566, 0, 0, 0.495237,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.509434,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.220781,-99) , 
2, 0.868185, 0, 0, 0.398239,-99) , 
NN2(
0, 
0, 
-1, 5.67124, 1, -1, 0.0809772,-99) , 
5, 1.45916, 1, 0, 0.332032,-99) , 
0, 0.146343, 1, 0, 0.478488,-99)    );
  // itree = 24
  fBoostWeights.push_back(0.0895889);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.504381,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.404984,-99) , 
0, 0.112166, 1, 0, 0.485119,-99) , 
NN2(
0, 
0, 
-1, -0.26092, 0, -1, 0.307088,-99) , 
5, -2.16529, 0, 0, 0.474675,-99) , 
NN2(
0, 
0, 
-1, 1.69185, 1, -1, 0.283382,-99) , 
8, 1.15724, 1, 0, 0.461517,-99)    );
  // itree = 25
  fBoostWeights.push_back(0.0961518);
  fForest.push_back( 
NN2(
0, 
0, 
-1, 1.76666, 1, -1, 0.452072,-99)    );
  // itree = 26
  fBoostWeights.push_back(0.090872);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.515853,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.425225,-99) , 
5, 1.46352, 1, 0, 0.489677,-99) , 
NN2(
0, 
0, 
-1, 0.0592455, 1, -1, 0.287215,-99) , 
2, 0.519837, 0, 0, 0.481826,-99) , 
NN2(
0, 
0, 
-1, 0.994308, 1, -1, 0.150606,-99) , 
8, 1.76666, 1, 0, 0.475981,-99)    );
  // itree = 27
  fBoostWeights.push_back(0.0739577);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.503278,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.376438,-99) , 
6, -1.81365, 0, 0, 0.487876,-99) , 
NN2(
0, 
0, 
-1, -0.445861, 0, -1, 0.368683,-99) , 
5, -1.93117, 0, 0, 0.473394,-99) , 
NN2(
0, 
0, 
-1, 0.994308, 1, -1, 0.162603,-99) , 
8, 1.76666, 1, 0, 0.468061,-99)    );
  // itree = 28
  fBoostWeights.push_back(0.0917019);
  fForest.push_back( 
NN2(
NN2(
0, 
0, 
-1, 0.405512, 0, -1, 0.466356,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.682895,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.0828703,-99) , 
5, -1.27381, 0, 0, 0.471747,-99) , 
NN2(
0, 
0, 
-1, 1.03849, 1, -1, 0.129656,-99) , 
5, 0.879321, 1, 0, 0.273401,-99) , 
8, 1.40101, 1, 0, 0.45839,-99)    );
  // itree = 29
  fBoostWeights.push_back(0.045851);
  fForest.push_back( 
NN2(
NN2(
0, 
0, 
-1, 0.405512, 0, -1, 0.489233,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.662713,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.0901118,-99) , 
5, -1.27381, 0, 0, 0.464088,-99) , 
NN2(
0, 
0, 
-1, 1.03849, 1, -1, 0.14036,-99) , 
5, 0.879321, 1, 0, 0.277171,-99) , 
8, 1.40101, 1, 0, 0.480703,-99)    );
  // itree = 30
  fBoostWeights.push_back(0.057959);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.509366,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.225421,-99) , 
1, 55.6969, 1, 0, 0.504568,-99) , 
NN2(
0, 
0, 
-1, 9.26167, 0, -1, 0.158658,-99) , 
2, 0.405512, 0, 0, 0.500694,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.652389,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.0939425,-99) , 
5, -1.27381, 0, 0, 0.460124,-99) , 
NN2(
0, 
0, 
-1, 1.03849, 1, -1, 0.145984,-99) , 
5, 0.879321, 1, 0, 0.27914,-99) , 
8, 1.40101, 1, 0, 0.491901,-99)    );
  // itree = 31
  fBoostWeights.push_back(0.0492524);
  fForest.push_back( 
NN2(
NN2(
0, 
0, 
-1, 0.405512, 0, -1, 0.487003,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.648838,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.128985,-99) , 
6, 0.990821, 1, 0, 0.417549,-99) , 
NN2(
0, 
0, 
-1, 0.00363464, 1, -1, 0.111003,-99) , 
9, 2.43902, 1, 0, 0.281708,-99) , 
8, 1.40101, 1, 0, 0.478987,-99)    );
  // itree = 32
  fBoostWeights.push_back(0.0598307);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.509773,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.338477,-99) , 
6, 2.16468, 1, 0, 0.502877,-99) , 
NN2(
0, 
0, 
-1, 9.26167, 0, -1, 0.173498,-99) , 
2, 0.405512, 0, 0, 0.499314,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.622179,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.0380438,-99) , 
6, -1.54156, 0, 0, 0.455003,-99) , 
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.510904,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.00780748,-99) , 
6, 1.03849, 1, 0, 0.153379,-99) , 
5, 0.879321, 1, 0, 0.283094,-99) , 
8, 1.40101, 1, 0, 0.490989,-99)    );
  // itree = 33
  fBoostWeights.push_back(0.0501236);
  fForest.push_back( 
NN2(
NN2(
0, 
0, 
-1, -1.54812, 0, -1, 0.485859,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.631609,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.134844,-99) , 
6, 0.990821, 1, 0, 0.41276,-99) , 
NN2(
0, 
0, 
-1, 0.00363464, 1, -1, 0.113845,-99) , 
9, 2.43902, 1, 0, 0.280047,-99) , 
8, 1.40101, 1, 0, 0.478061,-99)    );
  // itree = 34
  fBoostWeights.push_back(0.0591058);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.509268,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.243141,-99) , 
1, 55.6969, 1, 0, 0.504781,-99) , 
NN2(
0, 
0, 
-1, 0.810348, 0, -1, 0.327103,-99) , 
8, -1.54812, 0, 0, 0.498386,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.635126,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.0923421,-99) , 
5, -1.27381, 0, 0, 0.447978,-99) , 
NN2(
0, 
0, 
-1, 1.03849, 1, -1, 0.152364,-99) , 
5, 0.879321, 1, 0, 0.281475,-99) , 
8, 1.40101, 1, 0, 0.490282,-99)    );
  // itree = 35
  fBoostWeights.push_back(0.107958);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.511417,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.236349,-99) , 
1, 51.0882, 1, 0, 0.504556,-99) , 
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.539724,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.223405,-99) , 
0, 0.0686754, 1, 0, 0.428468,-99) , 
2, 0.771351, 0, 0, 0.48514,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.623485,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.141234,-99) , 
6, 0.990821, 1, 0, 0.414912,-99) , 
NN2(
0, 
0, 
-1, 0.00363464, 1, -1, 0.117904,-99) , 
9, 2.43902, 1, 0, 0.284017,-99) , 
8, 1.40101, 1, 0, 0.477745,-99)    );
  // itree = 36
  fBoostWeights.push_back(0.141377);
  fForest.push_back( 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0.0031486, 1, 1, 0.860608,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.497226,-99) , 
6, -1.4581, 0, 0, 0.678801,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.550609,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.438776,-99) , 
9, 0.487805, 1, 0, 0.461812,-99) , 
NN2(
0, 
0, 
-1, 13.8241, 1, -1, 0.30258,-99) , 
3, 14.6822, 1, 0, 0.452087,-99) , 
0, 0.00493283, 1, 0, 0.457516,-99)    );
  // itree = 37
  fBoostWeights.push_back(0.0445885);
  fForest.push_back( 
NN2(
NN2(
NN2(
0, 
0, 
-1, -2.16529, 0, -1, 0.489266,-99) , 
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.517698,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.109358,-99) , 
2, 0.894638, 0, 0, 0.3122,-99) , 
8, -1.45894, 0, 0, 0.482564,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.504598,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0,-99) , 
0, 0.136278, 1, 0, 0.340063,-99) , 
NN2(
0, 
0, 
-1, 0.640277, 1, -1, 0.0113358,-99) , 
6, 0.994308, 1, 0, 0.18126,-99) , 
8, 1.76666, 1, 0, 0.478421,-99)    );
  // itree = 38
  fBoostWeights.push_back(0.0518625);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.508334,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.366234,-99) , 
5, -2.16529, 0, 0, 0.500412,-99) , 
NN2(
0, 
0, 
-1, 0.810348, 0, -1, 0.310537,-99) , 
8, -1.45894, 0, 0, 0.493284,-99) , 
NN2(
0, 
0, 
-1, 0.994308, 1, -1, 0.180048,-99) , 
8, 1.76666, 1, 0, 0.489035,-99)    );
  // itree = 39
  fBoostWeights.push_back(0.0656522);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.506849,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.416322,-99) , 
6, -1.73124, 0, 0, 0.489264,-99) , 
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.529864,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.176499,-99) , 
0, 0.0343442, 1, 0, 0.324858,-99) , 
6, 2.16468, 1, 0, 0.482666,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.506417,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0,-99) , 
0, 0.136278, 1, 0, 0.346614,-99) , 
NN2(
0, 
0, 
-1, 0.640277, 1, -1, 0.0124693,-99) , 
6, 0.994308, 1, 0, 0.187832,-99) , 
8, 1.76666, 1, 0, 0.478728,-99)    );
  // itree = 40
  fBoostWeights.push_back(0.0785163);
  fForest.push_back( 
NN2(
NN2(
0, 
0, 
-1, -1.4581, 0, 1, 0.681896,-99) , 
NN2(
0, 
0, 
-1, 0.382647, 0, -1, 0.464238,-99) , 
0, 0.00493283, 1, 0, 0.469326,-99)    );
  // itree = 41
  fBoostWeights.push_back(0.0430022);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.505179,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.351613,-99) , 
5, 2.16587, 1, 0, 0.497967,-99) , 
NN2(
0, 
0, 
-1, 0.810348, 0, -1, 0.329297,-99) , 
8, -1.45894, 0, 0, 0.491757,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.509632,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0,-99) , 
0, 0.136278, 1, 0, 0.356653,-99) , 
NN2(
0, 
0, 
-1, 0.640277, 1, -1, 0.0143752,-99) , 
6, 0.994308, 1, 0, 0.198101,-99) , 
8, 1.76666, 1, 0, 0.487991,-99)    );
  // itree = 42
  fBoostWeights.push_back(0.0597375);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.505699,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.409236,-99) , 
3, 11.1994, 1, 0, 0.49121,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.248677,-99) , 
1, 55.6969, 1, 0, 0.487159,-99) , 
NN2(
0, 
0, 
-1, 1.69185, 1, -1, 0.349251,-99) , 
8, 1.15724, 1, 0, 0.479136,-99)    );
  // itree = 43
  fBoostWeights.push_back(0.0922602);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.743935,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.26455,-99) , 
7, 0.0834664, 1, 0, 0.671756,-99) , 
NN2(
0, 
0, 
-1, 14.2183, 1, -1, 0.47221,-99) , 
0, 0.007191, 1, 0, 0.481007,-99) , 
NN2(
0, 
0, 
-1, 9.54879, 1, -1, 0.371738,-99) , 
0, 0.146343, 1, 0, 0.470514,-99)    );
  // itree = 44
  fBoostWeights.push_back(0.0385074);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.506067,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.00262068,-99) , 
8, -2.01174, 0, 0, 0.503368,-99) , 
NN2(
0, 
0, 
-1, 0.811607, 0, -1, 0.371252,-99) , 
0, 0.160971, 1, 0, 0.495241,-99) , 
NN2(
0, 
0, 
-1, 9.26167, 0, -1, 0.170438,-99) , 
2, 0.405512, 0, 0, 0.491968,-99)    );
  // itree = 45
  fBoostWeights.push_back(0.0803799);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.518521,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.4537,-99) , 
1, 9.60953, 1, 0, 0.493181,-99) , 
NN2(
0, 
0, 
-1, 9.26167, 0, -1, 0.177553,-99) , 
2, 0.405512, 0, 0, 0.490073,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.570705,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.195266,-99) , 
0, 0.0537717, 1, 0, 0.463427,-99) , 
NN2(
0, 
0, 
-1, 0.752178, 1, -1, 0.119887,-99) , 
8, 0.196281, 1, 0, 0.330449,-99) , 
6, 2.16468, 1, 0, 0.483793,-99)    );
  // itree = 46
  fBoostWeights.push_back(0.085891);
  fForest.push_back( 
NN2(
NN2(
NN2(
0, 
0, 
-1, -1.4581, 0, 1, 0.658539,-99) , 
NN2(
0, 
0, 
-1, 2.16468, 1, -1, 0.482369,-99) , 
0, 0.007191, 1, 0, 0.490075,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.738254,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.342707,-99) , 
6, -0.686186, 0, 0, 0.474145,-99) , 
NN2(
0, 
0, 
-1, 0.875837, 0, -1, 0.193205,-99) , 
5, 1.10852, 1, 0, 0.382483,-99) , 
0, 0.146343, 1, 0, 0.479849,-99)    );
  // itree = 47
  fBoostWeights.push_back(0.0732259);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.52098,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.390925,-99) , 
9, 8.29268, 1, 0, 0.511967,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.00356086,-99) , 
8, -2.01174, 0, 0, 0.509677,-99) , 
NN2(
0, 
0, 
-1, 9.54879, 1, -1, 0.39226,-99) , 
0, 0.146343, 1, 0, 0.498692,-99)    );
  // itree = 48
  fBoostWeights.push_back(0.102166);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.530275,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.450258,-99) , 
5, 1.44928, 1, 0, 0.504537,-99) , 
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.504187,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.0953212,-99) , 
8, -0.433463, 0, 0, 0.406776,-99) , 
5, -1.93117, 0, 0, 0.492442,-99) , 
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.551437,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.211662,-99) , 
0, 0.0537717, 1, 0, 0.455844,-99) , 
NN2(
0, 
0, 
-1, 0.752178, 1, -1, 0.129741,-99) , 
8, 0.196281, 1, 0, 0.331191,-99) , 
6, 2.16468, 1, 0, 0.486218,-99)    );
  // itree = 49
  fBoostWeights.push_back(0.0821447);
  fForest.push_back( 
NN2(
NN2(
NN2(
NN2(
0, 
0, 
-1, 0, 1, 1, 0.504972,-99) , 
NN2(
0, 
0, 
-1, 0, 1, -1, 0.329805,-99) , 
6, 2.16469, 1, 0, 0.497507,-99) , 
NN2(
0, 
0, 
-1, 14.4329, 1, -1, 0.355271,-99) , 
3, 14.5463, 1, 0, 0.488312,-99) , 
NN2(
NN2(
0, 
0, 
-1, 7.66311, 0, 1, 0.648488,-99) , 
NN2(
0, 
0, 
-1, -0.433463, 0, -1, 0.289302,-99) , 
0, 0.0245723, 1, 0, 0.392322,-99) , 
5, -1.93117, 0, 0, 0.476952,-99)    );
   return;
};
 
// Clean up
inline void ReadBDT2::Clear() 
{
   for (unsigned int itree=0; itree<fForest.size(); itree++) { 
      delete fForest[itree]; 
   }
}
   inline double ReadBDT2::GetMvaValue( const std::vector<double>& inputValues ) const
   {
      // classifier response value
      double retval = 0;

      // classifier response, sanity check first
      if (!IsStatusClean()) {
         std::cout << "Problem in class \"" << fClassName << "\": cannot return classifier response"
                   << " because status is dirty" << std::endl;
         retval = 0;
      }
      else {
         if (IsNormalised()) {
            // normalise variables
            std::vector<double> iV;
            int ivar = 0;
            for (std::vector<double>::const_iterator varIt = inputValues.begin();
                 varIt != inputValues.end(); varIt++, ivar++) {
               iV.push_back(NormVariable( *varIt, fVmin[ivar], fVmax[ivar] ));
            }
            retval = GetMvaValue__( iV );
         }
         else {
            retval = GetMvaValue__( inputValues );
         }
      }

      return retval;
   }
