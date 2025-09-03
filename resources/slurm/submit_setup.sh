#!/bin/bash

#------------------------------------------------------------------------------
module load profile/eng
module load intel-oneapi-compilers
module load autoload openfoam+/${OF_VERSION}

cd $SLURM_SUBMIT_DIR

#------------------------------------------------------------------------------
# Same as "Allrun_analysis" script but adapted for slurm job submission

. ${WM_PROJECT_DIR:?}/bin/tools/RunFunctions        

dirSetup="setups/${SETUP}"
dirSetupOrig="setups.orig/${SETUP}"
dirResults="results/${SETUP}"
dirOrig="$dirSetupOrig/0.orig"
dirConstant="$dirSetupOrig/constant"
dirSystem="$dirSetupOrig/system"
dirResources="$dirSetupOrig/resources"

# Create setup directory if it does not exist and copy case files
if [ ! -d "$dirSetup" ]; then
    mkdir -p "$dirSetup"
    cp -aRfL "setups.orig/common/." "$dirSetup"
    cp -afL "$dirSetupOrig"/All* "$dirSetup" 2>/dev/null || :
    [ -d "$dirOrig" ] && cp -aRfL "$dirOrig/." "$dirSetup/0.orig"
    [ -d "$dirConstant" ] && cp -aRfL "$dirConstant/." "$dirSetup/constant"
    [ -d "$dirSystem" ] && cp -aRfL "$dirSystem/." "$dirSetup/system"
    [ -d "$dirResources" ] && cp -aRfL "$dirResources/." "$dirSetup/resources"
fi

# Create results directory if it does not exist and copy case files
[ -d results ] || mkdir -p results
cp -Rf "$dirSetup" "$dirResults"

# Link common mesh and dynamic code if specified
if [ "${COMMON_MESH}" = "true" ] && [ -d results/mesh ]; then
    echo "Copy common mesh from results/mesh"
    cp -Rf results/mesh/polyMesh "$dirResults/constant/."
fi

if [ "${COMMON_DYNAMIC_CODE}" = "true" ] && [ -d results/dynamicCode ]; then
    echo "Copy common dynamic code from results/dynamicCode"
    cp -Rf results/dynamicCode "$dirResults/."
fi

# Run the case making sure the right nprocessors are used
cd "$dirResults"
sed -i "s/\(n_proc=\)[0-9]\+/\1${SLURM_NTASKS}/" Allrun_case
./Allrun_case

# Store for reuse if not alread present
if [ "${COMMON_MESH}" = "true" ] && [ ! -d results/mesh ]; then
    echo "Storing common mesh in results/common_mesh"
    mkdir -p results/mesh
    cp -Rf "$dirResults/constant/polyMesh" results/mesh/.
fi

if [ "${COMMON_DYNAMIC_CODE}" = "true" ] && [ ! -d results/dynamicCode ]; then
    echo "Storing common dynamic code in results/dynamicCode"
    mkdir -p results/common_dynamic_code
    cp -Rf "$dirResults/system" results/dynamicCode/.
fi

#------------------------------------------------------------------------------





