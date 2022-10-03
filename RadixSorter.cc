
#include "Sorters.hh"
  
#include <vector>
#include <algorithm>
#include <cassert>
#include<iostream>
#include <queue>
using namespace std;



int getIndex(uint64_t value, uint64_t div)
{
    //cout<<value/div<<endl;
    return (value / div) % 10;
}

void radix_sort(uint64_t *arr, int n)
{
    uint64_t max = arr[0], div = 1;
    vector<queue<uint64_t>> radix(10);
    for(int i = 1; i < n; i++)
    {
        if(arr[i] > max) max = arr[i];
    }
    //cout<<max<<endl;
    while(max > 0)
    {
        for(int i = 0; i < n; i++)
        {
            radix[getIndex(arr[i], div)].push(arr[i]);
        }
        for(int i = 0, j = 0; i < radix.size(); i++)
        {
            while(!radix[i].empty())
            {
                arr[j] = radix[i].front();
                radix[i].pop();
                j++;
            }
        }
        div *= 10;
        max /= 10;
        // for(int i=0;i<n;i++){
        //     cout<<arr[i]<<" ";
        // }
        // cout<<endl;
    }
}

void RadixSorter::sort(uint64_t* array, int array_size)
{
  // TODO: Implement Serial RadixSort
   radix_sort(array,array_size);
//    for(int i=0;i<array_size;i++){
//        cout<<array[i]<<endl;
//    }

}


/* function arguments have to be passsed as a pointer for pthreads */
struct radix_args {
    pthread_t thread;
    uint64_t *a, *b;
    size_t shift, n, count[512], index[512];
};

std::mutex mtx;

/* put values into buckets based on indices */
void* put_into_buckets(void *ptr){
    struct radix_args *arg = (struct radix_args*)ptr;
    uint64_t *a = arg->a, *b = arg->b;
    size_t i, shift = arg->shift, n = arg->n, *index = arg->index;
    mtx.lock();
    for (i = 0; i < n; i++){
        size_t bucket = (a[i] >> shift) & 0xff;
        b[index[bucket]++] = a[i];
    }
    mtx.unlock();
    return NULL;
}

void* sort_range(void *ptr){
    struct radix_args *arg = (struct radix_args*)ptr;
    uint64_t *a = arg->a;
    size_t next_index, shift = arg->shift, i, n = arg->n, *count = arg->count, *index = arg->index;

    for (i = 0; i < 512; i++) count[i] = 0;

    /* count numbers in buckets */
    for (i = 0; i < n; i++){
        size_t bucket = (a[i] >> shift) & 0xff;
        count[bucket]++;
    }

    /* accumulate indices */
    next_index = 0;
    for (i = 0; i < 512; i++){
        index[i] = next_index;
        next_index += count[i];
    }

    put_into_buckets(arg);

    return NULL;
}

void ParallelRadixSorter::parallel_radix_sort(uint64_t *a, uint64_t *b, size_t n,int m_nthreads){
    size_t i, j, shift, next_index;
	int N_THREADS=m_nthreads;
    struct radix_args args[N_THREADS];
    /* numbers of elements to be sorted must be multiple of number of threads */
    assert(n % N_THREADS == 0);

    for (shift = 0; shift < 64; shift += 8){
        /* divide elements to be sorted into N_THREAD many ranges and sort those */
        for (j = 0; j < N_THREADS; j++){
            struct radix_args *arg = &args[j];
            arg->n = n / N_THREADS;
            arg->a = a + j*arg->n;
            arg->b = b + j*arg->n;
            arg->shift = shift;
            pthread_create(&arg->thread, NULL, sort_range, arg);
        }

        /* wait until all threads are done */
        for (j = 0; j < N_THREADS; j++){
            pthread_join(args[j].thread, NULL);
        }

        /* calculate indices for buckets after merge */
        next_index = 0;
        for (i = 0; i < 256; i++){
            for (j = 0; j < N_THREADS; j++){
                args[j].index[i] = next_index;
                next_index += args[j].count[i];
            }
        }

        /* merge buckets of threads */
        for (j = 0; j < N_THREADS; j++){
            struct radix_args *arg = &args[j];
            arg->n = n / N_THREADS;
            arg->a = b + j*arg->n;
            arg->b = a;
            arg->shift = shift;
            pthread_create(&arg->thread, NULL, put_into_buckets, arg);
        }

        /* wait until all threads are done */
        for (j = 0; j < N_THREADS; j++){
            pthread_join(args[j].thread, NULL);
        }
    }
}

void ParallelRadixSorter::sort(uint64_t* array, int array_size)
{
  // TODO: Implement Parallel RadixSort
	uint64_t *temp;
	temp = (uint64_t*)malloc(sizeof(*temp)*array_size);
	parallel_radix_sort(array,temp,array_size,m_nthreads);

}
     
