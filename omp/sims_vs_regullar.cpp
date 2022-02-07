#include <omp.h>
#include <chrono>
#include <iostream>

int main(int argc, char *argv[]) {

    long N = 100000;
    int* a = new int[N];
    int* b = new int[N];
    int* c = new int[N];

    auto start = std::chrono::system_clock::now();
    int iters = 100;
    
    for(int j = 0; j < iters; j++) {
        #pragma omp simd // multiple instructions single data
        for(int i = 0; i < N;i++) {
            int k = 0;
            a[k] = b[k]+c[k];
        }
    }
    
    
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Time elapsed: " << elapsed_seconds.count() * 1000 / iters << " ms" << std::endl;
    delete a;
    delete b;
    delete c;


 
    return 0;
}