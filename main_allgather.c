#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

//#define TYPE int
//#define MPI_TYPE MPI_INT
#define TYPE float
#define MPI_TYPE MPI_FLOAT
#define ROOT 0
void init(TYPE * in, TYPE * out, TYPE * sol, int size, int wsize){

    int i;
    for (i=0; i<size; i++)
    {
        *(in + i) = i;
        *(sol + i) = i*wsize;
        *(out + i) = 0;
    }

}

void check(TYPE *out, TYPE * sol, int size, int rank){

    int i,fnderr=0;
    for (i=0; i<size; i++)
    {
        if (*(out + i) != *(sol + i))
        {
            fnderr++;
        }
    }
    if (fnderr)
    {
    	fprintf( stderr, "(%d) Error for MPI_SUM\n", rank );
    	fflush(stderr);
        exit(1);
    }

}

#define START_TEST      double time,time2;\
			double time_all = 0.0;\
			int r;\
			for (r = 0; r < reps; r++){\
		        init(in,out,sol,s,wsize);\
		        time = MPI_Wtime();\
        		
#define END_TEST	time = MPI_Wtime() - time;\
        		MPI_Barrier(MPI_COMM_WORLD);\
        		MPI_Reduce(&time,&time2,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);\
                        if(rank == 0) time = time2;\
        		/*check(out,sol,s,rank);*/\
        		time_all+=time;\
			}\
    			return time_all/reps;


#define END_TEST2	time = MPI_Wtime() - time;\
        		MPI_Barrier(MPI_COMM_WORLD);\
        		MPI_Reduce(&time,&time,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);\
                        /*if(rank < bcast_size){*/\
        		/*check(out,sol,s,rank);}*/\
        		time_all+=time;\
			}\
    			return time_all/reps;

double original_allgather(TYPE * in, TYPE * out, TYPE * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm){
    START_TEST;
    MPI_Allgather(in, s, MPI_TYPE, out, s, MPI_TYPE, comm );
    END_TEST;
}

//DYNAMIC for testing
double half_iallgather(TYPE * in, TYPE * out, TYPE * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm, int halfs){
    if(s < halfs){halfs=1;}
    size_t half_size = s/halfs;
    size_t half_sizeo = (s*wsize)/halfs;
    MPI_Request * request = malloc(halfs*sizeof(MPI_Request));
    MPI_Status * status = malloc(halfs*sizeof(MPI_Status));
    int h;
    START_TEST;
    size_t sent = 0;
    for(h=0;h<halfs-1;h++)
    {
        //printf("Proc %d enviando %d de %d envios con tamaño %d a partir de posicion: %d y almaceno en posicion %d\n", rank, h, halfs, half_size, h*half_size, h*half_sizeo); fflush(stdout);
        MPI_Iallgather( &in[h*half_size], half_size, MPI_TYPE, &out[h*half_sizeo], half_size, MPI_TYPE, comm, &request[h] );
        sent+=half_size;
    }
    h=halfs-1;
    //printf("Proc %d enviando %d de %d envios con tamaño %d a partir de posicion: %d y almaceno en posicion %d\n", rank, h, halfs, s-sent, h*half_size, h*half_sizeo); fflush(stdout);
    MPI_Iallgather( &in[h*half_size], s-sent, MPI_TYPE, &out[h*half_sizeo], s-sent, MPI_TYPE, comm, &request[h] );
    MPI_Waitall(halfs,request,status);
    END_TEST;
}
double chunk_iallgather(TYPE * in, TYPE * out, TYPE * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm, int chunk){
    int chunks = ( s <= chunk) ? 1 :  s/chunk;
    if (chunks > 1 &&  s % chunk != 0) chunks++;
    size_t half_sizeo = (s*wsize)/chunks;
    MPI_Request * request = malloc(chunks*sizeof(MPI_Request));
    MPI_Status * status = malloc(chunks*sizeof(MPI_Status));
    //NOT CHECKED!!!!
    int h;
    START_TEST;
    size_t sent = 0;
    for(h=0;h<chunks-1;h++)
    {
        MPI_Iallgather( &in[h*chunk], chunk, MPI_TYPE, &out[h*half_sizeo], chunk, MPI_TYPE, comm, &request[h] );
        sent += chunk;
    }
    h = chunks-1;
    MPI_Iallgather( &in[h*chunk], s-sent, MPI_TYPE, &out[h*half_sizeo], chunk, MPI_TYPE, comm, &request[h] );
    MPI_Waitall(chunks,request,status);
    END_TEST;
}

