#pragma once

#define _USE_MATH_DEFINES 1
#include <cmath>

namespace hwcad {

/**
 * @brief 数值类型特性。
 * @param T 类型参数，通常为浮点型。
*/
template <typename T>
struct Trait {
  /**
   * @brief 容限特性。默认为0。浮点型需要特化。
  */
  static constexpr T EPS = 0;
};

/**
 * @brief 单精度浮点类型特性。
*/
template <>
struct Trait<float> {
  /**
   * @brief 单精度容限。
  */
  static constexpr float EPS = 1e-4f;

  /**
   * @brief 最大尺寸
  */
  static constexpr float MAX_DIM = 1e4f;
};

/**
 * @brief 双精度浮点类型特性。
*/
template <>
struct Trait<double> {
  /**
   * @brief 双精度容限。
  */
  static constexpr double EPS = 1e-8;

  /**
   * @brief 最大尺寸
  */
  static constexpr double MAX_DIM = 1e4;

  /**
   * @brief 最小尺寸
  */
  static constexpr double MIN_DIM = 1e-9;
};

/**
 * @brief 长精度浮点类型特性。
*/
template <>
struct Trait<long double> {
  /**
   * @brief 长精度容限。
  */
  static constexpr long double EPS = 1e-12L; // FIXME: long double不一定更长。
};

/**
 * @brief 判断参数近似为0。
 * @tparam T 类型参数，应为浮点类型。
 * @param x 待测数值
 * @param eps 容限（非负）
 * @return 若x的绝对值不超过eps则返回true，否则返回false（包括NaN）。
*/
template <typename T>
constexpr bool is_zero(const T x, const T eps = Trait<T>::EPS) {
  return std::fabs(x) <= eps;
}

/**
 * @brief 判断参数非0。
 * @tparam T 类型参数，应为浮点类型。
 * @param x 待测数值
 * @param eps 容限（非负）
 * @return 与is_zero结果相反。对于NaN返回true。
*/
template <typename T>
constexpr bool nonzero(const T x, const T eps = Trait<T>::EPS) {
  return !is_zero(x, eps);
}

/**
 * @brief 符号函数。
 * @tparam T 类型参数
 * @param x 待测数值
 * @param eps 容限（非负）
 * @return 若|x|不超过eps时返回0, 若x大于eps返回1，其他情况（包括NaN）返回-1。
*/
template <typename T>
constexpr int sgn(const T x, const T eps = Trait<T>::EPS) {
  return is_zero(x, eps) ? 0 : (x > 0 ? 1 : -1);
}

/**
 * @brief 判断两浮点数近似相等。
 *
 * 算法是先计算(x - y）。如差为NaN，可能参数是NaN，也可能是无穷值相减。
 * 再用库函数isunordered判断两者是否无序（至少其一为NaN），无序返回false，否则直接比较两个无穷值。
 * 如果差不是NaN，返回is_zero(差)的判定结果。
 *
 * @param x 一个数
 * @param y 另一个数
 * @param eps 容限
 * @return 两数差异不超过容限时true，其他情况false。
 */
template <typename T>
constexpr bool equals(const T x, const T y, const T eps = Trait<T>::EPS) {
  const T d = x - y;
  if (std::isnan(d)) { // x,y 可能NaNa，也可能(inf - inf)之类。
    if (std::isunordered(x, y)) { // 其中之一是NaN，无法比较
      return false;
    } else { // 正、负无穷，可以直接比较
      return (x == y);
    }
  } else {
    return is_zero(d, eps);
  }
}

/**
 * @brief 比较两浮点数大小。
 * @param x 一个数
 * @param y 另一个数
 * @param eps 容限
 * @return 若两数之差的绝对值不超过eps返回0, 若差大于eps返回1，其他情况（包括NaN）返回-1。
 */
template <typename T>
constexpr int compare(const T x, const T y, T eps = Trait<T>::EPS) {
  const T d = x - y;
  if (std::isnan(d)) { // x,y 可能NaNa，也可能(inf - inf)之类。
    if (std::isunordered(x, y)) { // 其中之一是NaN，无法比较，处理为-1。
      return -1;
    } else {  // (inf - inf), (-inf + inf)导致NaN。
      return (x == y) ? 0 : (x < y ? -1 : 1); // 正、负无穷，可以直接比较
    }
  } else {
    return sgn(x - y, eps);
  }
}

} // namespace hwcad
