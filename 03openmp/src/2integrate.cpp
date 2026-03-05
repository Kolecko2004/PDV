#include "2integrate.hpp"
#include "../pdv_lib/pdv_lib.hpp"
#include <omp.h>
#include <algorithm>


// Nasledujici funkce odhaduji urcity integral funkce 'integrand'. Dolni mez integralu odpovida promenne
// 'a', horni mez pak ziskame jako a + step_size*step_count. Integral odhadujeme pomoci obdelnikove metody
// za pouziti 'step_count' stejne sirokych obdelniku.


double integrate_sequential(Integrand integrand, double a, double step_size, size_t step_count) {
    // Vypocet urciteho integralu se da chapat jako vypocet plochy pod krivkou funkce. Tuto plochu nebudeme
    // pocitat exaktne, ale odhadneme ji pomoci mnoziny obdelniku (a jejich obsahu). Kazdy z techto obdelniku
    // ma svuj levy okraj na pozici a + step_size*i a pravy okraj na pozici a + step_size*(i+1). Pro vypocet
    // vysky obdelniku existuje mnoho pristupu. My budeme uvazovat ten nejsnazsi a to sice pouzijeme hodnotu
    // integrovane funkce "uprostred" intervalu (tedy na pozici a + step_size*i + step_size/2 ).

    // Obsahy obdelniku scitame do promenne 'acc'
    double acc = 0.0;

    for (size_t i = 0; i < step_count; i++) {
        // Nyni prochazime jednotlive obdelniky a pricitame jejich obsah do promenne 'acc'.
        // Nejprve si vypocteme "prostredek" intervalu [a + step_size*i, a + step_size*(i+1):
        double cx = a + (double) (2 * i + 1) * step_size / 2.0;

        // Nyni pricteme obsah obdelniku. Jeho sirka odpovida promenne 'step_size'. Vysku odhadneme
        // hodnotou funkce uprostred intervalu (tj., hodnotou integrand(cx)).
        acc += integrand(cx) * step_size;
    }

    // Nakonec nascitany obsah vratime jako odhad urciteho integralu
    return acc;
}

double integrate_omp_critical(Integrand integrand, double a, double step_size, size_t step_count) {
    // Nyni vypocet integralu z funkce 'integrate_sequential' zparalelizujeme za pouziti direktiv OpenMP
    // #pragma omp parallel (slouzi ke spusteni tymu vlaken) a #pragma omp critical (ktera slouzi k vytvoreni
    // kriticke sekce, kterou v jeden cas muze vykonavat pouze jedno vlakno).
    // Obsahy obdelniku opet budeme scitat do promenne 'acc'.
    // TODO: Rozdelte celkovy interval na podintervaly prislusici jednotlivym vlaknum.
    //  Identifikujte kritickou sekci, kde musi dojit k zajisteni konzistence mezi vlakny.

    double acc = 0.0;
    #pragma omp parallel
    {
        const auto thread_block = 1 + step_count / omp_get_num_threads();

        auto end = std::min(thread_block * (omp_get_thread_num() + 1), step_count);

        for (size_t i = thread_block * omp_get_thread_num(); i < end; i++) {
            double cx = a + (double) (2 * i + 1) * step_size / 2.0;

            #pragma omp critical
            acc += integrand(cx) * step_size;

        }
    }

    return acc;
}

double
integrate_omp_atomic(Integrand integrand, double a, double step_size, size_t step_count) {
    double acc = 0.0;
    #pragma omp parallel
    {
        const auto thread_block = 1 + step_count / omp_get_num_threads();

        auto end = std::min(thread_block * (omp_get_thread_num() + 1), step_count);

        for (size_t i = thread_block * omp_get_thread_num(); i < end; i++) {
            double cx = a + (double) (2 * i + 1) * step_size / 2.0;

            #pragma omp atomic
            acc += integrand(cx) * step_size;
        }
    }

    return acc;
}

double integrate_omp_reduction(Integrand integrand, double a, double step_size, size_t step_count) {
    double acc = 0.0;
    #pragma omp parallel reduction(+:acc)
    {
        const auto thread_block = 1 + step_count / omp_get_num_threads();

        auto end = std::min(thread_block * (omp_get_thread_num() + 1), step_count);

        for (size_t i = thread_block * omp_get_thread_num(); i < end; i++) {
            double cx = a + (double) (2 * i + 1) * step_size / 2.0;

            // #pragma omp atomic
            acc += integrand(cx) * step_size;
        }
    }

    return acc;
}

double integrate_omp_for_static(Integrand integrand, double a, double step_size, size_t step_count) {
    double acc = 0.0;
    #pragma omp parallel for reduction(+:acc)
    // {
    //     const auto thread_block = 1 + step_count / omp_get_num_threads();
    //
    //     auto end = std::min(thread_block * (omp_get_thread_num() + 1), step_count);

        for (size_t i = 0; i < step_count; i++) {
            double cx = a + (double) (2 * i + 1) * step_size / 2.0;

            // #pragma omp atomic
            acc += integrand(cx) * step_size;
        }


    return acc;
}

double integrate_omp_for_dynamic(Integrand integrand, double a, double step_size, size_t step_count) {
    double acc = 0.0;
    #pragma omp parallel for reduction(+:acc) schedule(dynamic, 4)
    // {
    //     const auto thread_block = 1 + step_count / omp_get_num_threads();
    //
    //     auto end = std::min(thread_block * (omp_get_thread_num() + 1), step_count);

    for (size_t i = 0; i < step_count; i++) {
        double cx = a + (double) (2 * i + 1) * step_size / 2.0;

        // #pragma omp atomic
        acc += integrand(cx) * step_size;
    }


    return acc;
}
// domaci ukol