int main(int argc, char *argv[])
{

    TYPE *in, *out, *sol;
    int i, r; 
    size_t s, ss=1;
    int rank, wsize;
    int reps = 100;
    int reps2;
    

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    
    //Used for distinc fan-in and fan-out
    int procs_red = wsize; 
    int procs_bcast = 2;

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message so we ensure the correct MPI mapping (one per node)
    printf("#Hello world from processor %s, rank %d out of %d processors\n",
            processor_name, rank, wsize);
    
    int total_wsize = wsize; // Do not modify it
    /* Warm-up zone */
    MPI_Bcast(&reps,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Allreduce(&reps,&reps2,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
    //Reps2 is never used but we want to make the allreduce movement

    // USAGE: ./exec [ori [opt [part [chunk [chunksize [range]]]]]]
    // ori -> 0|1. If 1, the blocking collective is used
    // opt -> 0|1. Disable or enable the optimization (segementation) (default=0)
    // part -> number > 0. If opt is enabled, indicates the number of parts for iallreduce (default=4)
    // chunk -> 0|1. Disable or enable the decomposition of iallreduce in chunks (default=0)
    // chunksize -> number > 0. Number of elements for the chunk version. (default 1MB)
    // range -> Number >= 0. If 0, the range for message sizes is from 1 to count elements.
    //          If > 0, the elements of a message (default=0)
    // Example: ./exe 0 1 3 1 128 [0] will execute the test without the blocking call, with segmented iallreduce
    //           divided into 3 messages, with chunksizes of 128 elements. The range will be from 1 to count elements
    size_t max_count = 1024*1024*256; // 256 MB per proc for send
    int ori = (argc > 1) ? atoi(argv[1]):1;
    int opt= (argc > 2) ?atoi(argv[2]):0;
    int part = (argc > 3) ? atoi(argv[3]):4;
    int chunk = (argc > 4) ? atoi(argv[4]): 0;
    int chunksize = (argc > 5) ? atoi(argv[5]): 1024*1024; //1MB
    if (chunk) {chunksize = chunksize/sizeof(TYPE);}
    size_t range = (argc > 6) ? atol(argv[6]): 0;
    size_t min_range = (argc > 7) ? atol(argv[7]): 4;
    size_t max_range = (argc > 8) ? atol(argv[8]): max_count;
    size_t count;
    if(range){
        ss = min_range/sizeof(TYPE);
        count = max_range/sizeof(TYPE);
    }
    else{count = max_count/sizeof(TYPE);}
    
    in = (TYPE *)malloc( count * sizeof(TYPE) );
    out = (TYPE *)malloc( count * wsize *sizeof(TYPE) );
    sol = (TYPE *)malloc( count * sizeof(TYPE) );
    if(rank == 0){
        // Print a summary of the test
        printf("#Test with %d proceses\n",wsize);
        printf("#IAllgather division: %sabled with %d parts\n",(opt == 0)? "dis" : "en", part);
        printf("#Chunk Iallgather: %sabled with chunksize of %d (elems) %d~MB \n",
               (chunk == 0)? "dis" : "en", chunksize, chunksize*sizeof(TYPE)/1024/1024);
    }
    
    if(rank == 0){
        // Prepare the output header
        printf("#SIZE(bytes)\t");
        if(ori == 1){
            printf("Allgather(%d)\t",wsize);
        }
    
        if(opt == 1){
            printf("%d_half_iallg(%d)\t",part,wsize);
        }
        if(chunk == 1){
            printf("%d_chunk_iallg(%d)\t",chunksize,wsize);
        }
    printf("\n");
    }
    
    for (s=ss; s<=count; s*=2){
 
        if(rank == 0)
            printf("%lu\t\t",s*sizeof(TYPE));
 
        wsize = total_wsize;
        if(ori == 1){   
            double time_bcast = original_allgather(in,out,sol,s,wsize,rank,reps,MPI_COMM_WORLD);
            if(rank == 0){
              printf("%f\t",time_bcast);
            }
        }
        if( opt == 1){ 
	    double time_4hiallreduce = half_iallgather(in,out,sol,s,wsize,rank,reps,MPI_COMM_WORLD,part);
            if(rank == 0){
                printf("%f\t",time_4hiallreduce);
            } 
        }
        if ( chunk == 1){
	    double t_chunk = chunk_iallgather(in,out,sol,s,wsize,rank,reps,MPI_COMM_WORLD,chunksize);       
            if(rank == 0){
                printf("%f\t",t_chunk);
            } 

        }

        if(rank == 0)
            printf("\n");
    }
    
    free( in );
    free( out );
    free( sol );
    MPI_Finalize();
    return 0;
}
