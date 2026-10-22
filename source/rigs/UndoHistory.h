#pragma once

#include <optional>
#include <vector>

// UNDO / REDO for the sound: a list of snapshots and a position in it.
// A snapshot is recorded each time an edit is finished (a knob let go, a
// switch clicked, a file or rig loaded), so one undo takes back one whole
// action rather than one tiny step of a knob turn.
//
// `State` needs an `==` that says whether two snapshots sound the same, so
// recording an unchanged sound (for example, after an undo reloads a file)
// doesn't add an empty step.
template <typename State>
class UndoHistory
{
public:
    static constexpr size_t maxSteps = 100;

    // Records a finished edit. Returns false if nothing actually changed.
    bool record (const State& state)
    {
        if (! states.empty() && states[position] == state)
        {
            states[position] = state;   // keep details that don't count as a change (e.g. the modified dot)
            return false;
        }

        // A new edit after some undos throws away what could have been redone.
        if (! states.empty())
            states.erase (states.begin() + (long) position + 1, states.end());

        states.push_back (state);
        if (states.size() > maxSteps)
            states.erase (states.begin());
        position = states.size() - 1;
        return true;
    }

    // Updates the current snapshot without adding a step (after saving, say).
    void replaceCurrent (const State& state)
    {
        if (states.empty())
            record (state);
        else
            states[position] = state;
    }

    bool canUndo() const noexcept { return position > 0; }
    bool canRedo() const noexcept { return ! states.empty() && position + 1 < states.size(); }

    std::optional<State> undo()
    {
        if (! canUndo())
            return std::nullopt;
        return states[--position];
    }

    std::optional<State> redo()
    {
        if (! canRedo())
            return std::nullopt;
        return states[++position];
    }

    void clear() { states.clear(); position = 0; }
    bool isEmpty() const noexcept { return states.empty(); }

private:
    std::vector<State> states;
    size_t position = 0;
};
