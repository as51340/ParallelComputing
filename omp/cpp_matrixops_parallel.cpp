/**
 * Parallel implementation of various matrix products using OpenMP. 
 * 
 * Andi Škrgat 2021
 */

#include <omp.h>
#include <boost/numeric/ublas/matrix.hpp>

namespace ublas = boost::numeric::ublas;

void fullTimesDiagonalParallel(ublas::matrix<double> &left, ublas::matrix<double> &right, ublas::matrix<double> &result)
{
    size_t N = result.size1();;
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            result(i, j) = left(i, j) * right(j, j);
        }
    }
}

// TODO EXPLAIN CACHE MISSES WHEN PARALLELIZING OUTER AND INNER LOOP
// WHY IT DOESN'T WORK WHEN WHEN PARALLELIZING K LOOP
void fullTimesFullParallel(ublas::matrix<double> &left, ublas::matrix<double> &right, ublas::matrix<double> &result)
{
    size_t N = result.size1();


    for (size_t i = 0; i < N; ++i)
    {   
        for (size_t j = 0; j < N; ++j)
        {
            result(i, j) = 0.0;
            // Why pragma cannot be here?
            double loc_result = 0.0 
            // #pragma omp parallel for shared(loc_result, le,ft right) 
            #pragma omp simd reduction(+:loc_result)
            for (size_t k = 0; k < N; ++k)
            {
                loc_result += left(i, k) * right(k, j);
            }
            result(i, j) = loc_result;
        }
    }
}

void fullTimesFullBlockedParallel(ublas::matrix<double> &left, ublas::matrix<double> &right, ublas::matrix<double> &result)
{
    size_t const blocksize = 50;
    size_t N = result.size1();
    #pragma omp parallel for collapse(2)
    for (size_t i = 0; i < N / blocksize; ++i)
    {    
        for (size_t j = 0; j < N / blocksize; ++j)
        {
            for (size_t i_block = 0; i_block < blocksize; ++i_block)
            {
                for (size_t j_block = 0; j_block < blocksize; ++j_block)
                {
                    result(i * blocksize + i_block, j * blocksize + j_block) = 0.0;
                }
            }
            for (size_t k = 0; k < N / blocksize; ++k)
            {   
                // #pragma omp parallel for schedule(static) 
                for (size_t i_block = 0; i_block < blocksize; ++i_block)
                {
                    for (size_t j_block = 0; j_block < blocksize; ++j_block)
                    {
                        double local_sum = 0.0;
                        #pragma omp simd reduction(+:local_sum)
                        for (size_t k_block = 0; k_block < blocksize; ++k_block)
                        {
                            local_sum += left(i * blocksize + i_block, k * blocksize + k_block) * right(k * blocksize + k_block, j * blocksize + j_block);
                        }
                        result(i * blocksize + i_block, j * blocksize + j_block) = local_sum;
                    }
                }
            }
        }
    }
}

void triangularTimesFullParallel(ublas::matrix<double> &left, ublas::matrix<double> &right, ublas::matrix<double> &result)
{
    size_t N = result.size1();
    // Here I think we can again parallelize only outer loop
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            result(i, j) = 0.0;
            for (size_t k = i; k <= N; ++k)
            {
                result(i, j) += left(i, k) * right(k, j);
            }
        }
    }
}