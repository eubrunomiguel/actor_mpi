#!/bin/bash

#SBATCH -N 3
#SBATCH -q regular
#SBATCH -J square_root
#SBATCH --mail-user=bruno.miguel@tum.de
#SBATCH -C haswell
#SBATCH --mail-type=ALL
#SBATCH -t 01:00:00

export UPCXX_CODEMODE=O3
export UPCXX_THREADMODE=par

#run the application:
srun -n 3    /home/hpc/pr63so/ga27nuk2/actor-upcxx/build/square_root_example > /home/hpc/pr63so/ga27nuk2/actor-upcxx/logs/square_root_example.txt