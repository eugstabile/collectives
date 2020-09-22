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
exe="main_ompi"
dir="full_test_ompi_algs"
procs="2 3 4 5 6 7 8" # 16 24 32" # 40 48 56 64 72 80 88 96 104 112 120 128"
part="1 2 3 4 5 6 7 8"
mkdir -p ${dir}
srun -l bash -c 'hostname' | sort | awk '{print $2}' > $NODELIST

cat $NODELIST > myhostfile 
echo "-----------------------------------------------"

#export MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM=auto
#for i in ${procs}
#do
#	mpirun -np $i -iface ib0 -f myhostfile  ./${exe} 0 > ${dir}/allreduce_auto_${i}.dat
#done

for i in ${procs}
do
    mpirun -n $i --map-by node -mca btl openib --mca btl_openib_allow_ib true --oversubscribe   --hostfile myhostfile --mca  mpi_warn_on_fork 0 ./${exe} 0 0 1 > ${dir}/allreduce_base_${i}.dat
    for a in 1 2 3 4 5 6
    do 
        mpirun -n $i --map-by node -mca btl openib --mca btl_openib_allow_ib true --oversubscribe --mca coll_tuned_use_dynamic_rules 1 --mca coll_tuned_allreduce_algorithm $a  --hostfile myhostfile --mca  mpi_warn_on_fork 0 ./${exe} 0 0 1 > ${dir}/alreduce_${a}_alg_${i}.dat
    done
    for p in ${part}
    do
	mpirun -n $i --map-by node -mca btl openib --mca btl_openib_allow_ib true --oversubscribe   --hostfile myhostfile --mca  mpi_warn_on_fork 0 ./${exe} 1 $p 0 > ${dir}/iallreduce_${i}_procs_${p}_parts.dat
    done
done




# End of submit file
