#include "bfs.h"
#include <vector>
#include <unordered_set>
#include <omp.h>

state_ptr bfs(state_ptr root) {
    if (root->goal()) return root;

    const size_t NUM_BUCKETS = 65536;
    std::vector<std::unordered_set<uint64_t>> cl_lists(NUM_BUCKETS);
    std::vector<omp_lock_t> locks(NUM_BUCKETS);

    for (size_t i = 0; i < NUM_BUCKETS; ++i) omp_init_lock(&locks[i]);

    std::vector<state_ptr> curr_layer;
    curr_layer.push_back(root);

    uint64_t root_id = root->id();
    cl_lists[root_id % NUM_BUCKETS].insert(root_id);

    state_ptr best_goal = nullptr;

    while (!curr_layer.empty()) {
        std::vector<state_ptr> new_layer;
        size_t current_size = curr_layer.size();
        bool goal_at_this_level = false;

        #pragma omp parallel
        {
            std::vector<state_ptr> local_new;
            local_new.reserve(current_size / omp_get_num_threads() * 2);

            #pragma omp for schedule(dynamic, 64)
            for (size_t i = 0; i < current_size; ++i) {
                auto neighbors = curr_layer[i]->next_states();

                for (auto& next_st : neighbors) {
                    uint64_t nid = next_st->id();
                    size_t b_idx = nid % NUM_BUCKETS;

                    bool is_new = false;
                    omp_set_lock(&locks[b_idx]);
                    if (cl_lists[b_idx].find(nid) == cl_lists[b_idx].end()) {
                        cl_lists[b_idx].insert(nid);
                        is_new = true;
                    }
                    omp_unset_lock(&locks[b_idx]);

                    if (is_new) {
                        if (next_st->goal()) {
                            #pragma omp critical(goal_found)
                            {
                                if (best_goal == nullptr || next_st->id() < best_goal->id()) {
                                    best_goal = next_st;
                                }
                                goal_at_this_level = true;
                            }
                        }

                        if (!goal_at_this_level) {
                            local_new.push_back(std::move(next_st));
                        }
                    }
                }
            }

            if (!local_new.empty()) {
                #pragma omp critical(layer_merge)
                {
                    new_layer.insert(new_layer.end(),
                                     std::make_move_iterator(local_new.begin()),
                                     std::make_move_iterator(local_new.end()));
                }
            }
        }

        if (best_goal != nullptr) break;
        curr_layer = std::move(new_layer);
    }

    for (size_t i = 0; i < NUM_BUCKETS; ++i) omp_destroy_lock(&locks[i]);
    return best_goal;
}