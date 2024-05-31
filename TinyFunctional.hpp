#pragma once

#include <functional>
#include <optional>
#include <type_traits>

#ifdef PERFECT_CAPTURE_BREAKS_GCC
#   define FORWARD(VARIABLE) std::forward<decltype(VARIABLE)>(VARIABLE)
#   define CAPTURE_FORWARD(VARIABLE) detail::perfect_capture_t<decltype(VARIABLE)>{ FORWARD(VARIABLE) }
#else
#   define FORWARD(VARIABLE) VARIABLE
#   define CAPTURE_FORWARD(VARIABLE) VARIABLE
#endif

namespace f {

namespace detail {

// perfect_capture_t is taken from http://stackoverflow.com/a/31410880/2622629
template <class T> using 
perfect_capture_t =
    std::conditional_t<std::is_lvalue_reference<T>::value,
                       std::reference_wrapper<std::remove_reference_t<T>>, T>;

template< class, class = std::void_t<> >
struct needs_unapply : std::true_type { };

template< class T >
struct needs_unapply<T, std::void_t<decltype(std::declval<T>()())>> : std::false_type { };

template<typename T, typename... Ts>
struct largest_type {
  static const size_t size =
    sizeof(T) > largest_type<Ts...>::size ? sizeof(T) : largest_type<Ts...>::size;
};

}

/* Basic tag type
 */
struct error {};

struct bad_access : error {};

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
 */
/*
template <typename T, class E = void>
struct InlineStorage {
    constexpr void set(T&& t) {
        m_filled = true;
        new (static_cast<void *>(&m_storage)) T(t);
    } 
    constexpr T& get(void) const {
        //return *reinterpret_cast<T*>(static_cast<void*>(&m_storage));
        return *reinterpret_cast<T>(&m_storage);
    } 
    ~InlineStorage(void) {
        if (m_filled)
            get().~T();
    }
    bool filled(void) {
        return m_filled;
    }
private:
    std::aligned_storage_t<sizeof(T), alignof(T)> m_storage;
    bool m_filled;
};

template <typename T>
struct InlineStorage<T, std::enable_if_t<std::is_trivially_destructible_v<T>>> {
    constexpr void set(T&& t) {
        m_filled = true;
        new (static_cast<void *>(&m_storage)) T(t);
    } 
    constexpr T& get(void) const {
        //return *reinterpret_cast<T*>(static_cast<void*>(&m_storage));
        return *reinterpret_cast<T>(&m_storage);
    } 
    ~InlineStorage(void) {
        if (m_filled)
            get().~T();
    }
    bool filled(void) {
        return m_filled;
    }
private:
    std::aligned_storage_t<sizeof(T), alignof(T)> m_storage;
    bool m_filled;
};
*/

/* Partial Specialization to eliminate sometimes trivial destructors
   http://www.club.cc.cmu.edu/~ajo/disseminate/2017-02-15-Optional-From-Scratch.pdf
 */

/*
template <typename T>
struct InlineStorage<T> 
    : InlineStorage<T, std::enable_if_t<std::is_trivially_destructible_v<T>>> {

};

template <typename T>
struct InlineStorage<T, std::enable_if_t<std::is_trivially_destructible_v<T>>> {
    constexpr void set(T&& t) {
        m_filled = true;
        new (static_cast<void *>(&m_storage)) T(t);
    } 
    constexpr T& get(void) const {
        return *reinterpret_cast<T*>(static_cast<void*>(&m_storage));
    } 
    ~InlineStorage(void) = default;
private:
    std::aligned_storage_t<sizeof(T), alignof(T)> m_storage;
    bool m_filled;
};
*/

    /*
template <typename T>
class ISOptional {
public:
    using value_type = T;
    using value_reference = value_type&;
    using rvalue_reference = value_type&&;
    using transformer = std::function<ISOptional(ISOptional)>;

    constexpr explicit ISOptional(rvalue_reference v) {
        m_storage.set(std::move(v));
    };

    constexpr explicit ISOptional(void) {};

    constexpr bool has_value(void) {
        return m_storage.filled();
    } 

    constexpr value_reference get_value(void) {
        if (!has_value())
            throw bad_access();
        return m_storage.get();
    }

    constexpr explicit operator bool(void) {
        return has_value();
    }
    constexpr value_reference operator*(void)  { return get_value(); }
    constexpr value_reference operator->(void) { return get_value(); }
    constexpr value_reference get_value_or(rvalue_reference other) {
        if (!has_value())
            return other;
        return m_storage.get();
    }
    constexpr ISOptional& when(transformer f) {
        
        if (m_storage.filled())
            raw() = f(raw());
        return this;
    } 

    constexpr ISOptional& unless(transformer f) {
        if (!m_storage.filled())
            raw() = f(raw());
        return this;
    } 
private:
    InlineStorage<T> m_storage;
};
    */


/* Storage of an templated type.
 *
 */
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
     */
    constexpr void operator=(T&& rhs) {
        reset();
        m_storage = Storage(std::move(rhs));
    }

