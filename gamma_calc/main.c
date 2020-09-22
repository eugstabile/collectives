#include <stdio.h>
#include <omp.h>

#define TYPE int

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


int main (int argc, char * argv[]){

size_t count = 134217728*2; // 1GB

TYPE *in1, *in2, *sol;
double t_start, t_end;


 int reps = 100;

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
    printf("# Size time gamma\n");
    }
}


for (size_t c = 1; c <= count; c*=2){

    double t_all =0.0;
    size_t res = 0;
    for (int r=0; r < reps; r++){
        t_start = omp_get_wtime();
       //#pragma omp parallel for 
       for (size_t i = 0; i < c; i++){
            sol[i] = in1[i]+in2[i];
        }
        t_end = omp_get_wtime();
        t_all+=t_end-t_start;
        #pragma omp parallel for
        for (size_t i = 0; i < c; i ++)
           res += sol[i];
        init(in1,in2,sol,count, f++);
    }
    t_all=t_all/reps;
    printf("%lu %f %f %lu\n", c*sizeof(TYPE), t_all, t_all/c, res);

}





}
