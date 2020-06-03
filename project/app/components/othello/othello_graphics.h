#pragma once

#include <array>
#include <filesystem>

#include "../../utils/image.h"
#include "othello_logic.h"

namespace fc
{
    namespace fs = std::filesystem;

    class OthelloGraphics
    {
    private:
        using Color = std::array<unsigned char, 4>;
        using State = OthelloLogic::State;
        using BitBoard = OthelloLogic::BitBoard;
    private:
        gfx::Image board_{ "data/othello/othello_board.bmp" };
        static constexpr int grid_size = 30;
        static constexpr int disk_radius = 10;
        static constexpr int hint_radius = 2;
        static constexpr int square_half_length = 12;
        static bool bit_test(const BitBoard bits, const int row, const int column)
        {
            return bits << (row * 8 + column) >> 63;
        }
        static void draw_disk(gfx::Image& image, int row, int column, bool is_black);
        static void draw_hint(gfx::Image& image, int row, int column, bool is_previous);
    public:
        OthelloGraphics() = default;
        void save_board(const fs::path& path, const State& state,
            BitBoard previous_mask, BitBoard mobility_mask) const;
    };
}
