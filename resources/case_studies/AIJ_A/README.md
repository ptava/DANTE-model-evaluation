# Notes

* Scale-Adaptive unsteady RANS

* Inflow conditions are experimental; user can specify OF boundary conditions implemented in the `libatmosphericModels` library with required parameters in `./ABLConditions`.

* Wind tunnel lateral and top boundaries effects on domain of interest are not considered.

* Structured grid for high-wall resolution built with `blockMesh` utility; two parameters available:
    - `cells_scaling`: scaling factor for the number of cells in the x, y, and z directions
    - `building_ratio`: ratio of the building height to the building width (default is 2 which corresponds to the reference experiment); the grid maintains the same distance between building and top boundary

* Enhanced solving stability with:
    - ramp up of inflow boundary condition (U, k, omega): 1  --> 100 *%* of velocity magnitude
    - ramp down of `maxCo`: 5 --> 0.25 
    - ramp down of `nOuterCorrectors`: 5 --> 1
    - ramp up inner loops under-relaxation factors: 0.6 --> 1

