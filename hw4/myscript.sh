#!/bin/bash

module load intel;
module load openmpi;

mpiexec -np 1 ../hw4 5000 5000 1;
