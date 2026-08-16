#pragma once

#include <qcompilerdetection.h> // for Q_REQUIRED_RESULT

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include <QObject>
#include <QStringList>

#include <memory>
#include <type_traits>

namespace ExtensionSystem {
namespace Utils {

//////////////////
// anyOf
/////////////////
template<typename T, typename F>
bool anyOf(const T &container, F predicate)
{
    return std::any_of(std::begin(container), std::end(container), predicate);
}

// anyOf taking a member function pointer
template<typename T, typename R, typename S>
bool anyOf(const T &container, R (S::*predicate)() const)
{
    return std::any_of(std::begin(container), std::end(container), std::mem_fn(predicate));
}

// anyOf taking a member pointer
template<typename T, typename R, typename S>
bool anyOf(const T &container, R S::*member)
{
    return std::any_of(std::begin(container), std::end(container), std::mem_fn(member));
}

//////////////////
// findOr
/////////////////
template<typename C, typename F>
Q_REQUIRED_RESULT
typename C::value_type findOr(const C &container, typename C::value_type other, F function)
{
    typename C::const_iterator begin = std::begin(container);
    typename C::const_iterator end = std::end(container);

    typename C::const_iterator it = std::find_if(begin, end, function);
    return it == end ? other : *it;
}

template<typename T, typename R, typename S>
Q_REQUIRED_RESULT
typename T::value_type findOr(const T &container, typename T::value_type other, R (S::*function)() const)
{
    return findOr(container, other, std::mem_fn(function));
}

template<typename T, typename R, typename S>
Q_REQUIRED_RESULT
typename T::value_type findOr(const T &container, typename T::value_type other, R S::*member)
{
    return findOr(container, other, std::mem_fn(member));
}

//////////////////
// findOrDefault
//////////////////
// Default implementation:
template<typename C, typename F>
Q_REQUIRED_RESULT
typename std::enable_if_t<std::is_copy_assignable<typename C::value_type>::value, typename C::value_type>
findOrDefault(const C &container, F function)
{
    return findOr(container, typename C::value_type(), function);
}

template<typename C, typename R, typename S>
Q_REQUIRED_RESULT
typename std::enable_if_t<std::is_copy_assignable<typename C::value_type>::value, typename C::value_type>
findOrDefault(const C &container, R (S::*function)() const)
{
    return findOr(container, typename C::value_type(), std::mem_fn(function));
}

template<typename C, typename R, typename S>
Q_REQUIRED_RESULT
typename std::enable_if_t<std::is_copy_assignable<typename C::value_type>::value, typename C::value_type>
findOrDefault(const C &container, R S::*member)
{
    return findOr(container, typename C::value_type(), std::mem_fn(member));
}

//////////////////
// sort
/////////////////
template <typename Container>
inline void sort(Container &container)
{
    std::sort(std::begin(container), std::end(container));
}

template <typename Container, typename Predicate>
inline void sort(Container &container, Predicate p)
{
    std::sort(std::begin(container), std::end(container), p);
}

// pointer to member
template <typename Container, typename R, typename S>
inline void sort(Container &container, R S::*member)
{
    auto f = std::mem_fn(member);
    using const_ref = typename Container::const_reference;
    std::sort(std::begin(container), std::end(container),
              [&f](const_ref a, const_ref b) {
        return f(a) < f(b);
    });
}

// pointer to member function
template <typename Container, typename R, typename S>
inline void sort(Container &container, R (S::*function)() const)
{
    auto f = std::mem_fn(function);
    using const_ref = typename Container::const_reference;
    std::sort(std::begin(container), std::end(container),
              [&f](const_ref a, const_ref b) {
        return f(a) < f(b);
    });
}

//////////////////
// reverseForeach
/////////////////
template <typename Container, typename Op>
inline void reverseForeach(const Container &c, const Op &operation)
{
    auto rend = c.rend();
    for (auto it = c.rbegin(); it != rend; ++it)
        operation(*it);
}

}
}