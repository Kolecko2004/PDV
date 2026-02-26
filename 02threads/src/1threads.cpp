#include "1threads.h"
#include "../pdv_lib/pdv_lib.hpp"
#include <cstddef>
#include <execution>
#include <thread>
#include <mutex>
#include <atomic>

const size_t THREAD_COUNT = std::thread::hardware_concurrency();

void map_sequential(std::vector<float>& data, MapFn map_fn) {
    for (float& f : data) {
        f = map_fn(f);
    }
    // C ekvivalent:
    //for (size_t i = 0; i < data.size(); i++) {
    //    data[i] = map_fn(data[i]);
    //}
}

void map_openmp(std::vector<float>& data, MapFn map_fn) {
    #pragma omp parallel for
    for (float& f : data) {
        f = map_fn(f);
    }
}


/** Naivni implementace paralelizace se sdilenym indexem. */
void map_manual(std::vector<float>& data, MapFn map_fn) {
    size_t i = 0;
    auto process = [&] {
        while (i < data.size()) {
            // kazde vlakno si ve smycce vezme a incrementuje dalsi index ke zpracovani...
            auto& entry = data[i++];
            // ...a prvek zpracuje
            entry = map_fn(entry);
        }
    };

    auto worker_threads = std::vector<std::jthread>();
    for (size_t j = 0; j < THREAD_COUNT; j++) {
        worker_threads.push_back(std::jthread(process));
    }
}

void map_manual_locking(std::vector<float>& data, MapFn map_fn) {
    std::mutex mtx;
    size_t i = 0;
    auto process = [&] {
        while (true) {
            auto lock = std::unique_lock(mtx);
            size_t idx = i++;
            lock.unlock();
            if (idx >= data.size()) {
                break;
            }
            // kazde vlakno si ve smycce vezme a incrementuje dalsi index ke zpracovani...
            auto& entry = data[idx];
            // ...a prvek zpracuje
            entry = map_fn(entry);
        }
    };

    auto worker_threads = std::vector<std::jthread>();
    for (size_t j = 0; j < THREAD_COUNT; j++) {
        worker_threads.push_back(std::jthread(process));
    }
}

void map_manual_atomic(std::vector<float>& data, MapFn map_fn) {
    std::atomic<size_t> counter = 0;
    auto process = [&] {
        while (true) {
            // auto lock = std::unique_lock(mtx);
            size_t idx = counter.fetch_add(1);
            if (idx >= data.size()) {
                break;
            }
            // kazde vlakno si ve smycce vezme a incrementuje dalsi index ke zpracovani...
            auto& entry = data[idx];
            // ...a prvek zpracuje
            entry = map_fn(entry);
        }
    };

    auto worker_threads = std::vector<std::jthread>();
    for (size_t j = 0; j < THREAD_COUNT; j++) {
        worker_threads.push_back(std::jthread(process));
    }
}

void map_manual_ranges(std::vector<float>& data, MapFn map_fn) {
    size_t data_size = data.size();
    size_t thread_count = THREAD_COUNT;
    auto worker_threads = std::vector<std::jthread>();
    worker_threads.reserve(thread_count);

    size_t size_of_part = data_size / thread_count;

    for (size_t j = 0; j < THREAD_COUNT; j++) {
        size_t start = j * size_of_part;

        size_t end = start + size_of_part;

        worker_threads.emplace_back([&, start, end]() {
            for (size_t i = start; i < end; ++i) {
                data[i] = map_fn(data[i]);
            }
        });
    }
}