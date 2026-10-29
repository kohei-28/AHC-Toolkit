#include <iostream>
#include <stdexcept>

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

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    State state;
    state.read(std::cin);

    // Add a solver here, or copy Action and State into another search template.
    return 0;
}
