#pragma once

#include <array>

namespace fc
{
    class UtttLogic final
    {
    public:
        enum class Spot : int8_t
        {
            x = -1,
            blank = 0,
            o = 1,
            draw = 2
        };
        enum class Result : int8_t
        {
            x_win = -1,
            not_finished = 0,
            o_win = 1,
            draw = 2
        };
        using State = std::array<std::array<Spot, 9>, 9>;
        using GlobalState = std::array<std::array<Spot, 3>, 3>;
    private:
        State state_ = {};
        GlobalState global_state_ = {};
        bool is_o_ = true;
        int current_local_ = -1;
        Result get_local_result(int local);
        Result get_global_result();
    public:
        UtttLogic() = default;
        bool is_o() const { return is_o_; }
        const State& get_state() const { return state_; }
        const GlobalState& get_global_state() const { return global_state_; }
        bool is_playable(int row, int column) const;
        int get_local() const { return current_local_; }
        Result play(int row, int column);
    };
}
