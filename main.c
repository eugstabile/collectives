#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define TYPE int
#define MPI_TYPE MPI_INT

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
        		check(out,sol,s,rank);\
        		time_all+=time;\
			}\
    			return time_all/reps;


#define END_TEST2	time = MPI_Wtime() - time;\
        		MPI_Barrier(MPI_COMM_WORLD);\
        		MPI_Reduce(&time,&time,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);\
                        if(rank < bcast_size){\
        		check(out,sol,s,rank);}\
        		time_all+=time;\
			}\
    			return time_all/reps;

double original_allreduce(TYPE * in, TYPE * out, TYPE * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm){
    START_TEST;
    MPI_Allreduce(in, out, s, MPI_TYPE, MPI_SUM, comm );
    END_TEST;
}

//DYNAMIC for testing
double half_iallreduce(TYPE * in, TYPE * out, TYPE * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm, int halfs){
    if(s < halfs){halfs=1;}
    size_t half_size = s/halfs;
    MPI_Request * request = malloc(halfs*sizeof(MPI_Request));
    MPI_Status * status = malloc(halfs*sizeof(MPI_Status));
    int h;
    START_TEST;
    size_t sent = 0;
    for(h=0;h<halfs-1;h++)
    {
        MPI_Iallreduce( &in[h*half_size], &out[h*half_size], half_size, MPI_TYPE, MPI_SUM, comm, &request[h] );
        sent+=half_size;
    }
    h=halfs-1;
    MPI_Iallreduce( &in[h*half_size], &out[h*half_size], s-sent, MPI_TYPE, MPI_SUM, comm, &request[h] );
    MPI_Waitall(halfs,request,status);
    END_TEST;
}
double chunk_iallreduce(TYPE * in, TYPE * out, TYPE * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm, int chunk){
    int chunks = ( s <= chunk) ? 1 :  s/chunk;
    if (chunks > 1 &&  s % chunk != 0) chunks++;
    MPI_Request * request = malloc(chunks*sizeof(MPI_Request));
    MPI_Status * status = malloc(chunks*sizeof(MPI_Status));

    int h;
    START_TEST;
    size_t sent = 0;
    for(h=0;h<chunks-1;h++)
    {
        MPI_Iallreduce( &in[h*chunk], &out[h*chunk], chunk, MPI_TYPE, MPI_SUM, comm, &request[h] );
        sent += chunk;
    }
    h = chunks-1;
    MPI_Iallreduce( &in[h*chunk], &out[h*chunk], s-sent, MPI_TYPE, MPI_SUM, comm, &request[h] );
    MPI_Waitall(chunks,request,status);
    END_TEST;
}

double allreduce_dynamic_opt(int * in, int * out, int * sol, int s, int wsize,int red_size,int bcast_size,int rank, int reps,MPI_Comm comm){
    if (red_size == bcast_size){
        return original_allreduce(in, out, sol, s, wsize, rank, reps, comm);
    }
    MPI_Comm max_procs_comm;
    MPI_Comm red_comm,bcast_comm_p;
    MPI_Group max_procs_group, world_group;
    MPI_Group red_gr, bcast_gr_p;
    int * ranks = malloc(wsize * sizeof(int));
    int i;
    for(i = 0; i < wsize; i++){
        ranks[i] = i;
    }
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Group_incl(world_group, wsize, ranks, &max_procs_group); //Used for barriers inside simulation
    MPI_Group_incl(world_group, red_size, ranks, &red_gr);
    MPI_Group_incl(world_group, bcast_size, ranks, &bcast_gr_p);
    if (rank < red_size) {
        MPI_Comm_create_group(MPI_COMM_WORLD, red_gr, 0, &red_comm);
    }
    if (rank < bcast_size) {
        MPI_Comm_create_group(MPI_COMM_WORLD, bcast_gr_p, 0, &bcast_comm_p);
    }
    //MPI_Group_free(&world_group);

    if(red_size > bcast_size){
        MPI_Group * reduction_grs = malloc(bcast_size*sizeof(MPI_Group));
        MPI_Comm * reduction_comms = malloc(bcast_size*sizeof(MPI_Comm));
        int total = wsize;
        int * new_ranks = malloc(total*sizeof(int));
        int * sizes = malloc(red_size*sizeof(int));
        int chunk=(int)ceil((1.0f*total)/bcast_size);

        for(i = 0; i < total; i++){
            new_ranks[(chunk*(i%bcast_size))+(i/bcast_size)] = ranks[i];
        }

        chunk = total/bcast_size;
        int mod = total % bcast_size;
        for(i=0;i<bcast_size;i++){
            sizes[i] = chunk;
            if(i < mod) sizes[i]++;

        }
        int start = 0;
        for(i = 0; i < bcast_size; i++){
            MPI_Group_incl(world_group, sizes[i], &new_ranks[start], &reduction_grs[i]);
            start += sizes[i];
        }
        for(i = 0; i < bcast_size; i++){
            if(rank%bcast_size == i){
                MPI_Comm_create_group(MPI_COMM_WORLD, reduction_grs[i], 0, &reduction_comms[i]);
            }
        }
        wsize = red_size; //Because the reduction size computes the result
        START_TEST;

            MPI_Reduce( in, out, s, MPI_INT, MPI_SUM,0, reduction_comms[rank%bcast_size]);


            if(rank < bcast_size){
                MPI_Allreduce(out,out,s,MPI_INT,MPI_SUM,bcast_comm_p);
            }
            END_TEST2
        }
        else{ // bcast_size > red_size
            MPI_Group *bcast_gr;
            bcast_gr= malloc(red_size*sizeof(MPI_Group));
            MPI_Comm * bcast_comm = malloc(red_size*sizeof(MPI_Comm));
            int total = wsize;
            wsize = red_size; //Because the reduction size computes the result
            int * new_ranks = malloc(total*sizeof(int));
            int * sizes = malloc(red_size*sizeof(int));
            int chunk=(int)ceil((1.0f*total)/red_size);
            for(i = 0; i < total; i++){
                //new_ranks[i] = ranks[(chunk*(i%red_size))+(i/red_size)];
                new_ranks[(chunk*(i%red_size))+(i/red_size)] = ranks[i];
                //printf("rank %d. new_ranks[%d]=%d\n",rank,i,new_ranks[i]);
            }
            chunk = total/red_size;
            int mod = total % red_size;
            for(i=0;i<red_size;i++){
                sizes[i] = chunk;
                if(i < mod) sizes[i]++;
                //    printf("rank %d. sizes[%d]=%d\n",rank,i,sizes[i]);
            }
            int start = 0;
            for(i = 0; i < red_size; i++){
                MPI_Group_incl(world_group, sizes[i], &new_ranks[start], &bcast_gr[i]);
                start += sizes[i];
            }
            for(i = 0; i < red_size; i++){
                if(rank%red_size == i){
                    MPI_Comm_create_group(MPI_COMM_WORLD, bcast_gr[i], 0, &bcast_comm[i]);
                }
            }
            START_TEST;
                if(rank < red_size){
                    MPI_Allreduce( in, out, s, MPI_INT, MPI_SUM, red_comm );
                }//Now we broadcast with many communicators as process in the reduction

                MPI_Bcast( out, s, MPI_INT, 0, bcast_comm[rank%red_size]);

                END_TEST2

            }
            return -1;
        }





