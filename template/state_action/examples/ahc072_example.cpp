// Example of applying the state_action template to AHC072.
// Includes the problem-specific Action and State and a simple demonstration solver.

#include <array>
#include <iostream>
#include <istream>
#include <ostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

// ----- Problem-specific Action and State -----
inline constexpr int DR[4] = {-1, 1, 0, 0};
inline constexpr int DC[4] = {0, 0, -1, 1};
inline constexpr char DIRECTION[] = "UDLR";
inline constexpr int MAX_HEIGHT = 8;
inline constexpr int MAX_OPERATIONS = 100000;

struct Action {
    enum class Type { Jump };

    int row, col;      // Starting cell.
    int keep;          // Number of slimes left at the source.
    int direction;
    int length = 1;
    Type type = Type::Jump;

    void write(std::ostream& out) const {
        switch (type) {
            case Type::Jump:
                out << row << ' ' << col << ' ' << keep << ' '
                    << DIRECTION[direction] << ' ' << length << '\n';
                return;
        }
        throw std::invalid_argument("unknown operation type");
    }
};

struct State {
    int n = 0, colors = 0;
    std::vector<std::string> grid;
    std::vector<int> nest;  // Nest color at each cell, or -1.
    std::vector<std::array<int, MAX_HEIGHT>> tower;  // Bottom-to-top colors.
    std::vector<int> height;
    int remaining = 0;
    int operations = 0;

    void read(std::istream& in) {
        if (!(in >> n >> colors)) return;
        grid.resize(n);
        for (auto& row : grid) in >> row;
        nest.assign(n * n, -1);
        tower.assign(n * n, std::array<int, MAX_HEIGHT>{});
        height.assign(n * n, 0);
        remaining = operations = 0;
        for (int row = 0; row < n; ++row) {
            for (int col = 0; col < n; ++col) {
                const int cell = row * n + col;
                const char ch = grid[row][col];
                if ('A' <= ch && ch < 'A' + colors) nest[cell] = ch - 'A';
                if ('a' <= ch && ch < 'a' + colors) {
                    tower[cell][height[cell]++] = ch - 'a';
                    ++remaining;
                }
            }
        }
    }

    bool is_floor(int row, int col) const {
        return 0 <= row && row < n && 0 <= col && col < n && grid[row][col] != '#';
    }

    int destination(const Action& action) const {
        if (!is_floor(action.row, action.col) || action.direction < 0 ||
            action.direction >= 4 || action.length < 1 || action.length > MAX_HEIGHT) {
            return -1;
        }
        for (int step = 1; step <= action.length; ++step) {
            const int row = action.row + DR[action.direction] * step;
            const int col = action.col + DC[action.direction] * step;
            if (!is_floor(row, col)) return -1;
        }
        return (action.row + DR[action.direction] * action.length) * n +
               action.col + DC[action.direction] * action.length;
    }

    bool can_apply(const Action& action) const {
        switch (action.type) {
            case Action::Type::Jump: return can_jump(action);
        }
        return false;
    }

    void apply(const Action& action) {
        if (!can_apply(action)) throw std::invalid_argument("illegal action");
        switch (action.type) {
            case Action::Type::Jump: apply_jump(action); break;
        }
    }

private:
    // ----- Jump -----
    bool can_jump(const Action& action) const {
        if (operations >= MAX_OPERATIONS) return false;
        const int to = destination(action);
        if (to == -1) return false;
        const int from = action.row * n + action.col;
        const int h = height[from];
        if (action.keep < 0 || action.keep >= h || action.length > action.keep + 1) return false;
        // Check capacity before returning home. Intermediate towers do not matter.
        return height[to] + h - action.keep <= MAX_HEIGHT;
    }

    void apply_jump(const Action& action) {
        const int from = action.row * n + action.col;
        const int to = destination(action);
        // Pop from the top to reverse the entire moving group.
        while (height[from] > action.keep) {
            const int color = tower[from][--height[from]];
            tower[from][height[from]] = 0;
            tower[to][height[to]++] = color;
        }
        return_home(from);
        return_home(to);
        ++operations;
    }

    // ----- Automatic effects -----
    void return_home(int cell) {
        while (height[cell] > 0 && tower[cell][height[cell] - 1] == nest[cell]) {
            tower[cell][--height[cell]] = 0;
            --remaining;
        }
    }
};

// ----- Simple solver example -----
using namespace std;

// This file only demonstrates how a simple solver uses Action and State.
// Copy Action and State unchanged when switching search algorithms.
vector<vector<int>> build_distances(const State& state) {
    vector<vector<int>> distance(state.colors, vector<int>(state.n * state.n, -1));
    for (int home = 0; home < state.n * state.n; ++home) {
        const int color = state.nest[home];
        if (color == -1) continue;
        queue<int> queue;
        queue.push(home);
        distance[color][home] = 0;
        while (!queue.empty()) {
            const int from = queue.front();
            queue.pop();
            for (int direction = 0; direction < 4; ++direction) {
                const int row = from / state.n + DR[direction];
                const int col = from % state.n + DC[direction];
                if (!state.is_floor(row, col)) continue;
                const int to = row * state.n + col;
                if (distance[color][to] != -1) continue;
                distance[color][to] = distance[color][from] + 1;
                queue.push(to);
            }
        }
    }
    return distance;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    State state;
    state.read(cin);
    const auto distance = build_distances(state);
    vector<Action> answer;
    int active = -1;

    // Carry the same top slime until it reaches its nest.
    while (state.remaining > 0 && state.operations < MAX_OPERATIONS) {
        if (active == -1) {
            for (int cell = 0; cell < state.n * state.n; ++cell) {
                if (state.height[cell] > 0) {
                    active = cell;
                    break;
                }
            }
        }
        const int color = state.tower[active][state.height[active] - 1];
        bool moved = false;
        for (int direction = 0; direction < 4; ++direction) {
            const Action action{active / state.n, active % state.n,
                                state.height[active] - 1, direction, 1};
            if (!state.can_apply(action)) continue;
            const int to = state.destination(action);
            if (distance[color][to] >= distance[color][active]) continue;
            state.apply(action);
            answer.push_back(action);
            active = state.nest[to] == color ? -1 : to;
            moved = true;
            break;
        }
        if (!moved) break;
    }

    for (const Action& action : answer) action.write(cout);
    return 0;
}
