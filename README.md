# collectives

usage: mpirun -np X main enable_parts parts ori [fixed_size]

enable_parts (1|0) indicates if we are decomposing an allreduce call into several smaller calls
parts (integer): number of parts. Ignored if enable_parts=0. If parts=1, the asynchronous function is called
ori (1|0): executes the synchronous algorithm
fixed_size (size_t): indicates a fixed number of elements (ints) for test. If it is not indicated, the test will run from size 4 bytes to 1GB 
