// Incremental beam search: one State, reversible updates, and a compact Euler tour.
// No per-candidate State copies. Each transition advances exactly one search layer.
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

// ----- Shared hashing helpers -----
using Hash = std::uint64_t;

struct ZobristTable {
    int value_count;
    std::vector<Hash> keys;

    static Hash mix(Hash value) {
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        return value ^ (value >> 31);
    }

    ZobristTable(int feature_count, int values, Hash seed = 0x123456789abcdef0ULL)
        : value_count(values) {
        if (feature_count < 0 || values <= 0) throw std::invalid_argument("invalid Zobrist dimensions");
        keys.resize(static_cast<std::size_t>(feature_count) * values);
        for (Hash& key : keys) key = mix(seed += 0x9e3779b97f4a7c15ULL);
    }

    Hash operator()(int feature, int value) const {
        assert(feature >= 0 && value >= 0 && value < value_count);
        const auto index = static_cast<std::size_t>(feature) * value_count + value;
        assert(index < keys.size());
        return keys[index];
    }

    void replace(Hash& hash, int feature, int old_value, int new_value) const {
        hash ^= (*this)(feature, old_value) ^ (*this)(feature, new_value);
    }
};

// Reusable hash -> candidate index table. clear() normally costs O(1).
// Table-slot collisions are resolved by comparing the full 64-bit fingerprints.
class LayerHashIndex {
    struct Entry {
        Hash key = 0;
        int index = -1;
        std::uint32_t generation = 0;
    };
    std::vector<Entry> entries_;
    std::uint32_t generation_ = 1;
    std::size_t count_ = 0;

    std::size_t slot(Hash key) const {
        std::size_t index = ZobristTable::mix(key) & (entries_.size() - 1);
        while (entries_[index].generation == generation_ && entries_[index].key != key) {
            index = (index + 1) & (entries_.size() - 1);
        }
        return index;
    }

    void grow() {
        auto old = std::move(entries_);
        entries_.resize(old.size() * 2);
        count_ = 0;
        for (const Entry& entry : old) {
            if (entry.generation == generation_) insert(entry.key, entry.index);
        }
    }

public:
    explicit LayerHashIndex(std::size_t capacity = 1024) {
        std::size_t size = 2;
        while (size < capacity) size *= 2;
        entries_.resize(size);
    }

    void clear() {
        count_ = 0;
        if (++generation_ == 0) {
            for (Entry& entry : entries_) entry.generation = 0;
            generation_ = 1;
        }
    }

    int find(Hash key) const {
        const Entry& entry = entries_[slot(key)];
        return entry.generation == generation_ ? entry.index : -1;
    }

    void insert(Hash key, int index) {
        if ((count_ + 1) * 2 > entries_.size()) grow();
        Entry& entry = entries_[slot(key)];
        if (entry.generation != generation_) ++count_;
        entry = {key, index, generation_};
    }
};

// ----- Problem-specific Action, Undo, and State -----
struct Action {
    enum class Type { OpA, OpB };
    Type type = Type::OpA;
    // TODO: Add operation parameters.

    void write(std::ostream& out) const {
        (void)out;
        switch (type) {
            case Type::OpA: return; // TODO: Write OpA.
            case Type::OpB: return; // TODO: Write OpB.
        }
        throw std::invalid_argument("unknown operation type");
    }
};

struct Undo {
    // TODO: Store only changed data needed to restore the previous state.
    // Include old counters, cached evaluations, and hash values when necessary.
};

struct State {
    // TODO: Store the mutable problem state; immutable input can be shared.
    void read(std::istream& in) {
        (void)in;
        // TODO: Read and initialize.
    }

    bool can_apply(const Action& action) const {
        switch (action.type) {
            case Action::Type::OpA: return can_op_a(action);
            case Action::Type::OpB: return can_op_b(action);
        }
        return false;
    }

    void apply(const Action& action) {
        if (!can_apply(action)) throw std::invalid_argument("illegal action");
        switch (action.type) {
            case Action::Type::OpA: apply_op_a(action); return;
            case Action::Type::OpB: apply_op_b(action); return;
        }
        throw std::invalid_argument("unknown operation type");
    }

    Undo apply_with_undo(const Action& action) {
        if (!can_apply(action)) throw std::invalid_argument("illegal action");
        switch (action.type) {
            case Action::Type::OpA: {
                Undo previous = capture_op_a(action);
                apply_op_a(action);
                return previous;
            }
            case Action::Type::OpB: {
                Undo previous = capture_op_b(action);
                apply_op_b(action);
                return previous;
            }
        }
        throw std::invalid_argument("unknown operation type");
    }

