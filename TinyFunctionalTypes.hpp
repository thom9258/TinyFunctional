#pragma once

#include <string>
#include <functional>
#include <type_traits>

namespace f {

/* Basic tag type
 */
/**
 * @brief Base error type.
 * Can be used as a generic thown type on error, or be derived to
 * create more explicit errors.
 */
class Error {
public:
    Error() = default;
    Error(std::string&& what) : m_what(what) {};
    const std::string& what() { return m_what; };
private:
    std::string m_what{""};
};

class BadAccess : Error {};

/* nullvalue_t is a tag to have compile-time invalid types.
 * nullvalue_t defines an explicit constructor to ensure that automatic
 * template deduction does not interpret {} as a nullvalue_t when used
 * for construction.
 */
struct nullvalue_t {
    explicit constexpr nullvalue_t(int) {};
};
/* we define a typed invalid value called nullvalue, this serves the same
 * purpose as nullptr.
 */
constexpr nullvalue_t nullvalue{0};

/**
 * @brief Storage of a single value.
 * This storage template is instansiated for non-trivial types and enforces
 * explicit destruction.
 * Storage can be seen as a vector of size 1.
 * @see class Storage
 */
template <class T, class E = void>
struct Storage {
    ~Storage(void) { reset(); }
    union { nullvalue_t null; T value; };
    bool has_value = false;
    constexpr Storage(void) noexcept {};
    template <class U>
    constexpr Storage(const U& v) noexcept : value(v), has_value(true) {};
    template <class U>
    constexpr Storage(U &&v) noexcept : value(v), has_value(true) {};
    constexpr void reset(void) {
        if (has_value) value.T::~T();
        has_value = false;
    }
};

/**
 * @brief Storage of a single value.
 * This storage template is instansiated for trivial types and omits explicit
 * destruction.
 * Storage can be seen as a vector of size 1.
 * @see class Storage
 */
template <class T>
struct Storage<T, std::enable_if_t<std::is_trivially_destructible_v<T>>> {
    ~Storage(void) = default;
    union { nullvalue_t null; T value; };
    bool has_value = false;
    constexpr Storage(void) noexcept {};
    template <class U>
    constexpr Storage(const U& v) noexcept : value(v), has_value(true) {};
    template <class U>
    constexpr Storage(U &&v) noexcept : value(v), has_value(true) {};
    constexpr void reset(void) {
        //if (has_value) value.T::~T();
        has_value = false;
    }
};


template <typename T>
class Optional {
public:
    using value_type = T;
    using value_reference = T&;
    using rvalue_reference = T&&;

    static_assert(!std::is_reference_v<value_type>,
                  "Creating f::Optional of a reference type is ill-formed");
    static_assert(!std::is_same_v<value_type, nullvalue_t>,
                  "Creating f::Optional of type 'nullvalue_t' is ill-formed");

    /**
     * @brief Destructor.
     *
     * Destructor is maybe-trivial due to the chosen overloaded member of Storage.
     *
     * @see class Storage
     */
    ~Optional(void) noexcept = default;

    /**
     * @brief Default Constructor.
     *
     * Default constructor that creates an empty optional.
     */
    explicit constexpr Optional(void) noexcept {};

    /**
     * @brief Disengaged Constructor.
     *
     * Optional Default constructor that creates an empty optional by using
     * explicit nullvalue as input.
     * @see nullvalue_t
     */
    explicit constexpr Optional(nullvalue_t) noexcept {};

    /**
     * @brief Engaged Constructor.
     *
     * @param u the value to store captured as an rvalue reference
     * Constructor that creates an optional containing the value u.
     * @todo implement some sort of SFINAE template rules
     */
    //template <class U, std::enable_if<std::is_convertible_v<std::decay_t<U>, T>>>
    //template <class U, std::enable_if<std::is_copy_constructible_v<U>>>
    template<class U>
    constexpr Optional(U&& u) : m_storage(std::forward<U>(u)) {}

