COMP := /mnt/beegfs/uji/stabile/openmpi/bin/mpicc
FLAGS:= -lm

all: latency bandwidth hostname

latency:
	$(COMP) lat_bw_mpi.c -o lat_bw_ompi $(FLAGS)

bandwidth:
	$(COMP) bw_multiping.c -o bw_multiping $(FLAGS)

hostname:
	$(COMP) hostname_mpi.c -o hostname $(FLAGS)

clean:
	rm -rf lat_bw_ompi bw_multiping hostname




   


