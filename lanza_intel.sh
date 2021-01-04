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
dir="full_test_intel_ib"
procs=$1 #2 3 4 5 6 7 8 16 24 32" # 40 48 56 64 72 80 88 96 104 112 120 128"
part="1 2 3 4 5 6 7 8"
part=""
algs="1 2 3 4 5 6 7 8 9 10 11 12"
face="enp1s0f0"
face="ib0"
mkdir -p ${dir}
srun -l bash -c 'hostname' | sort | awk '{print $2}' > $NODELIST

cat $NODELIST > myhostfile.$$
echo "-----------------------------------------------"

#export MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM=auto
#for i in ${procs}
#do
#	mpirun -np $i -iface ib0 -f myhostfile  ./${exe} 0 > ${dir}/allreduce_auto_${i}.dat
#done
#export I_MPI_FABRIC=shm:ofi
#export FI_PROVIDER=mlx
export UCX_TLS=ud,sm,self

for i in ${procs}
do
        #FI_PROVIDER=sockets mpirun -np $i -ppn 1  -genv FI_SOCKETS_IFACE=enp1s0f0 -iface ${face} -f myhostfile.$$  ./${exe} 0 0 1 > ${dir}/allreduce_auto_${i}.dat
        mpirun -np $i -ppn 1  -iface ${face} -f myhostfile.$$  ./${exe} 0 0 1 > ${dir}/allreduce_auto_${i}.dat
        for a in ${algs}
        do
            mpirun -np $i -ppn 1  -genv I_MPI_ADJUST_ALLREDUCE $a -iface ${face} -f myhostfile.$$  ./${exe} 0 0 1 > ${dir}/allreduce_${a}_alg_${i}.dat
        done
        for p in ${part}
        do
            FI_PROVIDER=sockets mpirun -np $i  -genv FI_SOCKETS_IFACE=enp1s0f0 -iface ib0 -f myhostfile.$$  ./${exe} 1 $p 0 > ${dir}/iallreduce_${i}_procs_${p}_parts.dat
        done
done


#export MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM=mst
#for i in ${procs}
#do
#	mpiexec -np $i -iface ib0 -f myhostfile  ./${exe} $i > ${dir}/allreduce_mst_${i}.dat
#done


unset MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM


# End of submit file
