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
  int data = 16*1024*1024;
  char * buff = malloc(data);
  char * buff2 = malloc(data);
  int reps = 100000;
  //#if(world_size != 2) { printf("We need 2 process!\n"); return 0;}
  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  MPI_Status st;
  // Print off a hello world message
  printf("#Hello world from processor %s, rank %d out of %d processors\n",
         processor_name, world_rank, world_size);
  int dest;
  if(world_rank % 2 == 0){
     dest = world_rank+1;
  }
  else{
     dest = world_rank-1;
  }

  //printf("Test starting");
  //from 0 to 256 bytes
  MPI_Barrier(MPI_COMM_WORLD);
        if (world_rank < 2){ sleep(2);}
        if (world_rank < 4){ sleep(2);}
        if (world_rank < 6){ sleep(2);}
        double start = MPI_Wtime();
        int r;
	for(r = 0; r< reps; r++){
                double instart = MPI_Wtime();
		if(world_rank %2 == 0){
			MPI_Ssend(buff,data,MPI_CHAR,dest,0,MPI_COMM_WORLD);
			MPI_Recv(buff2,data,MPI_CHAR,dest,1,MPI_COMM_WORLD,&st);
			
                }
		else{
			MPI_Recv(buff2,data,MPI_CHAR,dest,0,MPI_COMM_WORLD, &st);
			MPI_Ssend(buff,data,MPI_CHAR,dest,1,MPI_COMM_WORLD);
		}
	        double inend = MPI_Wtime();
	        double time = (inend-instart)/2.0;
                if (world_rank % 2 == 0){
	            printf("%d %f %d %f %f\n",world_rank,MPI_Wtime(), data,time, data/time);

                }
        }
	double end = MPI_Wtime();

  MPI_Barrier(MPI_COMM_WORLD);

  // Finalize the MPI environment. No more MPI calls can be made after this
  MPI_Finalize();
}
