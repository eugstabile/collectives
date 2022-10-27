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
#SBATCH -J "bcast"
#
# Partition
##SBATCH --partition=mpi
#
##SBATCH --output=bandwidth_%a.out
##SBATCH --nodelist=nodo[06-07]
#SBATCH --distribution=cyclic

export NODELIST=outputfiles/nodelist.$$
exe="main_bcast"
dir="mapeado_sincrono/bcast/largo"
procs="8 16"
algs="0 1 2 3 4 5 6 7 8"
#algs: 0 default, 1 linear, 2 chain, 3 pipeline, 4 split_binary_tree, 5 binary_tree, 6 binomial, 7 knomial, 8 scatter_allgather, 9 scatter_allgather_ring
mkdir -p ${dir}
srun -l bash -c 'hostname' | sort | awk '{print $2}' > $NODELIST

cat $NODELIST > outputfiles/myhostfile.$$ 
echo "-----------------------------------------------"

for i in ${procs}
do
    for a in ${algs}
    do
        $(which mpirun) -np $i -rf rankfile_cons_${i} --mca btl '^openib' --mca pml ucx \
        --oversubscribe --mca coll_tuned_use_dynamic_rules 1 --mca coll_tuned_bcast_algorithm $a \
        --hostfile outputfiles/myhostfile.$$ --mca mpi_warn_on_fork 0 ./${exe} 1 0 0  > ${dir}/bcast_alg_${a}_procs_${i}_largo.dat
    done
done

# End of submit file