int main(int argc, char *argv[])
{
     size_t count = 134217728*2; // 1 GB

    TYPE *in, *out, *sol;
    int i, r; 
    size_t s, ss=1;
    int rank, wsize;
    int reps = 1;
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
    int ori = (argc > 1) ? atoi(argv[1]):1;
    int opt= (argc > 2) ?atoi(argv[2]):0;
    int part = (argc > 3) ? atoi(argv[3]):4;
    int chunk = (argc > 4) ? atoi(argv[4]): 0;
    int chunksize = (argc > 5) ? atoi(argv[5]): 262144; //262144 ints = 1MB
    size_t range = (argc > 6) ? atol(argv[6]): 0;

    if(range){
        ss = range;
        count = range;
    }
    in = (TYPE *)malloc( count * sizeof(TYPE) );
    out = (TYPE *)malloc( count * sizeof(TYPE) );
    sol = (TYPE *)malloc( count * sizeof(TYPE) );
    if(rank == 0){
        // Print a summary of the test
        printf("#Test with %d proceses\n",wsize);
        printf("#Iallreduce division: %sabled with %d parts\n",(opt == 0)? "dis" : "en", part);
        printf("#Chunk Iallreduce: %sabled with chunksize of %d (elems) %d~MB \n",
               (chunk == 0)? "dis" : "en", chunksize, chunksize*sizeof(TYPE)/1024/1024);
    }
    
    if(rank == 0){
        // Prepare the output header
        printf("#SIZE(bytes)\t");
        if(ori == 1){
            printf("allreduce(%d)\t",wsize);
        }
    
        if(opt == 1){
            printf("%d_half_iallreduce(%d)\t",part,wsize);
        }
        if(chunk == 1){
            printf("%d_chunk_iallreduce(%d)\t",chunksize,wsize);
        }
    printf("\n");
    }
    
    for (s=ss; s<=count; s*=2){
 
        if(rank == 0)
            printf("%lu\t\t",s*sizeof(TYPE));
 
        wsize = total_wsize;
        if(ori == 1){   
            double time_allreduce = original_allreduce(in,out,sol,s,wsize,rank,reps,MPI_COMM_WORLD);
            if(rank == 0){
              printf("%f\t",time_allreduce);
            }
        }
        if( opt == 1){ 
	    double time_4hiallreduce = half_iallreduce(in,out,sol,s,wsize,rank,reps,MPI_COMM_WORLD,part);
            if(rank == 0){
                printf("%f\t",time_4hiallreduce);
            } 
        }
        if ( chunk == 1){
	    double t_chunk = chunk_iallreduce(in,out,sol,s,wsize,rank,reps,MPI_COMM_WORLD,chunksize);       
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
