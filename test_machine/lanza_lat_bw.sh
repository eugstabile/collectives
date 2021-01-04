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

##export I_MPI_FABRICS=shm:ofi
##export FI_PROVIDER=mlx
#export UCX_TLS=all #ud,sm,self
#mpirun -np 2 -iface ib0 -f myhostfile.$$ ./lat_bw_mpi
##export FI_PROVIDER=tcp
##export I_MPI_FABRICS=shm
#export I_MPI_DEVICE=tcp
##export FI_SOCKETS_IFACE=enp1s0f0
FI_PROVIDER=sockets mpirun -np 2 -ppn 1 -genv FI_SOCKETS_IFACE=enp1s0f0 -iface enp1s0f0  -f myhostfile.$$ ./lat_bw_mpi > intel.dat #si añadimos a I_MPI_FABRICS=shm:ofi a

#I_MPI_OFI_PROVIDER=tcp mpirun -np 2 -ppn 1 -genv I_MPI_FABRICS=ofi -iface enp1s0f0  -f myhostfile.$$ ./lat_bw_mpi > intel.dat #si añadimos a I_MPI_FABRICS=shm:ofi a

#mpirun -np 2 -ppn 1 -genv FI_PROVIDER=tcp -iface enp1s0f0  -f myhostfile.$$ ./lat_bw_mpi > intel.dat #igual que el siguiente 
#FI_PROVIDER=tcp mpirun -np 2 -ppn 1 -iface enp1s0f0  -f myhostfile.$$ ./lat_bw_mpi > intel.dat # HASTA 32 MB funciona igual

# End of submit file
