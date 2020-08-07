#pragma once

#include <array>
#include <bitset>

namespace fc
{
    class UtttLogic final
    {
    public:
        enum class Spot : uint8_t
        {
            blank = 0,
            o = 1,
            draw = 2,
            x = 3,
        };
        enum class Result : uint8_t
        {
            not_finished = 0,
            o_win = 1,
            draw = 2,
            x_win = 3,
        };

        //class State final
        //{
        //private:
        //    std::bitset<180> xs_;
        //    //std::bitset<90> os_;
        //public:
        //    Spot operator[](size_t index)
        //    {
        //        
        //    }
        //    Spot operator()(const size_t row, const size_t column)
        //    {
        //        constexpr size_t size = sizeof(State);
        //    }
        //};

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
