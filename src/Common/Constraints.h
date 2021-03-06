// Copyright (c) 2010-present Bifrost Entertainment AS and Tommy Nguyen
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)

#ifndef COMMON_CONSTRAINTS_H_
#define COMMON_CONSTRAINTS_H_

#include <cstdint>
#include <type_traits>

template <typename T, typename U>
using EnableIfBaseOf = std::enable_if_t<std::is_base_of<T, U>::value>;

template <typename T, typename U = T>
using EnableIfFloatingPoint =
    std::enable_if_t<std::is_floating_point<T>::value, U>;

template <typename T, typename U = T>
using EnableIfIntegral = std::enable_if_t<std::is_integral<T>::value, U>;

template <typename T>
using Arithmetic = std::enable_if_t<std::is_arithmetic<T>::value>;

template <typename T>
using ByteType = std::enable_if_t<
    std::is_same<std::make_unsigned_t<std::decay_t<T>>, uint8_t>::value>;

template <typename T>
using FloatingPoint = EnableIfFloatingPoint<T>;

#endif
