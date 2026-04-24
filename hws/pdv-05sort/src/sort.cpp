#include "sort.h"
#include <omp.h>
#include <vector>
#include <string>
#include <algorithm>

void radix_par(std::vector<std::string*>& vector_to_sort, MappingFunction mapping_function,
               size_t alphabet_size, size_t str_size) {

    const size_t n = vector_to_sort.size();
    if (n <= 1) return;

    std::vector<std::string*> aux(n);
    std::vector<std::string*>* src = &vector_to_sort;
    std::vector<std::string*>* dst = &aux;

    const int max_thr = omp_get_max_threads();
    const size_t row_width = (alphabet_size + 7) & ~7ULL;
    std::vector<size_t> global_hist(max_thr * row_width);
    std::vector<size_t> bucket_cache(n);

    for (int d = (int)str_size - 1; d >= 0; --d) {
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            const int num_thr = omp_get_num_threads();

            size_t* my_hist = &global_hist[tid * row_width];
            for (size_t i = 0; i < alphabet_size; ++i) my_hist[i] = 0;

            const size_t chunk = (n + num_thr - 1) / num_thr;
            const size_t start = std::min((size_t)tid * chunk, n);
            const size_t end = std::min(start + chunk, n);

            for (size_t i = start; i < end; ++i) {
                size_t b = mapping_function((*(*src)[i])[d]);
                bucket_cache[i] = b;
                my_hist[b]++;
            }

            #pragma omp barrier

            #pragma omp single
            {
                size_t total_offset = 0;
                for (size_t b = 0; b < alphabet_size; ++b) {
                    for (int t = 0; t < num_thr; ++t) {
                        size_t count = global_hist[t * row_width + b];
                        global_hist[t * row_width + b] = total_offset;
                        total_offset += count;
                    }
                }
            }

            for (size_t i = start; i < end; ++i) {
                size_t b = bucket_cache[i];
                size_t target_pos = my_hist[b]++;
                (*dst)[target_pos] = (*src)[i];
            }
        }
        std::swap(src, dst);
    }

    if (src != &vector_to_sort) {
        #pragma omp parallel for
        for (size_t i = 0; i < n; ++i) {
            vector_to_sort[i] = (*src)[i];
        }
    }
}