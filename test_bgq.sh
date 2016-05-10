#!/bin/sh
#***********************************************************************/
#   @project Final Project - Parallel Programming and Computing
#   @file test_bgq.sh
#   @description This is a simple test for making sure everything
# 				 works on the Blue Gene/Q.
#
#   @authors Alex Vargas
#            Evan Thompson
#            Alexander Soloviev
#            Jim Olyha
#
#***********************************************************************/

srun --partition=small --time=30 \
	 --overcommit --open-mode=append --output=test_bgq.out \
	 --nodes=1 --ntasks=64 \
	 ./kmeans_run.xl -p 100 -n 5 -c 2 -o -d &