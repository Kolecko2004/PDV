#include <iostream>
#include <atomic>
#include <omp.h>
#include "../pdv_lib/benchmark.hpp"

// Collatzuv problem (znamy take jako uloha 3n+1) je definovany jako posloupnost cisel generovana podle
// nasledujicich pravidel:
//  1) Pokud je n liche, dalsi prvek posloupnosti je 3n+1
//  2) Pokud je n sude, dalsi prvek posloupnosti je n/2
// Neni znamo, zda pro libovolne prirozene cislo n posloupnost cisel "dosahne" cisla 1, ale je pravdepodobne,
// ze ano. Na dnesnim cviceni nas bude zajimat, kolik operaci (1) a (2) je pro to potreba. To muzeme zjistit
// pomoci nasledujici jednoduche funkce:
uint64_t collatz(uint64_t n) {
    uint64_t steps;
    for (steps = 0; n > 1; steps++) {
        if (n % 2) n = 3 * n + 1;
        else n /= 2;
    }
    return steps;
}

// V druhe sade uloh `findn_*` se zamerime na opacny problem. Dostaneme delku sekvence `criteria` a nasim ukolem
// bude nalezt cislo `n`, pro ktere je hodnota `collatz(n) >= criteria`. Hodnotu parametru 'criteria' si muzete
// upravit ve funkci `main`.
//
// Vsimnete si, ze v tomto pripade nevime predem kolik prvku budeme muset zpracovat, nez narazime na potrebne 'n'.
// Nevime proto predem, jak mame data rozdelit mezi jednotliva vlakna.

uint64_t findn_sequential(uint64_t criteria) {
    for (uint64_t i = 1;; i++) {
        if (collatz(i) >= criteria) return i;
    }
}

uint64_t findn_parallel(uint64_t criteria) {
    // TODO: paralelizujte hledani hodnoty `n`, pro kterou je pocet kroku vyssi nez `criteria`
    std::atomic<uint64_t> next_n {1};
    std::atomic<uint64_t> result {};

    #pragma omp parallel
    while(true) {
        auto n = next_n++;
        #pragma omp cancellation point parallel
        if (collatz(n) >= criteria) {
            result = n;
            #pragma omp cancel parallel
        }
    }

    return result;
}


int main() {
    if (!omp_get_cancellation()) {
        std::cout << "-----------------------------------------------------------------------------\n";
        std::cout << "| WARNING: OpenMP cancellations are not enabled                             |\n";
        std::cout << "| You can enable them by setting environment variable OMP_CANCELLATION=true |\n";
        std::cout << "-----------------------------------------------------------------------------\n\n";
    }

    constexpr uint64_t CRITERIA = 450;

    pdv::benchmark("findn_sequential", 15, [&] { return findn_sequential(pdv::launder_value(CRITERIA)); });
    pdv::benchmark("findn_parallel", 15, [&] { return findn_parallel(pdv::launder_value(CRITERIA)); });
}
