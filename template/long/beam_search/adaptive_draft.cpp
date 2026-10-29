// Draft: ordinary beam search with a measured-cost adaptive width.
// The width update is a design example, not code copied from the AHC066 editorial.
// Port Action/State unchanged; enumerate_actions now fills a reusable output buffer.
// For a simple-search comparison, fix all three width bounds and disable deduplicate.
// Implement state_hash before enabling deduplication; cache its key in State as needed.
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

// ----- Problem-specific Action and State -----
struct Action {
    enum class Type { OpA, OpB };  // Rename, add, or remove operation types.
    Type type = Type::OpA;

    // TODO: Add parameters for OpA.

    // TODO: Add parameters for OpB.

    void write(std::ostream& out) const {
        (void)out;
        switch (type) {
            case Type::OpA:
                // TODO: Write OpA and its parameters.
                return;
            case Type::OpB:
                // TODO: Write OpB and its parameters.
                return;
        }
        throw std::invalid_argument("unknown operation type");
    }
};

struct State {
    // TODO: Store the problem data. Copies must be independent.

    void read(std::istream& in) {
        // TODO: Read the input and initialize the state.
        (void)in;
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
            case Action::Type::OpA: apply_op_a(action); break;
            case Action::Type::OpB: apply_op_b(action); break;
        }
    }

private:
    // ----- OpA -----
    bool can_op_a(const Action& action) const {
        // TODO: Check the official preconditions for OpA.
        (void)action;
        return false;
    }

    void apply_op_a(const Action& action) {
        // TODO: Apply OpA, including its automatic effects and counters.
        (void)action;
    }

    // ----- OpB -----
    bool can_op_b(const Action& action) const {
        // TODO: Check the official preconditions for OpB.
        (void)action;
        return false;
    }

    void apply_op_b(const Action& action) {
        // TODO: Apply OpB, including its automatic effects and counters.
        (void)action;
    }
};

// ----- Problem-specific search policy -----
struct SearchPolicy {
    void enumerate_actions(const State& state, std::vector<Action>& actions) const {
        // TODO: Append legal actions to the reusable buffer, already cleared by the search.
        // State::apply keeps its validation; the search does not repeat can_apply.
        (void)state;
        (void)actions;
    }

    double evaluate(const State& state) const {
        // TODO: Compute the full evaluation. Larger is better.
        // Values must be comparable across depths; this also selects the answer.
        (void)state;
        return 0.0;
    }

    bool is_terminal(const State& state) const {
        // TODO: Return true when this state should not be expanded further.
        (void)state;
        return false;
    }

    bool is_answer(const State& state) const {
        // TODO: Return true only when stopping here gives a valid final answer.
        (void)state;
        return true;
    }

    double estimate_remaining_steps(const State& state, int depth, int max_depth) const {
        // Known depth: leave this as max_depth - depth.
        // Unknown depth: estimate layers until completion using problem progress.
        // The search passes the best-ranked state as a representative of the beam.
        // Include the current layer. Return a finite, nonnegative estimate.
        (void)state;
        return std::max(0, max_depth - depth);
    }

