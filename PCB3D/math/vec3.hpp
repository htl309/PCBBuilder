#pragma once

#include "float.hpp"
#include <algorithm>
#include <stdexcept>
#include <string>

namespace hwcad {

/**
 * @brief 三维向量类，用于表示位置、方向等。设计为基础类型，不应被继承。
 * @tparam T 类型参数通常为浮点，以double居多。
*/
template <typename T>
class Vec3 {
public:
  /**
   * @brief 维数
  */
  static constexpr int DIM = 3;

  /**
   * @brief 零向量
  */
  static const Vec3 zero;

  /**
   * @brief X轴正向单位向量
  */
  static const Vec3 UX;

  /**
   * @brief Y轴正向单位向量
  */
  static const Vec3 UY;

  /**
   * @brief Z轴正向单位向量
  */
  static const Vec3 UZ;

  /**
   * @brief X轴负向单位向量
  */
  static const Vec3 NX;

  /**
   * @brief Y轴负向单位向量
  */
  static const Vec3 NY;

  /**
   * @brief Z轴负向单位向量
  */
  static const Vec3 NZ;


  /**
   * @brief X分量。公有成员。
  */
  T x;

  /**
   * @brief Y分量。公有成员。
  */
  T y;

  /**
   * @brief Z分量。公有成员。
  */
  T z;

  /**
   * @brief 默认构造函数。分量全0。
  */
  Vec3() : x(), y(), z() {}

  /**
   * @brief 分量构造函数。
   * @param tx X分量初值
   * @param ty Y分量初值
   * @param tz Z分量初值
  */
  Vec3(const T tx, const T ty, const T tz) : x(tx), y(ty), z(tz) {}

  /**
   * @brief 等量构造函数。
   * @param t 分量初值
  */
  explicit Vec3(const T t) : x(t), y(t), z(t) {}

  /**
   * @brief 向量判等。如果设计为重载==算符则不易支持容限。
   * @param v 另一向量
   * @param eps 容限
   * @return 相等为true，不等为false。
  */
  bool equals(const Vec3& v, const T eps = Trait<T>::EPS) const {
    return hwcad::equals(x, v.x, eps) && hwcad::equals(y, v.y, eps) &&
      hwcad::equals(z, v.z, eps);
  }

  /**
   * @brief 索引获取分量。
   * @param i 索引（0~2）
   * @return 分量值
  */
  T operator[](int i) const {
    check_index(i);
    return (&x)[i];
  }

  /**
   * @brief 索引存取分量。
   * @param i 索引（0~2）
   * @return 分量的引用
  */
  T& operator[](int i) {
    check_index(i);
    return (&x)[i];
  }

  /**
   * @brief 设置向量的XYZ分量。
  */
  void set(T tx, T ty, T tz) {
    x = tx;
    y = ty;
    z = tz;
  }

  /**
   * @brief 向量自加
   * @param v 待加向量
   * @return 自加后的当前向量引用
  */
  Vec3& operator+=(const Vec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }

  /**
   * @brief 向量自减
   * @param v 待减向量
   * @return 自减后的当前向量引用
  */
  Vec3& operator-=(const Vec3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }

  /**
   * @brief 向量数乘
   * @param t 待乘的标量
   * @return 数乘后的当前向量引用
  */
  Vec3& operator*=(const T t) {
    x *= t;
    y *= t;
    z *= t;
    return *this;
  }

  /**
   * @brief 获取反向量。
   * @return 反向量。
  */
  Vec3 operator-() const { return Vec3(-x, -y, -z); }

  /**
   * @brief 向量取反。
  */
  void reverse() {
    x = -x;
    y = -y;
    z = -z;
  }

  /**
   * @brief 向量相加
   * @param v 待加向量
   * @return 向量和
  */
  Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }

