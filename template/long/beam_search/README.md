# incremental.cpp: Implementation Guide

## What to implement

Keep the default `expand()` initially. Fill in the following parts in order.

| Part | What you implement |
| --- | --- |
| `Action` | Operation types, parameters, and `write()` |
| `State` members and `read()` | Mutable state, input, and initialization |
| `can_op_a/b()` | Preconditions for each operation |
| `apply_op_a/b()` | All effects of each operation, including counters and caches |
| `Undo`, `capture_op_a/b()`, `undo_op_a/b()` | Information needed for restoration and the restoration itself |
| `SearchPolicy::enumerate_actions()` | Legal actions for the current state |
| `SearchPolicy::describe()` | Evaluation, answer score, output validity, termination, and remaining work |
| `main()` | Depth, time, width bounds, output format, and any required fallback |

Reuse the operations, enumeration, evaluation, and termination rules developed in `basic.cpp`,
adapting their interfaces to the functions above. **Larger evaluations are better in this template.**
If the basic version minimizes a cost, use a convention such as `value = -cost` when porting it.

The dispatch in `apply_with_undo()` and `undo()` is already implemented.
Update the switches when adding or renaming an `Action::Type`.
Keep each operation's `can`, `apply`, `capture`, and `undo` functions together.
The shared width controller and search engine need no problem-specific implementation.

## How apply, describe, and undo fit together

At a parent state, the default `expand()` tries each action and restores the parent:

```cpp
actions.clear();
enumerate_actions(state, actions);
for (const Action& action : actions) {
    Undo previous = state.apply_with_undo(action);
    Evaluation next = describe(state, depth + 1, max_depth);
    out.push_back({action, next});
    state.undo(action, previous);
}
```

`describe()` sees the state after the action. Each candidate stores the action and its
`Evaluation`, rather than a full child `State`. `expand()` must leave the state unchanged on return.

## What capture and undo must preserve

**Store information that cannot be recovered from the Action and the state after the operation.**
There is no need to copy the whole state into `Undo`.

| Operation or information | How to restore it | What to capture |
| --- | --- | --- |
| `cell += action.delta` | Subtract `action.delta` | Nothing for this value |
| `cell = action.value` | Assign the previous value | The old cell value |
| Operation position | Read `action.pos` | No duplicate position field needed |
| An invertible counter update | Perform the inverse update | Nothing for this counter |
| A cache that is hard to reverse | Assign the previous cache value | The old cache value |

Saving an invertible value is also fine if it makes restoration simpler.
Restore every changed component, including progress, counters, cached evaluations, and enabled hashes.
Immutable input and precomputed data can be shared and do not need restoration.

The pairing is:

```cpp
// Inside apply_with_undo(action):
Undo previous = capture_op_a(action);
apply_op_a(action);

// Later, undo(action, previous) dispatches to:
undo_op_a(action, previous);
```

Capture happens before the operation. Its result is returned to the caller and later passed
to the undo function for that same action. If operations A and B are applied in that order,
undo B before undoing A. The engine manages this last-in, first-out order.

## Updating the score incrementally

Reversible state updates do not automatically provide incremental evaluation.
You can recompute the whole evaluation in `describe()` and still use this template.

To maintain an incremental score with the default interface:

1. Store the score or its components in `State` and initialize them in `read()`.
2. Update them in `apply_op_a/b()` using the affected parts of the state.
3. Restore them in `undo_op_a/b()`.
4. Read them in `describe()`.

For example, suppose the score is the sum of squared values in `cells`.
Operation A overwrites `cells[action.pos]` with `action.value`.
The following are excerpts for this example; values are assumed to fit in `long long`.

```cpp
struct Undo {
    long long old_cell;
    long long old_score;
};

// State members:
std::vector<long long> cells;
long long score = 0;
```

Initialize the score once after reading the array:

```cpp
score = 0;
for (long long value : cells) score += value * value;
```

Implement these functions inside `State`:

```cpp
Undo capture_op_a(const Action& action) const {
    return {cells[action.pos], score};
}

void apply_op_a(const Action& action) {
    long long old_value = cells[action.pos];
    long long new_value = action.value;
    cells[action.pos] = new_value;
    score += new_value * new_value - old_value * old_value;
}

void undo_op_a(const Action& action, const Undo& previous) {
    cells[action.pos] = previous.old_cell;
    score = previous.old_score;
}
```

The scoring part of `describe()` becomes:

```cpp
out.value = static_cast<double>(state.score);
out.answer_value = static_cast<double>(state.score);
```

Keep the cached score consistent whenever an operation is applied or undone.
Assigning `Evaluation::value` does not update `State`.

## What describe returns

`describe()` receives a state, its depth, and the depth cap.
The default `expand()` calls it after applying a candidate action.
The engine also calls it for the initial state.

| Field | Meaning |
| --- | --- |
| `value` | Beam ranking: larger values are preferred |
| `answer_value` | Final-answer ranking: larger values are preferred |
| `answer` | Whether this state may be output as a valid answer |
| `terminal` | Whether expansion should stop at this state |
| `remaining_steps` | Estimated remaining expansion work for adaptive width |
| `key` | State hash, needed only when enabling deduplication |

### value versus answer_value

Use `value` to decide which candidates survive. Use `answer_value` to choose the answer to output.
For example:

```cpp
out.value = actual_score + future_bonus;
out.answer_value = actual_score;
```

This lets the search consider future potential while comparing output solutions by their actual score.
If the distinction is unnecessary, set both fields to the same score.
Only states with `answer == true` participate in final-answer comparison.

### Where is_terminal and is_answer went

This version has no separate `is_terminal()` or `is_answer()` hooks.
Set `terminal` and `answer` in `describe()` instead.

