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
exe="main_intel"
dir2="full_test_intel_iall_best"
dir="full_test_intel_algs"
procs=$1 #"2 3 4 5 6 7 8 9" #2 3 4 5 6 7 8 16 24 32" # 40 48 56 64 72 80 88 96 104 112 120 128"
part="1 2 4 6 8"
algs="" #1 2 3 4 5 6 7 8 9"
#mkdir -p ${dir}
mkdir -p ${dir2}
srun -l bash -c 'hostname' | sort | awk '{print $2}' > $NODELIST

cat $NODELIST > myhostfile.$$
echo "-----------------------------------------------"

export UCX_TLS=ud,sm,self

for i in ${procs}
do
        unset I_MPI_ADJUST_IALLREDUCE
        #mpirun -np $i -ppn 1 -iface ib0 -f myhostfile.$$  ./${exe} 0 0 1 > ${dir}/allreduce_auto_${i}.dat
        mpirun -np $i -ppn 1 -genv I_MPI_ADJUST_IALLREDUCE 4 -iface ib0 -f myhostfile.$$  ./${exe} 1 1 0 > ${dir2}/iallreduce_auto_${i}.dat
        for a in ${algs}
        do
            mpirun -np $i -ppn 1 -genv I_MPI_ADJUST_IALLREDUCE $a -iface ib0 -f myhostfile.$$  ./${exe} 1 1 0 > ${dir}/iallreduce_${a}_alg_${i}.dat
        done
        for p in ${part}
        do
            unset I_MPI_ADJUST_IALLREDUCE
            mpirun -np $i -ppn 1 -genv I_MPI_ADJUST_IALLREDUCE 4 -iface ib0 -f myhostfile.$$  ./${exe} 1 $p 0 > ${dir2}/iallreduce_${i}_procs_${p}_parts.dat
        done
done


#export MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM=mst
#for i in ${procs}
#do
#	mpiexec -np $i -iface ib0 -f myhostfile  ./${exe} $i > ${dir}/allreduce_mst_${i}.dat
#done


unset MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM


# End of submit file
