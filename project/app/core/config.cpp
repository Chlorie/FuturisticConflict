#include "config.h"

namespace fc
{
    void from_json(const json& j, Config::ComponentCommon& comp)
    {
        if (const auto iter = j.find("help_string"); iter != j.end())
            comp.help_string = *iter;
        if (const auto iter = j.find("description"); iter != j.end())
            comp.description = *iter;
        j["group_whitelist"].get_to(comp.group_whitelist);
        j["user_blacklist"].get_to(comp.user_blacklist);
    }

    void to_json(json& j, const Config::ComponentCommon& comp)
    {
        j["help_string"] = comp.help_string;
        j["description"] = comp.description;
        j["group_whitelist"] = comp.group_whitelist;
        j["user_blacklist"] = comp.user_blacklist;
    }

    void from_json(const json& j, Config& conf)
    {
        j["auth_key"].get_to(conf.auth_key);
        j["bot_id"].get_to(conf.bot_id);
        j["dev_id"].get_to(conf.dev_id);
        j["components"].get_to(conf.components);
    }

    void to_json(json& j, const Config& conf)
    {
        j["auth_key"] = conf.auth_key;
        j["bot_id"] = conf.bot_id;
        j["dev_id"] = conf.dev_id;
        j["components"] = conf.components;
    }
}
