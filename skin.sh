#!/bin/bash
set -x #echo on

INPUT_PATH="Data"
OUTPUT_PATH="Data"

for i in $(seq -f "%04g" 1 10000); do

if [[ ! -f $INPUT_PATH/frame.$i.pos ]] ; then
    exit;
fi

#./particleskinner 0.1 /Users/nghia/Programming/MPMMultiPhase/fixed3D-surface/skin/frame.$i /Users/nghia/Programming/MPMMultiPhase/fixed3D-surface/OBJ/frame.$i.obj

./particleskinner 0.005 $INPUT_PATH/frame.$i.pos $INPUT_PATH/frame.$i.vel $OUTPUT_PATH/frame.$i.obj

echo $i; sleep 1;
done;