    /**
     * @brief Engaged Assignment Operator.
     *
     * @param rhs the lvalue to assign from as an lvalue reference
     * Copying operator that creates an optional containing a copy of rhs.
     * @todo implement some sort of SFINAE template rules
     * @todo use U templating like above
     */
    constexpr void operator=(const T& rhs) {
        reset();
        m_storage = Storage(rhs);
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
     * @brief Accessor.
     * @throw bad_access if optional is disengaged
     * @see class bad_access
     */
    constexpr value_reference get_value(void) & {
        if (!has_value()) throw bad_access();
        return m_storage.value;
    }

    /**
     * @brief Accessor.
     * @throw bad_access if optional is disengaged
     * @see class bad_access
     */
    constexpr rvalue_reference get_value(void) && {
        if (!has_value()) throw bad_access();
        return std::move(m_storage.value);
    }

    /**
     * @brief Accessor.
     * @throw bad_access if optional is disengaged
     * @see class bad_access
     */
    constexpr value_reference get_value(void) const& {
        if (!has_value()) throw bad_access();
        return m_storage.value;
    }

    /**
     * @brief Accessor.
     * @throw bad_access if optional is disengaged
     * @see class bad_access
     */
    constexpr rvalue_reference get_value(void) const&& {
        if (!has_value()) throw bad_access();
        return std::move(m_storage.value);
    }

    /**
     * @brief Accessor.
     */
    constexpr value_reference operator*(void) & noexcept {
        return m_storage.value;
    }

    /**
     * @brief Accessor.
     */
    constexpr rvalue_reference operator*(void) && noexcept {
        return std::move(m_storage.value);
    }

    /**
     * @brief Accessor.
     */
    constexpr value_reference operator*(void) const& noexcept {
        return m_storage.value;
    }

    /**
     * @brief Accessor.
     */
    constexpr rvalue_reference operator*(void) const&& noexcept {
        return std::move(m_storage.value);
    }

    /**
     * @brief Accessor.
     */
    constexpr value_reference operator->(void) noexcept {
        return m_storage.value;
    }

private:
    Storage<T> m_storage; ///< maybe-trivial destructor invocated storage
};

template <typename T, typename... Args>
constexpr auto make_optional(Args&&... args) {
    return Optional<T>{T{std::forward<Args>(args)...}};
}

template <typename T>
class NotOptional {
    //BasicStorage<T> storage;
public:
    static_assert(!std::is_reference<T>::value, "f::NotOptional does not work with reference types");

    using value_type = T;
    using value_reference = T&;
    using rvalue_reference = T&&;

    union {
        nullvalue_t null;
        T m_value;
    };
    bool m_has_value = false;

    explicit constexpr NotOptional(void) noexcept : m_has_value(false) {};
    explicit constexpr NotOptional(nullvalue_t) noexcept : m_has_value(false) {};

    template <class U>
    explicit constexpr NotOptional(U&& v) : m_value(std::forward<U>(v)), m_has_value(true) {}

    template <class U>
    explicit constexpr NotOptional(const U& v) : m_value(std::forward<U>(v)), m_has_value(true) {}

    //template <class U, std::enable_if<std::is_trivially_copy_assignable<U>::value>>
    constexpr NotOptional& operator=(const NotOptional &rhs) {
        reset();
        if (rhs.has_value())
            m_value = *rhs;
        return *this;
    }
    constexpr NotOptional& operator=(NotOptional&& rhs) {
        reset();
        if (rhs.has_value())
            m_value = rhs.get_value();
        return *this;
    }



    constexpr void reset(void) noexcept {
        if (m_has_value) m_value.T::~T();
        m_has_value = false;
    }

    constexpr ~NotOptional(void) {
        reset();
    };

    constexpr bool has_value(void) const noexcept {
        return m_has_value;
    }

    constexpr explicit operator bool(void) const noexcept {
        return has_value();
    }

    constexpr value_reference get_value(void) & {
        if (!has_value())
            throw bad_access();
        return m_value;
    }
    constexpr rvalue_reference get_value(void) && {
        if (!has_value())
            throw bad_access();
        return m_value;
    }



    constexpr value_reference operator*(void) noexcept {
        return m_value;
    }

