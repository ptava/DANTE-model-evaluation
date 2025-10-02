# Notes

<p align="center">
    <img src="../../../docs/validation-master-document/imgs/aij_C_geometry.png" width="50%" height="50%">
</p>

* Scale-Adaptive unsteady RANS

* Inflow conditions are experimental; user can specify OF boundary conditions implemented in the `libatmosphericModels` library with required parameters in `./ABLConditions`.

* Wind tunnel lateral and top boundaries effects on domain of interest are not considered.

* Unstructured grid built with `blockMesh` and `snappyHexMesh` utilities.

* Grid smoothed by `improveMeshQuality` utility with no changes in next-to-wall cells (`bl_cells` extracted with `topoSet`).

### Mesh generation

To generate the grid with SnappyHexMesh, we need to create a background mesh via blockMesh and then use the snappyHexMesh utility. Already defined to account for multiple directions of the inflow.

This meshing procedure can be applied to all scenarios, with central building height set to 0H, 1H, and 2H (set `case` variable in `userDict` file, values available `0H`, `1H`, `2H`).
Refinement regions limits and background-mesh-related parameters are defined in `userDict` file, along with the scaling parameter `cells_scaling` (this parameter is the actual characteristic length of cubes of main region of interest.



|   Dictionary file        | Description                    |
|--------------------------|--------------------------------|
| blockMeshDict         | to create the background mesh         |
| snappyHexMeshDict     | to perform `castellated` procedure (no snap required if `cells_scaling` is set appropriately) |
| topoSetDict           | to extract next-to-wall cells for mesh smoothing |
| userDict              | to control grid definition parameters |



|STL file           | Description                                       |
|-------------------|---------------------------------------------------|
| BASE.stl          | common to all configurations (no central building)|
| 1H.stl            | central building height equal to 1H               |
| 2H.stl            | central building height equal to 2H               |



