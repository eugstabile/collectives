#!/bin/bash
#
# Project/Account (use your own)
#SBATCH -A stabile
#
# Number of MPI tasks
##SBATCH -n 2
#
# Number of tasks per node
##SBATCH --tasks-per-node=1
#
# Runtime of this jobs is less then 12 hours.
##SBATCH --time=12:00:00
#
# Name
#SBATCH -J "gamma"
#
# Partition
##SBATCH --partition=mpi
#
##SBATCH --output=bandwidth_%a.out
##SBATCH --nodelist=nodo[06-07]
#SBATCH --distribution=cyclic


echo $SLURM_JOB_NODELIST

export NODELIST=nodelist.$$

srun -l bash -c 'hostname' | sort | awk '{print $2}' > $NODELIST

cat $NODELIST > myhostfile.$$ 
echo "-----------------------------------------------"

$(which mpirun) -np 1 --map-by node --display-map --mca btl '^openib' --mca pml ucx --oversubscribe \
        --hostfile myhostfile.$$ --mca mpi_warn_on_fork 0 ./main > gamma_resultsNew.dat

# End of submit file
