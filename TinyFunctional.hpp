#pragma once

#include <functional>
#include <optional>

namespace f {

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
    using reference_type = value_type&;
    
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

    constexpr reference_type get(void) {
        if (!evaluated) {
            evaluated = true;
            new(static_cast<void*>(&b)) value_type(f());
        }
        return *reinterpret_cast<value_type*>(static_cast<void*>(&b));
    }

    constexpr reference_type operator*(void)  { return get(); }
    constexpr reference_type operator->(void) { return get(); }

private:
    std::function<value_type(void)> f;
    std::aligned_storage_t<sizeof(value_type), alignof(value_type)> b;
    bool evaluated;
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

template <class F>
auto compose(F&& arg) {
    return std::forward(arg);
}

template <class F, class... Fs>
auto compose(F&& arg, Fs&& ...args)
{
    return [ fun = std::forward(arg), ... functions = std::forward(args) ]
        <class... Xs>(Xs&& ...xs) mutable {
        return compose(std::forward(functions)...)(std::invoke(std::forward(fun), std::forward(xs)...));
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
#ifdef PERFECT_CAPTURE_BREAKS_GCC
#   define FORWARD(VARIABLE) std::forward<decltype(VARIABLE)>(VARIABLE)
#   define CAPTURE_FORWARD(VARIABLE) detail::perfect_capture_t<decltype(VARIABLE)>{ FORWARD(VARIABLE) }
#else
#   define FORWARD(VARIABLE) VARIABLE
#   define CAPTURE_FORWARD(VARIABLE) VARIABLE
#endif

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
}

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
