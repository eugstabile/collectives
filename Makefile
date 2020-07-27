




COMP := mpicc
FLAGS:= -lm


all: test coll

coll:
	$(COMP) main.c -o main $(FLAGS)

test:
	make -C test_machine


clean:
	rm -rf  main
	make clean -C test_machine




   