    /**
     * @brief Engaged Constructor.
     *
     * @param u the value to store captured as an lvalue reference
     * Constructor that creates an optional containing the value u.
     * @todo implement some sort of SFINAE template rules
     */
    //template <class U, std::enable_if<std::is_convertible_v<std::decay_t<U>, T>>>
    //template <class U, std::enable_if<std::is_copy_constructible_v<U>>>
    template <class U>
    constexpr Optional(const U& v) : Optional(std::move(v)) {}

    /**
     * @brief Engaged Constructor.
     *
     * @param opt the value to copy construct from captured as an lvalue reference
     * Constructor that creates an optional containing the value inside opt.
     * @todo implement some sort of SFINAE template rules
     */
    //template <class U, std::enable_if<std::is_copy_constructible_v<U>
    //                                  && std::is_convertible_v<std::decay_t<U>, T>>>
    template <class U>
    constexpr Optional(const Optional<U>& opt) {
        reset();
        *this = opt.get_value();
    }

    /**
     * @brief Engaged Constructor.
     *
     * @param opt the value to copy construct from captured as an rvalue reference
     * Constructor that creates an optional containing the value inside opt.
     * @todo implement some sort of SFINAE template rules
     */
    //template <class U, std::enable_if<std::is_copy_constructible_v<U>
    //                                  && std::is_convertible_v<std::decay_t<U>, T>>>
    template <class U>
    constexpr Optional(Optional<U>&& opt) {
        reset();
        *this = opt.get_value();
    }

    /**
     * @brief Engaged Assignment Operator.
     *
     * @param rhs the lvalue to assign from as an lvalue reference
     * Copying operator that creates an optional containing rhs.
     * @todo implement some sort of SFINAE template rules
     * @todo use U templating like above
     * @todo implement assignment to class Storage directly instead
     */
    constexpr void operator=(T&& rhs) {
        reset();
        m_storage.value = rhs;
        m_storage.has_value = true;
    }

    /**
     * @brief Engaged Assignment Operator.
     *
     * @param rhs the lvalue to assign from as an lvalue reference
     * Copying operator that creates an optional containing a copy of rhs.
     * @todo implement some sort of SFINAE template rules
     * @todo use U templating like above
     * @todo implement assignment to class Storage directly instead
     */
    constexpr void operator=(const T& rhs) {
        reset();
        m_storage.value = rhs;
        m_storage.has_value = true;
    }

    /**
     * @brief Engaged Assignment Operator.
     *
     * @param rhs the optional lvalue to assign from as an lvalue reference
     * Copying operator that creates an optional containing a copy of the optional rhs.
     * @todo implement some sort of SFINAE template rules
     */
    template <class U>
    constexpr void operator=(const Optional<U>& rhs) {
        reset();
        if (rhs.has_value())
            m_storage = rhs.m_storage;
    }

    /**
     * @brief Engaged Assignment Operator.
     *
     * @param rhs the optional rvalue to capture from as an rvalue reference
     * Copying operator that creates an optional containing a the optional rhs.
     * @todo implement some sort of SFINAE template rules
     */
    //template <class U, std::enable_if<std::is_copy_assignable_v<U>
    //                                  && std::is_convertible_v<std::decay_t<U>, T>>>
    template <class U>
    constexpr void operator=(Optional<U>&& rhs) {
        reset();
        if (rhs.has_value())
            m_storage = std::move(rhs.m_storage);
    }

    /**
     * @brief Disengaged Assignment Operator.
     *
     * @param nullvalue
     * Explicit disengage assignment using nullvalue type.
     * Results in disengaged optional.
     * @see nullvalue_t
     */
    constexpr void operator=(nullvalue_t) noexcept {
        reset();
    }

    /**
     * @brief Emplacement Construction
     *
     * @param args... the variadic parameter pack rvalues
     * The provided parameter pack is forward expanded and passed
     * to make_optional to construct T, the created optional is then copy
     * assigned to this.
     * 
     * @todo implement some sort of SFINAE template rules
     * @see operator=()
     * @see make_optional()
     */
    template <typename... Args>
    constexpr void emplace(Args&&... args) {
        reset();
        *this = make_optional<T>(std::forward<Args>(args)...);
    }