    Hash state_hash(const State& state) const {
        // TODO: Return the cached Zobrist hash, or compute it here initially.
        // A cached hash can be initialized in State::read and updated in State::apply.
        // Include all information that changes future choices or their value.
        // Equal keys are merged only within one layer, keeping the better evaluation.
        // This assumes the better evaluation dominates for equivalent future states.
        // Hash collisions between different states remain probabilistically possible.
        (void)state;
        throw std::logic_error("implement state_hash or set deduplicate = false");
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

// ----- Ordinary beam search with adaptive width -----
struct BeamSearchResult {
    bool found = false;
    bool timed_out = false;
    double evaluation = -std::numeric_limits<double>::infinity();
    std::vector<Action> actions;
    std::uint64_t generated = 0;
    std::uint64_t duplicates = 0;
    std::size_t history_nodes = 0;
};

struct BeamSearch {
    int initial_width = 50;
    int min_width = 1;
    int max_width = 500;
    int max_depth = 100;       // Hard depth cap, also used by the default estimator.
    double time_limit = 1.9;    // Seconds from entry to run(); excludes prior work.
    double reserve_time = 0.05; // Slack for reconstruction, output, and timing checks.
    bool deduplicate = false;   // Enable after implementing state_hash.
    SearchPolicy policy;

    BeamSearchResult run(const State& initial) const {
        if (min_width <= 0 || initial_width < min_width || max_width < initial_width ||
            max_depth < 0 || !std::isfinite(time_limit) || time_limit < 0.0 ||
            !std::isfinite(reserve_time) || reserve_time < 0.0) {
            throw std::invalid_argument("invalid beam search parameters");
        }
        using Clock = BeamBudget::Clock;
        const auto deadline = Clock::now() + std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(std::max(0.0, time_limit - reserve_time)));
        BeamBudget budget{deadline, initial_width, min_width, max_width};

        struct Node {
            State state;
            double evaluation = 0.0;
            int history_id = -1;
        };
        struct Candidate {
            State state;
            double evaluation = 0.0;
            int parent = -1;
            Action action{};
            std::uint64_t order = 0;
        };
        struct History {
            int parent;
            Action action;
        };

        // Keep pool objects alive so State/Action member allocations can be reused.
        // Only the first beam_count/candidate_count slots are active.
        std::vector<History> history;
        std::vector<Node> beam(1), next_beam;
        std::size_t beam_count = 1;
        beam[0].state = initial;
        beam[0].evaluation = policy.evaluate(initial);
        std::vector<Candidate> candidates;
        std::vector<int> order;
        std::vector<Action> actions;
        State scratch;
        LayerHashIndex seen;
        BeamSearchResult result;
        int best_parent = -1;
        std::optional<Action> best_action;
        if (policy.is_answer(initial)) {
            result.found = true;
            result.evaluation = beam[0].evaluation;
        }

        for (int depth = 0; depth < max_depth && beam_count != 0; ++depth) {
            if (budget.expired()) { result.timed_out = true; break; }
            const auto layer_started = Clock::now();
            const double remaining_steps = policy.estimate_remaining_steps(
                beam.front().state, depth, max_depth);
            const int expansion_width = budget.suggest_width(remaining_steps);
            int expanded_parents = 0;
            std::size_t candidate_count = 0;
            if (deduplicate) seen.clear();

            // A lower predicted width also reduces work in this layer immediately.
            const int parent_count = std::min(expansion_width, static_cast<int>(beam_count));
            for (int parent_index = 0; parent_index < parent_count; ++parent_index) {
                if (budget.expired()) { result.timed_out = true; break; }
                const Node& node = beam[parent_index];
                if (policy.is_terminal(node.state)) continue;
                ++expanded_parents;
                actions.clear();
                policy.enumerate_actions(node.state, actions);
                for (const Action& action : actions) {
                    // Checks are cooperative: one expensive call can exceed the deadline.
                    if (budget.expired()) { result.timed_out = true; break; }
                    scratch = node.state;
                    scratch.apply(action);
                    const double value = policy.evaluate(scratch);
                    const auto generation_order = result.generated++;
                    if ((!result.found || value > result.evaluation) && policy.is_answer(scratch)) {
                        result.found = true;
                        result.evaluation = value;
                        best_parent = node.history_id;
                        best_action = action;
                    }
                    if (depth + 1 == max_depth || policy.is_terminal(scratch)) continue;

                    // Evaluate before merging so a better path to the same key is preserved.
                    Hash key = 0;
                    int index = -1;
                    if (deduplicate) {
                        key = policy.state_hash(scratch);
                        index = seen.find(key);
                    }
                    if (index != -1) {
                        ++result.duplicates;
                        if (value <= candidates[index].evaluation) continue;
                    } else {
                        index = static_cast<int>(candidate_count++);
                        if (candidate_count > candidates.size()) candidates.emplace_back();
                        if (deduplicate) seen.insert(key, index);
                    }
                    Candidate& candidate = candidates[index];
                    std::swap(candidate.state, scratch);
                    candidate.evaluation = value;
                    candidate.parent = node.history_id;
                    candidate.action = action;
                    candidate.order = generation_order;
                }
                if (result.timed_out) break;
            }

            // Answers were saved as they were generated; sorting is unnecessary on timeout.
            if (budget.expired()) result.timed_out = true;
            if (result.timed_out || candidate_count == 0) break;

            // Sort integer indices, not the full State objects. Explicit ties retain
            // the generation order even though nth_element/sort are not stable.
            order.resize(candidate_count);
            std::iota(order.begin(), order.end(), 0);
            const auto better = [&](int lhs, int rhs) {
                const Candidate& a = candidates[lhs];
                const Candidate& b = candidates[rhs];
                return a.evaluation != b.evaluation ? a.evaluation > b.evaluation : a.order < b.order;
            };
            const int best = *std::min_element(order.begin(), order.end(), better);
            const double next_steps = policy.estimate_remaining_steps(
                candidates[best].state, depth + 1, max_depth);
            // Predict from the last completed layer; record this entire layer below.
            const int next_width = budget.suggest_width(next_steps);
            if (next_width == 0) { result.timed_out = true; break; }
            if (order.size() > static_cast<std::size_t>(next_width)) {
                std::nth_element(order.begin(), order.begin() + next_width, order.end(), better);
                order.resize(next_width);
            }
            std::sort(order.begin(), order.end(), better);
            if (budget.expired()) { result.timed_out = true; break; }

            // Commit history only for selected successors. A best answer not selected
            // still has its retained parent history and its separate last action.
            if (next_beam.size() < order.size()) next_beam.resize(order.size());
            for (std::size_t index = 0; index < order.size(); ++index) {
                Candidate& candidate = candidates[order[index]];
                Node& next = next_beam[index];
                std::swap(next.state, candidate.state);
                next.evaluation = candidate.evaluation;
                next.history_id = static_cast<int>(history.size());
                history.push_back({candidate.parent, std::move(candidate.action)});
            }
            beam_count = order.size();
            beam.swap(next_beam);
            const double layer_seconds = std::chrono::duration<double>(Clock::now() - layer_started).count();
            budget.observe(expanded_parents, layer_seconds);
        }

        result.history_nodes = history.size();
        if (!result.found) return result;
        for (int id = best_parent; id != -1; id = history[id].parent) {
            result.actions.push_back(history[id].action);
        }
        std::reverse(result.actions.begin(), result.actions.end());
        if (best_action) result.actions.push_back(std::move(*best_action));
        return result;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    State initial;
    initial.read(std::cin);

    BeamSearch search;
    // TODO: Set the time budget, depth cap, and width bounds for this problem.
    // Deduplication is disabled by default; implement state_hash before enabling it.
    const BeamSearchResult result = search.run(initial);
    if (!result.found) {
        // TODO: Use a precomputed valid fallback if incomplete states cannot be submitted.
        std::cerr << "no valid answer found within the search limits\n";
        return 1;
    }

    // TODO: Adjust headers and separators for the required output format.
    for (const Action& action : result.actions) action.write(std::cout);
    return 0;
}
