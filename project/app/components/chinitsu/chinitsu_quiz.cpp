#include "chinitsu_quiz.h"

#include <fmt/core.h>

#include "../commands/command_view.h"
#include "info_table.h"
#include "../../utils/random.h"

namespace fc
{
    namespace
    {
        enum class QuizType
        {
            invalid, tenpai
        };

        struct Result final
        {
            std::optional<std::string> question;
            uint32_t time = 0;
            std::optional<std::string> answer;
        };

        Result tenpai_quiz(uint32_t time, const bool no_riipai)
        {
            const auto& indices = cit::info_table.tenpai_indices;
            const size_t no = static_cast<size_t>(random::uniform_int(
                0, static_cast<int32_t>(cit::info_table.tamenchan_count - 1)));
            if (time == 0) time = 20u;
            time = std::clamp(time, 5u, 60u);
            const size_t index = indices[no];
            const auto entry = cit::info_table.info_table[index];
            auto tiles = cit::NoRiipaiTiles(cit::Tiles::from_base5_index(index));
            if (no_riipai) tiles.shuffle();
            return
            {
                fmt::format(u8"问题 T{}：以下的手牌分别听哪些牌？\n{} ({})",
                    no, tiles.format_as_unicode(), tiles.format_as_numbers()),
                time,
                fmt::format(u8"答案：{} ({})", entry.format_as_unicode(), entry.format_as_numbers())
            };
        }

        Result parse_args(const std::string_view msg)
        {
            const CommandView view(msg);
            if (view.size() == 0) return {};
            if (view[0] != u8"清一色" && view[0] != u8"chinitsu" && view[0] != u8"チンイツ") return {};

            QuizType quiz_type = QuizType::invalid;
            uint32_t time = 0;
            bool no_riipai = false;
            for (size_t i = 1; i < view.size(); i++)
            {
                std::string_view arg = view[i];
                if (arg == "t")
                    quiz_type = QuizType::tenpai;
                else if (arg[0] == 's')
                {
                    arg.remove_prefix(1);
                    auto seconds = mirai::utils::parse<std::optional<uint32_t>>(arg);
                    if (!seconds) return { u8"无法分析出思考时间", 0u, {} };
                    time = *seconds;
                }
                else if (arg == "n")
                    no_riipai = true;
            }

            switch (quiz_type)
            {
                case QuizType::invalid: return { u8"未选择题目类型", 0u, {} };
                case QuizType::tenpai: return tenpai_quiz(time, no_riipai);
                default: return {};
            }
        }
    }


    void Chinitsu::do_on_event(mirai::Session& sess, const mirai::Event& event)
    {
        const auto opt = check_lists_and_trigger(sess.qq(), event);
        if (!opt) return;
        Result res = parse_args(opt->text);
        mirai::msgid_t msgid;
        if (res.question)
        {
            if (opt->group != mirai::gid_t{})
                msgid = sess.send_message(opt->group, *res.question);
            else
                msgid = sess.send_message(opt->user, *res.question);
            if (res.answer)
            {
                if (opt->group != mirai::gid_t{})
                    scheduler_.schedule(msgid,
                        [&sess, msgid, group = opt->group, ans = std::move(*res.answer)](bool)
                        {
                            sess.send_message(group, ans, msgid);
                        }, std::chrono::seconds(res.time));
                else
                    scheduler_.schedule(msgid,
                        [&sess, msgid, user = opt->user, ans = std::move(*res.answer)](bool)
                        {
                            sess.send_message(user, ans, msgid);
                        }, std::chrono::seconds(res.time));
            }
        }
    }
}
