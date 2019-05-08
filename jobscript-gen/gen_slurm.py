import argparse

pondRaw = """#!/bin/bash
#SBATCH -N ##NUM_NODES##
#SBATCH -q regular
#SBATCH -J Pond_seq-WS-##NUM_NODES##
#SBATCH -C haswell
#SBATCH --mail-user=poeppl@in.tum.de
#SBATCH --mail-type=ALL
#SBATCH -t 00:30:00

#OpenMP settings:
export OMP_NUM_THREADS=1
export OMP_PLACES=threads
export OMP_PROC_BIND=spread

#run the application:
srun -n ##NUM_RANKS## -c 1 --cpu-bind=threads /global/homes/a/apoeppl/actor-upcxx/build/cori_seq_release_nowrite/pond -x ##X_SIZE## -y ##Y_SIZE## -p ##PATCH_SIZE## -e ##END## -c 1 --scenario 2 -o out > /global/homes/a/apoeppl/##LOG_PREFIX##/pond_seq/ws-##NUM_NODES##.txt"""

pondParRaw = """#!/bin/bash
#SBATCH -N ##NUM_NODES##
#SBATCH -q regular
#SBATCH -J Pond_Par-WS-##NUM_NODES##
#SBATCH -C haswell
#SBATCH --mail-user=poeppl@in.tum.de
#SBATCH --mail-type=ALL
#SBATCH -t 00:30:00

#OpenMP settings:
export OMP_NUM_THREADS=1
export OMP_PLACES=threads
export OMP_PROC_BIND=spread

#run the application:
srun -n ##NUM_RANKS## -c ##NUM_CORES## --cpu-bind=threads /global/homes/a/apoeppl/actor-upcxx/build/cori_par_release_nowrite/pond -x ##X_SIZE## -y ##Y_SIZE## -p ##PATCH_SIZE## -e ##END## -c 1 --scenario 2 -o out > /global/homes/a/apoeppl/##LOG_PREFIX##/pond_par/ws-##NUM_NODES##.txt"""

swex10Raw = """#!/bin/bash
#SBATCH -N ##NUM_NODES##
#SBATCH -q regular
#SBATCH -J SWE-X10-ws-##NUM_NODES##
#SBATCH --mail-user=poeppl@in.tum.de
#SBATCH -C haswell
#SBATCH --mail-type=ALL
#SBATCH -t 01:00:00

export X10_NTHREADS=##NUM_THREADS##

#run the application:
srun -n ##NUM_SOCKETS## -c ##NUM_CORES## --cpu_bind=cores /global/homes/a/apoeppl/swex10/swex10/bin/Main -x ##X_SIZE## -y ##Y_SIZE## -ps ##PATCH_SIZE## -write false -solver n_hlle -end ##END## -ds false -ts fixed > /global/homes/a/apoeppl/##LOG_PREFIX##/swex10/ws-##NUM_NODES##.txt"""

#def application_type(arg):
#    if arg == "swex10":
#        return 0
#    elif arg == "pond_seq":
#        return 1
#    elif arg == "pond_par":
#        return 2
#    else:
#        raise argparse.ArgumentError("Application type must be either swex10 (for SWE-X10), pond_seq (for Pond with sequential UPC++ backend), or pond_par (for Pond with parallel UPC++ backend).")
#
#def valid_node_number(arg):
#    value = int(arg)
#    for i in range(0, 20):
#        if value == 2 ** i:
#            return i
#    raise argparse.ArgumentError("Number of nodes must be an even 2^x, e.g. 1, 2, 4, 8, 16, ...")

#parser = argparse.ArgumentParser(description='Generate SLURM Scripts for Scaling tests of Pond & SWE-X10 on Cori')
#parser.add_argument("--nodes", dest='nodes', type=valid_node_number, default=0, help="Indicate how many nodes the problem should be run on. Must be an even 2^x, e.g. 1, 2, 4, 8, 16, ...")
#parser.add_argument("--app", dest='application', type=application_type, default=0, help="Indicate the application the script needs to be generated for. Possible values are: swex10 (for SWE-X10), pond_seq (for Pond with sequential UPC++ backend), or pond_par (for Pond with parallel UPC++ backend).")
#args = parser.parse_args()

#app = args.application
#nodes = args.nodes

patchSize = 256
end = 1
xBase = 4096
yBase = 4096
coresPerRank = 64

def generatePondSeq(timesDoubled, rawString, logPrefix):
    numNodes = 1
    xMult = 1
    yMult = 1
    coresPerRank = 64
    for i in range(0,timesDoubled):
        numNodes = numNodes * 2
        if (i % 2 == 0):
            xMult = xMult * 2
        else:
            yMult = yMult * 2
    cores = coresPerRank * numNodes
    xSize = xBase * xMult
    ySize = yBase * yMult
    slurmScript = pondRaw.replace("##NUM_NODES##", str(numNodes))
    slurmScript = slurmScript.replace("##NUM_RANKS##", str(cores))
    slurmScript = slurmScript.replace("##Y_SIZE##", str(ySize))
    slurmScript = slurmScript.replace("##X_SIZE##", str(xSize))
    slurmScript = slurmScript.replace("##END##", str(end))
    slurmScript = slurmScript.replace("##PATCH_SIZE##", str(patchSize))
    slurmScript = slurmScript.replace("##LOG_PREFIX##", str(logPrefix))
    return slurmScript


