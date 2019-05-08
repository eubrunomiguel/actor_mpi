#!/bin/bash

#SBATCH -N 2
#SBATCH -J test
#SBATCH --mail-user=bruno.miguel@tum.de
#SBATCH --mail-type=ALL
#SBATCH -t 00:03:00

srun -n 2 /home/hpc/pr63so/ga27nuk2/actor-upcxx/test/a.out > a.log


#mpiCC -std=c++11 main.cpp
#mpiexec -n 2 /home/hpc/pr63so/ga27nuk2/actor-upcxx/test/a.out

#mpiCC -std=c++11 main.cpp; mpiexec -n 2 /home/hpc/pr63so/ga27nuk2/actor-upcxx/test/a.out
mpiCC -std=c++11 main.cpp; srun -n 2 /home/hpc/pr63so/ga27nuk2/actor-upcxx/test/a.out