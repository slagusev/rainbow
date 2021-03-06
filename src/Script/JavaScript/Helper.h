// Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef SCRIPT_JAVASCRIPT_HELPER_H_
#define SCRIPT_JAVASCRIPT_HELPER_H_

#include <tuple>
#include <type_traits>
#include <utility>

#include <Rainbow/TextAlignment.h>
#include <duktape.h>

#include "Audio/Mixer.h"
#include "Common/Logging.h"
#include "Common/TypeInfo.h"
#include "Graphics/Animation.h"
#include "Graphics/Sprite.h"
#include "Memory/Array.h"
#include "Memory/SharedPtr.h"

#define dukr_type_error(ctx, ...)                                              \
    duk_error_raw((ctx),                                                       \
                  DUK_ERR_TYPE_ERROR,                                          \
                  DUK_FILE_MACRO,                                              \
                  DUK_LINE_MACRO,                                              \
                  __VA_ARGS__)

#define DUKR_HIDDEN_SYMBOL_ADDRESS DUK_HIDDEN_SYMBOL("address")
#define DUKR_HIDDEN_SYMBOL_CALLBACK DUK_HIDDEN_SYMBOL("callback")
#define DUKR_HIDDEN_SYMBOL_INDEX DUK_HIDDEN_SYMBOL("index")
#define DUKR_HIDDEN_SYMBOL_SPRITEBATCH DUK_HIDDEN_SYMBOL("spritebatch")
#define DUKR_HIDDEN_SYMBOL_TYPE DUK_HIDDEN_SYMBOL("type")
#define DUKR_WELLKNOWN_SYMBOL_TOSTRINGTAG                                      \
    DUK_WELLKNOWN_SYMBOL("Symbol.toStringTag")

namespace rainbow::duk
{
    enum class Allocation
    {
        NoHeap,
        HeapAllocated,
    };

    class ScopedStack
    {
    public:
        ScopedStack(duk_context* ctx) : context_(ctx), index_(duk_get_top(ctx))
        {
        }

        ~ScopedStack() { duk_set_top(context_, index_); }

    private:
        duk_context* context_;
        duk_idx_t index_;
    };

    template <typename T>
    constexpr auto buffer_object_type() -> duk_uint_t
    {
        if constexpr (std::is_same_v<T, double>)
        {
            return DUK_BUFOBJ_FLOAT64ARRAY;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            return DUK_BUFOBJ_FLOAT32ARRAY;
        }
        else if constexpr (std::is_integral_v<T> && sizeof(T) == sizeof(int8_t))
        {
            return DUK_BUFOBJ_INT8ARRAY;
        }
        else if constexpr (std::is_integral_v<T> &&
                           sizeof(T) == sizeof(int16_t))
        {
            return DUK_BUFOBJ_INT16ARRAY;
        }
        else if constexpr (std::is_integral_v<T> &&
                           sizeof(T) == sizeof(int32_t))
        {
            return DUK_BUFOBJ_INT32ARRAY;
        }
        else
        {
            return DUK_BUFOBJ_DATAVIEW;
        }
    }

    void dump_context(duk_context* ctx);

    template <typename T>
    auto forward(T&& t) noexcept
    {
        return std::forward<T>(t);
    }

    template <typename T>
    auto forward(std::unique_ptr<T>& p) noexcept
    {
        return std::move(p);
    }

    template <size_t N>
    auto get_prop_literal(duk_context* ctx,
                          duk_idx_t obj_idx,
                          const char (&key)[N])
    {
        return duk_get_prop_lstring(ctx, obj_idx, key, N - 1);
    }

    template <typename T>
    auto get(duk_context* ctx, duk_idx_t idx) -> T;

    template <>
    inline auto get<bool>(duk_context* ctx, duk_idx_t idx) -> bool
    {
        return duk_require_boolean(ctx, idx);
    }

    template <>
    inline auto get<int>(duk_context* ctx, duk_idx_t idx) -> int
    {
        return duk_require_int(ctx, idx);
    }

    template <>
    inline auto get<uint32_t>(duk_context* ctx, duk_idx_t idx) -> uint32_t
    {
        return duk_require_uint(ctx, idx);
    }

    template <>
    inline auto get<float>(duk_context* ctx, duk_idx_t idx) -> float
    {
        return duk_require_number(ctx, idx);
    }

    template <>
    inline auto get<const char*>(duk_context* ctx, duk_idx_t idx) -> const char*
    {
        return duk_require_string(ctx, idx);
    }

