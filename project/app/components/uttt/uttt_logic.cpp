#include "uttt_logic.h"

#include <stdexcept>

namespace fc
{
    UtttLogic::Result UtttLogic::get_local_result(const int local)
    {
        const int row = local / 3 * 3;
        const int column = local % 3 * 3;
        // Someone wins the local 3x3
        for (int i = 0; i < 3; i++)
        {
            const Spot row_spot = state_[row + i][column];
            if (row_spot != Spot::blank &&
                row_spot == state_[row + i][column + 1] &&
                row_spot == state_[row + i][column + 2])
                return row_spot == Spot::o ? Result::o_win : Result::x_win;
            const Spot column_spot = state_[row][column + i];
            if (column_spot != Spot::blank &&
                column_spot == state_[row + 1][column + i] &&
                column_spot == state_[row + 2][column + i])
                return column_spot == Spot::o ? Result::o_win : Result::x_win;
        }
        // Or diagonals
        const Spot nw_diagonal = state_[row][column];
        if (nw_diagonal != Spot::blank &&
            nw_diagonal == state_[row + 1][column + 1] &&
            nw_diagonal == state_[row + 2][column + 2])
            return nw_diagonal == Spot::o ? Result::o_win : Result::x_win;
        const Spot ne_diagonal = state_[row + 2][column];
        if (ne_diagonal != Spot::blank &&
            ne_diagonal == state_[row + 1][column + 1] &&
            ne_diagonal == state_[row][column + 2])
            return ne_diagonal == Spot::o ? Result::o_win : Result::x_win;
        // Draw case
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (state_[row + i][column + j] == Spot::blank)
                    return Result::not_finished;
        return Result::draw;
    }

    UtttLogic::Result UtttLogic::get_global_result()
    {
        for (int i = 0; i < 3; i++)
        {
            const Spot row_spot = global_state_[i][0];
            if (row_spot != Spot::blank &&
                row_spot != Spot::draw &&
                row_spot == global_state_[i][1] &&
                row_spot == global_state_[i][2])
                return row_spot == Spot::o ? Result::o_win : Result::x_win;
            const Spot column_spot = global_state_[0][i];
            if (column_spot != Spot::blank &&
                column_spot != Spot::draw &&
                column_spot == global_state_[1][i] &&
                column_spot == global_state_[2][i])
                return column_spot == Spot::o ? Result::o_win : Result::x_win;
        }
        const Spot nw_diagonal = global_state_[0][0];
        if (nw_diagonal != Spot::blank &&
            nw_diagonal != Spot::draw &&
            nw_diagonal == global_state_[1][1] &&
            nw_diagonal == global_state_[2][2])
            return nw_diagonal == Spot::o ? Result::o_win : Result::x_win;
        const Spot ne_diagonal = global_state_[0][2];
        if (ne_diagonal != Spot::blank &&
            ne_diagonal != Spot::draw &&
            ne_diagonal == global_state_[1][1] &&
            ne_diagonal == global_state_[2][0])
            return ne_diagonal == Spot::o ? Result::o_win : Result::x_win;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (global_state_[i][j] == Spot::blank)
                    return Result::not_finished;
        return Result::draw;
    }

    bool UtttLogic::is_playable(const int row, const int column) const
    {
        if (state_[row][column] != Spot::blank) return false; // Local spot occupied
        if (global_state_[row / 3][column / 3] != Spot::blank) return false; // Global spot occupied
        return !(current_local_ != -1 && (row / 3) * 3 + column / 3 != current_local_); // Incorrect global spot
    }

    UtttLogic::Result UtttLogic::play(const int row, const int column)
    {
        state_[row][column] = is_o_ ? Spot::o : Spot::x;
        is_o_ = !is_o_;
        const int local_row = row / 3;
        const int local_column = column / 3;
        const int local = local_row * 3 + local_column;
        const Result local_result = get_local_result(local);
        bool no_global_change = false;
        switch (local_result)
        {
            case Result::x_win: global_state_[local_row][local_column] = Spot::x;
                break;
            case Result::not_finished: no_global_change = true;
                break;
            case Result::o_win: global_state_[local_row][local_column] = Spot::o;
                break;
            case Result::draw: global_state_[local_row][local_column] = Spot::draw;
                break;
            default: throw std::out_of_range("Enum out of range");
        }
        const int global_row = row % 3;
        const int global_column = column % 3;
        const int global = global_row * 3 + global_column;
        current_local_ = global_state_[global_row][global_column] == Spot::blank ? global : -1;
        if (no_global_change) return Result::not_finished;
        return get_global_result();
    }
}
