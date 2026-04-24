#include "query.h"
#include <atomic>

bool is_satisfied_for_all(const std::vector<Predicate<uint32_t> >& predicates, const std::vector<uint32_t>& data) {
    bool all_true = true;
    #pragma omp parallel for schedule(dynamic)
    for (const auto& predicate : predicates) {
        bool found = false;
        for (const auto& data : data) {
            if (predicate(data)) {
                found = true;
                break;
            }
        }
        if (!found) {
            all_true = false;
            #pragma omp cancel for
        }
    }

    return all_true;
}

bool is_satisfied_for_any(const std::vector<Predicate<uint32_t>>& predicates,
                          const std::vector<uint32_t>& data) {

    std::atomic<int> next_pred{0};
    std::atomic<bool> found{false};

    #pragma omp parallel
    {
        while (true) {

            int i = next_pred.fetch_add(1, std::memory_order_relaxed);

            if (i >= (int)predicates.size())
                break;

            for (const auto& val : data) {
                #pragma omp cancellation point parallel

                if (predicates[i](val)) {
                    found = true;

                    #pragma omp cancel parallel
                    break;
                }
            }


        }
    }

    return found.load();
}