# Digital twins for urban environments: model evaluation

This repository contains the up-to-date model evaluation strategy defined for cities digital twins model evaluation. It is maintained as part of the **DANTE** project to:
- collect all results from model evaluation;
- enable reproducibility of the results;
- allow the community to build on our work;
- foster collaboration and knowledge sharing.

## Structure
    model-evaluation.git
    ├── docs
    │   └── validation-master-document      # strategy definition, overview,
    │                                       # and test cases results
    ├── modules                             # additional modules for
    │                                       # model evaluation
    └── resources
        ├── bin                             # bash executables 
        ├── case-studies                    # available case studies with
        │                                   # all required files
        └── dicts                           # common dictionaries for
                                            # OpenFOAM simulations
## Notes

### Pre-processing

#### Grid smoothing
Grid smoothing is recommended. Grid generation based on the usage of [integration-cfmesh](https://develop.openfoam.com/Community/integration-cfmesh). Add the sub-module to your OpenFOAM installation with from [https://github.com/ptava/integration-cfmesh]:

```bash
    git submodule init modules/integration-cfmesh
```

#### Comparative analysis
To enable quick set-up of OpenFOAM comparative analysis for a prepared case study (available in 'resources/case-studies/', user might exploit *prepare* function defined in 'resources/bin/UtilFunctions'


### Post-processing
To enable quick analysis assessment in single simulations or comparative analyses, user might exploit optional post-processing tool:

Automatically embed the post-processing tool in your OpenFOAM comparative analysis folder setting 'compare=True' in the 'Allrun' script.

Install with:

```bash
    git+https://github.com/ptava/postprocess4validation.git@main#egg=postprocess4validation
```

