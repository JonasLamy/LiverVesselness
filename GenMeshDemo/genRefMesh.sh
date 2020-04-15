#!/usr/local/bin/bash

NAME_REF=vesselsIso
OUTPUT_MESHNAME=ref.obj
COLOR="255 0 0 255"
TEMPDIR=$1
shift

# param


param=$*

for path in ${param}
do
    echo Processing $path
    itk2vol -i $path/${NAME_REF}.nii -o ${TEMPDIR}/${NAME_REF}.vol --inputMin 0 --inputMax 1 -t integer
    volBoundary2obj -i ${TEMPDIR}/${NAME_REF}.vol -o ${path}/${OUTPUT_MESHNAME} --customDiffuse $COLOR

done