def generatePondPar(timesDoubled, rawString, logPrefix):
    ppPatchSize = patchSize * 2
    numNodes = 1
    yMult = 1
    xMult = 1
    for i in range(0,timesDoubled):
        numNodes = numNodes * 2
        if i % 2 == 0:
            xMult = xMult * 2
        else:
            yMult = yMult * 2
    cores = coresPerRank * numNodes
    xSize = xBase * xMult
    ySize = yBase * yMult
    slurmScript = rawString.replace("##NUM_NODES##", str(numNodes))
    slurmScript = slurmScript.replace("##NUM_RANKS##", str(numNodes))
    slurmScript = slurmScript.replace("##NUM_CORES##", str(64))
    slurmScript = slurmScript.replace("##Y_SIZE##", str(ySize))
    slurmScript = slurmScript.replace("##X_SIZE##", str(xSize))
    slurmScript = slurmScript.replace("##END##", str(end))
    slurmScript = slurmScript.replace("##PATCH_SIZE##", str(ppPatchSize))
    slurmScript = slurmScript.replace("##LOG_PREFIX##", str(logPrefix))
    return slurmScript

def generateSwex10(timesDoubled, rawString, logPrefix):
    numNodes = 1
    xMult = 1
    yMult = 1
    for i in range(0,timesDoubled):
        numNodes = numNodes * 2
        if (i % 2 == 0):
            xMult = xMult * 2
        else:
            yMult = yMult * 2
    cores = coresPerRank * numNodes
    xSize = xBase * xMult
    ySize = yBase * yMult
    slurmScript = rawString.replace("##NUM_NODES##", str(numNodes))
    slurmScript = slurmScript.replace("##NUM_SOCKETS##", str(2 * numNodes))
    slurmScript = slurmScript.replace("##NUM_THREADS##", str(coresPerRank))
    slurmScript = slurmScript.replace("##NUM_CORES##", str(coresPerRank >> 1))
    slurmScript = slurmScript.replace("##Y_SIZE##", str(ySize))
    slurmScript = slurmScript.replace("##X_SIZE##", str(xSize))
    slurmScript = slurmScript.replace("##END##", str(end))
    slurmScript = slurmScript.replace("##PATCH_SIZE##", str(patchSize))
    slurmScript = slurmScript.replace("##LOG_PREFIX##", str(logPrefix))
    return slurmScript

def generateSwex10Batch(mul, basepath):
    f = open(basepath + "/ws-" + str(2**mul), "w")
    f.write(generateSwex10(mul,swex10Raw))
    f.close()

def generatePondSeqBatch(mul, basepath):
    f = open(basepath + "ws-" + str(2**mul), "w")
    f.write(generatePondSeq(mul,pondRaw))
    f.close()

def application_type(arg):
    if arg == "swex10":
        return 0
    elif arg == "pond_seq":
        return 1
    elif arg == "pond_par":
        return 2
    else:
        raise argparse.ArgumentError("Application type must be either swex10 (for SWE-X10), pond_seq (for Pond with sequential UPC++ backend), or pond_par (for Pond with parallel UPC++ backend).")

def valid_node_number(arg):
    value = int(arg)
    for i in range(0, 20):
        if value == 2 ** i:
            return i
    raise argparse.ArgumentError("Number of nodes must be an even 2^x, e.g. 1, 2, 4, 8, 16, ...")

parser = argparse.ArgumentParser(description='Generate SLURM Scripts for Scaling tests of Pond & SWE-X10 on Cori')
parser.add_argument("--nodes", dest='nodes', type=valid_node_number, default=0, help="Indicate how many nodes the problem should be run on. Must be an even 2^x, e.g. 1, 2, 4, 8, 16, ...")
parser.add_argument("--app", dest='application', type=application_type, default=0, help="Indicate the application the script needs to be generated for. Possible values are: swex10 (for SWE-X10), pond_seq (for Pond with sequential UPC++ backend), or pond_par (for Pond with parallel UPC++ backend).")
parser.add_argument("--log-prefix", dest='logPrefix', default='misc', help="Indicate the prefix for the test. Logs are placed in <my home dir>/prefix/<app>/ws-x.txt")
args = parser.parse_args()

app = args.application
nodes = args.nodes
logPrefix = args.logPrefix

if app == 0:
    print(generateSwex10(nodes, swex10Raw, logPrefix))
elif app == 1:
    print(generatePondSeq(nodes, pondRaw, logPrefix))
else:
    print(generatePondPar(nodes, pondParRaw, logPrefix))
