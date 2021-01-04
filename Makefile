
COMP := mpicc
FLAGS:= -lm
EXE:= main_ompi
EXE2:=main_gpu
all: coll gpu

coll:
	$(COMP) main.c -o $(EXE) $(FLAGS)

gpu:
    $(COMP) main_gpu.c -o $(EXE2) $(FLAGS)

clean:
	rm -rf  $(EXE)




   


