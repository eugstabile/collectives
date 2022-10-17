// Author: Wes Kendall
// Copyright 2011 www.mpitutorial.com
// This code is provided freely with the tutorials on mpitutorial.com. Feel
// free to modify it for your own use. Any distribution of the code must
// either provide a link to www.mpitutorial.com or keep this header intact.
//
// An intro MPI hello world program that uses MPI_Init, MPI_Comm_size,
// MPI_Comm_rank, MPI_Finalize, and MPI_Get_processor_name.
//
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  // Initialize the MPI environment. The two arguments to MPI Init are not
  // currently used by MPI implementations, but are there in case future
  // implementations might need the arguments.
  MPI_Init(NULL, NULL);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int data = 1024*1024*256;
  char * buff = malloc(data);
  char * buff2 = malloc(data);
  int reps = 1000;
  if(world_size != 2) { printf("We need 2 process!\n"); return 0;}
  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  MPI_Status st;
  // Print off a hello world message
  printf("Hello world from processor %s, rank %d out of %d processors\n",
         processor_name, world_rank, world_size);

  printf("Test starting\n");
  //from 0 to 256 bytes
  for(size_t i = 4; i < 256; i+=2){
        double start = MPI_Wtime();
	for(int r = 0; r< reps; r++){
		if(world_rank == 0){
			MPI_Ssend(buff,i,MPI_CHAR,1,0,MPI_COMM_WORLD);
			MPI_Recv(buff2,i,MPI_CHAR,1,1,MPI_COMM_WORLD,&st);
			
                }
		else{
			MPI_Recv(buff2,i,MPI_CHAR,0,0,MPI_COMM_WORLD, &st);
			MPI_Ssend(buff,i,MPI_CHAR,0,1,MPI_COMM_WORLD);
		}
        }
	double end = MPI_Wtime();
	double time = (end-start)/(reps*2.0);
	if(world_rank == 0){
	    printf("%ld %.12f %f \n",i, time, i/time);
        }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  for(size_t i = 256; i< 2048; i+=2){
        double start = MPI_Wtime();
	for(int r = 0; r< reps; r++){
		if(world_rank == 0){
			MPI_Ssend(buff,i,MPI_CHAR,1,0,MPI_COMM_WORLD);
			MPI_Recv(buff2,i,MPI_CHAR,1,1,MPI_COMM_WORLD,&st);
			
                }
		else{
			MPI_Recv(buff2,i,MPI_CHAR,0,0,MPI_COMM_WORLD, &st);
			MPI_Ssend(buff,i,MPI_CHAR,0,1,MPI_COMM_WORLD);
		}
        }
	double end = MPI_Wtime();
	double time = (end-start)/reps/2.0;
	if(world_rank == 0){
	    printf("%ld %f %f\n",i,time, i/time);
        }
  }

  MPI_Barrier(MPI_COMM_WORLD);


  for(size_t i = 2048; i<= data; i*=2){
        double start = MPI_Wtime();
	for(int r = 0; r< reps; r++){
		if(world_rank == 0){
			MPI_Ssend(buff,i,MPI_CHAR,1,0,MPI_COMM_WORLD);
			MPI_Recv(buff2,i,MPI_CHAR,1,1,MPI_COMM_WORLD,&st);
			
                }
		else{
			MPI_Recv(buff2,i,MPI_CHAR,0,0,MPI_COMM_WORLD, &st);
			MPI_Ssend(buff,i,MPI_CHAR,0,1,MPI_COMM_WORLD);
		}
        }
	double end = MPI_Wtime();
	double time = (end-start)/reps/2.0;
	if(world_rank == 0){
	    printf("%ld %f %f\n",i,time, i/time);
        }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  // Finalize the MPI environment. No more MPI calls can be made after this
  MPI_Finalize();
}
