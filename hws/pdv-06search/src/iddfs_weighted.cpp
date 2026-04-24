#include "iddfs_weighted.h"
#include <limits>
#include <omp.h>
#include <algorithm>

// Pomocná funkce pro kontrolu cyklů (používá metodu previous_state() ze state.h)
bool is_on_path_weighted(state_ptr s) {
    auto current = s->previous_state();
    while (current) {
        if (current->id() == s->id()) return true;
        current = current->previous_state();
    }
    return false;
}

void ida_worker(state_ptr current, uint64_t limit, uint64_t& next_limit,
                state_ptr& best_goal, bool& found) {

    // OPRAVA: Použití total_cost() místo cost()
    uint64_t current_cost = current->total_cost();

    if (current_cost > limit) {
        #pragma omp critical(limit_lock)
        {
            if (current_cost < next_limit) next_limit = current_cost;
        }
        return;
    }

    if (current->goal()) {
        #pragma omp critical(goal_lock)
        {
            if (best_goal == nullptr || current->id() < best_goal->id()) {
                best_goal = current;
            }
            found = true;
        }
        return;
    }

    auto neighbors = current->next_states();
    for (auto& next_st : neighbors) {
        if (is_on_path_weighted(next_st)) continue;

        // Tasking pro paralelizaci (hloubka/cena omezena pro snížení overheadu)
        if (current_cost < limit / 2) {
            #pragma omp task shared(found, best_goal, next_limit)
            ida_worker(next_st, limit, next_limit, best_goal, found);
        } else {
            ida_worker(next_st, limit, next_limit, best_goal, found);
        }
    }
}

state_ptr iddfs_weighted(state_ptr root) {
    if (root->goal()) return root;

    // OPRAVA: Použití total_cost() místo cost()
    uint64_t limit = root->total_cost();
    state_ptr best_goal = nullptr;

    while (best_goal == nullptr) {
        uint64_t next_limit = std::numeric_limits<uint64_t>::max();
        bool found = false;

        #pragma omp parallel
        {
            #pragma omp single
            {
                ida_worker(root, limit, next_limit, best_goal, found);
            }
        }

        if (best_goal == nullptr && next_limit == std::numeric_limits<uint64_t>::max()) {
            break;
        }

        limit = next_limit;
    }

    return best_goal;
}