  /**
   * @brief 向量相减
   * @param v 待减向量
   * @return 向量差
  */
  Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }

  /**
   * @brief 向量数乘
   * @param t 待乘标量
   * @return 数乘后的向量
  */
  Vec3 operator*(const T t) const { return Vec3(x * t, y * t, z * t); }

  /**
   * @brief 计算向量点积。数值上等于this在B方向上的投影长度乘以B的长度。可用于求夹角和投影。
   * @param v 另一向量
   * @return 点积值。两向量成锐角时大于0、钝角时小于0、直角时为0。
  */
  T dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }

  /**
   * @brief 计算叉积。从this到v遵循“右手定则”。交换运算顺序则结果相反。
   * @param v 另一向量
   * @return 叉积向量
  */
  Vec3 cross(const Vec3& v) const {
    return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }

  /**
   * @brief 计算平方范数。用于比较长度时无需开方。
   * @return 平方范数
  */
  T norm2() const { return sq(x) + sq(y) + sq(z); }

  /**
   * @brief 计算向量长度。
   * @return 长度值
  */
  T length() const { return std::sqrt(norm2()); }

  /**
   * @brief 向量归一化。this长度如果非零，则缩放为1。
   * @param eps 容限
  */
  void normalize(const T eps = Trait<T>::EPS) {
    if (const T L = length(); nonzero(L, eps)) {
      const T t = 1 / L;
      x *= t;
      y *= t;
      z *= t;
    }
  }

  /**
   * @brief 计算向量之间的夹角。
   * @param v 另一向量
   * @param eps 容限
   * @return 夹角弧度，值域为[0, pi]。若任一向量为0则结果也为0。
  */
  T angle(const Vec3& v, const T eps = Trait<T>::EPS) const {
    auto la = length();
    auto lb = v.length();
    if(is_zero(la, eps) || is_zero(lb, eps)) {
      return 0;
    } else {
      const T dp = this->dot(v) / la / lb;
      return std::acos(std::clamp(dp, T(-1), T(1)));
    }
  }

  /**
   * @brief 获取单位向量
   * @param eps 容限
   * @return 单位向量
  */
  Vec3 unit(const T eps = Trait<T>::EPS) const {
    if (const T L = length(); is_zero(L, eps)) {
      return {};
    }
    else {
      const T t = 1 / L;
      return Vec3{ x * t, y * t, z * t };
    }
  }

  /**
   * @brief 向量的仿射组合
   * @param v 另一向量
   * @param t 组合系数
   * @return 向量this与v的仿射组合。比如t=0时等于this，t=1时等于v。
  */
  Vec3 affine(const Vec3& v, const T t) const {
    // return *this + (v - *this) * t;
    return Vec3{ x + (v.x - x) * t, y + (v.y - y) * t, z + (v.z - z) * t };
  }

  /**
   * @brief 判断向量正交（点积为0）
   * @param v 另一向量
   * @param eps 容限
   * @return 正交返回true，其他情况false。
  */
  bool orthogonal(const Vec3& v, const T eps = Trait<T>::EPS) const {
    return is_zero(this->dot(v), eps);
  }

  /**
   * @brief 判断向量平行（重合，同向或反向），叉积模长为0。
   * @param v 另一向量
   * @param eps 容限
   * @return this与v平行时true，其他情况false。
  */
  bool parallel(const Vec3<T>& v, T eps = Trait<T>::EPS) const {
    return is_zero(this->cross(v).norm2(), eps); // FIXME: eps是否也要平方？
  }

private:
  /**
   * @brief 平方辅助函数
   *
   * 功能太简单，不想写成自由函数占据命名空间。
   * 
   * @param t 参数
   * @return 参数的平方
  */
  static constexpr T sq(const T t) { return t * t; }

  /**
   * @brief 检查索引范围。
   * @param i 索引值
  */
  static void check_index(int i) {
    if (!(i >= 0 && i < DIM)) {
      throw std::out_of_range("index out of range: expect [0, 3) actual " + std::to_string(i));
    }
  }
};

template <typename T> const Vec3<T> Vec3<T>::zero;
template <typename T> const Vec3<T> Vec3<T>::UX(1, 0, 0);
template <typename T> const Vec3<T> Vec3<T>::UY(0, 1, 0);
template <typename T> const Vec3<T> Vec3<T>::UZ(0, 0, 1);
template <typename T> const Vec3<T> Vec3<T>::NX = -UX;
template <typename T> const Vec3<T> Vec3<T>::NY = -UY;
template <typename T> const Vec3<T> Vec3<T>::NZ = -UZ;

using V3d = Vec3<double>;
using V3f = Vec3<float>;

}
