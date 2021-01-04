
COMP := mpicc
FLAGS:= -lm
EXE:= main_ompi

all: coll

coll:
	$(COMP) main.c -o $(EXE) $(FLAGS)

clean:
	rm -rf  $(EXE)




   


