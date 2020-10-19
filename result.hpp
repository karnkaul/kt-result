// KT header-only library
// Requirements: C++17

#pragma once
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace kt {
///
/// \brief Default success/failure indicator type
///
enum class result_signal { ok, error };
///
/// \brief Default result dispatch tag type
///
template <typename Err = std::string, typename Sig = result_signal>
struct result_dispatch;
///
/// \brief Default result dispatch tag type
///
template <typename Err>
struct result_dispatch<Err, result_signal> {
	using sig = result_signal;
	using err = Err;

	static constexpr sig success = result_signal::ok;
	static constexpr sig failure = result_signal::error;
};

namespace detail {
template <typename T, typename Sig, Sig Null, typename E>
struct result_storage {
	std::optional<T> data;
	E error = {};
	Sig signal = Null;
};
template <typename T, typename Sig, Sig Null>
struct result_storage<T, Sig, Null, void> {
	std::optional<T> data;
	Sig signal = Null;
};
template <typename E>
struct result_err {
	using type = E const&;
};
template <>
struct result_err<void> {
	using type = bool;
};
template <typename E>
using result_err_t = typename result_err<E>::type;
template <typename...>
constexpr bool false_t = false;
template <std::size_t N, typename... T>
constexpr bool is_ne = sizeof...(T) != N;
} // namespace detail

///
/// \brief structure containing T / RD::err (if non-void) and RD::sig
///
template <typename T, typename RD = result_dispatch<>>
struct result {
	using type = T;
	using sig = typename RD::sig;
	using err = typename RD::err;
	using err_t = detail::result_err_t<err>;

	static_assert(std::is_void_v<err> || std::is_default_constructible_v<err>, "err must be default constructible!");

	///
	/// \brief (Default) constructor for failure signal
	///
	constexpr result() noexcept = default;
	///
	/// \brief Constructor for failure signal
	///
	template <typename... Args, typename = std::enable_if_t<detail::is_ne<0, Args...>>>
	constexpr result(Args&&... args) noexcept(noexcept(std::is_nothrow_move_constructible_v<err>));
	///
	/// \brief Constructor for success signal
	///
	constexpr result(T&& value) noexcept(noexcept(std::is_nothrow_move_constructible_v<T>));
	///
	/// \brief Constructor for success signal
	///
	constexpr result(T& value) noexcept(noexcept(std::is_nothrow_constructible_v<T>));
	///
	/// \brief Constructor for non-success signal
	///
	template <typename... Args>
	constexpr result(sig signal, Args&&... args) noexcept(noexcept(std::is_nothrow_move_constructible_v<err>));

	///
	/// \brief Check if signal represents success
	///
	constexpr operator bool() const noexcept;
	///
	/// \brief Obtain signal
	///
	constexpr sig signal() const;
	///
	/// \brief Obtain value if success (throws otherwise)
	///
	constexpr T const& operator*() const;
	///
	/// \brief Move value if success (throws otherwise)
	///
	constexpr T&& move();
	///
	/// \brief Access value if success (throws otherwise)
	///
	constexpr T const* operator->() const;
	///
	/// \brief Obtain result value if success, else fallback
	///
	constexpr T const& value_or(T const& fallback) const noexcept;
	///
	/// \brief Obtain error if not success (RD::err{} / false otherwise)
	///
	err_t error() const noexcept;

  private:
	///
	/// \brief Storage for result signal and data (and error, if not void)
	///
	detail::result_storage<T, sig, RD::failure, err> storage;
};

///
/// \brief Typedef for result with no error value (void)
///
template <typename T, typename Sig = result_signal>
using result_void = result<T, result_dispatch<void, Sig>>;
///
/// \brief Typedef for result with std::string_view error value
///
template <typename T, typename Sig = result_signal>
using result_sv = result<T, result_dispatch<std::string_view, Sig>>;

template <typename Err, typename Sig>
struct result_dispatch {
	static_assert(detail::false_t<Sig>, "result_dispatch missing specialisation!");
};

template <typename T, typename RD>
template <typename... Args, typename>
constexpr result<T, RD>::result(Args&&... args) noexcept(noexcept(std::is_nothrow_move_constructible_v<err>))
	: result(RD::failure, std::forward<Args>(args)...) {
}

template <typename T, typename RD>
constexpr result<T, RD>::result(T&& value) noexcept(noexcept(std::is_nothrow_move_constructible_v<T>)) {
	storage.data = std::move(value);
	storage.signal = RD::success;
}

template <typename T, typename RD>
constexpr result<T, RD>::result(T& value) noexcept(noexcept(std::is_nothrow_constructible_v<T>)) {
	storage.data = value;
	storage.signal = RD::success;
}

template <typename T, typename RD>
template <typename... Args>
constexpr result<T, RD>::result(sig signal, Args&&... args) noexcept(noexcept(std::is_nothrow_move_constructible_v<err>)) {
	if constexpr (!std::is_void_v<err>) {
		if constexpr (sizeof...(Args) > 0) {
			storage.error = err{std::forward<Args>(args)...};
		}
	} else {
		static_assert(sizeof...(Args) == 0, "Invalid args: err is void!");
	}
	storage.signal = signal;
}

template <typename T, typename RD>
constexpr result<T, RD>::operator bool() const noexcept {
	return storage.signal == RD::success;
}

template <typename T, typename RD>
constexpr T const& result<T, RD>::operator*() const {
	if (storage.signal == RD::success) {
		return *storage.data;
	}
	throw std::runtime_error("Invalid result!");
}

template <typename T, typename RD>
constexpr T&& result<T, RD>::move() {
	if (storage.signal == RD::success) {
		storage.signal = RD::failure;
		return std::move(*storage.data);
	}
	throw std::runtime_error("Invalid result!");
}

template <typename T, typename RD>
constexpr T const* result<T, RD>::operator->() const {
	if (storage.signal == RD::success) {
		return &(*storage.data);
	}
	throw std::runtime_error("Invalid result!");
}

template <typename T, typename RD>
constexpr T const& result<T, RD>::value_or(const T& fallback) const noexcept {
	if (storage.signal == RD::success) {
		return *storage.data;
	}
	return fallback;
}

template <typename T, typename RD>
typename result<T, RD>::err_t result<T, RD>::error() const noexcept {
	bool const b_error = storage.signal != RD::success;
	if constexpr (std::is_void_v<err>) {
		return b_error;
	} else {
		static err const none{};
		return b_error ? storage.error : none;
	}
}
} // namespace kt
