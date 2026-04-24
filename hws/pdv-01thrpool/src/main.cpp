#include "ThreadPool.h"

#include <cstdint>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <syncstream>

// Nastaveni poctu konzumentu a producentu
constexpr size_t NWORKERS = 4;
constexpr size_t NPRODUCERS = 4;

// Nastaveni poctu uloh pro kazdeho producenta a rozsah,
// ze ktereho se budou ulohy generovat
constexpr size_t NDATA = 30;
constexpr uint64_t MIN_DATA = 1000;
constexpr uint64_t MAX_DATA = 10000;

// Delka sleepu pro producenty a konzumenty, ovlivnujici
// jak rychle se ulohy vytvari a zpracovavaji
constexpr auto SLEEP_PER_STEP_WORKER = std::chrono::milliseconds(2);
constexpr auto SLEEP_PER_FASTEST_PRODUCER = std::chrono::milliseconds(20);

// Povoleni kontrolnich vypisu
constexpr bool VERBOSE = true;

/// Tato funkce pocita ulohu zadanou konzumentovi. V tomto pripade jde o problem 3n+1,
/// tzv. Colatzuv problem (https://cs.wikipedia.org/wiki/Collatzův_problém).
///
/// \remark Na testovani Vasi odevzdane ulohy budeme pouzivat i jine funkce!
void worker(uint64_t n) {
    auto orig_n = n;
    auto this_id = std::this_thread::get_id();

    uint64_t steps = 0;
    while (n > 1) {
        steps++;
        if (n % 2) n = 3 * n + 1;
        else n /= 2;
    }

    std::this_thread::sleep_for(SLEEP_PER_STEP_WORKER * steps);
    if (VERBOSE) {
        std::osyncstream(std::cerr) << "Thread " << this_id << " working on: " << orig_n << " " << steps << "\n";
    }
}

int main() {
    // Inicializace poolu vlaken, ktere zpracovavaji ulohy vytvarene producenty
    auto pool = ThreadPool<uint64_t, void (*)(uint64_t)>(NWORKERS, &worker);

    // Vytvareni vlaken producentu, produkujicich ulohy
    auto producer_threads = std::vector<std::thread>();
    for (size_t i = 1; i <= NPRODUCERS; i++) {
        producer_threads.emplace_back([&pool, i] {
            auto this_id = std::this_thread::get_id();
            auto sleep_time = i * SLEEP_PER_FASTEST_PRODUCER;

            // inicializace generatoru nahodnych cisel
            auto rng = std::mt19937(i);
            auto dist = std::uniform_int_distribution(MIN_DATA, MAX_DATA);

            for (size_t j = 1; j <= NDATA; j++) {
                // vytvorime nahodnou ulohu
                auto data = dist(rng);

                if (VERBOSE) {
                    std::osyncstream(std::cerr) << "Thread " << this_id << " pushing to queue: " << data <<
                        ", sleeping for: " << sleep_time.count() << "ms\n";
                }

                // vlozime ulohu do fronty
                pool.process(data);
                // a pockame chvili, nez zadame dalsi ulohu ke zpracovani
                std::this_thread::sleep_for(sleep_time);
            }
        });
    }

    // pockame na producenty
    for (size_t i = 0; i < NPRODUCERS; i++) producer_threads[i].join();

    if (VERBOSE) {
        std::osyncstream(std::cerr) << "Producers finished\n";
    }

    // na konec fronty vlozime 0, ktera znaci, ze ma ctouci vlakno skoncit
    for (size_t i = 0; i < NWORKERS; i++) pool.process(0);
    // Join pro konzumenty
    pool.join();
}
