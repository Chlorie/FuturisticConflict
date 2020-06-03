#pragma once

#include <optional>
#include <asio.hpp>

#include "lock_container.h"

namespace fc
{
    template <typename Key>
    class Scheduler final
    {
    private:
        using Work = asio::io_context::work;
        using Timer = asio::steady_timer;
        std::thread thread_;
        asio::io_context io_;
        std::optional<Work> work_;
        lock::map<Key, Timer> timers_;

    public:
        Scheduler(): work_(io_)
        {
            thread_ = std::thread([&]() { io_.run(); });
        }

        ~Scheduler() noexcept
        {
            try
            {
                work_.reset();
                {
                    const auto locked = timers_.to_write();
                    for (auto& [key, timer] : *locked)
                        timer.cancel();
                    locked->clear();
                }
                if (thread_.joinable())
                    thread_.join();
            }
            catch (...) {}
        }

        template <typename Func, typename Rep, typename Period>
        bool schedule(const Key& key, Func&& func, const std::chrono::duration<Rep, Period> delay)
        {
            const auto locked = timers_.to_write();
            const auto iter = locked->find(key);
            if (iter != locked->end()) return false;
            const auto [new_iter, inserted] = locked->emplace(std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(io_, std::chrono::duration_cast<std::chrono::nanoseconds>(delay)));
            auto& timer = new_iter->second;
            timer.async_wait([this, func = std::forward<Func>(func), key](const std::error_code& ec)
            {
                func(ec == asio::error::operation_aborted);
                const auto locked = timers_.to_write();
                const auto iter = locked->find(key);
                if (iter != locked->end()) locked->erase(iter);
            });
            return true;
        }

        bool cancel(const Key& key)
        {
            const auto locked = timers_.to_write();
            const auto iter = locked->find(key);
            if (iter == locked->end()) return false;
            iter->second.cancel();
            return true;
        }
    };
}
