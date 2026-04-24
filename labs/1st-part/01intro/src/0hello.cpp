#include <cstdio>
#include <omp.h>

int main() {
#pragma omp parallel num_threads(16)
    {
        printf("Hello from thread %d of %d\n",
               omp_get_thread_num(),
               omp_get_num_threads());
    }
}