#include <bits/stdc++.h>
using namespace std;

struct Input {
    void read() {
        // TODO: Read the problem input.
    }
};

struct Action {
    // TODO: Store one action.
};

struct State {
    // TODO: Store one solution state.
};

struct Candidate {
    State state;
    double evaluation;
    int parent;
    Action action;
};

struct Node {
    State state;
    int parent;
    Action action;
};

class Solver {
private:
    const Input& input;

    static constexpr int BEAM_WIDTH = 100;
    static constexpr int MAX_DEPTH = 1000;

    State build_initial_state() const {
        State state;

        // TODO: Build the initial state.

        return state;
    }

    vector<Action> generate_actions(const State& state) const {
        vector<Action> actions;

        // TODO: Generate actions from the current state.

        return actions;
    }

    void apply_action(State& state, const Action& action) const {
        // TODO: Apply the action to the state.
    }

    double evaluate(const State& state, int depth) const {
        // Smaller is better.
        return 0.0;
    }

    bool is_finished(const State& state, int depth) const {
        // TODO: Add a problem-specific termination condition if needed.
        return depth >= MAX_DEPTH;
    }

    vector<Action> beam_search() const {
        vector<Node> nodes;
        nodes.push_back({
            build_initial_state(),
            -1,
            Action{}
        });

        vector<int> beam = {0};

        for (int depth = 0; depth < MAX_DEPTH; depth++) {
            vector<Candidate> candidates;

            for (int node_id : beam) {
                const State& current_state = nodes[node_id].state;

                if (is_finished(current_state, depth)) {
                    continue;
                }

                vector<Action> actions = generate_actions(current_state);

                for (const Action& action : actions) {
                    State next_state = current_state;
                    apply_action(next_state, action);
                    double evaluation = evaluate(next_state, depth + 1);

                    candidates.push_back({
                        move(next_state),
                        evaluation,
                        node_id,
                        action
                    });
                }
            }

            if (candidates.empty()) {
                break;
            }

            sort(
                candidates.begin(),
                candidates.end(),
                [](const Candidate& lhs, const Candidate& rhs) {
                    return lhs.evaluation < rhs.evaluation;
                }
            );

            if (static_cast<int>(candidates.size()) > BEAM_WIDTH) {
                candidates.resize(BEAM_WIDTH);
            }

            vector<int> next_beam;
            next_beam.reserve(candidates.size());

            for (Candidate& candidate : candidates) {
                int node_id = static_cast<int>(nodes.size());

                nodes.push_back({
                    move(candidate.state),
                    candidate.parent,
                    candidate.action
                });

                next_beam.push_back(node_id);
            }

            beam = move(next_beam);
        }

        int best_node = beam.front();
        vector<Action> answer;

        while (nodes[best_node].parent != -1) {
            answer.push_back(nodes[best_node].action);
            best_node = nodes[best_node].parent;
        }

        reverse(answer.begin(), answer.end());
        return answer;
    }

    void output(const vector<Action>& answer) const {
        // TODO: Print the answer.
    }

public:
    explicit Solver(const Input& input) : input(input) {}

    void solve() const {
        vector<Action> answer = beam_search();
        output(answer);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Input input;
    input.read();

    Solver solver(input);
    solver.solve();
}
