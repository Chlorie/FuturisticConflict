#include "uttt_game.h"

namespace
{
    constexpr std::string_view image_path = "data/image/";
}

namespace fc
{
    mirai::msg::Image UtttGame::upload_board(mirai::Session& sess, const mirai::gid_t group,
        const MatchInfo& game, const int previous) const
    {
        const std::string file_name = fmt::format("{}uttt{}.bmp", image_path, group);
        gfx_.save_board(file_name, game.state->get_state(), game.state->get_global_state(),
            previous, game.state->get_local());
        return sess.upload_image(mirai::TargetType::group, file_name);
    }

    bool UtttGame::play_at(mirai::Session& sess, const mirai::gid_t group, MatchInfo& game,
        const int row, const int column)
    {
        UtttLogic& game_logic = *game.state;
        const auto result = game_logic.play(row, column);
        const bool is_o = game_logic.is_o();
        const auto current_player = is_o ? game.first : game.second;
        auto image = upload_board(sess, group, game, row * 9 + column);

        mirai::Message msg;
        switch (result)
        {
            case UtttLogic::Result::not_finished:
                msg = mirai::Message
                {
                    mirai::msg::Plain{ fmt::format(u8"轮到画{}一方", (is_o ? u8"圈" : u8"叉")) },
                    mirai::msg::At{ current_player },
                    mirai::msg::Plain{ u8"了。目前" }
                };
                break;
            case UtttLogic::Result::draw:
                msg = u8"双方均未在全局棋盘中连成三子，故双方打成平局，本局结束。最终";
                break;
            default:
                msg = mirai::Message
                {
                    mirai::msg::Plain
                    {
                        fmt::format(u8"全局棋盘中{0}号连成三子，故画{0}方",
                            (result == UtttLogic::Result::o_win ? u8"圈" : u8"叉"))
                    },
                    mirai::msg::At{ result == UtttLogic::Result::o_win ? game.first : game.second },
                    mirai::msg::Plain{ u8"获胜。最终" }
                };
                break;
        }
        msg += u8"的局面如下：\n";
        msg += std::move(image);
        sess.send_message(group, msg);
        return result != UtttLogic::Result::not_finished;
    }

    bool UtttGame::check_show_state(mirai::Session& sess, const mirai::GroupMessage& e) const
    {
        mirai::msg::Image image;
        {
            if (e.message.content != u8"显示棋盘") return false;
            const auto locked = matches_.to_read();
            const auto iter = locked->find(e.sender.group.id);
            if (iter == locked->end()) return false;
            const MatchInfo& game = iter->second;
            if (!game.state || !game.is_player(e.sender.id)) return false;
            image = upload_board(sess, e.sender.group.id, game);
        }
        sess.send_message(e.sender.group.id, mirai::Message
            {
                mirai::msg::Plain{ u8"目前的局面如下：\n" },
                std::move(image)
            });
        return true;
    }

    bool UtttGame::check_move(mirai::Session& sess, const mirai::GroupMessage& e)
    {
        const auto opt = e.message.content.match_types<mirai::msg::Plain>();
        if (!opt) return false;
        const auto msg = std::get<0>(*opt).view();
        if (msg.length() != 2) return false;

        const char column_char = msg[0], row_char = msg[1];
        if ((column_char > 'i' || column_char < 'a') && (column_char > 'I' || column_char < 'A')) return false;
        if (row_char > '9' || row_char < '1') return false;
        const int row = row_char - '1';
        const int column = column_char > 'I' ? column_char - 'a' : column_char - 'A';

        const auto group = e.sender.group.id;
        const auto locked = matches_.to_write();
        const auto iter = locked->find(group);
        if (iter == locked->end() || !iter->second.state) return false;
        MatchInfo& game = iter->second;
        UtttLogic& game_logic = *game.state;
        const auto current_player = game.state->is_o() ? game.first : game.second;
        if (current_player != e.sender.id) return false;
        if (!game_logic.is_playable(row, column))
        {
            sess.send_quote_message(e, u8"你不能在这个位置落子。");
            return true;
        }
        if (play_at(sess, group, game, row, column)) end_game(group);
        return true;
    }

    void UtttGame::start_game(mirai::Session& sess, const mirai::gid_t group, MatchInfo& game)
    {
        mirai::msg::Image image = upload_board(sess, group, game);
        sess.send_message(group, mirai::Message
            {
                mirai::msg::Plain{ u8"比赛开始！" },
                mirai::msg::At{ game.first },
                mirai::msg::Plain{ u8"画圈，" },
                mirai::msg::At{ game.second },
                mirai::msg::Plain{ u8"画叉。目前的局面如下：\n" },
                std::move(image)
            });
    }

    bool UtttGame::process_msg(mirai::Session& sess, const mirai::GroupMessage& e)
    {
        if (check_show_state(sess, e)) return true;
        if (check_move(sess, e)) return true;
        return false;
    }

    void UtttGame::give_up_msg(mirai::Session& sess, const mirai::Member& member, const MatchInfo& game)
    {
        const mirai::gid_t group = member.group.id;
        sess.send_message(group, mirai::Message
            {
                mirai::msg::Plain{ fmt::format(u8"由于画{}一方", member.id == game.first ? u8"圈" : u8"叉") },
                mirai::msg::At{ member.id },
                mirai::msg::Plain{ u8"认输，本次比赛结束。最终的局面如下：\n" },
                upload_board(sess, group, game)
            });
    }
}
