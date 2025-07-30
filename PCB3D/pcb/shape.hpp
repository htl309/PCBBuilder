#pragma once

#include "pcb/trail.hpp"
#include <vector>

namespace hwpcb {

/**
 * @brief
 * 二维填充封闭形状，对应ShapeWithVoid关键字，表示一个连通区域，内部可能有“空洞”。
 * 期望外框边界方向逆时针，空洞边界方向为顺时针。
 */
class Shape {
public:
  /**
   * @brief 构造函数。
   */
  Shape(euid id) : m_id(id), m_trails() {}

  /**
   * @brief 拷贝构造函数。
   */
  Shape(const Shape& that) : m_id(that.m_id), m_trails(that.m_trails) {
  }

  /**
   * @brief 移动构造函数。
   */
  Shape(Shape&& that) : m_id(that.m_id), m_trails(std::move(that.m_trails)) {
    that.m_id = 0;
  }

  /**
   * @brief 析构函数，非虚函数。Shape属于基础类，不期望被继承。
   */
  ~Shape() {}

  /**
   * @brief 重置内容。
   */
  void reset(euid id) {
    m_id = id;
    m_trails.clear();
  }

  /**
   * @brief Line ID
   */
  euid id() const { return m_id; }

  /**
   * @brief 添加一个“挖孔”区域。后续通过trail(i)赋值或访问。
   */
  void addTrail() { m_trails.push_back(Trail{}); }

  /**
   * @brief 获取边界轨迹的数量。
   */
  uint trailNum() const { return static_cast<uint>(m_trails.size()); }

  /**
   * @brief 获取边界轨迹的常性引用。
   */
  const Trail& trail(uint i) const { return m_trails.at(i); }

  /**
   * @brief 获取边界轨迹的常性引用。
   */
  Trail& trail(uint i) { return m_trails.at(i); }

private:
  /**
   * @brief ID
   */
  euid m_id;

  /**
   * @brief 外边界和内部挖孔。
   */
  std::vector<Trail> m_trails;
};

} // namespace hwpcb