## Compile and run
```
mpicc -o gol mpi gol_parallel.cpp
mpirun -n 4 gol # -n for setting numer of processors, decreasing performance should be noted after using more than one machine
```

Note: commands may differ for execution on different supercomputers.
