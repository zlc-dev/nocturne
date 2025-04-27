#pragma once

#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <string_view>

template<typename T, typename E>
class Result;

template<typename T>
struct ResultTrait {
    inline static constexpr bool is_result = false;
};

template<typename T, typename E>
struct ResultTrait<Result<T, E>> {
    inline static constexpr bool is_result = true;
    using val_t = T;
    using err_t = E;
};

template<typename T>
concept ResultCons = ResultTrait<T>::is_result;

template<typename T = void>
struct Ok {
    T val;
};

template<>
struct Ok<void> {
    Ok() = default;
};

template<typename E = void>
struct Err {
    E val;
};

template<>
struct Err<void> {
    Err() = default;
};

template<typename T, typename E>
class [[nodiscard]] Result {
public:
    Result(Ok<T>&& ok): data(std::in_place_index_t<0>(), std::move(ok.val)) {}
    Result(Err<E>&& err): data(std::in_place_index_t<1>(), std::move(err.val)) {}

    bool is_ok() const {
        return std::holds_alternative<T>(data);
    }

    bool is_err() const {
        return std::holds_alternative<E>(data);
    }

    T unwrap() && {
        if (is_ok()) {
            return std::get<T>(std::move(data));
        }
        throw std::runtime_error("Called unwrap on an error result");
    }

    T expect(std::string_view message) && {
        if (is_ok()) {
            return std::get<T>(std::move(data));
        }
        throw std::runtime_error(message.data());
    }

    E unwrap_err() && {
        if (is_err()) {
            return std::get<E>(std::move(data));
        }
        throw std::runtime_error("Called unwrap_err on a success result");
    }

    template<typename Func, ResultCons Ret = typename std::invoke_result_t<Func, T>>
    requires std::is_same_v<typename ResultTrait<Ret>::err_t, E>
    auto and_then(Func&& f) && -> Result<typename ResultTrait<Ret>::val_t, E> {
        if (is_ok()) {
            return f(std::move(std::get<T>(data)));
        }
        return Err{ std::move(std::get<E>(data)) };
    }

    template<typename Func, ResultCons Ret = typename std::invoke_result_t<Func, T>>
    requires std::is_same_v<typename ResultTrait<Ret>::val_t, T>
    auto or_else(Func&& f) && -> Result<T, typename ResultTrait<Ret>::err_t> {
        if (is_err()) {
            return f(std::move(std::get<E>(data)));
        }
        return Ok{ std::move(std::get<T>(data)) };
    }

    template<typename Func, typename U = std::invoke_result_t<Func, T>>
    Result<U, E> map(Func&& f) && {
        if (is_ok()) {
            if constexpr (std::is_same_v<U, void>)
                return (f(std::move(std::get<T>(data))), Ok{});
            else
                return Ok{ f(std::move(std::get<T>(data))) };
        }
        return Err{ std::move(std::get<E>(data)) };
    }

    template<typename Func, typename F = std::invoke_result_t<Func, E>>
    Result<T, F> map_err(Func&& f) && {
        if (is_err()) {
            if constexpr (std::is_same_v<F, void>)
                return (f(std::move(std::get<E>(data))), Err{ });
            else
                return Err{ f(std::move(std::get<E>(data))) };
        }
        return Ok{ std::move(std::get<T>(data)) };
    }

private:
    std::variant<T, E> data;
};


template<typename E>
class [[nodiscard]] Result<void, E> {
public:
    Result(Ok<void>): error(std::nullopt) {}
    Result(Err<E>&& err): error(std::move(err.val)) {} 

    bool is_ok() const { return !error.has_value(); }
    bool is_err() const { return error.has_value(); }

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

    E unwrap_err() && {
        if (is_err()) {
            return std::move(error.value());
        }
        throw std::runtime_error("Called unwrap_err on a success result");
    }

    template<typename Func, ResultCons Ret = typename std::invoke_result_t<Func>>
    requires std::is_same_v<typename ResultTrait<Ret>::err_t, E>
    auto and_then(Func&& f) && -> Result<typename ResultTrait<Ret>::val_t, E> {
        if (is_ok()) {
            return f();
        }
        return Err{ std::move(error.value()) };
    }

