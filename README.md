# Digital twins for urban environments: model evaluation

This repository contains the up-to-date model evaluation strategy defined for cities digital twins model evaluation. It is maintained as part of the **DANTE** project to:
- collect all results from model evaluation;
- enable reproducibility of the results;
- allow the community to build on our work;
- foster collaboration and knowledge sharing.

## Structure
    model-evaluation.git
    ├── docs
    │   └── validation_master_document      # strategy definition, overview,
    │                                       # and test cases results
    ├── modules                             # additional modules for
    │                                       # model evaluation
    └── resources
        ├── bin                             # bash executables 
        ├── case_studies                    # available case studies with
        │                                   # all required files
        └── dicts                           # common dictionaries for
                                            # OpenFOAM simulations

 Ready-to-run test cases are located in the resources/case_studies directory
   and must be as follows:

    resources/case_studies/<case_name>
    ├── boundary_conditions
    │   ├── 0.orig                  # test case boundary conditions
    │   └── boundaryData            # mapped boundary data
    ├── data
    │   └── expData.csv             # experimental data for validation
    ├── dicts                       # test case dictionaries
    │   ├── controlDict
    │   :
    │   └── fvSolution
    ├── function_objects            # function objects for post-processing
    │   ├── FO_example_0
    │   ├── FO_example_1
    │   :
    │   └── FO_example_N
    ├── geometry                    # geometry files for the case
    │   ├── geometry_0.stl
    │   ├── geometry_1.stl
    │   :
    │   └── geometry_M.stl
    └── properties                  # test case properties
        ├── transportProperties
        :
        └── turbulenceProperties

## Notes

### Pre-processing

#### Grid smoothing
Grid smoothing is recommended. Grid generation based on the usage of [integration-cfmesh](https://develop.openfoam.com/Community/integration-cfmesh). Add the sub-module to your OpenFOAM installation with from [https://github.com/ptava/integration-cfmesh]:

```bash
    git submodule init modules/integration-cfmesh
```

#### Comparative analysis
To enable quick set-up of OpenFOAM comparative analysis for a prepared case study (available in 'resources/case_studies/', user might exploit *prepare* script to be executed within the repository:

```bash
    ./resources/bin/Prepare <case_name> <folder_name>
```

#### Function objects
Function objects should be included in case studies to enable post-processing of the results. The function objects are defined in the `function_objects` directory of each case study. The user can add or modify function objects as needed. User need to use following format of file names for function objects:

```
    FO_<function_object_name>_<function_object_number>  (e.g. FO_mag_2)
    <function_object_name> is a representative name of the function object
    <function_object_number> is for the order of computation of the function object
```

### Post-processing
To enable quick analysis assessment in single simulations or comparative analyses, user might exploit optional post-processing tool:

Automatically embed the post-processing tool in your OpenFOAM comparative analysis folder setting 'compare=True' in the 'Allrun' script.

Install with:

```bash
    git+https://github.com/ptava/postprocess4validation.git@main#egg=postprocess4validation
```

