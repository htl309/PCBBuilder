#pragma once

#include "pcb/layer.hpp"
#include <memory>
#include <vector>

namespace hwpcb {

/**
 * @brief 电路板，用于存放“层”对象。
 */
class Board {
public:
  /**
   * @brief 构造函数。
   */
  Board() : m_layers() {}

  /**
   * @brief 添加一“层”。
   */
  void addLayer(std::shared_ptr<Layer> layer) { m_layers.push_back(layer); }

  /**
   * @brief 设置某“层”对象。
   */
  void setLayer(uint i, std::shared_ptr<Layer> layer) {
    m_layers.at(i) = layer;
  }

  /**
   * @brief 设置“层”的数量。
   */
  void layerNum(uint n) { m_layers.resize(n); }

  /**
   * @brief 获取“层”的数量。
   */
  uint layerNum() const { return static_cast<uint>(m_layers.size()); }

  /**
   * @brief 获取“层”的引用。
   * 层数量如果改多，新增层指针为空，但调用layer()成员函数本身无妨。
   */
  const Layer& layer(uint i) const { return *m_layers.at(i); }

  /**
   * @brief 获取“层”的引用。
   */
  Layer& layer(uint i) { return *m_layers.at(i); }

private:
  std::vector<std::shared_ptr<Layer>> m_layers;
};

} // namespace hwpcb