    void undo(const Action& action, const Undo& previous) {
        // Precondition: undo the most recent action, after undoing its descendants.
        switch (action.type) {
            case Action::Type::OpA: undo_op_a(action, previous); return;
            case Action::Type::OpB: undo_op_b(action, previous); return;
        }
        throw std::invalid_argument("unknown operation type");
    }

private:
    // ----- OpA -----
    bool can_op_a(const Action& action) const {
        (void)action;
        return false; // TODO: Check preconditions.
    }
    void apply_op_a(const Action& action) {
        (void)action;
        // TODO: Perform the basic operation.
    }
    Undo capture_op_a(const Action& action) const {
        (void)action;
        return {}; // TODO: Save the values that apply_op_a will change.
    }
    void undo_op_a(const Action& action, const Undo& previous) {
        (void)action;
        (void)previous;
        // TODO: Restore the saved values.
    }

    // ----- OpB -----
    bool can_op_b(const Action& action) const {
        (void)action;
        return false; // TODO: Check preconditions.
    }
    void apply_op_b(const Action& action) {
        (void)action;
        // TODO: Perform the basic operation.
    }
    Undo capture_op_b(const Action& action) const {
        (void)action;
        return {}; // TODO: Save the values that apply_op_b will change.
    }
    void undo_op_b(const Action& action, const Undo& previous) {
        (void)action;
        (void)previous;
        // TODO: Restore the saved values.
    }
};

// ----- Problem-specific search policy -----
struct Evaluation {
    double value = 0;        // Larger is better for beam selection.
    double answer_value = 0; // Larger is better for final answer selection.
    double remaining_steps = 0;
    std::optional<Hash> key; // Missing keys are rejected when deduplication is enabled.
    bool terminal = false;
    bool answer = true;
};
struct Transition {
    Action action;
    Evaluation next;
};

struct SearchPolicy {
    void enumerate_actions(const State& state, std::vector<Action>& actions) const {
        // TODO: Append legal actions to the cleared reusable buffer.
        (void)state; (void)actions;
    }
    Evaluation describe(const State& state, int depth, int max_depth) const {
        // TODO: Set evaluation, answer score, hash, and stopping conditions.
        // Evaluation and final answer score can use different objective functions.
        (void)state;
        Evaluation out;
        out.remaining_steps = std::max(0, max_depth - depth);
        return out;
    }
    void expand(State& state, int depth, int max_depth,
                std::vector<Action>& actions, std::vector<Transition>& out) const {
        // Generic implementation: apply/describe/undo every child, without state copies.
        // For a cheap incremental evaluation, replace this body with direct generation
        // of Transition metadata. Only selected tree edges will then be applied.
        // Contract: leave state exactly as it was on entry, even when it has caches.
        actions.clear();
        enumerate_actions(state, actions);
        for (const Action& action : actions) {
            Undo previous = state.apply_with_undo(action);
            out.push_back({action, describe(state, depth + 1, max_depth)});
            state.undo(action, previous);
        }
    }
};

// ----- Shared adaptive width controller -----
struct BeamBudget {
    using Clock = std::chrono::steady_clock;
    Clock::time_point deadline;
    int initial_width;
    int min_width;
    int max_width;
    double seconds_per_parent = 0.0;

    double remaining_seconds() const {
        return std::max(0.0, std::chrono::duration<double>(deadline - Clock::now()).count());
    }

    bool expired() const { return Clock::now() >= deadline; }

    void observe(int expanded_parents, double seconds) {
        if (expanded_parents == 0) return;
        // Measure the whole layer, including enumeration, evaluation, and sorting.
        // Changing branching factors are reflected in the observed parent cost.
        const double sample = std::max(seconds / expanded_parents, 1e-9);
        constexpr double ALPHA = 0.3;
        seconds_per_parent = seconds_per_parent == 0.0 ? sample :
            ALPHA * sample + (1.0 - ALPHA) * seconds_per_parent;
    }

    int suggest_width(double remaining_steps) const {
        if (!std::isfinite(remaining_steps) || remaining_steps < 0.0) {
            throw std::invalid_argument("invalid remaining step estimate");
        }
        const double remaining = remaining_seconds();
        if (remaining == 0.0) return 0;
        if (seconds_per_parent == 0.0) return initial_width;

        // Leave slack for estimation error. This predicts cost; it is not a guarantee.
        constexpr double UTILIZATION = 0.8;
        const double width = UTILIZATION * remaining /
            (std::max(1.0, remaining_steps) * seconds_per_parent);
        return static_cast<int>(std::clamp(width, static_cast<double>(min_width),
                                                static_cast<double>(max_width)));
    }
};

