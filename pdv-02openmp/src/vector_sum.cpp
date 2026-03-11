#include <omp.h>
// V teto domaci uloze se Vas budeme snazit presvedcit, ze vykon vasi implementace zavisi do znacne miry na podobe
// vstupnich dat. Pri navrhu efektivnich algoritmu byste se tedy meli rozhodovat i na zaklade datove sady.
//
// Vasim ukolem je doimplementovat 4 ruzne metody na pocitani sumy kazdeho vstupniho vektoru (`data`).
// V kazde metode budete pouzitivat OpenMP, ale pokazde trochu jinym zpusobem. Rychlost vypoctu Vasi implementace
// budeme porovnavat s nasi referencni implementaci. Vychazet muzete z metody `vector_sum_sequential` implementovane
// ve `vector_sum.h`.

#include "vector_sum.h"

// typove aliasy pouzite v argumentech jsou definovane ve `vector_sum.h`
void vector_sum_omp_per_vector(const InputVectors& data, OutputVector& solution, size_t min_vector_size) {
    // V teto metode si vyzkousite paralelizaci na urovni vektoru. Naimplementujte paralelni pristup
    // k vypocteni sum jednotlivych vektoru (sumu vektoru data[i] ulozte do solution[i]). V teto
    // funkci zpracovavejte jednotlive vektory sekvencne a paralelizujte nalezeni sumy v jednom
    // konkretnim vektoru. Tento pristup by mel byt zejmena vhodny, pokud mate maly pocet vektoru
    // velke delky.

    #pragma omp parallel
    {
        int64_t sum = 0;
        const auto thread_block = 1 + data.size() / omp_get_num_threads();
        auto end = std::min(thread_block * (omp_get_thread_num() + 1), data.size());

        for (size_t i = thread_block * omp_get_thread_num(); i < end; i++) {
            for (auto n : data[i]) {
                sum += n;
            }
            solution[i] = sum;
        }
    }

}

void vector_sum_omp_static(const InputVectors& data, OutputVector& solution, size_t min_vector_size) {
    // Pokud vektory, ktere zpracovavame nejsou prilis dlouhe, ale naopak jich musime zpracovat
    // velky pocet, muzeme zparalelizovat vnejsi for smycku, ktera prochazi pres jednotlive
    // vektory. Vyuzijte paralelizaci pres #pragma omp parallel for se statickym schedulingem.
}

void vector_sum_omp_dynamic(const InputVectors& data, OutputVector& solution, size_t min_vector_size) {
    // Na cviceni jsme si ukazali, ze staticky scheduling je nevhodny, pokud praci mezi
    // jednotliva vlakna nedokaze rozdelit rovnomerne. V teto situaci muze byt vhodnym
    // resenim pouziti dynamickeho schedulingu. Ten je rovnez vhodny v situacich, kdy
    // nevime predem, jak dlouho budou jednotlive vypocty trvat. Dani za to je vyssi
    // rezie pri zjistovani indexu 'i', ktery ma vlakno v dane chvili zpracovavat.
}

void vector_sum_omp_shuffle(const InputVectors& data, OutputVector& solution, size_t min_vector_size) {
    // Dalsi moznosti, jak se vyhnout problemum pri pouziti statickeho schedulingu (tj.,
    // zejmena nevyvazenemu rozdeleni prace) je predzpracovani dat. V teto funkci zkuste
    // data pred zparalelizovanim vnejsi for smycky (se statickym schedulingem) nejprve
    // prohazet. To samozrejme predpoklada, ze cas potrebny na predzpracovani je radove
    // mensi, nez zisk, ktery ziskame nahrazenim dynamickeho schedulingu za staticky.
    //
    // Tip: Abyste se vyhnuli kopirovani vektoru pri "prohazovani" (ktere muze byt velmi
    // drahe!), muzete prohazovat pouze pointery na vektory. Alternativou je vytvorit si
    // nejprve pole nahodne serazenych indexu a ty pak pouzit pro pristup k poli.
    // Uzitecnymi metodami mohou byt metody `std::iota(...)` (ktera Vam vygeneruje posloupnost
    // indexu od 0 do N) a `std::shuffle(...)`, ktera zajisti nahodne prohazeni prvku.
}
