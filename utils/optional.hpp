/*
    optional
    Copyright (C) 2025 zlc-dev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.
*/


#pragma once

#include "result.hpp"
#include <optional>
#include <stdexcept>

template<typename T>
struct Optional;

template<typename T>
struct OptionalTrait {
    inline static constexpr bool is_optional = false;
};

template<typename T>
struct OptionalTrait<Optional<T>> {
    inline static constexpr bool is_optional = true;
    using val_t = T;
};

template<typename T>
concept OptionalCons = OptionalTrait<T>::is_optional;


template <typename T = void>
struct Some {
    T val;
};

template <>
struct Some<void> {
};

struct None {};

#define NONE {}

template <typename T>
struct Optional {
    Optional(Some<T>&& some): data( std::move(some.val) ) {}
    Optional(None&& none): data( std::nullopt ) {}
    Optional(): Optional( None{} ) {}

    bool is_ok() const { return data.has_value(); }
    bool is_err() const { return !data.has_value(); }

    T unwrap() && {
        if (is_ok()) {
            return std::move(data.value());
        }
        throw std::runtime_error("Called unwrap on an error result");
    }

    T expect(std::string_view message) && {
        if (is_ok()) {
            return std::move(data.value());
        }
        throw std::runtime_error(message.data());
    }

    void unwrap_err() && {
        if (is_err()) {
            return;
        }
        throw std::runtime_error("Called unwrap_err on a success result");
    }

    template<typename Func, OptionalCons Ret = typename std::invoke_result_t<Func, T>>
    auto and_then(Func&& f) && -> Optional<typename OptionalTrait<Ret>::val_t> {
        if (is_ok()) {
            return f(std::move(data).value());
        }
        return None{};
    }

    template<typename Func, OptionalCons Ret = typename std::invoke_result_t<Func, T>>
    requires std::is_same_v<typename OptionalTrait<Ret>::val_t, T>
    auto or_else(Func&& f) && -> Optional<T> {
        if (is_err()) {
            return f();
        }
        return Some{ std::move(data).value() };
    }

    template<typename Func, typename U = std::invoke_result_t<Func, T>>
    Optional<U> map(Func&& f) && {
        if (is_ok()) {
            if constexpr (std::is_same_v<U, void>)
                return (f(std::move(data).value()), Some{});
            else
                return Some { f(std::move(data).value()) };
        }
        return None{};
    }


    template<typename E>
    auto ok_or(E&& err) && -> Result<T, E> {
        if(is_ok()) {
            Ok { std::move(data).value() };
        } else {
            Err { std::move(err) };
        }
    }

    auto ok_or() && -> Result<T, void> {
        if(is_ok()) {
            Ok { std::move(data).value() };
        } else {
            Err{};
        }
    }

private:
    std::optional<T> data;
};

template <>
struct Optional<void> {
    Optional(Some<void>&& some): has_value( true ) {}
    Optional(None&& none): has_value( false ) {}
    Optional(): Optional( None{} ) {}

    bool is_ok() const { return has_value; }
    bool is_err() const { return has_value; }

    void unwrap() && {
        if (is_ok()) {
            return;
        }
        throw std::runtime_error("Called unwrap on an error result");
    }

    void expect(std::string_view message) && {
        if (is_ok()) {
            return;
        }
        throw std::runtime_error(message.data());
    }

    void unwrap_err() && {
        if (is_err()) {
            return;
        }
        throw std::runtime_error("Called unwrap_err on a success result");
    }

    template<typename Func, OptionalCons Ret = typename std::invoke_result_t<Func>>
    auto and_then(Func&& f) && -> Optional<typename OptionalTrait<Ret>::val_t> {
        if (is_ok()) {
            return f();
        }
        return None{};
    }

    template<typename Func, OptionalCons Ret = typename std::invoke_result_t<Func>>
    requires std::is_same_v<typename OptionalTrait<Ret>::val_t, void>
    auto or_else(Func&& f) && -> Optional<void> {
        if (is_err()) {
            return f();
        }
        return Some{};
    }

    template<typename Func, typename U = std::invoke_result_t<Func>>
    Optional<U> map(Func&& f) && {
        if (is_ok()) {
            if constexpr (std::is_same_v<U, void>)
                return (f(), Some{});
            else
                return Some { f() };
        }
        return None{};
    }


    template<typename E>
    auto ok_or(E&& err) && -> Result<void, E> {
        if(is_ok()) {
            return Ok {};
        } else {
            return Err { std::move(err) };
        }
    }

    auto ok_or() && -> Result<void, void> {
        if(is_ok()) {
            return Ok {};
        } else {
            return Err{};
        }
    }

private:
    bool has_value { false };
};
