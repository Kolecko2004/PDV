#include "iddfs_weighted.h"
#include <limits>
#include <omp.h>
#include <atomic>
#include <vector>
#include <algorithm>

static inline bool on_path(const std::vector<unsigned long long>& path, unsigned long long id) {
    return std::find(path.begin(), path.end(), id) != path.end();
}

static inline bool publish_goal(unsigned long long cid, std::atomic<unsigned long long>& best_id) {
    auto expected = best_id.load(std::memory_order_relaxed);
    while (cid < expected) {
        if (best_id.compare_exchange_weak(expected, cid, std::memory_order_relaxed, std::memory_order_relaxed)) {
            return true;
        }
    }
    return false;
}

static void ida_worker(const state_ptr& current, uint64_t limit, std::atomic<uint64_t>& next_limit,
                       state_ptr& best_goal, std::atomic<unsigned long long>& best_id,
                       int depth, std::vector<unsigned long long>& path) {
    uint64_t current_cost = current->total_cost();

    if (current_cost > limit) {
        uint64_t cur = next_limit.load(std::memory_order_relaxed);
        while (current_cost < cur &&
               !next_limit.compare_exchange_weak(cur, current_cost, std::memory_order_relaxed, std::memory_order_relaxed)) {
        }
        return;
    }

    if (current->goal()) {
        unsigned long long cid = current->id();
        if (publish_goal(cid, best_id)) {
#pragma omp critical(goal_lock)
            {
                if (best_goal == nullptr || cid < best_goal->id()) {
                    best_goal = current;
                }
            }
        }
        return;
    }

    auto neighbors = current->next_states();
    for (auto& n : neighbors) {
        unsigned long long nid = n->id();
        if (on_path(path, nid)) continue;

        if (depth < 2) {
            auto child_path = path;
            child_path.push_back(nid);
#pragma omp task shared(best_goal, next_limit, best_id) firstprivate(n, limit, depth, child_path)
            ida_worker(n, limit, next_limit, best_goal, best_id, depth + 1, child_path);
        } else {
            path.push_back(nid);
            ida_worker(n, limit, next_limit, best_goal, best_id, depth + 1, path);
            path.pop_back();
        }
    }
}

state_ptr iddfs_weighted(state_ptr root) {
    if (root->goal()) return root;

    uint64_t limit = root->total_cost();
    state_ptr best_goal = nullptr;

    while (best_goal == nullptr) {
        std::atomic<uint64_t> next_limit(std::numeric_limits<uint64_t>::max());
        std::atomic<unsigned long long> best_id(~0ull);
        std::vector<unsigned long long> path;
        path.reserve(64);
        path.push_back(root->id());

#pragma omp parallel
        {
#pragma omp single
            {
                ida_worker(root, limit, next_limit, best_goal, best_id, 0, path);
            }
        }

        if (best_goal != nullptr) return best_goal;

        uint64_t n_lim = next_limit.load(std::memory_order_relaxed);
        if (n_lim == std::numeric_limits<uint64_t>::max()) break;
        limit = n_lim;
    }

    return best_goal;
}