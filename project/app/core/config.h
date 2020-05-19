#pragma once

#include <nlohmann/json.hpp>
#include <mirai/mirai.h>

namespace fc
{
    using nlohmann::json;

    struct Config final
    {
        struct ComponentCommon final
        {
            std::string help_string;
            std::string description;
            std::vector<mirai::gid_t> group_whitelist;
            std::vector<mirai::uid_t> user_blacklist;
        };

        std::string auth_key;
        mirai::uid_t bot_id;
        mirai::uid_t dev_id;

        std::unordered_map<std::string, ComponentCommon> components;
    };

    void from_json(const json& j, Config::ComponentCommon& comp);
    void to_json(json& j, const Config::ComponentCommon& comp);
    void from_json(const json& j, Config& conf);
    void to_json(json& j, const Config& conf);
}
