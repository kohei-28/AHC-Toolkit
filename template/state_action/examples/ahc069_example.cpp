// Example of applying the state_action template to AHC069.
// Includes the problem-specific Action and State and a simple demonstration solver.

#include <algorithm>
#include <cmath>
#include <iostream>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// ----- Problem-specific Action and State -----
struct Action {
    enum class Type { Relocate, Accept, Reject };
    Type type = Type::Reject;

    // Relocate parameters. The entire list is one simultaneous operation.
    std::vector<std::pair<int, std::vector<int>>> moves;  // (group id, new cells).

    // Accept parameters. The group is the currently arriving group.
    std::vector<int> cells;  // Cell ids are row * N + col.

    void write(std::ostream& out, int n) const {
        switch (type) {
            case Type::Relocate:
                out << moves.size() << '\n';
                for (const auto& [group, region] : moves) {
                    out << group << '\n';
                    for (const int cell : region) out << cell / n << ' ' << cell % n << '\n';
                }
                break;
            case Type::Accept:
                out << "Yes\n";
                for (const int cell : cells) out << cell / n << ' ' << cell % n << '\n';
                break;
            case Type::Reject:
                out << "No\n";
                break;
            default:
                throw std::invalid_argument("unknown operation type");
        }
        out.flush();
    }
};

struct State {
    struct Group {
        int start = 0, end = 0, people = 0;
        long long value = 0;
        bool active = false;
        std::vector<int> cells;
        int max_perimeter = 0;  // Largest perimeter means smallest compactness.
    };

    enum class Phase { Arrival, Relocation, Decision, Finished };

    int n = 0, m = 0, rate_milli = 0;
    std::vector<std::string> grid;
    std::vector<int> owner;  // Group id, or -1 for an empty cell.
    std::vector<Group> groups;
    int next_group = 0, current = -1, now = -1;
    long long money = 0;
    Phase phase = Phase::Arrival;

    void read(std::istream& in) {
        double rate;
        if (!(in >> n >> m >> rate)) return;
        rate_milli = static_cast<int>(std::llround(rate * 1000));
        grid.resize(n);
        for (auto& row : grid) in >> row;
        owner.assign(n * n, -1);
        groups.assign(m, Group{});
        next_group = 0;
        current = now = -1;
        money = 0;
        phase = m == 0 ? Phase::Finished : Phase::Arrival;
    }

    bool read_group(std::istream& in) {
        int id, start, end, people;
        long long value;
        if (!(in >> id >> start >> end >> people >> value)) return false;
        arrive(id, start, end, people, value);
        return true;
    }

    // Arrival is an input event, not an action chosen by the solver.
    void arrive(int id, int start, int end, int people, long long value) {
        if (phase != Phase::Arrival || id != next_group || id >= m ||
            start <= now || end <= start || people <= 0 || value <= 0) {
            throw std::invalid_argument("invalid arrival");
        }
        for (int group = 0; group < next_group; ++group) {
            if (groups[group].active && groups[group].end < start) depart(group);
        }
        now = start;
        current = id;
        Group& group = groups[id];
        group.start = start;
        group.end = end;
        group.people = people;
        group.value = value;
        ++next_group;
        phase = Phase::Relocation;
    }

    bool is_lawn(int cell) const {
        return 0 <= cell && cell < n * n && grid[cell / n][cell % n] == '.';
    }

    bool can_apply(const Action& action) const {
        switch (action.type) {
            case Action::Type::Relocate: return can_relocate(action);
            case Action::Type::Accept: return can_accept(action);
            case Action::Type::Reject: return can_reject(action);
        }
        return false;
    }

    void apply(const Action& action) {
        if (!can_apply(action)) throw std::invalid_argument("illegal action");
        switch (action.type) {
            case Action::Type::Relocate: apply_relocate(action); break;
            case Action::Type::Accept: apply_accept(action); break;
            case Action::Type::Reject: apply_reject(action); break;
        }
    }

private:
    // ----- Shared region checks -----
    bool valid_region(const std::vector<int>& cells, int people,
                      const std::vector<int>& occupancy) const {
        if (static_cast<int>(cells.size()) != people || cells.empty()) return false;
        std::vector<bool> marked(n * n, false);
        for (const int cell : cells) {
            if (!is_lawn(cell) || marked[cell] || occupancy[cell] != -1) return false;
            marked[cell] = true;
        }
        std::vector<int> queue{cells.front()};
        marked[cells.front()] = false;
        for (std::size_t index = 0; index < queue.size(); ++index) {
            const int cell = queue[index];
            const int dr[4] = {-1, 1, 0, 0}, dc[4] = {0, 0, -1, 1};
            for (int direction = 0; direction < 4; ++direction) {
                const int row = cell / n + dr[direction];
                const int col = cell % n + dc[direction];
                if (row < 0 || row >= n || col < 0 || col >= n) continue;
                const int to = row * n + col;
                if (!marked[to]) continue;
                marked[to] = false;
                queue.push_back(to);
            }
        }
        return queue.size() == cells.size();
    }

