#include "iddfs.h"
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

static void iddfs_worker(const state_ptr& current, int depth, int limit,
                         state_ptr& best_goal, std::atomic<unsigned long long>& best_id,
                         std::vector<unsigned long long>& path) {
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

    if (depth == limit) return;

    auto next_states = current->next_states();
    for (auto& n : next_states) {
        unsigned long long nid = n->id();
        if (on_path(path, nid)) continue;

        if (depth < 2) {
            auto child_path = path;
            child_path.push_back(nid);
#pragma omp task shared(best_goal, best_id) firstprivate(n, depth, limit, child_path)
            iddfs_worker(n, depth + 1, limit, best_goal, best_id, child_path);
        } else {
            path.push_back(nid);
            iddfs_worker(n, depth + 1, limit, best_goal, best_id, path);
            path.pop_back();
        }
    }
}

state_ptr iddfs(state_ptr root) {
    if (root->goal()) return root;

    int limit = 0;
    state_ptr best_goal = nullptr;

    while (best_goal == nullptr) {
        std::atomic<unsigned long long> best_id(~0ull);
        std::vector<unsigned long long> path;
        path.reserve(64);
        path.push_back(root->id());

#pragma omp parallel
        {
#pragma omp single
            {
                iddfs_worker(root, 0, limit, best_goal, best_id, path);
            }
        }

        if (best_goal != nullptr) return best_goal;
        ++limit;
    }

    return best_goal;
}