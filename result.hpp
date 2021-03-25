// KT header-only library
// Requirements: C++17

#pragma once
#include <cassert>
#include <optional>
#include <type_traits>
#include <variant>

namespace kt {
namespace detail {
template <typename T, typename E>
struct result_storage_t;
template <typename... T>
constexpr bool false_v = false;
} // namespace detail

///
/// \brief Models a result (T) or an error (E) value
/// Note: T cannot be void
/// Specializations:
/// 	- T, T : homogeneous result and error types
/// 	- T, void : result type only (like optional)
/// 	- bool, void : boolean result only (like bool)
///
template <typename T, typename E = void>
class result;

///
/// \brief Type alias for no result
///
constexpr auto null_result = nullptr;

///
/// \brief Models a result (T) or an error (E) value
/// Note: T cannot be void
///
template <typename T, typename E>
class result {
	static_assert(!std::is_same_v<T, void>, "T = void is not supported");

  public:
	///
	/// \brief Result type
	///
	using type = T;
	///
	/// \brief Error type
	///
	using err_t = E;

	///
	/// \brief Default constructor (failure)
	///
	constexpr result() = default;
	///
	/// \brief Constructor for result (success)
	///
	constexpr result(type&& t);
	///
	/// \brief Constructor for result (success)
	///
	constexpr result(type const& t);
	///
	/// \brief Constructor for error (failure)
	///
	constexpr result(err_t&& e);
	///
	/// \brief Constructor for error (failure)
	///
	constexpr result(err_t const& e);
	///
	/// \brief Constructor for implicit failure
	///
	constexpr result(std::nullptr_t);

	///
	/// \brief Operator to check for success
	///
	constexpr explicit operator bool() const noexcept;
	///
	/// \brief Check for success
	///
	constexpr bool has_result() const noexcept;
	///
	/// \brief Check for failure
	///
	constexpr bool has_error() const noexcept;

	///
	/// \brief Obtain result
	///
	constexpr type const& get_result() const;
	///
	/// \brief Obtain result if success else fallback
	///
	constexpr type const& value_or(type const& fallback) const;
	///
	/// \brief Obtain error
	///
	constexpr err_t const& error() const;
	///
	/// \brief Move result
	///
	constexpr type move();

	constexpr type const& operator*() const;
	constexpr type const* operator->() const;

  private:
	detail::result_storage_t<type, err_t> m_storage;
};

///
/// \brief Models a result or an error value (T)
/// Note: T cannot be void
///
template <typename T>
class result<T, T> {
	static_assert(!std::is_same_v<T, void>, "T = void is not supported");

  public:
	///
	/// \brief Result type
	///
	using type = T;
	///
	/// \brief Error type
	///
	using err_t = T;

	///
	/// \brief Default constructor (failure)
	///
	constexpr result();
	///
	/// \brief Constructor for implicit failure
	///
	constexpr result(std::nullptr_t);

	///
	/// \brief Set result (success)
	///
	constexpr void set_result(type&& t);
	///
	/// \brief Set result (success)
	///
	constexpr void set_result(type const& t);
	///
	/// \brief Set error (failure)
	///
	constexpr void set_error(err_t&& t);
	///
	/// \brief Set error (failure)
	///
	constexpr void set_error(err_t const& t);

	///
	/// \brief Operator to check for success
	///
	constexpr explicit operator bool() const noexcept;
	///
	/// \brief Check for success
	///
	constexpr bool has_result() const noexcept;
	///
	/// \brief Check for failure
	///
	constexpr bool has_error() const noexcept;

	///
	/// \brief Obtain result
	///
	constexpr type const& get_result() const;
	///
	/// \brief Obtain result if success else fallback
	///
	constexpr type const& value_or(type const& fallback) const;
	///
	/// \brief Obtain error
	///
	constexpr err_t const& error() const;
	///
	/// \brief Move result
	///
	constexpr type move();

	constexpr type const& operator*() const;
	constexpr type const* operator->() const;