// ----- Shared incremental beam search -----
struct BeamSearchResult {
    bool found = false;
    bool timed_out = false;
    double evaluation = -std::numeric_limits<double>::infinity();
    std::vector<Action> actions;
    std::uint64_t generated = 0, duplicates = 0, applied = 0, reverted = 0;
    std::size_t history_nodes = 0, peak_tour = 0;
};

struct BeamSearch {
    int initial_width = 50, min_width = 1, max_width = 500;
    int max_depth = 100;
    double time_limit = 1.9, reserve_time = 0.05;
    bool deduplicate = false; // Enable after implementing the state hash.
    bool stop_on_first_answer = false; // Use only when any first answer is sufficient.
    SearchPolicy policy;

    BeamSearchResult run(const State& initial) const {
        if (min_width <= 0 || initial_width < min_width || max_width < initial_width ||
            max_depth < 0 || !std::isfinite(time_limit) || time_limit < 0 ||
            !std::isfinite(reserve_time) || reserve_time < 0) {
            throw std::invalid_argument("invalid beam search parameters");
        }
        using Clock = BeamBudget::Clock;
        const auto deadline = Clock::now() + std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(std::max(0.0, time_limit - reserve_time)));
        BeamBudget budget{deadline, initial_width, min_width, max_width};
        struct Leaf { Evaluation data; int history = -1; };
        struct History { int parent; Action action; };
        struct Candidate { Transition move; int parent; std::size_t ordinal; };
        enum class Kind { Enter, Leaf, Exit };
        struct Event { Kind kind; int leaf = -1; Action action{}; };
        struct Applied { Action action; Undo previous; };
        struct Frame { int enter; bool needed; };

        State state = initial; // The only full State copy made by the search.
        std::vector<Leaf> leaves{{policy.describe(state, 0, max_depth), -1}}, next_leaves;
        std::vector<Event> tour{{Kind::Leaf, 0, {}}}, next_tour;
        std::vector<History> history;
        std::vector<Applied> applied;
        std::vector<Frame> frames;
        std::vector<unsigned char> needed;
        std::vector<int> order, head, sibling, matching, build_stack;
        std::vector<int> heap, heap_position;
        std::vector<Action> actions;
        std::vector<Transition> transitions;
        std::vector<Candidate> candidates;
        LayerHashIndex seen;
        BeamSearchResult result;
        int best_parent = -1, best_depth = 0, best_rank = 0;
        std::size_t best_ordinal = 0;
        std::optional<Action> best_action;
        if (leaves[0].data.answer) {
            result.found = true;
            result.evaluation = leaves[0].data.answer_value;
        }

        const auto restore_root = [&] {
            while (!applied.empty()) {
                state.undo(applied.back().action, applied.back().previous);
                applied.pop_back();
                ++result.reverted;
            }
        };
        const auto mark_needed = [&](const auto& wanted) {
            needed.assign(tour.size(), false);
            frames.clear();
            for (std::size_t i = 0; i < tour.size(); ++i) {
                const Event& event = tour[i];
                if (event.kind == Kind::Enter) {
                    frames.push_back({static_cast<int>(i), false});
                } else if (event.kind == Kind::Leaf) {
                    if (wanted(event.leaf)) {
                        needed[i] = true;
                        if (!frames.empty()) frames.back().needed = true;
                    }
                } else {
                    Frame frame = frames.back();
                    frames.pop_back();
                    needed[frame.enter] = needed[i] = frame.needed;
                    if (frame.needed && !frames.empty()) frames.back().needed = true;
                }
            }
            assert(frames.empty());
        };

