#pragma once

#include <limits>
#include <vector>
#include <atomic>
#include "./3prefixsum.h"

template<typename T>
void countsort_seq(std::vector<T>& data) {
    // find the lowest and highest values in the input
    auto min = std::numeric_limits<T>::max();
    auto max = std::numeric_limits<T>::min();
    for (auto& n : data) {
        if (n < min) min = n;
        if (n > max) max = n;
    }

    // count each value in the vector in the min/max range
    auto range = max - min + 1;
    auto counts = std::vector<size_t>(range);
    for (auto& n : data) {
        counts[n - min]++;
    }

    // reconstruct the vector content from the counts
    auto it = data.begin();
    for (size_t i = 0; i < counts.size(); i++) {
        // set `counts[i]` values to `min + i`, return iterator to the end of the range
        it = std::fill_n(it, counts[i], min + i);
    }
}

// Jedna varianta jak spocitat histogram paralelne je pouzit atomicky histogram. Tento pristup se muze vyplatit,
// pokud je relativne malo vlaken, velky rozsah cisel na vstupu (max - min), a jsou relativne rovnomerne rozlozena,
// pak se bude stavat relativne vzacne, ze se budou vlakna "pretahovat" o jeden index, a usetrime pametovy overhead
// z toho, ze by kazde vlakno melo svuj histogram.
template<typename T>
std::vector<size_t> build_histogram_par_atomic(const std::vector<T>& data, T min, T max) {
    // create a shared histogram for all threads; if we used std::atomic, we would have to copy the vector over to
    //  a non-atomic one before returning it, since C++ does not have any way to say "I no longer need these value to
    //  be atomic", which is somewhat inconvenient; instead, we use `std::atomic_ref` below, which allows us to
    //  temporarily treat a non-atomic value as atomic
    auto histogram = std::vector<size_t>(max - min + 1);
    #pragma omp parallel for
    for (auto& n : data) {
        // temporarily treat the histogram value as atomic
        auto ref = std::atomic_ref(histogram[n - min]);
        // atomically increment the corresponding counter
        ref.fetch_add(1, std::memory_order::relaxed);
    }
    return histogram;
}

template <typename T>
static void vec_reduce_add(std::vector<T>& out, const std::vector<T>& in) {
    for (auto it = in.begin(); auto& n : out) {
        n += *it++;
    }
}

// Druha alternativa jak paralelne spocitat histogram je pouzit lokalni histogram v kazdem vlakne a pote histogramy
// zkombinovat pomoci redukce. Bud je mozne redukovat manualne pomoci kriticke sekce, nebo muzeme vyuzit moznosti
// nadefinovat vlastni redukci, ktera bude mirne efektivnejsi a (subjektivne) citelnejsi.
//
// Tento pristup se hodi, pokud mame hodne vlaken a relativne maly rozsah cisel na vstupu, coz u counting sortu
// zpravidla plati (kdyby byl rozsah vstupu velky, proc bychom pouzivali counting sort?).
template<typename T>
std::vector<size_t> build_histogram_par_reduction(const std::vector<T>& data, T min, T max) {
    #pragma omp declare \
        reduction(vec_add : std::vector<size_t> : vec_reduce_add(omp_out, omp_in)) \
        initializer(omp_priv = omp_orig)

    // `reduction` will create a private copy of this histogram for each thread, and then combine the copies using
    //  the `vec_add` custom reduction we defined above, which calls `vec_reduce_add` on each pair of histograms
    auto histogram = std::vector<size_t>(max - min + 1);
    #pragma omp parallel for reduction(vec_add : histogram)
    for (auto& n : data) {
        histogram[n - min]++;
    }
    return histogram;
}

template<typename T>
void countsort_par(std::vector<T>& data) {

    // Provedeme nad polem redukci pocitajici minimum a maximum.
    auto min = std::numeric_limits<T>::max();
    auto max = std::numeric_limits<T>::min();
    // it would be elegant to use a for-range loop here, but in this specific case, GCC is unable to see through
    //  the iterator and properly vectorize it, probably because the OpenMP directive is compiled before the iterator
    //  gets inlined to raw pointer arithmetic; see `pdv_lib/tests/perf_reproductions/vec_iterator_simd.cpp` for a repro
    #pragma omp parallel for reduction(max:max) reduction(min:min)
    for (size_t i = 0; i < data.size(); i++) {
        auto& el = data[i];
        if (el < min) min = el;
        if (el > max) max = el;
    }

    // Jednim ze dvou implementovanych zpusobu spocitame histogram.
    auto counts = build_histogram_par_reduction(data, min, max);
    //auto counts = build_histogram_par_atomic(data, min, max);

    // Abychom mohli setridene pole vypisovat paralelne, musi kazde vlakno vedet,
    // na jake indexy ma zapisovat jake prvky, bez toho aby muselo cekat na
    // vypsani prvku mensich. Takovou informaci lze ziskat vypoctem prefixni
    // sumy (viz slidy).
    prefix_sum_par(counts.begin(), counts.end());

    // Nakonec jiz staci pole vypsat. Diky prefixni sume presne vime,
    // jaky prvek ma byt na jakem indexu nezavisle na poctu mensich prvku
    #pragma omp parallel for
    for (size_t i = 0; i < counts.size(); i++) {
        // Vytvorime iteratory na zacatek a konec zpracovavaneho rozsahu.
        auto begin = data.begin() + (i == 0 ? 0 : counts[i - 1]);
        auto end = data.begin() + counts[i];
        // A pouzijeme funkci ze standardni knihovny, ktera vyplni dany rozsah hodnotou `min + i`.
        std::fill(begin, end, min + i);
    }
}
