#pragma once

#include <mirai/mirai.h>

namespace fc
{
    class Component final
    {
    private:
        struct Vtable final
        {
            const char* name = nullptr;
            void (*destruct)(void* p) noexcept = nullptr;
            void (*on_event)(void* p, const mirai::Session& s, const mirai::Event& e) = nullptr;
        };

        template <typename T>
        static constexpr Vtable vtable_of
        {
            T::name,
            [](void* p) noexcept { delete static_cast<T*>(p); },
            [](void* p, const mirai::Session& s, const mirai::Event& e)
            {
                static_cast<T*>(p)->on_event(s, e);
            }
        };

    public:
        template <typename> struct type_tag {};

    private:
        void* ptr_ = nullptr;
        const Vtable* vtable_ = nullptr;

    public:
        Component() = default;

        template <typename T, typename... Ts>
        explicit Component(type_tag<T>, Ts&&... args):
            ptr_(new T(std::forward<Ts>(args)...)),
            vtable_(&vtable_of<T>) {}

        template <typename T, std::enable_if_t<!std::is_same_v<T, Component>>* = nullptr>
        Component(T component):
            ptr_(new T(std::move(component))),
            vtable_(&vtable_of<T>) {}

        ~Component() noexcept
        {
            if (ptr_)
                vtable_->destruct(ptr_);
        }

        Component(const Component&) = delete;

        Component(Component&& other) noexcept:
            ptr_(std::exchange(other.ptr_, nullptr)),
            vtable_(std::exchange(other.vtable_, nullptr)) {}

        Component& operator=(const Component&) = delete;

        Component& operator=(Component&& other) noexcept
        {
            swap(other);
            return *this;
        }

        void swap(Component& other) noexcept
        {
            std::swap(ptr_, other.ptr_);
            std::swap(vtable_, other.vtable_);
        }

        friend void swap(Component& lhs, Component& rhs) noexcept { lhs.swap(rhs); }

        const char* name() const { return vtable_ ? vtable_->name : nullptr; }

        void on_event(const mirai::Session& session, const mirai::Event& event)
        {
            if (ptr_)
                vtable_->on_event(ptr_, session, event);
        }
    };

    template <typename T, typename... Ts>
    Component make_component(Ts&&... args)
    {
        return Component(Component::type_tag<T>{}, std::forward<Ts>(args)...);
    }
}
