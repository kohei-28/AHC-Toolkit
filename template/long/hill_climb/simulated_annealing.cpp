// Simulated annealing: edit a copied Solution and fully evaluate it.
// Action/State model the problem. Move/Solution model the local search.
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>

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
                // TODO: Write OpA and its parameters, if needed.
                return;
            case Type::OpB:
                // TODO: Write OpB and its parameters, if needed.
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
        // TODO: Check the problem's preconditions for OpA.
        (void)action;
        return false;
    }

    void apply_op_a(const Action& action) {
        // TODO: Apply OpA, including its automatic effects and counters.
        (void)action;
    }

    // ----- OpB -----
    bool can_op_b(const Action& action) const {
        // TODO: Check the problem's preconditions for OpB.
        (void)action;
        return false;
    }

    void apply_op_b(const Action& action) {
        // TODO: Apply OpB, including its automatic effects and counters.
        (void)action;
    }
};

// ----- Problem-specific solution representation and local edits -----
struct Solution {
    // TODO: Store the solution to optimize. Copies must be independent.
    // Examples: an Action sequence, a permutation, a layout, or construction rules.
    // This is separate from the State reached by simulating the solution.
};

struct Move {
    enum class Type { EditA, EditB };  // Examples: Swap, Replace, RebuildSegment.
    Type type = Type::EditA;

    // TODO: Add parameters for EditA.

    // TODO: Add parameters for EditB.
    // One Move may change many Actions or rebuild part of the solution.
};

// ----- Problem-specific search policy -----
using Score = long long;  // Change to double if needed. Larger is better.

struct SearchPolicy {
    // Relative sampling weights for EditA and EditB, independent of Action::Type.
    // Keep at least one weight positive. Update this and the switch for new types.
    std::discrete_distribution<int> neighborhood_distribution{1.0, 1.0};

    Solution build_initial_solution(const State& initial) const {
        // TODO: Construct a valid complete solution for the initial problem state.
        (void)initial;
        return {};
    }

    std::optional<Move> generate_move(
        const State& initial, const Solution& solution, std::mt19937_64& random
    ) {
        switch (neighborhood_distribution(random)) {
            case 0: return generate_edit_a(initial, solution, random);
            case 1: return generate_edit_b(initial, solution, random);
        }
        throw std::invalid_argument("unknown neighborhood type");
    }

    bool apply_move(const State& initial, Solution& solution, const Move& move) const {
        // Edit the solution representation, without assuming Move is an Action.
        // Return false if the edit cannot be made (e.g. invalid indices).
        // A failed edit may modify this copy; the search will discard it.
        switch (move.type) {
            case Move::Type::EditA: return apply_edit_a(initial, solution, move);
            case Move::Type::EditB: return apply_edit_b(initial, solution, move);
        }
        throw std::invalid_argument("unknown move type");
    }

    std::optional<Score> evaluate(const State& initial, const Solution& solution) const {
        // TODO: Recompute the entire evaluation. Larger is better.
        // For a minimization problem, return the negative cost.
        // For an Action sequence: copy initial, replay the sequence using
        // State::can_apply/apply, and score the resulting State.
        // Return std::nullopt if the edited solution is infeasible.
        // Valid edit indices do not guarantee a feasible simulation.
        (void)initial;
        (void)solution;
        return 0;
    }

    void write_answer(const Solution& solution, std::ostream& out) const {
        // TODO: Write the final solution, rather than the history of Moves.
        (void)solution;
        (void)out;
    }

private:
    // ----- EditA neighborhood -----
    std::optional<Move> generate_edit_a(
        const State& initial, const Solution& solution, std::mt19937_64& random
    ) const {
        // TODO: Sample edit parameters and return a Move with type = Move::Type::EditA.
        // Return std::nullopt if this attempt cannot generate a candidate.
        (void)initial;
        (void)solution;
        (void)random;
        return std::nullopt;
    }

    bool apply_edit_a(const State& initial, Solution& solution, const Move& move) const {
        // TODO: Check the edit parameters, then modify the copied solution.
        // Use initial and State/Action here if rebuilding requires simulation.
        (void)initial;
        (void)solution;
        (void)move;
        return false;
    }

    // ----- EditB neighborhood -----
    std::optional<Move> generate_edit_b(
        const State& initial, const Solution& solution, std::mt19937_64& random
    ) const {
        // TODO: Sample edit parameters and return a Move with type = Move::Type::EditB.
        (void)initial;
        (void)solution;
        (void)random;
        return std::nullopt;
    }

    bool apply_edit_b(const State& initial, Solution& solution, const Move& move) const {
        // TODO: Check the edit parameters, then modify the copied solution.
        (void)initial;
        (void)solution;
        (void)move;
        return false;
    }
};

