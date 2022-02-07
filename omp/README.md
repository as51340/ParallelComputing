## Compile and run code
```
g++ -o omp cpp_matrixops_parallel.cpp cpp_main.cpp -fopenmp
OMP_NUM_THREADS=8 ./omp
```

Note: which implementation is being tested can be changed in cpp_main.cpp
