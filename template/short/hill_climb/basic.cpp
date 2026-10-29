#include <bits/stdc++.h>

using namespace std;
namespace ahc {

class Timer {
public:
    using Clock = std::chrono::steady_clock;

    explicit Timer(double limit_seconds = std::numeric_limits<double>::infinity())
        : started_at_(Clock::now()), limit_seconds_(limit_seconds) {}

    void reset() { started_at_ = Clock::now(); }

    [[nodiscard]] double elapsed() const {
        return std::chrono::duration<double>(Clock::now() - started_at_).count();
    }

    [[nodiscard]] long long elapsed_ms() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - started_at_).count();
    }

    [[nodiscard]] double limit() const { return limit_seconds_; }

    [[nodiscard]] double remaining() const {
        return std::max(0.0, limit_seconds_ - elapsed());
    }

    [[nodiscard]] double progress() const {
        if (limit_seconds_ <= 0.0) return 1.0;
        if (limit_seconds_ == std::numeric_limits<double>::infinity()) return 0.0;
        return std::clamp(elapsed() / limit_seconds_, 0.0, 1.0);
    }

    [[nodiscard]] bool expired(double margin_seconds = 0.0) const {
        return elapsed() + margin_seconds >= limit_seconds_;
    }

private:
    Clock::time_point started_at_;
    double limit_seconds_;
};

}  // namespace ahc

using Score = long long;

struct Input {
    void read() {
    }
};

struct Solution {
    // Store a complete solution.
};

struct Move {
    // Store one local modification.
};

class Solver {
private:
    static constexpr double SAFETY_MARGIN = 0.01;

    const Input& input;
    ahc::Timer& timer;


    Solution build_initial_solution() {
        Solution solution;
        return solution;
    }

    Score evaluate(const Solution& solution) const {
        // Calculate the entire cost.
        // Smaller is better.
        return 0LL;
    }

    vector<Move> generate_moves(const Solution& solution) const {
        //Return candidates of moves. 
        vector<Move> moves;
        return moves;
    }

    bool can_apply_move(
        const Solution& solution,
        const Move& move
    ) const {
        // Return whether the move preserves validity.
        return true;
    }

    Score calculate_delta(
        const Solution& solution,
        const Move& move
    ) const {
        // Return:
        // cost after the move - cost before the move
        return 0;
    }

    void apply_move(
        Solution& solution,
        const Move& move
    ) const {
        //Apply the move.
    }

    void output(const Solution& solution) const {
        // Convert the final solution into the required output.
    }

public:
    Solver(
        const Input& input,
        ahc::Timer& timer
    ) : input(input), timer(timer) {}

    void solve() {
        Solution current = build_initial_solution();
        Score current_score = evaluate(current);

        long long rounds = 0;
        long long evaluated_moves = 0;
        long long accepted_moves = 0;

        while (!timer.expired(SAFETY_MARGIN)) {
            optional<Move> best_move;
            Score best_delta = 0;
            bool time_up = false;

            vector<Move> moves = generate_moves(current);

            for (const Move& move : moves) {
                if (timer.expired(SAFETY_MARGIN)) {
                    time_up = true;
                    break;
                }

                if (!can_apply_move(current, move)) {
                    continue;
                }

                Score delta = calculate_delta(current, move);
                ++evaluated_moves;

                if (delta < best_delta) {
                    best_delta = delta;
                    best_move = move;
                }
            }
            if (best_move.has_value()) {
#ifdef VERIFY_DELTA
                Score score_before = evaluate(current);
#endif

                apply_move(current, *best_move);
                current_score += best_delta;
                ++accepted_moves;

#ifdef VERIFY_DELTA
                Score actual_score = evaluate(current);
                assert(score_before + best_delta == actual_score);
                assert(current_score == actual_score);
#endif
            }

            ++rounds;

            if (time_up) {
                break;
            }

            if (!best_move.has_value()) {
                // No improving neighbor exists.
                break;
            }
        }

#ifdef LOCAL
        cerr << "score=" << current_score << '\n';
        cerr << "rounds=" << rounds << '\n';
        cerr << "evaluated_moves=" << evaluated_moves << '\n';
        cerr << "accepted_moves=" << accepted_moves << '\n';
        cerr << "elapsed=" << timer.elapsed() << '\n';
#endif

        output(current);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    constexpr double TIME_LIMIT = 1.90;
    ahc::Timer timer(TIME_LIMIT);

    Input input;
    input.read();

    Solver solver(input, timer);
    solver.solve();

    return 0;
}