// ----- Search infrastructure -----
namespace ahc {

class Timer {
public:
    using Clock = std::chrono::steady_clock;

    explicit Timer(double limit_seconds)
        : started_at_(Clock::now()), limit_seconds_(limit_seconds) {}

    double elapsed() const {
        return std::chrono::duration<double>(Clock::now() - started_at_).count();
    }

    double limit() const { return limit_seconds_; }

    bool expired(double margin_seconds = 0.0) const {
        return elapsed() + margin_seconds >= limit_seconds_;
    }

private:
    Clock::time_point started_at_;
    double limit_seconds_;
};

}  // namespace ahc

struct AnnealingResult {
    Solution solution;  // Best feasible solution, not the last visited solution.
    Score value;
    std::uint64_t iterations = 0;
    std::uint64_t evaluated = 0;
    std::uint64_t accepted = 0;
};

struct SimulatedAnnealing {
    SearchPolicy policy;
    std::uint64_t seed = 0;
    std::uint64_t max_iterations = std::numeric_limits<std::uint64_t>::max();
    double safety_margin = 0.01;  // Allow enough time for one iteration and output.
    double start_temperature = 100.0;  // Tune to the scale of score differences.
    double end_temperature = 1.0;

    AnnealingResult run(
        const State& initial, const Solution& start, const ahc::Timer& timer
    ) {
        if (!(start_temperature > 0.0) || !(end_temperature > 0.0)
            || !std::isfinite(start_temperature) || !std::isfinite(end_temperature)) {
            throw std::invalid_argument("temperatures must be finite and positive");
        }

        std::mt19937_64 random(seed);
        std::uniform_real_distribution<double> uniform(0.0, 1.0);
        const std::optional<Score> start_value = policy.evaluate(initial, start);
        if (!start_value) throw std::invalid_argument("initial solution is infeasible");
        Solution current = start;
        Score current_value = *start_value;
        AnnealingResult result{start, *start_value};

        const double search_start = timer.elapsed();
        const double search_end = timer.limit() - safety_margin;
        const double log_start = std::log(start_temperature);
        const double log_end = std::log(end_temperature);
        const auto temperature = [&]() {
            double progress;
            if (max_iterations != std::numeric_limits<std::uint64_t>::max()) {
                // An explicit iteration limit also makes cooling deterministic.
                progress = max_iterations <= 1 ? 0.0
                    : static_cast<double>(result.iterations - 1)
                        / static_cast<double>(max_iterations - 1);
            } else {
                progress = search_end <= search_start ? 1.0
                    : (timer.elapsed() - search_start) / (search_end - search_start);
            }
            progress = std::clamp(progress, 0.0, 1.0);
            return std::exp(log_start + progress * (log_end - log_start));
        };
        const auto accept = [&](Score next_value) {
            if (next_value >= current_value) return true;  // Accept ties too.
            // Convert before subtracting to avoid integer overflow.
            const long double delta =
                static_cast<long double>(next_value) - static_cast<long double>(current_value);
            return uniform(random) < std::exp(delta / temperature());
        };

        while (result.iterations < max_iterations && !timer.expired(safety_margin)) {
            ++result.iterations;
            const std::optional<Move> move = policy.generate_move(initial, current, random);
            // A failed sample does not mean that all neighbors are exhausted.
            if (!move) continue;
            if (timer.expired(safety_margin)) break;

            Solution next = current;
            if (!policy.apply_move(initial, next, *move)) continue;
            const std::optional<Score> next_value = policy.evaluate(initial, next);
            ++result.evaluated;

            if (next_value && accept(*next_value)) {
                current = std::move(next);
                current_value = *next_value;
                ++result.accepted;
                if (current_value > result.value) {
                    result.solution = current;
                    result.value = current_value;
                }
            }
            // Discard infeasible or rejected candidates without changing current.
        }
        return result;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    constexpr double TIME_LIMIT = 1.90;
    ahc::Timer timer(TIME_LIMIT);  // Include input and initialization time.

    State initial;
    initial.read(std::cin);

    SimulatedAnnealing search;
    // TODO: Tune the time limit, temperatures, and neighborhood weights.
    // Set max_iterations for deterministic debugging, or leave it unlimited.
    const Solution start = search.policy.build_initial_solution(initial);
    const AnnealingResult result = search.run(initial, start, timer);

#ifdef LOCAL
    std::cerr << "best_score=" << result.value
              << " iterations=" << result.iterations
              << " evaluated=" << result.evaluated
              << " accepted=" << result.accepted
              << " elapsed=" << timer.elapsed() << '\n';
#endif

    search.policy.write_answer(result.solution, std::cout);
    return 0;
}
