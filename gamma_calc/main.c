#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>


#define TYPE float

void init(TYPE * in1, TYPE * in2, TYPE * sol, int size, int factor){

    int i;
    #pragma omp parallel for 
    for (i=0; i<size; i++)
    {
        *(in1 + i) = i;
        *(in2 + i) = i * factor;
        *(sol + i) = 0;
    }

}
void calculaRango(size_t start, size_t end, size_t inc, TYPE * in1, TYPE * in2, TYPE * sol, TYPE f, int reps){


   struct timeval t_start, t_end;
   for (size_t c = start; c <= end; c*=inc){

    double t_all =0.0;
    size_t res = 0;
    for (int r=0; r < reps; r++){
    gettimeofday(&t_start, NULL);
       //#pragma omp parallel for 
       for (size_t i = 0; i < c; i++){
            sol[i] = in1[i]+in2[i];
        }
    gettimeofday(&t_end, NULL);
    t_all+=((t_end.tv_sec * 1000000 + t_end.tv_usec) - (t_start.tv_sec * 1000000 + t_start.tv_usec));
    //#pragma omp parallel for
        for (size_t i = 0; i < c; i ++)
           res += sol[i];
    //    init(in1,in2,sol,c, f++);
    }
    t_all=t_all/reps;
    printf("%lu %f %f %lu\n", c*sizeof(TYPE), t_all, t_all/(c*sizeof(TYPE)), res);

}

}

void calculaRango_inc(size_t start, size_t end, size_t inc, TYPE * in1, TYPE * in2, TYPE * sol, TYPE f, int reps){

   struct timeval t_start, t_end;
for (size_t c = start; c <= end; c+=inc){

    double t_all =0.0;
    size_t res = 0;
    for (int r=0; r < reps; r++){
        gettimeofday(&t_start, NULL);
       //#pragma omp parallel for 
       for (size_t i = 0; i < c; i++){
            sol[i] = in1[i]+in2[i];
        }
        gettimeofday(&t_end, NULL);
        t_all+=((t_end.tv_sec * 1000000 + t_end.tv_usec) - (t_start.tv_sec * 1000000 + t_start.tv_usec));
        #pragma omp parallel for
        for (size_t i = 0; i < c; i ++)
           res += sol[i];
        init(in1,in2,sol,c, f++);
    }
    t_all=t_all/reps;
    printf("%lu %f %f %lu\n", c*sizeof(TYPE), t_all, t_all/c, res);

}

}

int main (int argc, char * argv[]){

size_t count = 134217728*2; // 1GB
//count = count/1024;
TYPE *in1, *in2, *sol;


 int reps = 1000;

size_t s, ss=1;

in1 = (TYPE *)malloc( count * sizeof(TYPE) );
in2 = (TYPE *)malloc( count * sizeof(TYPE) );
sol = (TYPE *)malloc( count * sizeof(TYPE) );

TYPE f = (argc > 1) ? atoi(argv[1]) : 2;

init(in1,in2,sol,count, f);

#pragma omp parallel
{
    #pragma omp master
    {
    printf("# Gamma test with %d threads\n", omp_get_num_threads());
    printf("# Size time(us) gamma(per byte)\n");
    }
}


//calculaRango_inc(256, 16384, 256, in1, in2, sol, f, reps);
calculaRango(1, count, 2, in1, in2, sol, f, reps);




}
