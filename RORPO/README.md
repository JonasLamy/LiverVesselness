# libRORPO
Welcome to libRORPO, a mathematical morphology library for Path Operators. RORPO
is a morphological vesselness operator, meant to detect vessels and other tubular
structures in medical images.

To compile this code you will need to also install the pybind11 submodule. This
is achieved with the command:
```
git submodule update --init
```

**WARNING :
All images are expected to have an isotropic image resolution (cubic voxels).
The software will produce a result if this not the case but the interpretation
of this result will be questionable.**

## File PO.hpp
**PO_3D**: Compute the Path Opening operator in one orientation. The 7 orientations are defined in the function RPO.
```
template<typename T, typename MaskType>
void PO_3D(const Image3D<T> &image, int L, std::vector<IndexType> &index_image, std::vector<int> &orientations, Image3D<T> &Output, std::vector<bool> b)
```

- image : Input image
- L : Path length
- index_image : sorted index of the image. Result of the sort_image_value function of sorting.hpp 
- orientations : defined the orientation used. Choices are [0,0,1] ; [1,0,0] ; [0,1,0] ; [1,1,1] ; [-1,1,1] ; [1,1,-1] ; [-1,1,-1]
- Output : Result of the Path Opening

## File RPO.hpp

**RPO** : Compute the 7 orientations of the Robust Path Opening and return them.
```
template<typename T, typename MaskType>
std::array<std::vector<int>, 7> RPO(const Image3D<T> &image, int L, Image3D<T> &RPO1, Image3D<T> &RPO2, Image3D<T> &RPO3, Image3D<T> &RPO4,
                                    Image3D<T> &RPO5, Image3D<T> &RPO6, Image3D<T> &RPO7, int nb_core, int dilationSize, Image3D<MaskType> &Mask) {
```
- image : input image
- L : Path length
- RPO1 : resulting Robust Path Opening in the first orientation.
- RPO2 : resulting Robust Path Opening in the second orientation
- RPO3 : resulting Robust Path Opening in the third orientation
- RPO4 : resulting Robust Path Opening in the fourth orientation
- RPO5 : resulting Robust Path Opening in the fifth orientation
- RPO6 : resulting Robust Path Opening in the sixth orientation
- RPO7 : resulting Robust Path Opening in the seventh orientation
- nb_core : number of cores used to compute the Path Opening (choose between 1 and 7)


## File RORPO.hpp 
**RORPO**: Compute the Ranking Orientations Responses of Path Operators

```
template<typename T, typename MaskType>
Image3D<T> RORPO(const Image3D<T> &image, int L, int nbCores, int dilationSize, Image3D<MaskType> &mask) {
```
- image: input image
- L: Path length
- nb_core: number of cores used to compute the Path Opening (choose between 1 and 7)
- dilationSize:  Size of the dilation for the noise robustness step.
- mask: optional mask image

## File RORPO_multiscale.hpp 
**RORPO_multiscale**: Compute the multiscale RORPO
```
template<typename PixelType, typename MaskType>
Image3D<PixelType> RORPO_multiscale(const Image3D<PixelType> &I, const std::vector<int>& S_list, int nb_core, int dilationSize, int debug_flag, Image3D<MaskType> &Mask)
```
- I: input image
- S_list : vector containing the different path length (scales)
- nb_core : number of cores used to compute the Path Opening (choose between 1 and 7)
- debug_flag : 1 (activated) or 0 (desactivated)
- Mask : optional mask image
	

