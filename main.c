#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
void init(int * in, int * out, int * sol, int size, int wsize){

    int i;
    for (i=0; i<size; i++)
    {
        *(in + i) = i;
        *(sol + i) = i*wsize;
        *(out + i) = 0;
    }

}

void check(int *out, int * sol, int size, int rank){

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
    	fprintf( stderr, "(%d) Error for type MPI_INT and op MPI_SUM\n", rank );
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


double original_allreduce(int * in, int * out, int * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm){
    START_TEST;
    MPI_Allreduce(in, out, s, MPI_INT, MPI_SUM, comm );
    //MPI_Allreduce(MPI_IN_PLACE,in, s, MPI_INT, MPI_SUM, comm );
    END_TEST;
}

double half_allreduce(int * in, int * out, int * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm, int halfs){
    size_t half_size = s/halfs;
    int h;
    START_TEST;
    for(h=0;h<halfs;h++)
    {
        MPI_Allreduce( &in[h*half_size], &out[h*half_size], half_size, MPI_INT, MPI_SUM, comm );
    }
    END_TEST;

}
double half_iallreduce_old(int * in, int * out, int * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm, int halfs){
    if(s < halfs){halfs=1;}
    size_t half_size = s/halfs;
    MPI_Request * request = malloc(halfs*sizeof(MPI_Request));
    MPI_Status * status = malloc(halfs*sizeof(MPI_Status));
    int h;
    START_TEST;
    for(h=0;h<halfs;h++)
    {
        MPI_Iallreduce( &in[h*half_size], &out[h*half_size], half_size, MPI_INT, MPI_SUM, comm, &request[h] );
    }
    MPI_Waitall(halfs,request,status);
    END_TEST;
}
//DYNAMIC for testing
double half_iallreduce(int * in, int * out, int * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm, int halfs){
    if(s < halfs){halfs=1;}
    size_t half_size = s/halfs;
    MPI_Request * request = malloc(halfs*sizeof(MPI_Request));
    MPI_Status * status = malloc(halfs*sizeof(MPI_Status));
    int h;
    START_TEST;
    size_t sent = 0;
    for(h=0;h<halfs-1;h++)
    {
        //printf("proc %d Iallreduce %d de %lu\n", rank, h, half_size);
        MPI_Iallreduce( &in[h*half_size], &out[h*half_size], half_size, MPI_INT, MPI_SUM, comm, &request[h] );
        //MPI_Iallreduce( MPI_IN_PLACE,&in[h*half_size], half_size, MPI_INT, MPI_SUM, comm, &request[h] );
        sent+=half_size;
    }
    h=halfs-1;
    //printf("proc %d Iallreduce %d de %lu\n", rank, h, half_size);
    MPI_Iallreduce( &in[h*half_size], &out[h*half_size], s-sent, MPI_INT, MPI_SUM, comm, &request[h] );
    //MPI_Iallreduce( MPI_IN_PLACE,&in[h*half_size], s-sent, MPI_INT, MPI_SUM, comm, &request[h] );
    MPI_Waitall(halfs,request,status);
    END_TEST;
}
double chunk_iallreduce(int * in, int * out, int * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm, int chunk){
    int chunks = ( s <= chunk) ? 1 :  s/chunk;
    if (chunks > 1 &&  s % chunk != 0) chunks++;
    MPI_Request * request = malloc(chunks*sizeof(MPI_Request));
    MPI_Status * status = malloc(chunks*sizeof(MPI_Status));
    //if(rank == 0)
    //printf("chunks %d\n", chunks);
    int h;
    START_TEST;
    size_t sent = 0;
    for(h=0;h<chunks-1;h++)
    {
        MPI_Iallreduce( &in[h*chunk], &out[h*chunk], chunk, MPI_INT, MPI_SUM, comm, &request[h] );
        sent += chunk;
    }
    h = chunks-1;
    MPI_Iallreduce( &in[h*chunk], &out[h*chunk], s-sent, MPI_INT, MPI_SUM, comm, &request[h] );
    MPI_Waitall(chunks,request,status);
    END_TEST;
}
double allreduce_rsa(int * in, int * out, int * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm, int halfs){
    int i;
    if(s < wsize){return -0.001;}
    MPI_Request request;
    MPI_Status status;
    int parte = s/wsize;
    START_TEST;
    MPI_Reduce( in, out, s, MPI_INT, MPI_SUM,0, comm );
    MPI_Scatter(out,parte,MPI_INT,out,parte,MPI_INT,0,comm);
    MPI_Allgather(out, parte, MPI_INT, out, parte, MPI_INT,comm);
    END_TEST;
}
double allreduce_risa(int * in, int * out, int * sol, size_t s, int wsize,int rank, int reps, MPI_Comm  comm, int halfs){
    if(s < wsize){return -0.001;}
    int mitad = wsize/2;
    int parte = s/wsize;
    int recibo = (rank < mitad) ? 0: mitad;
    int h_datos = s/2;
    int i;
    MPI_Request request;
    MPI_Status status;
    START_TEST;
    //reduce
    //if(rank == 0){
    //    printf("Size %lu, wsize %d, mitad %d, parte %d, h_datos %d\n",s,wsize,mitad,parte,h_datos);
    //}
    MPI_Reduce( in, out, s, MPI_INT, MPI_SUM,0, comm );
    //Iscatter
    if(rank == 0){
//        printf("Rank %d en rank==0\n", rank);
        MPI_Send(&out[h_datos],h_datos,MPI_INT,mitad,0,comm);
        for(i=1;i<mitad;i++){
  //      printf("Rank %d en rank==0 Isend a %d\n", rank, rank+i);
	    MPI_Isend(&out[parte*i],parte,MPI_INT,rank+i,1,comm, &request);
        }
    }
    else if (rank == mitad){
    //    printf("Rank %d en rank==mitad\n", rank);
       MPI_Recv(&out[h_datos], h_datos, MPI_INT,0,0,comm, &status);
      //  printf("Rank %d en rank==mitad recibido de 0\n", rank);
        for(i=1;i<mitad;i++){
        //printf("Rank %d en rank==mitad Isend a %d\n", rank, rank+i);
	    MPI_Isend(&out[h_datos+(parte*i)],parte,MPI_INT,rank+i,1,comm, &request);
        }
    }
    else{
        //printf("Rank %d en otros\n", rank);
	MPI_Recv(&out[parte*rank],parte,MPI_INT,recibo,1,comm, &status);
    }
    //Allgather
    MPI_Allgather(&out[parte*rank], parte, MPI_INT, out, parte, MPI_INT,comm);
    END_TEST;

}
double allreduce_red_bcast_simple(int * in, int * out, int * sol, int s, int wsize,int rank, int reps, MPI_Comm comm){

    START_TEST;
    MPI_Reduce( in, out, s, MPI_INT, MPI_SUM,0, comm );
    MPI_Bcast( out, s, MPI_INT, 0, comm );
    END_TEST;
}

double allreduce_dynamic(int * in, int * out, int * sol, int s, int wsize,int red_size,int bcast_size,int rank, int reps,MPI_Comm comm){
//    if(rank == 0)
//    printf("Dynamic wsize %d, red %d, bcast %d\n",wsize,red_size,bcast_size);
    MPI_Comm communicators[2], max_procs_comm;
    MPI_Group groups[2], max_procs_group, world_group;
    int * ranks = malloc(wsize * sizeof(int));
    int i;
    for(i = 0; i < wsize; i++){
        ranks[i] = i;
    }

    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Group_incl(world_group, red_size, ranks, &groups[0]);
    MPI_Group_incl(world_group, bcast_size, ranks, &groups[1]);
    
    MPI_Group_incl(world_group, wsize, ranks, &max_procs_group); //Used for barriers inside simulation
    MPI_Group_free(&world_group);

    /* For each group we create a communicator */
    /* The if clause is needed because each process acts in the creation */
    if (rank < red_size) {
        MPI_Comm_create_group(MPI_COMM_WORLD, groups[0], 0, &communicators[0]);
    }
    if (rank < bcast_size) {
        MPI_Comm_create_group(MPI_COMM_WORLD, groups[1], 0, &communicators[1]);
    }
//    printf("Rank %d wsize=%d red_size %d bcast_size %d\n",rank,wsize,red_size,bcast_size);
    wsize = red_size; //Because the reduction size computes the result
    START_TEST;
    if(rank < red_size){
//    printf("(Red)Rank=%d red_size %d\n",rank,red_size);
        MPI_Reduce( in, out, s, MPI_INT, MPI_SUM,0, communicators[0] );
    }
    if(rank < bcast_size){
//    printf("(Bcast)Rank=%d bcast_size %d\n",rank,bcast_size);
        MPI_Bcast( out, s, MPI_INT, 0, communicators[1]);
    }
       END_TEST2;
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
        /*if(rank == 0){
          printf("total/red_size = %d/%d y por tanto chunk=%d\n",total,bcast_size,chunk);
        }*/
        for(i = 0; i < total; i++){
            new_ranks[(chunk*(i%bcast_size))+(i/bcast_size)] = ranks[i];
        }
        /*if(rank == 0){
        for(i = 0; i < total; i++){
            printf("new_ranks[%d] = %d\n",i, new_ranks[i]);
        }
        }*/
        //return -1;
        chunk = total/bcast_size;
        int mod = total % bcast_size;
        for(i=0;i<bcast_size;i++){
            sizes[i] = chunk;
            if(i < mod) sizes[i]++;
        //if(rank == 0)
        //    printf("rank %d. sizes[%d]=%d\n",rank,i,sizes[i]);
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
//        for(i = 0; i < bcast_size; i++){
//                if(rank%bcast_size == i){
                    MPI_Reduce( in, out, s, MPI_INT, MPI_SUM,0, reduction_comms[rank%bcast_size]);
//                }
//        }

        
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
        //for(i = 0; i < red_size; i++){
        //        if(rank%red_size == i){
                    MPI_Bcast( out, s, MPI_INT, 0, bcast_comm[rank%red_size]);
        //        }
        //}
        END_TEST2
        
    }
    return -1;
}
int main(int argc, char *argv[])
{
     size_t count = 134217728*2; // 1 GB
     //size_t count = 67108864; //500MB
 
    int *in, *out, *sol;
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

    // Print off a hello world message
    printf("#Hello world from processor %s, rank %d out of %d processors\n",
            processor_name, rank, wsize);
    
    int total_wsize = wsize; // Do not modify it
    /* Warm-up zone */
    MPI_Bcast(&reps,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Allreduce(&reps,&reps2,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
    //Reps2 is never used but we want to make the allreduce movement
    //Enable the decomposition alreduce     
    int opt= (argc > 1) ?atoi(argv[1]):0;
    int part = (argc > 2) ? atoi(argv[2]):4;
    // 1 will perform, the original allreduce
    int ori = (argc > 3) ? atoi(argv[3]):1;
    size_t range = (argc > 4) ? atol(argv[4]): 0;
    int chunk = (argc > 5) ? atoi(argv[5]): 0;
    int chunksize = (argc > 6) ? atoi(argv[6]): 262144; //262144 ints = 1MB
    if(range){
        ss = range;
        count = range;
    }
    in = (int *)malloc( count * sizeof(int) );
    out = (int *)malloc( count * sizeof(int) );
    sol = (int *)malloc( count * sizeof(int) );
    if(rank == 0){
            printf("#Test with %d proceses\n #Iallreduce division: %sabled with %d parts\n #Chunk Iallreduce: %sabled with chunksize of %d (elems) %d~MB \n", wsize, (opt == 0)? "dis" : "en", part, (chunk == 0)? "dis" : "en", chunksize, chunksize*sizeof(int)/1024/1024);
    }
    
    if(rank == 0){
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
            printf("%lu\t\t",s*sizeof(int));
 
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
        /* OTHER ALLREDUCE POSSIBILITIES !!!!
        double time_rsa = allreduce_rsa(in,out,sol,s,wsize,rank,reps,MPI_COMM_WORLD,4);
        if(rank == 0){
            printf("%f\t",time_rsa);
        }
        double time_risa = allreduce_risa(in,out,sol,s,wsize,rank,reps,MPI_COMM_WORLD,4);
        if(rank == 0){
            printf("%f\t",time_risa);
        } HASTA AQUI */
    //for(int c = 2048*4; c <= 32768; c*=2){ 
//	double t_chunk = chunk_iallreduce(in,out,sol,s,wsize,rank,reps,MPI_COMM_WORLD,c);       
  //      if(rank == 0){
    //        printf("%f\t",t_chunk);
      //  }
    //}
            /*printf("%f\t",time_hallreduce);
            printf("%f\t",time_4hallreduce);
            printf("%f\t",time_hiallreduce);
            printf("%f\t",time_2hiallreduce);
            printf("%f\t",time_4hiallreduce);*/
       /* wsize = total_wsize;
        double time_allreduce_db = allreduce_red_bcast_simple(in,out,sol,s,wsize,rank,reps,MPI_COMM_WORLD);
        if(rank == 0)
            printf("%f\t",time_allreduce_db);*/
        
/*        procs_red = total_wsize;
        procs_bcast = part;
        wsize = total_wsize;
        double time_allreduce_dynamic = allreduce_dynamic(in,out,sol,s,wsize,procs_red,procs_bcast,rank,reps,MPI_COMM_WORLD);
        if(rank == 0)
            printf("%f\t",time_allreduce_dynamic);
        
        procs_red = total_wsize;
        procs_bcast = part;
        wsize = total_wsize;
        double time_allreduce_dynamic_opt = allreduce_dynamic_opt(in,out,sol,s,wsize,procs_red,procs_bcast,rank,reps,MPI_COMM_WORLD);
        if(rank == 0)
            printf("%f\t",time_allreduce_dynamic_opt);
        
        
        procs_red = part;
        procs_bcast = total_wsize;
        wsize = total_wsize;
        time_allreduce_dynamic = allreduce_dynamic(in,out,sol,s,wsize,procs_red,procs_bcast,rank,reps,MPI_COMM_WORLD);
        if(rank == 0)
            printf("%f\t",time_allreduce_dynamic);
        
        procs_red = part;
        procs_bcast = total_wsize;
        wsize = total_wsize;
        time_allreduce_dynamic_opt = allreduce_dynamic_opt(in,out,sol,s,wsize,procs_red,procs_bcast,rank,reps,MPI_COMM_WORLD);
        if(rank == 0)
            printf("%f\t",time_allreduce_dynamic_opt);
  */      
        if(rank == 0)
            printf("\n");
    }
    
    free( in );
    free( out );
    free( sol );
    MPI_Finalize();
    return 0;
}
