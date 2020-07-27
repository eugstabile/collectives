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
exe="main"
dir="full_test_mpich"
procs="2 4 8 16 32 64 128 224 256 448 512"
procs="2 3 4 5 6 7 8 9 10"
mkdir -p ${dir}
srun -l bash -c 'hostname' | sort | awk '{print $2}' > $NODELIST

cat $NODELIST > myhostfile 
echo "-----------------------------------------------"

#export LD_LIBRARY_PATH=/opt/intel/compilers_and_libraries/linux/mkl/lib/intel64/:/opt/intel/compilers_and_libraries_2019.3.199/linux/compiler/lib/intel64_lin/:$LD_LIBRARY_PATH

#export LD_LIBRARY_PATH=/home/adcastel/opt/mpich_our/lib/:$LD_LIBRARY_PATH

#export MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM=auto
#for i in ${procs}
#do
#	mpirun -np $i -iface ib0 -f myhostfile  ./${exe} 0 > ${dir}/allreduce_auto_${i}.dat
#done

export MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM=recursive_doubling
for i in ${procs}
do
	mpirun -np $i -iface ib0 -f myhostfile  ./${exe} 1 > ${dir}/allreduce_rd_${i}.dat
done


#export MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM=reduce_scatter_allgather
#for i in ${procs}
#do
#	mpirun -np $i -iface ib0 -f myhostfile  ./${exe} 2 > ${dir}/allreduce_rsa_${i}.dat
#done


#export MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM=mst
#for i in ${procs}
#do
#	mpiexec -np $i -iface ib0 -f myhostfile  ./${exe} $i > ${dir}/allreduce_mst_${i}.dat
#done


unset MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM


# End of submit file
