#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

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
    std::vector<Action> enumerate_actions(const State& state) const {
        // TODO: Generate the actions to consider. A subset of legal actions is OK.
        (void)state;
        return {};
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
        // For problems requiring a complete solution, reject incomplete states.
        (void)state;
        return true;
    }
};

// ----- Ordinary beam search -----
struct BeamSearchResult {
    bool found = false;
    double evaluation = -std::numeric_limits<double>::infinity();
    std::vector<Action> actions;
};

struct BeamSearch {
    int width = 50;
    int max_depth = 100;
    SearchPolicy policy;

    BeamSearchResult run(const State& initial) const {
        if (width <= 0 || max_depth < 0) {
            throw std::invalid_argument("invalid beam search parameters");
        }

        struct Node {
            State state;
            double evaluation;
            int history_id;
        };
        struct History {
            int parent;
            Action action;
        };

        // Keep states only in the current/next layer; history stores actions.
        // History grows with the number of generated legal successors.
        std::vector<History> history;
        std::vector<Node> beam{{initial, policy.evaluate(initial), -1}};
        BeamSearchResult result;
        int best_history = -1;

        const auto consider_answer = [&](const State& state, double value, int id) {
            if (policy.is_answer(state) && (!result.found || value > result.evaluation)) {
                result.found = true;
                result.evaluation = value;
                best_history = id;
            }
        };
        consider_answer(initial, beam.front().evaluation, -1);

        for (int depth = 0; depth < max_depth && !beam.empty(); ++depth) {
            std::vector<Node> candidates;
            for (const Node& node : beam) {
                if (policy.is_terminal(node.state)) continue;
                for (const Action& action : policy.enumerate_actions(node.state)) {
                    if (!node.state.can_apply(action)) continue;

                    // Copy the entire state, apply one action, then fully evaluate.
                    State next = node.state;
                    next.apply(action);
                    const double value = policy.evaluate(next);
                    const int id = static_cast<int>(history.size());
                    history.push_back({node.history_id, action});

                    // Save valid answers even if they are terminal or later pruned.
                    consider_answer(next, value, id);
                    if (!policy.is_terminal(next)) {
                        candidates.push_back({std::move(next), value, id});
                    }
                }
            }

            // Stable ties preserve the action generation order.
            std::stable_sort(candidates.begin(), candidates.end(),
                [](const Node& lhs, const Node& rhs) {
                    return lhs.evaluation > rhs.evaluation;
                });
            if (candidates.size() > static_cast<std::size_t>(width)) {
                candidates.erase(candidates.begin() + width, candidates.end());
            }
            beam = std::move(candidates);
        }

        if (!result.found) return result;
        for (int id = best_history; id != -1; id = history[id].parent) {
            result.actions.push_back(history[id].action);
        }
        std::reverse(result.actions.begin(), result.actions.end());
        return result;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    State initial;
    initial.read(std::cin);

    BeamSearch search;
    // TODO: Set width and max_depth for the problem and time limit.
    const BeamSearchResult result = search.run(initial);
    if (!result.found) {
        std::cerr << "no valid answer found within the search limits\n";
        return 1;
    }

    // TODO: Adjust headers and separators for the required output format.
    for (const Action& action : result.actions) action.write(std::cout);
    return 0;
}