    /**
     * @brief Disengage.
     *
     * Explicit disengage function.
     * @see class Storage
     */
    constexpr void reset(void) {
        m_storage.reset();
    }

    /**
     * @brief Check if a optional is engaged or disengaged.
     * @see class Storage
     */
    constexpr bool has_value(void) const noexcept {
        return m_storage.has_value;
    }

    /**
     * @brief boolean conversion operator.
     * Check if a optional is engaged or disengaged through conversion.
     * @see class Storage
     */
    constexpr explicit operator bool(void) const noexcept {
        return has_value();
    }

    /**
     * @brief  Maybe-throwing accessor.
     * @throw BadAccess if optional is disengaged
     * @see class BadAccess
     */
    constexpr value_reference get_value(void) & {
        if (!has_value()) throw BadAccess();
        return m_storage.value;
    }

    /**
     * @brief  Maybe-throwing accessor.
     * @throw BadAccess if optional is disengaged
     * @see class BadAccess
     */
    constexpr rvalue_reference get_value(void) && {
        if (!has_value()) throw BadAccess();
        return std::move(m_storage.value);
    }

    /**
     * @brief  Maybe-throwing accessor.
     * @throw BadAccess if optional is disengaged
     * @see class BadAccess
     */
    constexpr value_reference get_value(void) const& {
        if (!has_value()) throw BadAccess();
        return m_storage.value;
    }

    /**
     * @brief  Maybe-throwing accessor.
     * @throw BadAccess if optional is disengaged
     * @see class BadAccess
     */
    constexpr rvalue_reference get_value(void) const&& {
        if (!has_value()) throw BadAccess();
        return std::move(m_storage.value);
    }

    /**
     * @brief  Non-throwing accessor.
     */
    constexpr value_reference operator*(void) & noexcept {
        return m_storage.value;
    }

    /**
     * @brief  Non-throwing accessor.
     */
    constexpr rvalue_reference operator*(void) && noexcept {
        return std::move(m_storage.value);
    }

    /**
     * @brief  Non-throwing accessor.
     */
    constexpr value_reference operator*(void) const& noexcept {
        return m_storage.value;
    }

    /**
     * @brief  Non-throwing accessor.
     */
    constexpr rvalue_reference operator*(void) const&& noexcept {
        return std::move(m_storage.value);
    }

    /**
     * @brief  Non-throwing accessor.
     */
    constexpr value_reference operator->(void) noexcept {
        return m_storage.value;
    }

    template <class F>
    constexpr auto and_then(F&& f) & {
        return bool(*this) ? std::invoke(std::forward<F>(f), **this)
                           : std::remove_cvref_t<std::invoke_result_t<F, T&>>{};
    }

    template <class F>
    constexpr auto and_then(F&& f) const& {
        return bool(*this) ? std::invoke(std::forward<F>(f), **this)
                           : std::remove_cvref_t<std::invoke_result_t<F, const T&>>{};
    }

    template <class F>
    constexpr auto and_then(F&& f) && {
        return bool(*this) ? std::invoke(std::forward<F>(f), std::move(**this))
                           : std::remove_cvref_t<std::invoke_result_t<F, T>>{};
    }

    template <class F>
    constexpr auto and_then(F&& f) const&& {
        return bool(*this) ? std::invoke(std::forward<F>(f), std::move(**this))
                           : std::remove_cvref_t<std::invoke_result_t<F, const T>>{};
    }

    template <class F>
    constexpr auto or_else(F&& f) const& {
        return bool(*this) ? *this : std::forward<F>(f)();
    }

    template <class F>
    constexpr auto or_else(F&& f) && {
        return bool(*this) ? std::move(*this) : std::forward<F>(f)();
    }

