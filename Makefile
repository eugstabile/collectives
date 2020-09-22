




COMP := mpicc
FLAGS:= -lm
EXE:= main_ompi

all: test coll

coll:
	$(COMP) main.c -o $(EXE) $(FLAGS)

test:
	make -C test_machine


clean:
	rm -rf  $(EXE)
	make clean -C test_machine




   


