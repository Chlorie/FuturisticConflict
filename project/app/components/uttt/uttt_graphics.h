#pragma once

#include <array>
#include <filesystem>

#include "uttt_logic.h"
#include "../../utils/image.h"

namespace fc
{
    namespace fs = std::filesystem;

    class UtttGraphics final
    {
    private:
        using State = UtttLogic::State;
        using GlobalState = UtttLogic::GlobalState;
        using Spot = UtttLogic::Spot;
        gfx::Image board_{ "data/uttt/board.bmp" };
        gfx::Image o_small_{ "data/uttt/o.bmp" };
        gfx::Image o_large_{ "data/uttt/olarge.bmp" };
        gfx::Image x_small_{ "data/uttt/x.bmp" };
        gfx::Image x_large_{ "data/uttt/xlarge.bmp" };
        gfx::Image draw_{ "data/uttt/draw.bmp" };
        inline static const int grid_size = 30;
        inline static const int offset = 16;
        inline static const int rect_small = 28;
        inline static const int rect_large = 88;
        void draw_small(gfx::Image& image, int row, int column, Spot spot) const;
        void draw_large(gfx::Image& image, int row, int column, Spot spot) const;
        static void draw_previous(gfx::Image& image, int row, int column);
        static void draw_current(gfx::Image& image, int row, int column);
    public:
        UtttGraphics() = default;
        void save_board(const fs::path& path, const State& state, const GlobalState& global_state,
            int previous = -1, int current = -1) const;
    };
}
