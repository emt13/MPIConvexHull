#!/bin/sh
#***********************************************************************/
#   @project Final Project - Parallel Programming and Computing
#   @file test_kratos.sh
#   @description This is a simple test for making sure everything
# 				 works on the Kratos class server.
#
#   @authors Alex Vargas
#            Evan Thompson
#            Alexander Soloviev
#            Jim Olyha
#
#***********************************************************************/

mpirun -np 64 -output-filename test_kratos \
	   ./kmeans_run.exe -p 100 -n 5 -c 2 -o -d &