#pragma once

#include "../two_player_game.h"
#include "othello_logic.h"
#include "othello_graphics.h"

namespace fc
{
    struct OthelloState
    {
        OthelloLogic game_logic;
        std::string history;
        OthelloState() { history.reserve(121); }
    };

    class OthelloGame final : public TwoPlayerGame<OthelloState>
    {
    public:
        inline static constexpr bool dev_only = false;
        inline static constexpr std::string_view name = "OthelloGame";

    private:
        OthelloGraphics gfx_;

        mirai::msg::Image upload_board(mirai::Session& sess, mirai::gid_t group,
            const MatchInfo& game, uint64_t prev_mask) const;
        bool self_play(mirai::Session& sess, mirai::gid_t group, MatchInfo& game);
        bool play_at(mirai::Session& sess, mirai::gid_t group, MatchInfo& game, int row, int column);
        bool check_show_state(mirai::Session& sess, const mirai::GroupMessage& e) const;
        bool check_solve_endgame(mirai::Session& sess, const mirai::GroupMessage& e) const;
        bool check_move(mirai::Session& sess, const mirai::GroupMessage& e);

    protected:
        void start_game(mirai::Session& sess, mirai::gid_t group, MatchInfo& game) override;
        bool process_msg(mirai::Session& sess, const mirai::GroupMessage& e) override;
        void give_up_msg(mirai::Session& sess, const mirai::Member& member, const MatchInfo& game) override;

    public:
        OthelloGame() : TwoPlayerGame(u8"黑白棋", true) {}
    };
}
