import os
import FWCore.ParameterSet.Config as cms

process = cms.Process("HFA")

# ----------------------------------------------------------------------
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.threshold = 'INFO'
process.MessageLogger.cerr.FwkReport.reportEvery = 1000
process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )


# ----------------------------------------------------------------------
# -- Database configuration
process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.load("CondCore.DBCommon.CondDBSetup_cfi")

# -- Conditions
process.load("Configuration.StandardSequences.MagneticField_38T_cff")
process.load("Configuration.StandardSequences.Geometry_cff")
process.load("RecoVertex.BeamSpotProducer.BeamSpot_cfi")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = "START53_V7G::All"

# ----------------------------------------------------------------------
# -- Input files
POOLSOURCE

# ----------------------------------------------------------------------
rootFileName = "hadronic-mc-2012-XXXX.root"


process.tree = cms.EDAnalyzer(
    "HFTree",
    verbose      = cms.untracked.int32(1),
    printFrequency = cms.untracked.int32(1000), 
    requireCand  =  cms.untracked.bool(True),
    fileName     = cms.untracked.string(rootFileName)
    )

# ----------------------------------------------------------------------
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load("HeavyFlavorAnalysis.Bs2MuMu.HFRecoStuff_cff")
process.load("HeavyFlavorAnalysis.Bs2MuMu.HFHadronic_cff")
process.load("HeavyFlavorAnalysis.Bs2MuMu.HFPhysicsDeclared_cff")
process.load("HeavyFlavorAnalysis.Bs2MuMu.HFMCTruth_cff")
process.load("HeavyFlavorAnalysis.Bs2MuMu.HFTruthCandidates_cff")

# ----------------------------------------------------------------------
process.genDump = cms.EDAnalyzer(
    "HFDumpGenerator",
    generatorCandidates = cms.untracked.string('genParticles'),
    generatorEvent = cms.untracked.string('generator')
    )

# ----------------------------------------------------------------------
process.p = cms.Path(
    process.genDump*
    process.recoStuffSequence*
    process.hadronicSequence*
    process.truthDstarToD0PiToKPiPi*
    process.truthBd2DstarPiDump*
    process.tree
)