    template <class U>
    constexpr T get_value_or( U&& other) const& {
        return bool(*this) ? m_storage.value : static_cast<T>(std::forward<U>(other));
    }

    template <class U>
    constexpr T get_value_or( U&& other) && {
        return bool(*this) ? std::move(m_storage.value) : static_cast<T>(std::forward<U>(other));
    }


private:
    Storage<T> m_storage; ///< maybe-trivial destructor invocated storage
};


/**
 * @brief  Optional creator for non-trivial construction 
 * @see class Optional
 */
template <typename T, typename... Args>
constexpr auto make_optional(Args&&... args) {
    return Optional<T>{T{std::forward<Args>(args)...}};
}
    

/* F(A...) -> B
 *
 * 'Lazy' models the evalutation from a function given arguments to a value;
 *
 * Lazy is an lazily evaluated return value of a functor.
 * Lazy imitates the interface of an optional, but modified so the value is
 * calculated and stored on first value-retrieval operation.
 * Lazy is considered a code flow optimization used for costly calculations.
 */
template <typename T>
class Lazy {
public:
    using value_type = T;
    using value_reference = value_type&;
    
    template <typename F, typename... As>
    constexpr explicit Lazy(F &&f, As &&...args)
        : f([&](void) -> value_type {
            return std::invoke(std::forward<F>(f),
                               std::forward<As>(args)...);}), evaluated(false) {};

    ~Lazy(void) {
        if (evaluated)
            get().~value_type();
    }

    constexpr bool has_value(void) const {
        return true;
    } 
    constexpr explicit operator bool(void) const {
        return has_value();
    }
    constexpr bool has_evaluated(void) const {
        return evaluated;
    } 

    constexpr value_reference get(void) {
        if (!evaluated) {
            new(static_cast<void*>(&b)) value_type(f());
            evaluated = true;
        }
        return *reinterpret_cast<value_type*>(static_cast<void*>(&b));
    }

    constexpr value_reference operator*(void)  { return get(); }
    constexpr value_reference operator->(void) { return get(); }

private:
    std::function<value_type(void)> f;
    std::aligned_storage_t<sizeof(value_type), alignof(value_type)> b;
    bool evaluated;
};

/* 'OneOf' models the discriminated union of 'type1' OR 'type2'.
 *
 * OneOf is to std::variant what std::pair is to std::tuple.
 * https://ojdip.net/2013/10/implementing-a-variant-type-in-cpp/
 * TODO: needs to be implemented using a union, to get move semantics
 *       and what about maybe-trivial destructors? what if one is trivial
 *       and the other is not?
 */
template <typename A, typename B>
class OneOf {
public:
    using value1_type = A;
    using value1_reference = value1_type&;
    using rvalue1_reference = value1_type&;

    using value2_type = B;
    using value2_reference = value2_type&;
    using rvalue2_reference = value2_type&;

    constexpr explicit OneOf(value1_type &&a) : m_is_value1(true) {
        new (static_cast<void *>(&m_storage)) value1_type(a);
    };
    constexpr explicit OneOf(value2_type &&b) : m_is_value1(false) {
        new (static_cast<void *>(&m_storage)) value2_type(b);
    };

    ~OneOf(void) {
        if (m_is_value1)
            get_value1().~value1_type();
        else
            get_value2().~value2_type();
    }

    constexpr bool is_value1(void) const {
        return m_is_value1;
    } 
    constexpr bool is_value2(void) const {
        return !m_is_value1;
    } 

    constexpr value1_reference get_value1(void) {
        if (!m_is_value1)
            throw BadAccess();
        return *reinterpret_cast<value1_type*>(static_cast<void*>(&m_storage));
    }
    constexpr value2_reference get_value2(void) {
        if (m_is_value1)
            throw BadAccess();
        return *reinterpret_cast<value2_type*>(static_cast<void*>(&m_storage));
    }

private:
  std::aligned_storage_t <
      std::max(sizeof(A), sizeof(B)),
      std::max(alignof(A), alignof(B))> m_storage;
    bool m_is_value1;
};

}
