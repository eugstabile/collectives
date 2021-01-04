
COMP := mpicc
FLAGS:= -lm
CUDA=/usr/local/cuda-10.2
FLAGS2:= -I/$(CUDA)/include -L/$(CUDA)/lib64 -lcudart -lnccl -lm
EXE:= main_ompi
EXE2:=main_gpu
all: coll gpu

coll:
	$(COMP) main.c -o $(EXE) $(FLAGS)

gpu:
	$(COMP) main_gpu.c -o $(EXE2) $(FLAGS2)

clean:
	rm -rf  $(EXE)




   


