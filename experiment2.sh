#!/bin/bash
make;

# This is a strong scaling experiment that uses 32 MPI ranks
# per Blue Gene/Q node.

# Small partitions
for j in {0..6}
do
  srun --partition=small --time=30 \
       --overcommit --open-mode=append --output=experiment1.out \
       --nodes=$((2**$j)) --ntasks=$((32*((2**$j)))) \
       ./kcluster.xl&
done

# Medium partitions
for j in 7
do
  srun --partition=medium --time=30 \
       --overcommit --open-mode=append --output=experiment1.out \
       --nodes=$((2**$j)) --ntasks=$((32*((2**$j)))) \
       ./kcluster.xl&
done