  private:
	detail::result_storage_t<type, void> m_storage;
	bool m_error = true;
};

///
/// \brief Models an optional result (T)
/// Note: T cannot be void
///
template <typename T>
class result<T, void> {
	static_assert(!std::is_same_v<T, void>, "T = void is not supported");

  public:
	///
	/// \brief Result type
	///
	using type = T;

	///
	/// \brief Default constructor (failure)
	///
	constexpr result() = default;
	///
	/// \brief Constructor for result (success)
	///
	constexpr result(type&& t);
	///
	/// \brief Constructor for result (success)
	///
	constexpr result(type const& t);
	///
	/// \brief Constructor for implicit failure
	///
	constexpr result(std::nullptr_t);

	///
	/// \brief Operator to check for success
	///
	constexpr explicit operator bool() const noexcept;
	///
	/// \brief Check for success
	///
	constexpr bool has_result() const noexcept;
	///
	/// \brief Check for failure
	///
	constexpr bool has_error() const noexcept;

	///
	/// \brief Obtain result
	///
	constexpr type const& get_result() const;
	///
	/// \brief Obtain result if success else fallback
	///
	constexpr type const& value_or(type const& fallback) const;
	///
	/// \brief Move result
	///
	constexpr type move();

	constexpr type const& operator*() const;
	constexpr type const* operator->() const;

  private:
	detail::result_storage_t<T, void> m_storage;
};

namespace detail {
template <typename T, typename E>
struct result_storage_t {
	std::variant<T, E> val;

	constexpr result_storage_t() : val(E{}) {
	}
	constexpr result_storage_t(T&& t) : val(std::move(t)) {
	}
	constexpr result_storage_t(T const& t) : val(t) {
	}
	constexpr result_storage_t(E&& e) : val(std::move(e)) {
	}
	constexpr result_storage_t(E const& e) : val(e) {
	}
	constexpr bool has_value() const noexcept {
		return std::holds_alternative<T>(val);
	}
	constexpr T const& value() const {
		assert(has_value());
		return std::get<T>(val);
	}
	constexpr T move() {
		assert(has_value());
		T ret = std::get<T>(std::move(val));
		val = E{};
		return ret;
	}
	constexpr E const& error() const {
		assert(!has_value());
		return std::get<E>(val);
	}
};
template <typename T>
struct result_storage_t<T, void> {
	std::optional<T> val;

	constexpr result_storage_t() = default;
	constexpr result_storage_t(T&& t) : val(std::move(t)) {
	}
	constexpr result_storage_t(T const& t) : val(t) {
	}
	constexpr bool has_value() const noexcept {
		return val.has_value();
	}
	constexpr T const& value() const {
		assert(has_value());
		return *val;
	}
	constexpr T move() {
		assert(has_value());
		T ret = std::move(*val);
		val.reset();
		return ret;
	}
};
template <>
struct result_storage_t<bool, void> {
	bool val;