    constexpr value_reference operator->(void) noexcept {
        return m_value;
    }
};

/* 'OneOf' models the discriminated union of 'type1' OR 'type2'. 
 *
 * OneOf is to std::variant what std::pair is to std::tuple.
 * https://ojdip.net/2013/10/implementing-a-variant-type-in-cpp/
 */
template <typename A, typename B>
class OneOf {
public:
    using value1_type = A;
    using value1_reference = value1_type&;
    using value2_type = B;
    using value2_reference = value2_type&;

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
            throw bad_access();
        return *reinterpret_cast<value1_type*>(static_cast<void*>(&m_storage));
    }
    constexpr value2_reference get_value2(void) {
        if (m_is_value1)
            throw bad_access();
        return *reinterpret_cast<value2_type*>(static_cast<void*>(&m_storage));
    }

private:
  std::aligned_storage_t <
      std::max(sizeof(A), sizeof(B)),
      std::max(alignof(A), alignof(B))> m_storage;
    bool m_is_value1;
};


/* F([A]) -> [B]
 *
 * 'LazyTransformation' models the transition from collection [A] to
 * collection [B] given the transformer F;
 *
 * LazyTransformation can be used as an transformation of data,
 * with the benefit of being able to transform between
 * different types of collections.
 * The transformation is considered Lazy as the the transform is only
 * handled when assigned to a typed collection value.
 */
template<typename A, typename F>
class LazyTransformation {
public:
    using collection_type = A;

    LazyTransformation(A const& in, F&& f) : in(in), f(std::move(f)) { }

    template <typename B>
    constexpr B get() const {
        B out;
        for_each([&](auto e) {out.push_back(f(e)); }, in);
        return out;
    }

    template <typename B>
    operator B() const { return get<B>(); }

    template<typename B> 
    constexpr B operator*(void) const { return get(); }

private:
    A const& in;
    F f;
};

/* 'strip' removing the monadic wrapper from its value.
 */
template <typename T>
[[nodiscard]]
constexpr T inline strip(std::optional<T> opt) {
    if (opt) return *opt;
    return {};
}

template <typename T>
[[nodiscard]]
constexpr inline T strip(Lazy<T> opt) {
    if (opt) return *opt;
    return {};
}

/* F(x) G(y) -> F(G(Y))
 *
 * compose models a joined transformation of input given transformers
 * joined together.
 */
/*
template <typename F>
auto compose(F f) {
    return [=] (auto x) { return f(x); };
}

template <typename F, typename... Fs>
auto compose(F&& f, Fs&&... fs) {
    //return [=] (auto x) { return f(compose(fs...)(x)); };
    return [=] (auto x) { return std::invoke(f, (compose(std::forward(fs)...))(x)); };
}
*/

template <class F, class... Fs>
auto compose(F&& f, Fs&& ...fs)
{
    return [ f = std::forward(f), ... fs = std::forward(fs) ]
        <class... Xs>(Xs&& ...xs) mutable {
        return compose(std::forward(fs)...)(std::invoke(std::forward(f), std::forward(xs)...));
    };
}

/* for_each collection traversal.
 *
 * Linearly evaluate function F on all values in a collection.
 */
template <typename F, typename It>
constexpr void for_each_iterator(F&& f, It begin, It end) {
    for (auto it = begin; it != end; it++)
        f(*it);
}

template <typename F, typename Arr>
constexpr inline void for_each(F f, Arr& arr) {
    for_each_iterator(f, arr.begin(), arr.end());
}


/* foldl(F, V, C) -> F(F(F(F(V, c0), c1), c2), ...cN)
 *
 * fold expressions models the compression of a collection into a single value,
 * given an initial value and a transformation function.
 *
 * One can see the fold expression as a recursive call of F given an
 * intermediate value and each value of the collection.
 * Thus a fold expression can be used to create concrete compressions
 * such as accumulate:
 *
 * a = [1 2 3]
 * sum = foldl(+, 0, a) -> (0 + (1 + (2 + (3))))
 * 
 */
template <typename V, typename F, typename It>
[[nodiscard]]
constexpr auto fold_iterator(F f, V init, It begin, It end) -> V {
    if (begin >= end)
        return init; 
    return fold_iterator(f, f(init, *begin), begin+1, end);
}

template <typename V, typename F, typename Arr>
[[nodiscard]]
constexpr auto foldl(F f, const V init, Arr arr) -> V {
    if (arr.begin() >= arr.end())
        return init; 
    return fold_iterator(f, f(init, *arr.begin()), arr.begin()+1, arr.end());
}

template <typename V, typename F, typename Arr>
[[nodiscard]]
constexpr auto foldr(F f, const V init, Arr arr) -> V {
    if (rbegin(arr) >= rend(arr))
        return init; 
    return fold_iterator(f, f(init, *arr.rbegin()), arr.rbegin()+1, arr.rend());
}


