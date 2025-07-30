#pragma once

#include "pcb/segment.hpp"
#include "verify.hpp"
#include <utility>

namespace hwpcb {

/**
 * @brief 迹。单独的一个圆，或是若干连续的线段和弧段。
 * 圆形/线段/弧段统计数量，比例约为40:30:1，如此看来圆形若持有vector<Seg>过于浪费。
 */
class Trail {
  /**
   * @brief 直接持有一个片段。圆形最常见，直接存储。
   */
  Seg m_seg;

  /**
   * @brief 片段个数多于一个则动态分配。
   */
  Seg* m_arr;

  /**
   * @brief 片段数量。
   */
  uint m_size;

public:
  /**
   * @brief 默认构造函数。
   */
  Trail() : m_seg(), m_arr(), m_size(1) {}

  /**
   * @brief 拷贝构造函数
   */
  Trail(const Trail& that) : m_seg(that.m_seg), m_arr(), m_size(that.m_size) {
    if (that.m_arr) {
      m_arr = new Seg[m_size];
      for (uint i = 0; i < m_size; ++i) {
        m_arr[i] = that.m_arr[i];
      }
    }
  }

  /**
   * @brief 移动构造函数
   */
  Trail(Trail&& that) noexcept
      : m_seg(that.m_seg), m_arr(that.m_arr), m_size(that.m_size) {
    that.m_size = 1;
    that.m_arr = nullptr;
  }

  /**
   * @brief 析构函数
   */
  ~Trail() {
    if (m_arr) {
      delete[] m_arr; // 可以处理nullptr，但自己判断快一点。
    }
  }

  /**
   * @brief 交换函数
   */
  inline void swap(Trail& that) noexcept {
    std::swap(m_size, that.m_size);
    std::swap(m_seg, that.m_seg);
    std::swap(m_arr, that.m_arr);
  }

  /**
   * @brief 拷贝赋值算符
   */
  Trail& operator=(const Trail& that) {
    if (this != &that) {
      if (that.m_size == 1) {
        if (m_size != 1) {
          delete[] m_arr;
          m_arr = nullptr;
          m_size = 1;
        }
        m_seg = that.m_seg;
      } else {
#if 1
        Trail(that).swap(*this); // 这样既快且安全
#else
        if (m_size < that.m_size) {
          delete[] m_arr;
          m_arr = new Seg[that.m_size];
        }
        for (uint i = 0; i < that.m_size; ++i) {
          m_arr[i] = that.m_arr[i];
        }
        m_size = that.m_size;
#endif
      }
    }
    return *this;
  }

  /**
   * @brief 移动赋值算符
   */
  Trail& operator=(Trail&& that) noexcept {
    if (this != &that) {
      if (m_size != 1) { // 已有动态分配的片段，先释放。
        delete[] m_arr;
        m_arr = nullptr;
        m_size = 1;
      }
      if (that.m_size == 1) {
        m_seg = that.m_seg;
      } else {
        m_size = that.m_size;
        m_arr = that.m_arr;
        that.m_size = 1;
        that.m_arr = nullptr;
      }
    }
    return *this;
  }

  /**
   * @brief 设为片段列表。
   */
  void resize(uint nodeNum) {
    const uint newSize = (nodeNum > 1 ? nodeNum - 1 : 1);
    if (newSize != m_size) {
      if (m_size != 1) { // 已有动态分配的片段，先释放。
        delete[] m_arr;
      }
      m_size = newSize;
      m_arr = new Seg[m_size];
    }
  }

  /**
   * @brief 轨迹是否为一个圆。
   */
  bool isCircle() const { return m_size == 1 && m_seg.isCircle(); }

  /**
   * @brief 获取片段数量。
   */
  uint size() const { return m_size; }

  /**
   * @brief 引用片段。
   */
  const Seg& seg(uint i) const {
    if (i == 0 && m_size == 1) {
      return m_seg;
    } else {
      verify(i < m_size);
      return m_arr[i];
    }
  }

  /**
   * @brief 引用片段。
   */
  Seg& seg(uint i) {
    if (i == 0 && m_size == 1) {
      return m_seg;
    } else {
      verify(i < m_size);
      return m_arr[i];
    }
  }
};

} // namespace hwpcb
