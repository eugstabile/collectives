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
exe="main_gpu"
dir="full_test_ompi_gpu_X"
part=""
streams=""
procs=$1
#procs="4" # 16 24 32" # 40 48 56 64 72 80 88 96 104 112 120 128"
part=""
algs=""
part=""
streams=""
part=""
streams="1"
algs="Tree" # Ring"

mkdir -p ${dir}
srun -l bash -c 'hostname' | sort | awk '{print $2}' > $NODELIST
net=" --mca pml ob1  --mca btl ^openib --mca btl_tcp_if_include enp1s0f0"
net="-mca btl openib --mca btl_openib_allow_ib true"
cat $NODELIST > myhostfile.$$ 
echo "-----------------------------------------------"
export LD_LIBRARY_PATH=/mnt/beegfs/users/adcastel/opt/openmpi-4.1.0-cuda/lib:$LD_LIBRARY_PATH

#export MPIR_CVAR_ALLREDUCE_INTRA_ALGORITHM=auto
#for i in ${procs}
#do
#	mpirun -np $i -iface ib0 -f myhostfile  ./${exe} 0 > ${dir}/allreduce_auto_${i}.dat
#done

for i in ${procs}
do
    for a in ${algs}
    do
        export NCCL_ALGO=${a}
        export NCCL_NSOCKS_PERTHREAD=8
        export NCCL_SOCKET_NTHREADS=8
        export NCCL_LAUNCH_MODE=PARALLEL
        /mnt/beegfs/users/adcastel/opt/openmpi-4.1.0-cuda/bin/mpirun -n $i --display-map --map-by node --oversubscribe  \
        ${net} \
       --hostfile myhostfile.$$ --mca  mpi_warn_on_fork 0 ./${exe} 1 > ${dir}/nccl_allreduce_alg_${a}_${i}.dat
    for p in ${part}
    do
        for s in $(seq 1 ${p})
        do
	    /mnt/beegfs/users/adcastel/opt/openmpi-4.1.0-cuda/bin/mpirun -n $i --map-by node -mca btl openib --mca btl_openib_allow_ib true --oversubscribe   --hostfile myhostfile.$$ --mca  mpi_warn_on_fork 0 ./${exe} 0 $p $s > ${dir}/nccl_allreduce_alg_${a}_${i}_procs_${p}_parts_${s}_streams.dat
    done
    done
done
done



# End of submit file
