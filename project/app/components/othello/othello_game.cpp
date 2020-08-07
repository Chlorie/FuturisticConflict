#include "othello_game.h"
#include "othello_ai.h"

namespace
{
    constexpr std::string_view image_path = "data/image/";
}

namespace fc
{
    mirai::msg::Image OthelloGame::upload_board(mirai::Session& sess, const mirai::gid_t group,
        const MatchInfo& game, const uint64_t prev_mask) const
    {
        const std::string file_name = fmt::format("{}othello{}.bmp", image_path, group);
        gfx_.save_board(file_name, game.state->game_logic.get_state(), prev_mask,
            game.state->game_logic.get_playable_spots());
        return sess.upload_image(mirai::TargetType::group, file_name);
    }

    bool OthelloGame::self_play(mirai::Session& sess, const mirai::gid_t group, MatchInfo& game)
    {
        OthelloLogic& game_logic = game.state->game_logic;
        const int decision = oth::take_action(game_logic.get_state(), game_logic.is_black());
        return play_at(sess, group, game, decision / 8, decision % 8);
    }

    bool OthelloGame::play_at(mirai::Session& sess, const mirai::gid_t group, MatchInfo& game,
        const int row, const int column)
    {
        OthelloLogic& game_logic = game.state->game_logic;
        std::string& history = game.state->history;
        const bool previous_black = game_logic.is_black();
        const OthelloLogic::Result result = game_logic.play(row, column);
        history.push_back(static_cast<char>('a' + column));
        history.push_back(static_cast<char>('1' + row));
        const bool current_black = game_logic.is_black();
        const auto current_player = current_black ? game.first : game.second;

        const uint64_t prev_mask = 0x8000000000000000ull >> (row * 8 + column);
        auto image = upload_board(sess, group, game, prev_mask);

        mirai::Message msg;
        switch (result)
        {
            case OthelloLogic::Result::not_finished:
                if (current_black == previous_black)
                    msg = mirai::Message
                    {
                        mirai::msg::Plain
                        {
                            fmt::format(u8"由于{}方没有可以落子的位置，所以由{}方",
                                (previous_black ? u8"白" : u8"黑"), (previous_black ? u8"黑" : u8"白"))
                        },
                        mirai::msg::At{ current_player },
                        mirai::msg::Plain{ u8"继续落子。目前" }
                    };
                else
                    msg = mirai::Message
                    {
                        mirai::msg::Plain{ fmt::format(u8"轮到{}方", (current_black ? u8"黑" : u8"白")) },
                        mirai::msg::At{ current_player },
                        mirai::msg::Plain{ u8"了。目前" }
                    };
                break;
            case OthelloLogic::Result::draw:
                msg = u8"由于双方都无子可下，本局结束。双方打成平局。最终";
                break;
            default:
                msg = mirai::Message
                {
                    mirai::msg::Plain
                    {
                        result == OthelloLogic::Result::black_win
                            ? u8"由于双方都无子可下，本局结束。黑方"
                            : u8"由于双方都无子可下，本局结束。白方"
                    },
                    mirai::msg::At
                    {
                        result == OthelloLogic::Result::black_win
                            ? game.first : game.second
                    },
                    mirai::msg::Plain{ u8"获胜。最终" }
                };
                break;
        }
        msg += fmt::format(u8"的局面如下，黑棋:白棋={}:{}。\n",
            game_logic.get_black_count(), game_logic.get_white_count());
        msg += std::move(image);
        if (result != OthelloLogic::Result::not_finished)
        {
            msg += u8"\n复盘：" + history;
            sess.send_message(group, msg);
            return true;
        }
        sess.send_message(group, msg);
        if (sess.qq() == current_player) return self_play(sess, group, game);
        return false;
    }

    bool OthelloGame::check_show_state(mirai::Session& sess, const mirai::GroupMessage& e) const
    {
        mirai::msg::Image image;
        {
            if (e.message.content != u8"显示棋盘") return false;
            const auto locked = matches_.to_read();
            const auto iter = locked->find(e.sender.group.id);
            if (iter == locked->end()) return false;
            const MatchInfo& game = iter->second;
            if (!game.state || !game.is_player(e.sender.id)) return false;
            image = upload_board(sess, e.sender.group.id, game, 0);
        }
        sess.send_message(e.sender.group.id, mirai::Message
            {
                mirai::msg::Plain{ u8"目前的局面如下：\n" },
                std::move(image)
            });
        return true;
    }

