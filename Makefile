
COMP := /mnt/beegfs/uji/stabile/openmpi/bin/mpicc
FLAGS:= -lm
CUDA=/usr/local/cuda-10.2
FLAGS2:= -I/$(CUDA)/include -L/$(CUDA)/lib64 -lcudart -lnccl -lm
EXE:= main_ompi
EXE2:=main_gpu
EXE3:=main_allgather
EXE4:=main_rescat
EXE5:=main_bcast
EXE6:=main_reduce
EXE7:=main_scatter
EXE8:=main_gather
all: coll allgather rescat bcast reduce scatter gather
	
coll:
	$(COMP) main.c -o $(EXE) $(FLAGS)

gpu:
	$(COMP) main_gpu.c -o $(EXE2) $(FLAGS2)

allgather:
	$(COMP) main_allgather.c -o $(EXE3) $(FLAGS)

rescat:
	$(COMP) main_redscat.c -o $(EXE4) $(FLAGS)

bcast:
	$(COMP) main_bcast.c -o $(EXE5) $(FLAGS)

reduce:	
	$(COMP) main_reduce.c -o $(EXE6) $(FLAGS)

scatter:	
	$(COMP) main_scatter.c -o $(EXE7) $(FLAGS)

gather:
	$(COMP) main_gather.c -o $(EXE8) $(FLAGS)

clean:
	rm -rf  $(EXE)
