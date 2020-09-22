




COMP := mpicc
FLAGS:= -std=c99 -lm


all: test hostname

test:
	$(COMP) lat_bw_mpi.c -o lat_bw_ompi $(FLAGS)

hostname:
	$(COMP) hostname_mpi.c -o hostname $(FLAGS)


clean:
	rm -rf lat_bw hostname




   