        for (int depth = 0; depth < max_depth && !leaves.empty(); ++depth) {
            if (stop_on_first_answer && result.found) break;
            if (budget.expired()) { result.timed_out = true; break; }
            const auto started = Clock::now();
            const int parent_count = std::min(static_cast<int>(leaves.size()),
                                             budget.suggest_width(leaves[0].data.remaining_steps));
            mark_needed([&](int rank) { return rank < parent_count && !leaves[rank].data.terminal; });
            candidates.clear();
            heap.clear();
            heap_position.clear();
            if (deduplicate) seen.clear();
            int inserted_since_rebuild = 0;
            const auto better_candidate = [](const Candidate& lhs, const Candidate& rhs) {
                if (lhs.move.next.value != rhs.move.next.value) return lhs.move.next.value > rhs.move.next.value;
                return std::tie(lhs.parent, lhs.ordinal) < std::tie(rhs.parent, rhs.ordinal);
            };
            const auto swap_heap = [&](int a, int b) {
                std::swap(heap[a], heap[b]);
                heap_position[heap[a]] = a;
                heap_position[heap[b]] = b;
            };
            const auto sift_down = [&](int position) {
                while (2 * position + 1 < static_cast<int>(heap.size())) {
                    int child = 2 * position + 1;
                    if (child + 1 < static_cast<int>(heap.size()) &&
                        better_candidate(candidates[heap[child]], candidates[heap[child + 1]])) ++child;
                    if (!better_candidate(candidates[heap[position]], candidates[heap[child]])) break;
                    swap_heap(position, child);
                    position = child;
                }
            };
            int expanded = 0;
            for (std::size_t event_index = 0; event_index < tour.size(); ++event_index) {
                if (!needed[event_index]) continue;
                if ((event_index & 63) == 0 && budget.expired()) { result.timed_out = true; break; }
                const Event& event = tour[event_index];
                if (event.kind == Kind::Enter) {
                    Undo previous = state.apply_with_undo(event.action);
                    applied.push_back({event.action, std::move(previous)});
                    ++result.applied;
                } else if (event.kind == Kind::Exit) {
                    state.undo(applied.back().action, applied.back().previous);
                    applied.pop_back();
                    ++result.reverted;
                } else {
                    if (budget.expired()) { result.timed_out = true; break; }
                    const int rank = event.leaf;
                    ++expanded;
                    transitions.clear();
                    policy.expand(state, depth, max_depth, actions, transitions);
                    for (std::size_t ordinal = 0; ordinal < transitions.size(); ++ordinal) {
                        const Transition& move = transitions[ordinal];
                        const Evaluation& data = move.next;
                        ++result.generated;
                        const int child_depth = depth + 1;
                        const bool earlier = std::tie(child_depth, rank, ordinal) <
                            std::tie(best_depth, best_rank, best_ordinal);
                        if (data.answer && (!result.found || data.answer_value > result.evaluation ||
                            (data.answer_value == result.evaluation && earlier))) {
                            result.found = true;
                            result.evaluation = data.answer_value;
                            best_parent = leaves[rank].history;
                            best_depth = depth + 1;
                            best_rank = rank;
                            best_ordinal = ordinal;
                            best_action = move.action;
                        }
                        // Keep an already generated valid answer before honoring a deadline.
                        if ((ordinal & 63) == 0 && budget.expired()) { result.timed_out = true; break; }
                        if (data.terminal || depth + 1 == max_depth) continue;
                        Candidate proposed{move, rank, ordinal};
                        if (candidates.size() == static_cast<std::size_t>(max_width) &&
                            !better_candidate(proposed, candidates[heap[0]])) continue;
                        int index = -1;
                        if (deduplicate) {
                            if (!data.key) throw std::logic_error("missing state hash; fill describe/expand or disable deduplicate");
                            index = seen.find(*data.key);
                            // Old mappings can refer to a slot replaced by a different key.
                            if (index != -1 && candidates[index].move.next.key != data.key) index = -1;
                        }
                        if (index != -1) {
                            ++result.duplicates;
                            const Candidate& old = candidates[index];
                            if (!better_candidate(proposed, old)) continue;
                            candidates[index] = std::move(proposed);
                            sift_down(heap_position[index]);
                        } else {
                            if (candidates.size() < static_cast<std::size_t>(max_width)) {
                                index = static_cast<int>(candidates.size());
                                candidates.push_back(std::move(proposed));
                                heap_position.push_back(static_cast<int>(heap.size()));
                                heap.push_back(index);
                                int position = static_cast<int>(heap.size()) - 1;
                                while (position != 0) {
                                    int parent = (position - 1) / 2;
                                    if (!better_candidate(candidates[heap[parent]], candidates[heap[position]])) break;
                                    swap_heap(parent, position);
                                    position = parent;
                                }
                            } else {
                                index = heap[0];
                                candidates[index] = std::move(proposed);
                                sift_down(0);
                            }
                            if (deduplicate) {
                                seen.insert(*data.key, index);
                                // Periodically remove stale mappings, bounding table memory by O(max_width).
                                if (++inserted_since_rebuild >= max_width) {
                                    seen.clear();
                                    for (int slot = 0; slot < static_cast<int>(candidates.size()); ++slot)
                                        seen.insert(*candidates[slot].move.next.key, slot);
                                    inserted_since_rebuild = 0;
                                }
                            }
                        }
                    }
                    if (result.timed_out || (stop_on_first_answer && result.found)) break;
                }
            }
            restore_root();
            if (budget.expired()) result.timed_out = true;
            if (result.timed_out || (stop_on_first_answer && result.found) || candidates.empty()) break;

            order.resize(candidates.size());
            std::iota(order.begin(), order.end(), 0);
            const auto better = [&](int a, int b) {
                const Candidate& lhs = candidates[a];
                const Candidate& rhs = candidates[b];
                if (lhs.move.next.value != rhs.move.next.value) return lhs.move.next.value > rhs.move.next.value;
                return std::tie(lhs.parent, lhs.ordinal) < std::tie(rhs.parent, rhs.ordinal);
            };
            const int best = *std::min_element(order.begin(), order.end(), better);
            const int width = budget.suggest_width(candidates[best].move.next.remaining_steps);
            if (width == 0) { result.timed_out = true; break; }
            if (order.size() > static_cast<std::size_t>(width)) {
                std::nth_element(order.begin(), order.begin() + width, order.end(), better);
                order.resize(width);
            }
            std::sort(order.begin(), order.end(), better);
            if (budget.expired()) { result.timed_out = true; break; }

            head.assign(leaves.size(), -1);
            sibling.resize(order.size());
            next_leaves.clear();
            for (std::size_t i = 0; i < order.size(); ++i) {
                const Candidate& candidate = candidates[order[i]];
                const int id = static_cast<int>(history.size());
                history.push_back({leaves[candidate.parent].history, candidate.move.action});
                next_leaves.push_back({candidate.move.next, id});
            }
            for (int i = static_cast<int>(order.size()) - 1; i >= 0; --i) {
                const int parent = candidates[order[i]].parent;
                sibling[i] = head[parent];
                head[parent] = i;
            }
            mark_needed([&](int rank) { return head[rank] != -1; });
            next_tour.clear();
            matching.clear();
            build_stack.clear();
            const auto append = [&](Event event) {
                const int index = static_cast<int>(next_tour.size());
                if (event.kind == Kind::Enter) build_stack.push_back(index);
                else if (event.kind == Kind::Exit) {
                    matching[build_stack.back()] = index;
                    build_stack.pop_back();
                }
                next_tour.push_back(std::move(event));
                matching.push_back(-1);
            };
            for (std::size_t i = 0; i < tour.size(); ++i) {
                if (!needed[i]) continue;
                const Event& event = tour[i];
                if (event.kind != Kind::Leaf) append(event);
                else for (int child = head[event.leaf]; child != -1; child = sibling[child]) {
                    append({Kind::Enter, -1, candidates[order[child]].move.action});
                    append({Kind::Leaf, child, {}});
                    append({Kind::Exit, -1, {}});
                }
            }
            assert(build_stack.empty());
            // Permanently advance the shared prefix instead of repeatedly undoing it.
            std::size_t first = 0, last = next_tour.size();
            while (first < last && next_tour[first].kind == Kind::Enter &&
                   matching[first] == static_cast<int>(last - 1)) {
                (void)state.apply_with_undo(next_tour[first].action);
                ++result.applied;
                ++first;
                --last;
            }
            if (first != 0) {
                std::move(next_tour.begin() + first, next_tour.begin() + last, next_tour.begin());
                next_tour.resize(last - first);
            }
            tour.swap(next_tour);
            leaves.swap(next_leaves);
            result.peak_tour = std::max(result.peak_tour, tour.size());
            budget.observe(expanded, std::chrono::duration<double>(Clock::now() - started).count());
        }
        result.history_nodes = history.size();
        if (result.found) {
            for (int id = best_parent; id != -1; id = history[id].parent) result.actions.push_back(history[id].action);
            std::reverse(result.actions.begin(), result.actions.end());
            if (best_action) result.actions.push_back(std::move(*best_action));
        }
        return result;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    State initial;
    initial.read(std::cin);
    BeamSearch search;
    // TODO: Set the time budget, depth cap, and width bounds.
    const auto result = search.run(initial);
    if (!result.found) return 1; // TODO: Use a valid fallback for completion-only problems.
    for (const auto& action : result.actions) action.write(std::cout);
    return 0;
}