    int perimeter(const std::vector<int>& cells) const {
        std::vector<bool> marked(n * n, false);
        for (const int cell : cells) marked[cell] = true;
        int result = 4 * static_cast<int>(cells.size());
        for (const int cell : cells) {
            if (cell % n + 1 < n && marked[cell + 1]) result -= 2;
            if (cell / n + 1 < n && marked[cell + n]) result -= 2;
        }
        return result;
    }

    // ----- Relocate -----
    bool can_relocate(const Action& action) const {
        if (phase != Phase::Relocation) return false;
        std::vector<bool> moved(m, false);
        std::vector<int> occupancy = owner;
        for (const auto& [id, cells] : action.moves) {
            (void)cells;
            if (id < 0 || id >= next_group || !groups[id].active || moved[id]) return false;
            moved[id] = true;
            for (const int cell : groups[id].cells) occupancy[cell] = -1;
        }
        for (const auto& [id, cells] : action.moves) {
            if (!valid_region(cells, groups[id].people, occupancy)) return false;
            for (const int cell : cells) occupancy[cell] = id;
        }
        return true;
    }

    void apply_relocate(const Action& action) {
        // Vacate every old region before placing any group, so swaps are allowed.
        for (const auto& [id, cells] : action.moves) {
            (void)cells;
            for (const int cell : groups[id].cells) owner[cell] = -1;
        }
        for (const auto& [id, cells] : action.moves) {
            Group& group = groups[id];
            group.cells = cells;
            group.max_perimeter = std::max(group.max_perimeter, perimeter(cells));
            for (const int cell : cells) owner[cell] = id;
            money -= std::max(1LL, (group.value * rate_milli + 500) / 1000);
        }
        phase = Phase::Decision;
    }

    // ----- Accept -----
    bool can_accept(const Action& action) const {
        return phase == Phase::Decision && valid_region(action.cells, groups[current].people, owner);
    }

    void apply_accept(const Action& action) {
        Group& group = groups[current];
        group.cells = action.cells;
        group.max_perimeter = perimeter(action.cells);
        group.active = true;
        for (const int cell : action.cells) owner[cell] = current;
        finish_decision();
    }

    // ----- Reject -----
    bool can_reject(const Action& action) const {
        (void)action;
        return phase == Phase::Decision;
    }

    void apply_reject(const Action& action) {
        (void)action;
        finish_decision();
    }

    // ----- Automatic departures and payments -----
    void depart(int id) {
        Group& group = groups[id];
        const double compactness = 4.0 * std::sqrt(group.people) / group.max_perimeter;
        money += std::llround(group.value * compactness);
        for (const int cell : group.cells) owner[cell] = -1;
        group.cells.clear();
        group.active = false;
    }

    void finish_decision() {
        phase = Phase::Arrival;
        if (next_group == m) {
            for (int id = 0; id < next_group; ++id) {
                if (groups[id].active) depart(id);
            }
            phase = Phase::Finished;
        }
    }
};

// ----- Simple solver example -----
// A BFS prefix is connected. Use the first empty component that fits the group.
std::vector<int> find_region(const State& state, int people) {
    std::vector<bool> visited(state.n * state.n, false);
    for (int start = 0; start < state.n * state.n; ++start) {
        if (!state.is_lawn(start) || state.owner[start] != -1 || visited[start]) continue;
        std::vector<int> queue{start};
        visited[start] = true;
        for (std::size_t index = 0; index < queue.size(); ++index) {
            if (static_cast<int>(queue.size()) >= people) {
                queue.resize(people);
                return queue;
            }
            const int cell = queue[index];
            const int dr[4] = {-1, 1, 0, 0}, dc[4] = {0, 0, -1, 1};
            for (int direction = 0; direction < 4; ++direction) {
                const int row = cell / state.n + dr[direction];
                const int col = cell % state.n + dc[direction];
                if (row < 0 || row >= state.n || col < 0 || col >= state.n) continue;
                const int to = row * state.n + col;
                if (!state.is_lawn(to) || state.owner[to] != -1 || visited[to]) continue;
                visited[to] = true;
                queue.push_back(to);
            }
        }
    }
    return {};
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    State state;
    state.read(std::cin);
    for (int turn = 0; turn < state.m; ++turn) {
        if (!state.read_group(std::cin)) return 1;

        // This baseline never relocates groups, but the state supports it fully.
        Action relocation;
        relocation.type = Action::Type::Relocate;
        state.apply(relocation);
        relocation.write(std::cout, state.n);

        Action decision;
        decision.cells = find_region(state, state.groups[state.current].people);
        decision.type = decision.cells.empty() ? Action::Type::Reject : Action::Type::Accept;
        state.apply(decision);
        decision.write(std::cout, state.n);
    }
    return 0;
}
