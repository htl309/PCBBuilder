#pragma once

#include "pcb/trail.hpp"

namespace hwpcb {

/**
 * @brief 连续的片段。节点数量为1时表示一个圆，弧段起止点相同。
 * Line用于表示两类对象：无填充的封闭图形（只需边界）、非封闭图形。
 * 片段数量在构建后就固定了，不会动态扩充，因此各Seg的引用持久有效。
 */
class Line {
public:
  /**
   * @brief 默认构造函数。
   */
  Line() : m_id(), m_trail() {}

  /**
   * @brief 拷贝构造函数。
   */
  Line(const Line& that) : m_id(that.m_id), m_trail(that.m_trail) {
  }

  /**
   * @brief 移动构造函数。
   */
  Line(Line&& that) : m_id(that.m_id), m_trail(std::move(that.m_trail)) {
    that.m_id = 0;
  }

  /**
   * @brief 构造函数。
   * @param id 实体ID
   */
  Line(euid id) : m_id(id), m_trail() {}

  /**
   * @brief 清空内容。
   */
  void reset(euid id) { m_id = id; }

  /**
   * @brief Line ID
   */
  euid id() const { return m_id; }

  /**
   * @brief 获取Seg列表。
   */
  const Trail& trail() const { return m_trail; }

  /**
   * @brief 获取Seg列表。
   */
  Trail& trail() { return m_trail; }

private:
  /**
   * @brief ID
   */
  euid m_id;

  /**
   * @brief 片段列表。
   */
  Trail m_trail;
};

} // namespace hwpcb