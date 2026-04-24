#include "iddfs.h"
#include <omp.h>

// Pomocná metoda pro kontrolu cyklů (na aktuální cestě)
bool is_on_path(state_ptr s) {
    auto current = s->previous_state();
    while (current != nullptr) {
        if (current->id() == s->id()) return true;
        current = current->previous_state();
    }
    return false;
}

void iddfs_worker(state_ptr current, int depth, int limit, bool& found, state_ptr& best_goal) {
    // Pokud už nějaké vlákno našlo řešení, ukonči větev
    if (found) return;

    if (current->goal()) {
#pragma omp critical (goal_lock)
        {
            if (best_goal == nullptr || current->id() < best_goal->id()) {
                best_goal = current;
            }
            found = true;
        }
        return;
    }

    if (depth == limit) return;

    auto next = current->next_states();
    for (auto& n : next) {
        if (is_on_path(n)) continue; // Vynecháme cykly

        // Tasking jen do určité hloubky, zbytek sekvenčně!
        if (depth < 4) { // Číslo 4 si případně vytuň podle domény
#pragma omp task shared(found, best_goal)
            iddfs_worker(n, depth + 1, limit, found, best_goal);
        } else {
            iddfs_worker(n, depth + 1, limit, found, best_goal);
        }
    }
}

state_ptr iddfs(state_ptr root) {
    int limit = 0;
    state_ptr best_goal = nullptr;

    while (best_goal == nullptr) {
        bool found = false;

#pragma omp parallel
        {
            // Inicializace tasků musí proběhnout jen v jednom vlákně (single)
#pragma omp single
            {
                iddfs_worker(root, 0, limit, found, best_goal);
            } // Na konci single bloku je implicitní bariéra, počká na všechny tasky
        }

        if (best_goal != nullptr) return best_goal;
        limit++;
    }
    return nullptr;
}