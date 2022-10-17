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

cat $NODELIST > myhostfile 
echo "-----------------------------------------------"

#export I_MPI_FABRICS=shm:ofi
#export FI_PROVIDER=mlx
#export UCX_TLS=all #ud,sm,self
#mpiexec -n 2 -host myhostfile ./lat_bw_mpi_mpich > mpich.dat
#mpirun -np 2 -f myhostfile ./lat_bw_mpi_mpich > mpich_noiface.dat
srun -n 2 -iface enp1s0f0 -f myhostfile ./lat_bw_mpi_mpich > mpich_iface_srun.dat
#mpirun -np 2 -f myhostfile ./lat_bw_mpi

# End of submit file
