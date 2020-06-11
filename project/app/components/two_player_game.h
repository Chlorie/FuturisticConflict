#pragma once

#include <fmt/core.h>

#include "../core/component.h"
#include "../utils/lock_algorithm.h"
#include "../utils/random.h"
#include "../utils/scheduler.h"

namespace fc
{
    template <typename GameState>
    class TwoPlayerGame : public ComponentBase
    {
    protected:
        struct MatchInfo final
        {
            mirai::uid_t first{};
            mirai::uid_t second{};
            std::optional<GameState> state;
            bool is_player(const mirai::uid_t user) const { return user == first || user == second; }
        };

    private:
        inline static lock::set<mirai::gid_t> gaming_groups_;
        inline static Scheduler<mirai::gid_t> response_timeout_scheduler_;
        std::string_view name_;
        bool has_ai_ = false;

        void timeout(mirai::Session& sess, const mirai::gid_t group)
        {
            const auto locked = matches_.to_write();
            const auto iter = locked->find(group);
            if (iter == locked->end()) return;
            sess.send_message(group, u8"由于一分钟内无人回应挑战，本次挑战自动结束。我们还是期待下次的对决吧！");
            locked->erase(iter);
            gaming_groups_->erase(group);
        }

        bool is_challenge_text(std::string_view str) const
        {
            static constexpr auto challenge_chars = u8"挑战"sv;
            str = mirai::utils::trim_whitespace(str);
            if (!mirai::utils::ends_with(str, challenge_chars)) return false;
            str.remove_suffix(challenge_chars.size());
            return str == name_;
        }

        std::optional<mirai::uid_t> match_challenge_text(const mirai::GroupMessage& e) const
        {
            const auto& msg = e.message.content;
            if (const auto opt = msg.match_types<mirai::msg::Plain>())
            {
                const auto& [plain] = *opt;
                if (!is_challenge_text(plain.view())) return {};
                return mirai::uid_t{}; // Group wide challenge
            }
            if (const auto opt = msg.match_types<mirai::msg::At, mirai::msg::Plain>())
            {
                const auto& [at, plain] = *opt;
                if (!is_challenge_text(plain.view())) return {};
                return at.target;
            }
            if (const auto opt = msg.match_types<mirai::msg::Plain, mirai::msg::At>())
            {
                const auto& [plain, at] = *opt;
                if (!is_challenge_text(plain.view())) return {};
                return at.target;
            }
            return {};
        }

        bool send_challenge(mirai::Session& sess, const mirai::GroupMessage& e)
        {
            const auto opt = match_challenge_text(e);
            if (!opt) return false;
            const auto group = e.sender.group.id;
            if (lock::contains(gaming_groups_, group))
            {
                sess.send_quote_message(e, u8"本群当前有正在进行中的游戏，等这一局结束再发起挑战吧！");
                return true;
            }
            const auto user = *opt;
            if (user == e.sender.id)
            {
                sess.send_quote_message(e, u8"自己挑战自己，你一定很空虚吧");
                return true;
            }
            if (user == sess.qq())
            {
                if (!has_ai_)
                {
                    sess.send_quote_message(e, u8"可是我还不太会玩这个，你先找别人挑战吧！");
                    return true;
                }
                sess.send_quote_message(e, u8"那我就接受挑战了！");
                gaming_groups_->insert(group);
                const auto locked = matches_.to_write();
                const auto insertion = locked->insert({
                    group, random::bernoulli_bool()
                               ? MatchInfo{ user, e.sender.id, {} }
                               : MatchInfo{ e.sender.id, user, {} }
                });
                MatchInfo& match = insertion.first->second;
                match.state = GameState{};
                start_game(sess, group, match);
                return true;
            }
            if (user != mirai::uid_t{})
            {
                gaming_groups_->insert(group);
                matches_->insert({
                    group, MatchInfo{ e.sender.id, user, {} }
                });
                sess.send_message(group, mirai::Message{
                    mirai::msg::At{ user },
                    mirai::msg::Plain{ u8"你愿意接受挑战吗？回复我接受挑战或者放弃挑战就好了！" }
                });
                response_timeout_scheduler_.schedule(group, [&, group](const bool aborted)
                {
                    if (!aborted)
                        timeout(sess, group);
                }, 1min);
                return true;
            }
            // user == mirai::uid_t{}
            {
                gaming_groups_->insert(group);
                matches_->insert({ group, MatchInfo{ e.sender.id, {}, {} } });
                sess.send_message(group, fmt::format(u8"有人发起了{}挑战！回复“接受挑战”就可以与挑战者对决了！", name_));
                response_timeout_scheduler_.schedule(group, [&, group](const bool aborted)
                {
                    if (!aborted)
                        timeout(sess, group);
                }, 1min);
                return true;
            }
        }