    bool OthelloGame::check_solve_endgame(mirai::Session& sess, const mirai::GroupMessage& e) const
    {
        using oth::EndGameResult;
        if (e.message.content != u8"终局求解") return false;
        EndGameResult result;
        bool is_black;
        {
            const auto games = matches_.to_read();
            const auto iter = games->find(e.sender.group.id);
            if (iter == games->end()) return false;
            const MatchInfo& game = iter->second;
            if (!game.state || !game.is_player(e.sender.id)) return false;
            const OthelloLogic& game_logic = game.state->game_logic;
            result = oth::perfect_end_game_solution(game_logic.get_state(), game_logic.is_black());
            is_black = game_logic.is_black();
        }
        if (result.action == -1)
            sess.send_quote_message(e, u8"抱歉，目前棋盘上剩余空位还很多，我还没办法很快地计算出来最佳的终盘策略……");
        else
        {
            const int black = is_black ? result.maximizer : result.minimizer;
            const int white = is_black ? result.minimizer : result.maximizer;
            sess.send_quote_message(e, fmt::format(u8"目前状态的终局求解结果是，黑棋:白棋={}:{}。", black, white));
        }
        return true;
    }

    bool OthelloGame::check_move(mirai::Session& sess, const mirai::GroupMessage& e)
    {
        const auto opt = e.message.content.match_types<mirai::msg::Plain>();
        if (!opt) return false;
        const auto msg = std::get<0>(*opt).view();
        if (msg.length() != 2) return false;

        const char column_char = msg[0], row_char = msg[1];
        if ((column_char > 'h' || column_char < 'a') && (column_char > 'H' || column_char < 'A')) return false;
        if (row_char > '8' || row_char < '1') return false;
        const int row = row_char - '1';
        const int column = column_char > 'H' ? column_char - 'a' : column_char - 'A';

        const auto group = e.sender.group.id;
        const auto locked = matches_.to_write();
        const auto iter = locked->find(group);
        if (iter == locked->end() || !iter->second.state) return false;
        MatchInfo& game = iter->second;
        OthelloLogic& game_logic = game.state->game_logic;
        const auto current_player = game_logic.is_black() ? game.first : game.second;
        if (current_player != e.sender.id) return false;
        if (!(game_logic.get_playable_spots() << (row * 8 + column) >> 63))
        {
            sess.send_quote_message(e, u8"你不能在这个位置落子。");
            return true;
        }
        if (play_at(sess, group, game, row, column))
        {
            locked->erase(iter);
            end_game(group);
        }
        return true;
    }

    void OthelloGame::start_game(mirai::Session& sess, const mirai::gid_t group, MatchInfo& game)
    {
        mirai::msg::Image image = upload_board(sess, group, game, 0);
        sess.send_message(group, mirai::Message
            {
                mirai::msg::Plain{ u8"比赛开始！" },
                mirai::msg::At{ game.first },
                mirai::msg::Plain{ u8"执黑，" },
                mirai::msg::At{ game.second },
                mirai::msg::Plain{ u8"执白。目前的局面如下：\n" },
                std::move(image)
            });
        if (game.first == sess.qq()) self_play(sess, group, game);
    }

    bool OthelloGame::process_msg(mirai::Session& sess, const mirai::GroupMessage& e)
    {
        if (check_show_state(sess, e)) return true;
        if (check_solve_endgame(sess, e)) return true;
        if (check_move(sess, e)) return true;
        return false;
    }

    void OthelloGame::give_up_msg(mirai::Session& sess, const mirai::Member& member, const MatchInfo& game)
    {
        const mirai::gid_t group = member.group.id;
        sess.send_message(group, mirai::Message
            {
                mirai::msg::Plain{ fmt::format(u8"由于{}方", member.id == game.first ? u8"黑" : u8"白") },
                mirai::msg::At{ member.id },
                mirai::msg::Plain{ u8"认输，本次比赛结束。最终的局面如下：\n" },
                upload_board(sess, group, game, 0)
            });
    }
}