    template<typename Func, ResultCons Ret = typename std::invoke_result_t<Func>>
    requires std::is_same_v<typename ResultTrait<Ret>::val_t, void>
    auto or_else(Func&& f) && -> Result<void, typename ResultTrait<Ret>::err_t> {
        if (is_err()) {
            return f(std::move(error.value()));
        }
        return Ok{};
    }

    template<typename Func, typename U = std::invoke_result_t<Func>>
    Result<U, E> map(Func&& f) && {
        if (is_ok()) {
            if constexpr (std::is_same_v<U, void>)
                return (f(), Ok{});
            else
                return Ok{ f() };
        }
        return Err{ std::move(error.value()) };
    }

    template<typename Func, typename F = std::invoke_result_t<Func, E>>
    Result<void, F> map_err(Func&& f) && {
        if (is_err()) {
            if constexpr (std::is_same_v<F, void>)
                return (f(std::move(error.value())), Err{ });
            else
                return Err{ f(std::move(error.value())) };
        }
        return Ok{};
    }

private:

    std::optional<E> error;
};

template<typename T>
class [[nodiscard]] Result<T, void> {
public:
    Result(Ok<T>&& ok): data(std::move(ok.val)) {}
    Result(Err<void>): data(std::nullopt) {} 

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

    template<typename Func, ResultCons Ret = typename std::invoke_result_t<Func, T>>
    requires std::is_same_v<typename ResultTrait<Ret>::err_t, void>
    auto and_then(Func&& f) && -> Result<typename ResultTrait<Ret>::val_t, void> {
        if (is_ok()) {
            return f(std::move(data.value()));
        }
        return Err{};
    }

    template<typename Func, ResultCons Ret = typename std::invoke_result_t<Func, T>>
    requires std::is_same_v<typename ResultTrait<Ret>::val_t, T>
    auto or_else(Func&& f) && -> Result<T, typename ResultTrait<Ret>::err_t> {
        if (is_err()) {
            return f();
        }
        return Ok{std::move(data.value())};
    }

    template<typename Func, typename U = std::invoke_result_t<Func, T>>
    Result<U, void> map(Func&& f) && {
        if (is_ok()) {
            if constexpr (std::is_same_v<U, void>)
                return (f(std::move(data.value())), Ok{});
            else
                return Ok{ f(std::move(data.value())) };
        }
        return Err{};
    }

    template<typename Func, typename F = std::invoke_result_t<Func>>
    Result<T, F> map_err(Func&& f) && {
        if (is_err()) {
            if constexpr (std::is_same_v<F, void>)
                return (f(), Err{ });
            else
                return Err{ f() };
        }
        return Ok{std::move(data.value())};
    }

private:

    std::optional<T> data;
};

template<>
class [[nodiscard]] Result<void, void> {
public:
    Result(Ok<void>): err(false) {}
    Result(Err<void>): err(true) {} 

    bool is_ok() const { return !err; }
    bool is_err() const { return err; }

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

    template<typename Func, ResultCons Ret = typename std::invoke_result_t<Func>>
    requires std::is_same_v<typename ResultTrait<Ret>::err_t, void>
    auto and_then(Func&& f) && -> Result<typename ResultTrait<Ret>::val_t, void> {
        if (is_ok()) {
            return f();
        }
        return Err{};
    }

    template<typename Func, ResultCons Ret = typename std::invoke_result_t<Func>>
    requires std::is_same_v<typename ResultTrait<Ret>::val_t, void>
    auto or_else(Func&& f) && -> Result<void, typename ResultTrait<Ret>::err_t> {
        if (is_err()) {
            return f();
        }
        return Ok{};
    }

    template<typename Func, typename U = std::invoke_result_t<Func>>
    Result<U, void> map(Func&& f) && {
        if (is_ok()) {
            if constexpr (std::is_same_v<U, void>)
                return (f(), Ok{});
            else
                return Ok{ f() };
        }
        return Err{};
    }

    template<typename Func, typename F = std::invoke_result_t<Func>>
    Result<void, F> map_err(Func&& f) && {
        if (is_err()) {
            if constexpr (std::is_same_v<F, void>)
                return (f(), Err{ });
            else
                return Err{ f() };
        }
        return Ok{};
    }

private:
    bool err;
};