    template <>
    inline auto get<SpriteRef>(duk_context* ctx, duk_idx_t idx) -> SpriteRef
    {
        ScopedStack stack{ctx};

        duk::get_prop_literal(ctx, idx, DUKR_HIDDEN_SYMBOL_SPRITEBATCH);
        duk::get_prop_literal(ctx, idx, DUKR_HIDDEN_SYMBOL_INDEX);

        return {*static_cast<SpriteBatch*>(duk_require_pointer(ctx, -2)),
                duk_require_uint(ctx, -1)};
    }

    template <>
    inline auto get<TextAlignment>(duk_context* ctx, duk_idx_t idx)
        -> TextAlignment
    {
        return static_cast<TextAlignment>(duk_require_int(ctx, idx));
    }

    inline void push(duk_context*) {}

    template <typename... Args>
    void push(duk_context* ctx, bool f, Args&&... args)
    {
        duk_push_boolean(ctx, f);
        push(ctx, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void push(duk_context* ctx, int32_t i, Args&&... args)
    {
        duk_push_int(ctx, i);
        push(ctx, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void push(duk_context* ctx, uint8_t u, Args&&... args)
    {
        duk_push_uint(ctx, u);
        push(ctx, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void push(duk_context* ctx, uint32_t u, Args&&... args)
    {
        duk_push_uint(ctx, u);
        push(ctx, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void push(duk_context* ctx, uint64_t u, Args&&... args)
    {
        duk_push_uint(ctx, u);
        push(ctx, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void push(duk_context* ctx, float f, Args&&... args)
    {
        duk_push_number(ctx, f);
        push(ctx, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void push(duk_context* ctx, const char* str, Args&&... args)
    {
        duk_push_string(ctx, str);
        push(ctx, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void push(duk_context* ctx, const void* p, Args&&... args)
    {
        duk_push_pointer(ctx, const_cast<void*>(p));
        push(ctx, std::forward<Args>(args)...);
    }

    void push(duk_context* ctx, const Color&);
    void push(duk_context* ctx, const SpriteRef&);
    void push(duk_context* ctx, const Vec2f&);
    void push(duk_context* ctx, audio::Channel*);
    void push(duk_context* ctx, audio::Sound*);

    inline void push(duk_context* ctx, TextAlignment alignment)
    {
        duk_push_int(ctx, static_cast<int>(alignment));
    }

    inline auto push_instance(duk_context* ctx, duk_idx_t obj_idx) -> void*
    {
        duk::get_prop_literal(ctx, obj_idx, DUKR_HIDDEN_SYMBOL_ADDRESS);
        return duk_require_pointer(ctx, -1);
    }

    template <size_t N>
    auto put_prop_literal(duk_context* ctx,
                          duk_idx_t obj_idx,
                          const char (&key)[N])
    {
        return duk_put_prop_lstring(ctx, obj_idx, key, N - 1);
    }

    inline void put_instance(duk_context* ctx, duk_idx_t obj_idx, void* ptr)
    {
        duk_push_pointer(ctx, ptr);
        duk::put_prop_literal(ctx, obj_idx, DUKR_HIDDEN_SYMBOL_ADDRESS);
    }

    namespace detail
    {
        template <typename R, size_t I, size_t J, typename T, typename... Types>
        void get_args(duk_context* ctx, R& args)
        {
            std::get<I>(args) = get<T>(ctx, I);
            if constexpr (I + 1 < J)
                get_args<R, I + 1, J, Types...>(ctx, args);
        }

        template <typename T, typename Tuple, size_t... I>
        constexpr auto make_from_tuple_in(void* ptr,
                                          [[maybe_unused]] Tuple&& t,
                                          std::index_sequence<I...>) -> T*
        {
            if constexpr (is_shared_ptr<T>)
            {
                return new (ptr) T(std::make_unique<typename T::element_type>(
                    forward(std::get<I>(std::forward<Tuple>(t)))...));
            }
            else
            {
                return new (ptr)
                    T(forward(std::get<I>(std::forward<Tuple>(t)))...);
            }
        }
    }

    template <typename... Args>
    bool call(duk_context* ctx, const char* func, Args&&... args)
    {
        duk_get_global_string(ctx, func);
        duk::push(ctx, std::forward<Args>(args)...);
#ifdef NDEBUG
        duk_call(ctx, sizeof...(args));
        constexpr bool success = true;
#else
        const bool success =
            duk_pcall(ctx, sizeof...(args)) == DUK_EXEC_SUCCESS;
        if (!success)
        {
            LOGF("JavaScript: An error occurred while calling '%s'", func);
            dump_context(ctx);
        }
#endif
        duk_pop(ctx);
        return success;
    }

    template <typename... Types>
    auto get_args(duk_context* ctx)
    {
        std::tuple<Types...> args;
        detail::get_args<decltype(args), 0, sizeof...(Types), Types...>(
            ctx, args);
        return args;
    }

    template <>
    inline auto get_args<>(duk_context*)
    {
        return std::make_tuple();
    }

    template <typename T, bool PerformTypeCheck = true>
    auto get_pointer(duk_context* ctx, duk_idx_t idx) -> T*
    {
        ScopedStack stack{ctx};

        if constexpr (PerformTypeCheck)
        {
            duk_require_object(ctx, idx);
            duk::get_prop_literal(ctx, idx, DUKR_HIDDEN_SYMBOL_TYPE);
            const auto type = duk_require_pointer(ctx, -1);
            if (type != type_id<T>().value())
                dukr_type_error(ctx, "Expected an opaque handle");
        }

        auto ptr = duk::push_instance(ctx, idx);
        return static_cast<T*>(ptr);
    }

    template <size_t N>
    auto has_prop_literal(duk_context* ctx,
                          duk_idx_t obj_idx,
                          const char (&key)[N])
    {
        return duk_has_prop_lstring(ctx, obj_idx, key, N - 1);
    }

    template <typename... Args>
    bool opt_call(duk_context* ctx, const char* func, Args&&... args)
    {
        if (duk_get_global_string(ctx, func))
            return call(ctx, func, std::forward<Args>(args)...);

        duk_pop(ctx);
        return true;
    }

    template <typename T, typename... Args>
    void push_constructor(duk_context* ctx)
    {
        duk_push_c_function(
            ctx,
            [](duk_context* ctx) -> duk_ret_t {
                if (!duk_is_constructor_call(ctx))
                    return DUK_RET_TYPE_ERROR;

                auto args = duk::get_args<Args...>(ctx);

                void* ptr = duk_alloc(ctx, sizeof(T));
                detail::make_from_tuple_in<T>(
                    ptr,
                    args,
                    std::make_index_sequence<std::tuple_size_v<
                        std::remove_reference_t<decltype(args)>>>{});

                duk_push_this(ctx);
                duk::put_instance(ctx, duk_get_top(ctx) - 1, ptr);
                return 0;
            },
            sizeof...(Args));
    }

    template <typename T, typename U>
    void push_external_buffer_object(duk_context* ctx, const U* ptr, size_t len)
    {
        duk_push_external_buffer(ctx);
        duk_config_buffer(ctx, -1, const_cast<U*>(ptr), len);
        duk_push_buffer_object(ctx, -1, 0, len, duk::buffer_object_type<T>());
        duk_remove(ctx, -2);
    }

    template <size_t N>
    void push_literal(duk_context* ctx, const char (&str)[N])
    {
        duk_push_lstring(ctx, str, N - 1);
    }

    template <typename T>
    auto push_this(duk_context* ctx)
    {
        duk_push_this(ctx);
        auto ptr = static_cast<T*>(duk::push_instance(ctx, -1));
        duk_pop(ctx);

        if constexpr (is_shared_ptr<T>)
            return *ptr;
        else
            return ptr;
    }

    template <>
    inline auto push_this<SpriteRef>(duk_context* ctx)
    {
        duk_push_this(ctx);
        return get<SpriteRef>(ctx, duk_get_top_index(ctx));
    }

    template <typename T, Allocation Alloc, typename F>
    void put_prototype(duk_context* ctx, F&& put_props)
    {
        const auto obj_idx = duk_push_object(ctx);
        duk::push(ctx, type_id<T>().value());
        duk::put_prop_literal(ctx, -2, DUKR_HIDDEN_SYMBOL_TYPE);

        put_props(ctx);

        if constexpr (Alloc == Allocation::HeapAllocated)
        {
            duk_push_c_function(
                ctx,
                [](duk_context* ctx) -> duk_ret_t {
                    auto ptr = static_cast<T*>(duk::push_instance(ctx, -1));
                    ptr->~T();
                    duk_free(ctx, ptr);
                    return 0;
                },
                1);
            duk_set_finalizer(ctx, obj_idx);
        }

        duk_freeze(ctx, obj_idx);
        duk::put_prop_literal(ctx, -2, "prototype");
    }

    template <typename T>
    void register_module(duk_context* ctx, duk_idx_t rainbow);

    template <typename F>
    void register_module(duk_context* ctx,
                         duk_idx_t rainbow,
                         const char* key,
                         F&& put_props)
    {
        const auto obj_idx = duk_push_bare_object(ctx);
        put_props(ctx);
        duk_freeze(ctx, obj_idx);
        duk_put_prop_string(ctx, rainbow, key);

        R_ASSERT(duk_get_top(ctx) == 1, "We didn't clean up properly!");
    }
}  // namespace rainbow::duk

#endif
