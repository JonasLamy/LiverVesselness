Welcome to libRORPO, a mathematical morphology library for Path Operators. RORPO
is a morphological vesselness operator, meant to detect vessels and other tubular
structures in medical images.

To compile this code you will need to also install the pybind11 submodule. This
is achieved with the command:

git submodule update --init


################ libRORPO ###############

WARNING :
All images are expected to have an isotropic image resolution (cubic voxels).
The software will produce a result if this not the case but the interpretation
of this result will be questionable.

---------- File PO.hpp ----------

PO_3D : 
Compute the Path Opening operator in one orientation. The 7 orientations are defined in the function createNeighbourhood.

	image : Input image
	L : Path length
	index_image : sorted index of the image. Result of the sort_image_value function of sorting.hpp 
	orientations : defined the orientation used. Choices are [0,0,1] ; [1,0,0] ; [0,1,0] ; [1,1,1] ; [-1,1,1] ; [1,1,-1] ; [-1,1,-1]
	Output : Result of the Path Opening
	

---------- File RPO.hpp ----------

RPO :
Compute the 7 orientations of the Robust Path Opening and return them.

	image : input image
	L : Path length
	RPO1 : resulting Robust Path Opening in the first orientation.
	RPO2 : resulting Robust Path Opening in the second orientation
	RPO3 : resulting Robust Path Opening in the third orientation
	RPO4 : resulting Robust Path Opening in the fourth orientation
	RPO5 : resulting Robust Path Opening in the fifth orientation
	RPO6 : resulting Robust Path Opening in the sixth orientation
	RPO7 : resulting Robust Path Opening in the seventh orientation
	nb_core : number of cores used to compute the Path Opening (choose between 1 and 7)
	

---------- File RORPO.hpp ----------
Compute the Ranking Orientations Responses of Path Operators (RORPO)

	image : input image
	L : Path length
	nb_core : number of cores used to compute the Path Opening (choose between 1 and 7)
	

---------- File RORPO_multiscale.hpp ----------
Compute the multiscale RORPO

	image : input image
	S_list : vector containing the different path length (scales)
	nb_core : number of cores used to compute the Path Opening (choose between 1 and 7)
	debug_flag : 1 (activated) or 0 (desactivated)
	Mask : optional mask image
	

