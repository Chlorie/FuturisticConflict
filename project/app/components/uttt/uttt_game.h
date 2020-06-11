#pragma once

#include "../two_player_game.h"
#include "uttt_logic.h"
#include "uttt_graphics.h"

namespace fc
{
    class UtttGame final : public TwoPlayerGame<UtttLogic>
    {
    public:
        inline static constexpr bool dev_only = false;
        inline static constexpr std::string_view name = "UtttGame";

    private:
        UtttGraphics gfx_;

        mirai::msg::Image upload_board(mirai::Session& sess, mirai::gid_t group,
            const MatchInfo& game, int previous = -1) const;
        bool play_at(mirai::Session& sess, mirai::gid_t group, MatchInfo& game, int row, int column);
        bool check_show_state(mirai::Session& sess, const mirai::GroupMessage& e) const;
        bool check_move(mirai::Session& sess, const mirai::GroupMessage& e);

    protected:
        void start_game(mirai::Session& sess, mirai::gid_t group, MatchInfo& game) override;
        bool process_msg(mirai::Session& sess, const mirai::GroupMessage& e) override;
        void give_up_msg(mirai::Session& sess, const mirai::Member& member, const MatchInfo& game) override;

    public:
        UtttGame() : TwoPlayerGame(u8"井字棋", false) {}
    };
}