| `answer` | `terminal` | Behavior |
| --- | --- | --- |
| `true` | `false` | May be output, and may also be expanded |
| `true` | `true` | May be output; do not expand further |
| `false` | `false` | Intermediate state; continue searching |
| `false` | `true` | Invalid dead end; neither output nor expand |

The scaffold defaults are `answer = true` and `terminal = false`.
For completion-only problems, set `answer` to the actual completion condition.
Reaching `max_depth` does not automatically make a state a valid answer.

### Remaining work and optional hashing

When layer costs are similar and the remaining depth is known, keep
`remaining_steps = std::max(0, max_depth - depth)`.
For strongly varying layer costs, estimate remaining work from problem progress.

**Hash deduplication is disabled by default.** Leave `key` unset unless you implement a state hash
and explicitly enable `deduplicate`.

## Why enumerate_actions takes actions by reference

```cpp
void enumerate_actions(const State& state, std::vector<Action>& actions) const;
```

`state` is a read-only input. `actions` is an output buffer.
The function returns `void` and appends actions to the caller's vector.
The `&` means it modifies that same vector rather than a copy.

Append legal actions with `actions.push_back(action)`.
You can filter with `can_apply()` or directly generate only legal actions.
The default `expand()` clears the buffer before calling this function, so append actions here.
Reusing the vector's allocated capacity reduces repeated memory allocation.

## Accessing the action during evaluation

`describe()` does not receive the action or the parent's `Evaluation` as arguments.
Inside the `expand()` loop, however, both the current `action` and its returned `Undo` are available.
If evaluation needs that information, call an evaluation helper from that location and pass it explicitly.
The engine keeps action history, but does not automatically pass that history to `describe()`.

If the parent state and action are enough to compute the child's evaluation and status,
you can also construct `Transition{action, next}` directly in `expand()` without temporarily applying it.
For example, compute the parent score once and add each action's score delta; storing a score
in `State` is not mandatory. Set all required `Evaluation` fields and leave the state unchanged.

## Optional: add state hashing when time permits

Keep `deduplicate = false` while implementing the basic operations and evaluation.
The template already provides `ZobristTable` and the deduplication table; implement the
problem-specific key with the following steps when you want to enable them.

### 1. Decide which states may be merged

The engine compares keys within the same search layer. Equal keys should represent states
with equivalent future choices and outcomes, where keeping the higher `value` is sufficient.
Include everything needed for that equivalence: the board, positions, remaining resources,
and operation counts when relevant. The same board with a different remaining budget may
need a different key. Derived caches do not need separate features if the included state
already determines them.

### 2. Initialize a cached key

For a board with a small finite set of cell values, assign one random key to each
`(position, value)` pair and XOR the keys together. For example, assume 81 cells with values 0-3:

```cpp
// Shared immutable data, outside State:
constexpr int CELL_COUNT = 81;
constexpr int VALUE_COUNT = 4;
inline const ZobristTable cell_keys(CELL_COUNT, VALUE_COUNT);

// Add to State:
Hash hash = 0;
```

After reading the board, compute its initial hash:

```cpp
hash = 0;
for (int pos = 0; pos < CELL_COUNT; ++pos) {
    hash ^= cell_keys(pos, static_cast<int>(cells[pos]));
}
```

Adapt the dimensions and value encoding to the problem. This example requires every
reachable cell value to be in `[0, VALUE_COUNT)`; it is not a table for arbitrary integers.
For a large or sparse domain, use compact IDs if all reachable values can be enumerated,
or compute keys from feature/value pairs instead of allocating the entire numeric range.
Create the table once and share it across states. If using multiple tables for different
feature groups, give them different seeds; the constructor's default seed is the same each time.
XOR any additional features into the initial hash too.

### 3. Update the key with each operation

For an overwrite, remove the old cell key and add the new one before assigning the cell:

```cpp
cell_keys.replace(hash, action.pos,
                  static_cast<int>(cells[action.pos]),
                  static_cast<int>(action.value));
cells[action.pos] = action.value;
```

`replace()` performs `hash ^= old_key ^ new_key`. Keep the existing score and counter
updates as well. Update every changed feature, including automatic effects and relevant counters,
in every operation type.

### 4. Restore the key in undo

One simple option is to extend the earlier `Undo` example with the previous hash:

```cpp
struct Undo {
    long long old_cell;
    long long old_score;
    Hash old_hash;
};

Undo capture_op_a(const Action& action) const {
    return {cells[action.pos], score, hash};
}

void undo_op_a(const Action& action, const Undo& previous) {
    cells[action.pos] = previous.old_cell;
    score = previous.old_score;
    hash = previous.old_hash;
}
```

Alternatively, reverse every XOR update using the action and restored values.
The hash is invertible, so saving `old_hash` is optional; saving it makes restoration simpler.
The restored hash must match the restored board and other hashed fields.

### 5. Return the key and enable deduplication

Add this to `describe()`:

```cpp
out.key = state.hash;
```

With the default `expand()`, this is sufficient to attach keys to generated candidates.
If `expand()` constructs transitions directly, set each `next.key` there as well.
For a board-only overwrite, the child key can be computed without changing State:

```cpp
Hash child_hash = state.hash;
cell_keys.replace(child_hash, action.pos,
                  static_cast<int>(state.cells[action.pos]),
                  static_cast<int>(action.value));
next.key = child_hash;
```

Include all other changed features in that child key before assigning it.
Then enable the existing engine in `main()`:

```cpp
search.deduplicate = true;
```

Equivalent candidates compete for one retained slot, with the higher `value` preferred.
Different 64-bit fingerprints sharing a table slot are handled by the table, but equal
fingerprints are treated as equivalent without comparing full states. Accidental fingerprint
collisions are therefore possible. The template default remains `deduplicate = false`.
