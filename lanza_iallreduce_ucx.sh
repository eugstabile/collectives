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
#SBATCH -J "iallreduce"
#
# Partition
##SBATCH --partition=mpi
#
##SBATCH --output=bandwidth_%a.out
##SBATCH --nodelist=nodo[06-07]
#SBATCH --distribution=cyclic

echo $SLURM_JOB_NODELIST

export NODELIST=outputfiles/nodelist.$$
export XILINX_XRT=/usr/src/xrt-2.8.743
exe="main_ompi"
dir="resultados_iallreduce/contiguo/segments_ringrd"
procs="8 16 32"
#parts="2 4 8 16"
#chunks="524288 1048576 2097152 4194304 8388608" #De 512KB a 8MB
algs="7"

#algs:0 default, 1 ring, 2 binomial, 3 rabensifner, 4 recursive_doubling, 5 knomial, 6 rd_knomial, 7 ring_rd
#-x UCX_NET_DEVICES=mlx5_1:1  --report-bindings 

mkdir -p ${dir}
srun -l bash -c 'hostname' | sort | awk '{print $2}' > $NODELIST

cat $NODELIST > outputfiles/myhostfile.$$ 
echo "-----------------------------------------------"

for i in ${procs}
do
 #   for a in ${algs}
 #   do
 #       for p in ${parts}
 #       do
            $(which mpirun) -np $i -rf rankfile_cons_${i}  --mca btl '^openib' --mca pml ucx\
            --oversubscribe --mca coll libnbc,basic --mca coll_libnbc_iallreduce_algorithm $a  \
            --hostfile outputfiles/myhostfile.$$ --mca  mpi_warn_on_fork 0 ./${exe} 0 1 1 > ${dir}/iallreduce_N8_alg_${a}_procs_${i}_sinOP.dat
 #       done
 #   done
done

# End of submit filex --rank-by core para repartir secuencialmente los procesos