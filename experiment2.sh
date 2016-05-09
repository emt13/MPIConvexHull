#!/bin/bash
#***********************************************************************/
#   @project Final Project - Parallel Programming and Computing
#   @file experiment1.sh
#   @description This is a strong scaling experiment that uses 32 MPI
#                ranks per Blue Gene/Q node.
#
#   @authors Alex Vargas
#            Evan Thompson
#            Alexander Soloviev
#            Jim Olyha
#
#***********************************************************************/

make;

# Small partitions
for j in {0..6}
do
	k=$((32*((2**$j))*15625))
	srun --partition=small --time=30 \
         --overcommit --open-mode=append --output=experiment2.out \
         --nodes=$((2**$j)) --ntasks=$((32*((2**$j)))) \
         ./kmeans_run.xl -p 15625000 -n $k -c 3 -o &
done

# Medium partitions
for j in 7
do
	k=$((32*((2**$j))*15625))
	srun --partition=medium --time=30 \
         --overcommit --open-mode=append --output=experiment2.out \
         --nodes=$((2**$j)) --ntasks=$((32*((2**$j)))) \
         ./kmeans_run.xl -p 15625000 -n $k -c 3 -o &
done