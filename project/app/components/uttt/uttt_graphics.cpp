#include "uttt_graphics.h"

namespace fc
{
    void UtttGraphics::draw_small(gfx::Image& image, const int row, const int column, const Spot spot) const
    {
        const int x = column * grid_size + offset;
        const int y = row * grid_size + offset;
        switch (spot)
        {
            case Spot::x: image.draw_image(x_small_, x, y);
                return;
            case Spot::blank: return;
            case Spot::o: image.draw_image(o_small_, x, y);
                return;
            default: throw std::out_of_range("Enum out of range");
        }
    }

    void UtttGraphics::draw_large(gfx::Image& image, const int row, const int column, const Spot spot) const
    {
        const int x = 3 * column * grid_size + offset;
        const int y = 3 * row * grid_size + offset;
        switch (spot)
        {
            case Spot::x: image.draw_image(x_large_, x, y);
                return;
            case Spot::blank: return;
            case Spot::o: image.draw_image(o_large_, x, y);
                return;
            case Spot::draw: image.draw_image(draw_, x, y);
                break;
            default: throw std::out_of_range("Enum out of range");
        }
    }

    void UtttGraphics::draw_previous(gfx::Image& image, const int row, const int column)
    {
        const int x = column * grid_size + offset;
        const int y = row * grid_size + offset;
        image.draw_rectangle(x, y, rect_small, rect_small, 0xffffff30u);
    }

    void UtttGraphics::draw_current(gfx::Image& image, const int row, const int column)
    {
        const int x = 3 * column * grid_size + offset;
        const int y = 3 * row * grid_size + offset;
        image.draw_rectangle(x, y, rect_large, rect_large, 0xffffff30u);
    }

    void UtttGraphics::save_board(const fs::path& path, const State& state, const GlobalState& global_state,
        const int previous, const int current) const
    {
        gfx::Image new_board(board_);
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                draw_small(new_board, i, j, state[i][j]);
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                draw_large(new_board, i, j, global_state[i][j]);
        if (previous != -1) draw_previous(new_board, previous / 9, previous % 9);
        if (current != -1) draw_current(new_board, current / 3, current % 3);
        new_board.save(path);
    }
}
