#!/bin/bash
set -x #echo on

for i in `seq 1 40`; do
#./particleskinner 0.1 /Users/nghia/Programming/MPMMultiPhase/fixed3D-surface/skin/frame.$i /Users/nghia/Programming/MPMMultiPhase/fixed3D-surface/OBJ/frame.$i.obj
./particleskinner 0.015624 D:/Scratch/SimData/FLIP.Bunny/FLIPFrame/frame.$i D:/Scratch/SimData/FLIP.Bunny/OBJ/frame.$i.obj
echo $i; sleep 1;
done;
