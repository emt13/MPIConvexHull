#!/bin/bash
#***********************************************************************/
#   @project Final Project - Parallel Programming and Computing
#   @file experiment1.sh
#   @description This is a strong scaling experiment that uses 64 MPI
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
  srun --partition=small --time=30 \
       --overcommit --open-mode=append --output=experiment1.out \
       --nodes=$((2**$j)) --ntasks=$((64*((2**$j)))) \
       ./kmeans_run.xl&
done

# Medium partitions
for j in 7
do
  srun --partition=medium --time=30 \
       --overcommit --open-mode=append --output=experiment1.out \
       --nodes=$((2**$j)) --ntasks=$((64*((2**$j)))) \
       ./kmeans_run.xl&
done