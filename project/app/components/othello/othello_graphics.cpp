#include "othello_graphics.h"

namespace fc
{
    void OthelloGraphics::draw_disk(gfx::Image& image, const int row, const int column, const bool is_black)
    {
        const int x = grid_size * (column + 1);
        const int y = grid_size * (row + 1);
        static constexpr uint32_t black = 0x000000ff;
        static constexpr uint32_t white = 0xd0d0d0ff;
        image.draw_circle(static_cast<float>(x), static_cast<float>(y),
            disk_radius, is_black ? black : white);
    }

    void OthelloGraphics::draw_hint(gfx::Image& image, const int row, const int column, const bool is_previous)
    {
        const int x = grid_size * (column + 1);
        const int y = grid_size * (row + 1);
        static constexpr uint32_t red = 0x7f0000ff;
        static constexpr uint32_t green = 0x007f20ff;
        image.draw_circle(static_cast<float>(x), static_cast<float>(y),
            hint_radius, is_previous ? red : green);
    }

    void OthelloGraphics::save_board(const fs::path& path, const State& state,
        const BitBoard previous_mask, const BitBoard mobility_mask) const
    {
        gfx::Image new_board(board_);
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
            {
                if (bit_test(state.black, i, j))
                    draw_disk(new_board, i, j, true);
                else if (bit_test(state.white, i, j))
                    draw_disk(new_board, i, j, false);
                if (bit_test(previous_mask, i, j))
                    draw_hint(new_board, i, j, true);
                if (bit_test(mobility_mask, i, j))
                    draw_hint(new_board, i, j, false);
            }
        (void)new_board.save(path);
    }
}
