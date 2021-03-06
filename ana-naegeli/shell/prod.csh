#! /bin/csh -f

setenv CMSSW       
setenv SCRAM_ARCH  
setenv SRMCP       

setenv JOB
setenv FILE1    $JOB.root
setenv STORAGE1

echo "========================"
echo "====> SGE  wrapper <===="
echo "========================"

echo "--> Running grid job wrapper"

# ----------------------------------------------------------------------
# -- The Basics
# ----------------------------------------------------------------------
echo "--> Environment"
date
hostname
cat /proc/cpuinfo
uname -a
df -kl 

echo $VO_CMS_SW_DIR
ls -l $VO_CMS_SW_DIR
source $VO_CMS_SW_DIR/cmsset_default.csh
echo "-> which edg-gridftp-ls"
which edg-gridftp-ls
echo "-> which globus-url-copy"
which globus-url-copy
echo "-> which srmcp"
which srmcp

pwd
echo "--> End of env testing"

# BATCH START

# ----------------------------------------------------------------------
# -- Setup CMSSW
# ----------------------------------------------------------------------
echo "--> Setup CMSSW"
pwd
date
cmsrel $CMSSW
cd $CMSSW/src
eval `scramv1 runtime -csh`
pwd

echo "--> Extract tar file"
date
tar zxf ../../$JOB.tar.gz
cd AnalysisDataFormats/HeavyFlavorObjects
make 
cd - 
scramv1 b 
mv ../../$JOB.py .


# ----------------------------------------------------------------------
# -- Run cmsRun
# ----------------------------------------------------------------------
echo "--> Run cmsRun"
pwd
date
which cmsRun
echo "cmsRun $JOB.py "
cmsRun $JOB.py |& tee $JOB.log
date
pwd
ls -rtl 

# ----------------------------------------------------------------------
# -- Save Output to SE
# ----------------------------------------------------------------------
echo "--> Save output to SE: $STORAGE1/$FILE1"

echo lcg-del -b -D srmv2 -l  "$STORAGE1/$FILE1"
lcg-del -b -D srmv2 -l "$STORAGE1/$FILE1"
echo $SRMCP    $FILE1 "$STORAGE1/$FILE1"
$SRMCP         $FILE1 "$STORAGE1/$FILE1"
echo lcg-ls -b -D srmv2 -l  "$STORAGE1/$FILE1"
lcg-ls -b -D srmv2 -l  "$STORAGE1/$FILE1"

date

# BATCH END

echo "run: This is the end, my friend"