	constexpr result_storage_t() : val(false) {
	}
	constexpr result_storage_t(bool val) : val(val) {
	}
	constexpr bool has_value() const noexcept {
		return val;
	}
	constexpr bool value() const {
		return val;
	}
	constexpr bool move() {
		bool const ret = val;
		val = false;
		return ret;
	}
};
} // namespace detail

template <typename T, typename E>
constexpr result<T, E>::result(type&& t) : m_storage(std::move(t)) {
}
template <typename T, typename E>
constexpr result<T, E>::result(type const& t) : m_storage(t) {
}
template <typename T, typename E>
constexpr result<T, E>::result(err_t&& e) : m_storage(std::move(e)) {
}
template <typename T, typename E>
constexpr result<T, E>::result(err_t const& e) : m_storage(e) {
}
template <typename T, typename E>
constexpr result<T, E>::result(std::nullptr_t) : result() {
}
template <typename T, typename E>
constexpr result<T, E>::operator bool() const noexcept {
	return has_result();
}
template <typename T, typename E>
constexpr bool result<T, E>::has_result() const noexcept {
	return m_storage.has_value();
}
template <typename T, typename E>
constexpr bool result<T, E>::has_error() const noexcept {
	return !has_result();
}
template <typename T, typename E>
constexpr typename result<T, E>::type const& result<T, E>::get_result() const {
	return m_storage.value();
}
template <typename T, typename E>
constexpr typename result<T, E>::type const& result<T, E>::value_or(type const& fallback) const {
	return has_result() ? get_result() : fallback;
}
template <typename T, typename E>
constexpr typename result<T, E>::err_t const& result<T, E>::error() const {
	return m_storage.error();
}
template <typename T, typename E>
constexpr typename result<T, E>::type result<T, E>::move() {
	return m_storage.move();
}
template <typename T, typename E>
constexpr typename result<T, E>::type const& result<T, E>::operator*() const {
	return get_result();
}
template <typename T, typename E>
constexpr typename result<T, E>::type const* result<T, E>::operator->() const {
	return &get_result();
}

template <typename T>
constexpr result<T, T>::result() : m_storage(err_t{}), m_error(true) {
}
template <typename T>
constexpr result<T, T>::result(std::nullptr_t) : result() {
}
template <typename T>
constexpr void result<T, T>::set_result(type&& t) {
	m_storage = std::move(t);
	m_error = false;
}
template <typename T>
constexpr void result<T, T>::set_result(type const& t) {
	m_storage = t;
	m_error = false;
}
template <typename T>
constexpr void result<T, T>::set_error(err_t&& e) {
	m_storage = std::move(e);
	m_error = true;
}
template <typename T>
constexpr void result<T, T>::set_error(err_t const& e) {
	m_storage = e;
	m_error = true;
}
template <typename T>
constexpr result<T, T>::operator bool() const noexcept {
	return has_result();
}
template <typename T>
constexpr bool result<T, T>::has_result() const noexcept {
	return !m_error;
}
template <typename T>
constexpr bool result<T, T>::has_error() const noexcept {
	return !has_result();
}
template <typename T>
constexpr typename result<T, T>::type const& result<T, T>::get_result() const {
	assert(!m_error);
	return m_storage.value();
}
template <typename T>
constexpr typename result<T, T>::type const& result<T, T>::value_or(type const& fallback) const {
	return has_result() ? get_result() : fallback;
}
template <typename T>
constexpr typename result<T, T>::err_t const& result<T, T>::error() const {
	assert(m_error);
	return m_storage.value();
}
template <typename T>
constexpr typename result<T, T>::type result<T, T>::move() {
	assert(!m_error);
	T ret = m_storage.move();
	error(err_t{});
	return ret;
}
template <typename T>
constexpr typename result<T, T>::type const& result<T, T>::operator*() const {
	return get_result();
}
template <typename T>
constexpr typename result<T, T>::type const* result<T, T>::operator->() const {
	return &get_result();
}

template <typename T>
constexpr result<T, void>::result(type&& t) : m_storage(std::move(t)) {
}
template <typename T>
constexpr result<T, void>::result(type const& t) : m_storage(t) {
}
template <typename T>
constexpr result<T, void>::result(std::nullptr_t) : result() {
}
template <typename T>
constexpr result<T, void>::operator bool() const noexcept {
	return has_result();
}
template <typename T>
constexpr bool result<T, void>::has_result() const noexcept {
	return m_storage.has_value();
}
template <typename T>
constexpr bool result<T, void>::has_error() const noexcept {
	return !has_result();
}
template <typename T>
constexpr typename result<T, void>::type const& result<T, void>::get_result() const {
	return m_storage.value();
}
template <typename T>
constexpr typename result<T, void>::type const& result<T, void>::value_or(type const& fallback) const {
	return has_result() ? get_result() : fallback;
}
template <typename T>
constexpr typename result<T, void>::type result<T, void>::move() {
	return m_storage.move();
}
template <typename T>
constexpr typename result<T, void>::type const& result<T, void>::operator*() const {
	return get_result();
}
template <typename T>
constexpr typename result<T, void>::type const* result<T, void>::operator->() const {
	return &get_result();
}
} // namespace kt
