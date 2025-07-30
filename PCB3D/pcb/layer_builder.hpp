#pragma once

#include "pcb/layer.hpp"
#include <memory>

namespace hwpcb {

/**
 * @brief 解析器中途构造“层”对象。
 * 层有三种子类，添加Line和Shape的方式各不相同。
 * LayerParser如果直接构造Layer的子类对象，matchLine/Shape/Seg需要重载或判断层类型。
 * 引入Builder则简化了解析器的责任。
 */
class LayerBuilder {
public:
  virtual ~LayerBuilder() {}

  /**
   * @brief 获取构造的“层”对象。
   */
  virtual std::shared_ptr<Layer> getProduct() = 0;

  /**
   * @brief 设定基线平面高度。
   */
  virtual void onBaseHeight(real h) = 0;

  /**
   * @brief 设定厚度。
   */
  virtual void onThickness(real t) = 0;

  /**
   * @brief 提示顶点数量。
   */
  virtual void onPointCount(uint n) = 0;

  /**
   * @brief 提示片段数量。
   */
  virtual void onSegCount(uint n) = 0;

  /**
   * @brief 响应布线数据。
   */
  virtual void newLine(euid id) = 0;

  /**
   * @brief 布线数据结束。
   */
  virtual void endLine() = 0;

  /**
   * @brief 响应形状数据。
   */
  virtual void newShape(euid id) = 0;

  /**
   * @brief 形状数据结束。
   */
  virtual void endShape() = 0;

  /**
   * @brief 响应挖孔数据。
   */
  virtual void newHole() = 0;

  /**
   * @brief 挖孔数据结束。
   */
  virtual void endHole() = 0;

  /**
   * @brief 响应“迹”，布线/区域/挖孔的数据。
   */
  virtual void onTrail(uint nodeNum) = 0;

  /**
   * @brief 响应起点坐标。
   */
  virtual void onStart(real sx, real sy) = 0;

  /**
   * @brief 响应圆形。
   */
  virtual void onCircle(euid id, real width, real centerX, real centerY,
                        real radius) = 0;

  /**
   * @brief 响应线段数据。
   */
  virtual void onLineSeg(euid id, real width, real x, real y) = 0;

  /**
   * @brief 响应弧段数据。
   */
  virtual void onArcSeg(euid id, real width, real centerX, real centerY, real x,
                        real y, bool clockwise) = 0;
};

/**
 * 信号层构造器。
 */
class RouteLayerBuilder : public LayerBuilder {
  RouteLayerBuilder(RouteLayerBuilder&) = delete;
  RouteLayerBuilder& operator=(RouteLayerBuilder&) = delete;

public:
  RouteLayerBuilder(const std::string& layerType);

  virtual std::shared_ptr<Layer> getProduct() override;

  virtual void onBaseHeight(real h) override;
  virtual void onThickness(real t) override;
  virtual void onPointCount(uint n) override;
  virtual void onSegCount(uint n) override;

  virtual void newLine(euid id) override;
  virtual void endLine() override;

  virtual void newShape(euid id) override;
  virtual void newHole() override;
  virtual void endHole() override;
  virtual void endShape() override;

  virtual void onTrail(uint nodeNum) override;
  virtual void onStart(real sx, real sy) override;

  virtual void onCircle(euid id, real width, real centerX, real centerY,
                        real radius) override;

  virtual void onLineSeg(euid id, real width, real x, real y) override;
  virtual void onArcSeg(euid id, real width, real centerX, real centerY, real x,
                        real y, bool clockwise) override;

private:
  std::shared_ptr<RouteLayer> m_layer;
  std::string m_layerType;
  real m_baseHeight;
  real m_thickness;
  uint m_headVid;
  Line m_line;
  Shape m_shape;
  Trail* m_trail;
  uint m_segIdx;
  Seg m_seg;
};

/**
 * @brief 平面层构造器。
 */
class PlaneLayerBuilder : public LayerBuilder {
  PlaneLayerBuilder(PlaneLayerBuilder&) = delete;
  PlaneLayerBuilder& operator=(PlaneLayerBuilder&) = delete;

public:
  PlaneLayerBuilder();

  virtual std::shared_ptr<Layer> getProduct() override;

  virtual void onBaseHeight(real h) override;
  virtual void onThickness(real t) override;
  virtual void onPointCount(uint n) override;
  virtual void onSegCount(uint n) override;

  virtual void newLine(euid id) override;
  virtual void endLine() override;

  virtual void newShape(euid id) override;
  virtual void endShape() override;

  virtual void newHole() override;
  virtual void endHole() override;

  virtual void onTrail(uint nodeNum) override;
  virtual void onStart(real sx, real sy) override;

  virtual void onCircle(euid id, real width, real centerX, real centerY,
                        real radius) override;

  virtual void onLineSeg(euid id, real width, real x, real y) override;
  virtual void onArcSeg(euid id, real width, real centerX, real centerY, real x,
                        real y, bool clockwise) override;

private:
  std::shared_ptr<PlaneLayer> m_layer;
  real m_baseHeight;
  real m_thickness;
  uint m_headVid;
  Line m_line;
  Shape m_shape;
  Trail* m_trail;
  uint m_segIdx;
  Seg m_seg;
};

/**
 * @brief 跨层实例构造器。
 */
class MultiLayerBuilder : public LayerBuilder {
  MultiLayerBuilder(MultiLayerBuilder&) = delete;
  MultiLayerBuilder& operator=(MultiLayerBuilder&) = delete;

public:
  MultiLayerBuilder();

  virtual std::shared_ptr<Layer> getProduct() override;

  virtual void onBaseHeight(real h) override;

  /**
   * @brief 对于跨层，遇到厚度意味着多了新的“子层”。
   */
  virtual void onThickness(real t) override;
  virtual void onPointCount(uint n) override;
  virtual void onSegCount(uint n) override;

  virtual void newLine(euid id) override;
  virtual void endLine() override;

  virtual void newShape(euid id) override;
  virtual void endShape() override;

  virtual void newHole() override;
  virtual void endHole() override;

  virtual void onTrail(uint nodeNum) override;
  virtual void onStart(real sx, real sy) override;

  virtual void onCircle(euid id, real width, real centerX, real centerY,
                        real radius) override;

  virtual void onLineSeg(euid id, real width, real x, real y) override;
  virtual void onArcSeg(euid id, real width, real centerX, real centerY, real x,
                        real y, bool clockwise) override;

private:
  std::shared_ptr<MultiLayer> m_layer;
  real m_baseHeight;
  real m_thickness;
  uint m_headVid;
  MultiLayer::Group* m_group;
  Line m_line;
  Shape m_shape;
  Trail* m_trail;
  uint m_segIdx;
  Seg m_seg;
};

} // namespace hwpcb
