#!/bin/bash
#
# Project/Account (use your own)
#SBATCH -A adcastel
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
#SBATCH -J "allreduce"
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

#export I_MPI_FABRICS=shm:ofi
#export FI_PROVIDER=mlx
#export UCX_TLS=all #ud,sm,self
mpirun -n 2 --map-by node --display-map  -mca btl openib --mca btl_openib_allow_ib true --oversubscribe   --hostfile myhostfile.$$ --mca  mpi_warn_on_fork 0 ./lat_bw_mpi_openmpi > ping_ompi_ib.dat
# mpirun -n 2 --map-by node --display-map  \
# --mca pml ob1  --mca btl ^openib --mca btl_tcp_if_include enp1s0f0 \
# --oversubscribe   --hostfile myhostfile.$$ --mca  mpi_warn_on_fork 0 ./lat_bw_mpi_openmpi > ompi.dat
#mpirun -np 2 -f myhostfile ./lat_bw_mpi

# End of submit file
