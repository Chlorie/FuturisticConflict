#include "dice_roll.h"

#include <fmt/core.h>
#include <boost/regex.hpp>

#include "../utils/random.h"

namespace fc
{
    std::string DiceRoll::roll(const int count, const int facade)
    {
        if (count == 0) return u8"你是在耍我吗？";
        if (facade > 100 || facade < 2) return u8"哪里有这样的骰子啊？我还真没见过呢……";
        if (facade < 4) return u8"二三面的骰子我真的找不到，硬币我也没有，不如就用六面骰子代替吧……";
        if (count > 1000) return u8"我摇累了，不想摇了";
        std::vector<int> rolls(static_cast<size_t>(count));
        int sum = 0;
        for (int i = 0; i < count; i++)
            sum += rolls[i] = random::uniform_int(1, facade);
        std::string rolls_str = fmt::format("{}", rolls[0]);
        for (int i = 1; i < std::min(count, 10); i++)
            rolls_str += fmt::format(", {}", rolls[i]);
        if (count > 10) rolls_str += "...";
        return fmt::format("{}d{} = {} ({})", count, facade, sum, rolls_str);
    }

    std::optional<std::string> DiceRoll::process(const std::string_view msg)
    {
        static const boost::regex reg{ R"((?:roll)? *(\d+)d(\d+))" };
        boost::match_results<std::string_view::const_iterator> match;
        if (!regex_match(msg.begin(), msg.end(), match, reg)) return {};
        const auto [count, facade] = mirai::utils::parse_captures
            <void, std::optional<int>, std::optional<int>>(match);
        if (!count || !facade) return {};
        return roll(*count, *facade);
    }

    void DiceRoll::do_on_event(mirai::Session& sess, const mirai::Event& event)
    {
        check_and_reply(sess, event, &DiceRoll::process);
    }
}
