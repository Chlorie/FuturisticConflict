#pragma once

#include <mirai/mirai.h>

#include "config.h"
#include "../utils/lock_container.h"

namespace fc
{
    using namespace mirai::literals;

    class ComponentBase
    {
        friend class Component;

    public:
        inline static constexpr bool dev_only = false;
        inline static constexpr std::string_view name = "[Undefined]";

    protected:
        struct TextMessage final
        {
            mirai::uid_t user;
            mirai::gid_t group;
            mirai::msgid_t msgid;
            std::string_view text;
        };

        std::string description_;
        std::string help_string_;
        lock::vector<mirai::gid_t> group_whitelist_;
        lock::vector<mirai::uid_t> user_blacklist_;

        bool passes(mirai::gid_t group) const;
        bool passes(mirai::uid_t user) const;

        std::optional<TextMessage> check_lists_and_trigger(mirai::uid_t bot_id, const mirai::Event& event) const;

        template <typename Func,
            std::void_t<std::invoke_result_t<Func, std::string_view>>* = nullptr>
        void check_and_reply(mirai::Session& sess, const mirai::Event& event, Func&& func) const
        {
            const auto opt = check_lists_and_trigger(sess.qq(), event);
            if (!opt) return;
            const auto res = std::forward<Func>(func)(opt->text);
            if (!res) return;
            if (opt->group != mirai::gid_t{})
                sess.send_message(opt->group, *res, opt->msgid);
            else
                sess.send_message(opt->user, *res, opt->msgid);
        }

        virtual void do_on_event(mirai::Session& sess, const mirai::Event& event) = 0;
        virtual void do_save_data(Config& config) const { (void)config; }
        virtual void do_load_data(const Config& config) { (void)config; }

    public:
        virtual ~ComponentBase() noexcept = default;
        std::string_view description() const { return description_; }
        std::string_view help_string() const { return help_string_; }
        auto& group_whitelist() { return group_whitelist_; }
        auto& user_blacklist() { return user_blacklist_; }
        const auto& group_whitelist() const { return group_whitelist_; }
        const auto& user_blacklist() const { return user_blacklist_; }
    };

    class Component final
    {
    private:
        bool dev_only_ = false;
        std::string_view name_{};
        std::unique_ptr<ComponentBase> ptr_;

    public:
        Component(const bool dev_only, const std::string_view name, std::unique_ptr<ComponentBase> ptr):
            dev_only_(dev_only), name_(name), ptr_(std::move(ptr)) {}

        bool dev_only() const { return dev_only_; }
        std::string_view name() const { return name_; }
        ComponentBase& get() { return *ptr_; }
        const ComponentBase& get() const { return *ptr_; }
        ComponentBase* operator->() { return ptr_.get(); }
        const ComponentBase* operator->() const { return ptr_.get(); }

        void save_data(Config& config) const;
        void load_data(const Config& config);
        void on_event(mirai::Session& session, const mirai::Event& event);
    };

    template <typename T, typename... Ts,
        std::enable_if_t<std::is_base_of_v<ComponentBase, T>>* = nullptr>
    Component make_component(Ts&&... args)
    {
        return Component(T::dev_only, T::name,
            std::make_unique<T>(std::forward<Ts>(args)...));
    }
}
