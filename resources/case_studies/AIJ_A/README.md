# Notes

<p align="center">
    <img src="../../../docs/validation-master-document/imgs/aij_A_geometry.png" width="50%" height="50%">
</p>

* Scale-Adaptive unsteady RANS

* Inflow conditions are experimental; user can specify OF boundary conditions implemented in the `libatmosphericModels` library with required parameters in `./ABLConditions`.

* Wind tunnel lateral and top boundaries effects on domain of interest are not considered.

* Structured grid for high-wall resolution built with `blockMesh` utility; two parameters available:
    - `cells_scaling`: scaling factor for the number of cells in the x, y, and z directions
    - `building_ratio`: ratio of the building height to the building width (default is 2 which corresponds to the reference experiment); the grid maintains the same distance between building and top boundary

<p align="center">
    <img src="../../../docs/validation-master-document/imgs/aij_A_grid.png" width="50%" height="50%">
</p>

<p align="center">
    <img src="../../../docs/validation-master-document/imgs/aij_A_smoothing.png" width="50%" height="50%">
</p>


* Enhanced solving stability with:
    - ramp up of inflow boundary condition (U, k, omega): 1  --> 100 *%* of velocity magnitude
    - ramp down of `maxCo`: 2.5 --> 0.2
    - ramp down of `nOuterCorrectors`: 5 --> 1
    - ramp up inner loops under-relaxation factors: 0.6 --> 1


* Mesh smoothing available based on external repository:
    - https://github.com/ptava/integration-cfmesh (for `improveMeshQuality` utility)


* Adaptive mesh refinement based on additional external repositories:
    - https://github.com/ptava/kOmegaSSTSAS (for `mykOmegaSSTSAS` turbulence model)
    - https://github.com/ptava/dynamicFvMesh (for `mydynamicFvMesh` library)
    - https://github.com/ptava/functionObjects (for additional function objects)

    ! Set `dumpRefinementInfo` in `dynamicMeshDict` to true to use additional function objects such as `FO_cuttingPlanesRefinement` and `FO_refinementInfo`

    ! Set `refineScale` in `dynamicMeshDict` as a Function1 to smoothly increase the number of refined cells over time


* Prepared for OpenFOAM by ESI-OpenCFD and OpenFOAM Foundation