/* F(A) -> B
 *
 * fmap models the transformation of inputs given a transformer function.
 * Given valid input arguments, transform inputs to valid return optional,
 * otherwise return invalid optional.
 */
template <typename F, typename... TOpts>
constexpr auto
fmap(F &&f, TOpts&&... opts)
    -> std::optional<decltype(std::invoke(std::forward<F>(f),
                                          *std::forward<TOpts>(opts)...))> {
    if ((... && opts))
        return std::invoke(std::forward<F>(f), *std::forward<TOpts>(opts)...);
    return {};
}

/* F([A]) -> [B]
 *
 * fmap models the transformation of input collection given a transformer
 * function into an output collection.
 */
template<typename F, typename C>
auto fmap(F&& f, C const& in) -> LazyTransformation<C, F> {
    return LazyTransformation<C, F>(in, f);
}


/* f(a, b, c, ...) -> f(a)(b)(c)...
 *
 * Currying is the principle of partial-application of functions
 * and converts a function of N arguments, into N partial functions.
 * This type of function manipulation is used as an alternative to bind,
 * allowing the binding of input values to the curried function, creating
 * polymorphed intermediate states.
 *
 * mul_times(n, v) {
 *     r = 1
 *     for (i = 1..n)
 *         r *=v
 *     return r
 * }
 *  pow = curry(mul_times)(2)
 *  cube = curry(mul_times)(3)
 *
 * Now, it is possible to use the partially-applied functions with their
 * pre-determined initial values.
 */
template <typename F>
decltype(auto) curry(F&& f) {
    if constexpr (detail::needs_unapply<decltype(f)>::value) {
        return [f=CAPTURE_FORWARD(f)](auto&& x) {
            return curry(
              [x=CAPTURE_FORWARD(x), f=CAPTURE_FORWARD(f)](auto&&...xs) -> decltype(f(FORWARD(x),FORWARD(xs)...)) {
                return f(FORWARD(x),FORWARD(xs)...);
              }
            );
        };
    }
    else return f();
}

}


namespace fp {
// Fluent C++ Blog, Jonathan Boccara
//   Pipes: How plumbing can make your cpp code more expressive
//   https://github.com/joboccara/pipes

// David Sankel Functional Design Explained
// https://github.com/graninas/cpp_functional_programming
// https://geo-ant.github.io/blog/2020/optional-pipe-syntax-part-3/
// https://pfultz2.com/blog/2014/09/05/pipable-functions/

/*
template <typename T>
using Sink = std::function<void(const T&)>;

template <typename T>
using Source = std::function<void(Sink<T>)>;

template <typename T>
void connect(Source<T> so, Sink<T> si) {
    so(si);
}

template <typename A, typename B>
using Transform = std::function<void(Sink<B>, A)>;

template <typename A, typename B>
void apply_to_sink(Transform<A, B> tf, Sink<A> si) {
    tf(si);
}

template <typename A, typename B>
void apply_to_source(Transform<A, B> tf, Source<A> so) {
    tf(so);
}
template <typename A, typename B>
void operator>>(Transform<A, B> tf, Source<A> so) {
    apply_to_source(tf, so);
}

template <typename A, typename B>
void operator>>(Transform<A, B> tf, Sink<A> si) {
    apply_to_sink(tf, si);
}

template <typename T>
void operator>>(Source<T> so, Sink<T> si) {
    connect(so, si);
}

struct pipe_base {};

template <typename F>
class TransformPipe : public pipe_base {
public:
    explicit TransformPipe(F f) : f(f) {};

    template <typename... V, typename P>
    void onRecieve(V&&... v, P&& next) {
        send(std::invoke(f, std::forward(v)...), next);
    }

private:
    F f;
};

template <typename F>
TransformPipe<F> transform(F f) {
    return TransformPipe<F>(f);
}

template <class F>
struct PipeClosure : F {
    template <class... Xs>
    PipeClosure(Xs&&... xs) : F(std::forward(xs)...) {}
};

template <typename T, typename F>
decltype(auto) operator>>=(T&& c, const PipeClosure<F> p) {
    return std::invoke(p, std::forward(p));
}

template <class F>
auto MakePipeClosure(F&& f) {
    return PipeClosure<F>(std::move(f));
}


template <class F>
struct Pipalbe {
    template <class... Xs>
    auto operator()(Xs&&... xs) {
        return MakePipeClosure([=](auto x) -> decltype(auto) {
            return F()(x, xs...);});
    }
};


template <typename C, typename P>
C operator>>(C&& c, P&& p) {
    using std::begin;
    using std::end;
    std::copy(
        std::make_move_iterator(begin(c), std::make_move_iterator(end(c))),
        p);
}
*/

}
