#pragma once

#include <numeric>
#include <vector>
#include <iterator>
#include <omp.h>
#include "../pdv_lib/pdv_lib.hpp"

template<typename It>
void prefix_sum_seq(It begin, It end) {
    // Standardni knihovna umi prefixni sumu spocitat za nas.
    std::inclusive_scan(begin, end, begin);
}

template<typename It>
void prefix_sum_par(It begin, It end) {
    // type that the iterator refers to (the actual type of values in the array)
    using T = typename std::iterator_traits<It>::value_type;

    auto size = end - begin;

    #pragma omp parallel
    {
        // Rozdelte vypocet prefixni sumy pole mezi jednotliva vlakna
        // Zamyslete se, jak byste se mohli "vyhnout" zavislosti sumy
        // na prefixni sume predchazejicich prvku. (Teto zavislosti se
        // nelze vyhnout - ale mozna by nam stacilo "vysbirat" vysledky
        // ostatnich vlaken a nemuseli bychom prochazet cele pole?)
    }
    throw pdv::not_implemented();
}
//
//
// template<typename It>
// void prefix_sum_par(It begin, It end) {
//     // type that the iterator refers to (the actual type of values in the array)
//     using T = typename std::iterator_traits<It>::value_type;
//
//     auto size = end - begin;
//
//
//     // Sdileny vektor sum prvku v castech pole prirazenych jednotlivym vlaknum.
//     auto sums = std::vector<T>(omp_get_max_threads());
//
// #pragma omp parallel
//     {
//         // Predpocitame si, kterou cast vstupnich dat bude zpracovavat nase vlakno.
//         auto thread_i = (size_t)omp_get_thread_num();
//         auto chunk_size = 1 + size / (size_t)omp_get_num_threads();
//
//         // Ulozime si iterator na zacatek a konec nasi casti vstupnich dat.
//         auto chunk_begin = begin + thread_i * chunk_size;
//         // Minimum zajisti, ze nepreteceme za konec vstupnich dat.
//         auto chunk_end = std::min(end, begin + (thread_i + 1) * chunk_size);
//
//
//         // Spocitame sumu prvku v prirazene casti pole a zapiseme ji do sdileneho vektoru.
//         sums[thread_i] = std::reduce(chunk_begin, chunk_end);
//
//         // Pockame, az vsechny vlakna spoctou a ulozi sumu ve sve casti.
// #pragma omp barrier
//
//         // V jednom vlakne spocitame prefixni sumy bloku ve spolecnem vektoru. Pouzijeme `exclusive_scan`,
//         // ktery do i-te pozice ulozi sumu predchozich prvku, ale uz ne i-teho prvku (tedy na 1. pozici bude 0).
// #pragma omp single
//         std::exclusive_scan(sums.begin(), sums.end(), sums.begin(), 0);
//
//         // Ted uz vime, jaka je suma vsech prvku pred nasim blokem (`sums[thread_i]`), tedy muzeme spocitat prefixni
//         // sumu pro prvky v nasem bloku stejne jako v sekvencni variante.
//         *chunk_begin += sums[thread_i];
//         std::inclusive_scan(chunk_begin, chunk_end, chunk_begin);
//     }
// }