        bool accept_challenge(mirai::Session& sess, const mirai::GroupMessage& e)
        {
            if (e.message.content != u8"接受挑战") return false;
            {
                const auto locked = matches_.to_write();
                const auto iter = locked->find(e.sender.group.id);
                if (iter == locked->end()) return false;
                auto& match = iter->second;
                if (match.state) return false;
                else if (match.second == mirai::uid_t{}) // Group-wide challenge
                {
                    if (match.first == e.sender.id)
                    {
                        sess.send_quote_message(e,
                            u8"左右手互搏就不要找我了，你考虑过你自己刷屏对群友造成的影响吗？");
                        locked->erase(iter);
                        gaming_groups_->erase(e.sender.group.id);
                        return true;
                    }
                }
                else if (match.second != e.sender.id) return false;
                response_timeout_scheduler_.cancel(e.sender.group.id);
                match.second = e.sender.id;
                match.state = GameState{};
                if (random::bernoulli_bool()) std::swap(match.first, match.second);
                start_game(sess, e.sender.group.id, match);
            }
            return true;
        }

        bool cancel_challenge(mirai::Session& sess, const mirai::GroupMessage& e)
        {
            if (e.message.content != u8"放弃挑战") return false;
            const auto locked = matches_.to_write();
            const auto iter = locked->find(e.sender.group.id);
            if (iter == locked->end()) return false;
            auto& match = iter->second;
            if (match.state || !match.is_player(e.sender.id)) return false;
            response_timeout_scheduler_.cancel(e.sender.group.id);
            sess.send_quote_message(e, match.first == e.sender.id
                                           ? u8"看来这次没找到人呢……下次有机会再说吧！"
                                           : u8"本次游戏因为被挑战方放弃而无法开始……我们还是期待下次的对决吧！");
            locked->erase(iter);
            gaming_groups_->erase(e.sender.group.id);
            return true;
        }

        bool give_up(mirai::Session& sess, const mirai::GroupMessage& e)
        {
            if (e.message.content != u8"认输" && e.message.content != u8"投降") return false;
            const auto locked = matches_.to_write();
            const auto iter = locked->find(e.sender.group.id);
            const auto& match = iter->second;
            if (!match.state || !match.is_player(e.sender.id)) return false;
            give_up_msg(sess, e.sender, match);
            locked->erase(iter);
            gaming_groups_->erase(e.sender.group.id);
            return true;
        }

    protected:
        lock::map<mirai::gid_t, MatchInfo> matches_;

        virtual void start_game(mirai::Session& sess, mirai::gid_t group, MatchInfo& game) = 0;
        virtual bool process_msg(mirai::Session& sess, const mirai::GroupMessage& e) = 0;
        virtual void give_up_msg(mirai::Session& sess, const mirai::Member& member, const MatchInfo& game) = 0;

        void end_game(const mirai::gid_t group)
        {
            matches_->erase(group);
            gaming_groups_->erase(group);
        }

        void do_on_event(mirai::Session& sess, const mirai::Event& event) override
        {
            event.dispatch([&](const mirai::GroupMessage& e)
            {
                if (!passes(e.sender.group.id)) return;
                if (e.message.quote) return;
                if (send_challenge(sess, e)) return;
                if (accept_challenge(sess, e)) return;
                if (cancel_challenge(sess, e)) return;
                if (give_up(sess, e)) return;
                process_msg(sess, e);
            });
        }

    public:
        TwoPlayerGame(const std::string_view game_name, const bool has_ai): name_(game_name), has_ai_(has_ai) {}
    };
}
