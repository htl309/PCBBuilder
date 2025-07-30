#include "pcb/layer_builder.hpp"
#include "verify.hpp"
using namespace std;

namespace hwpcb {

constexpr real NINF = -1e99; // 电路板不会有这种尺度。

RouteLayerBuilder::RouteLayerBuilder(const std::string& layerType)
    : m_layer(), m_layerType(layerType), m_baseHeight(NINF), m_thickness(NINF),
      m_headVid(), m_line(), m_shape(0), m_trail(), m_segIdx(), m_seg() {}

std::shared_ptr<Layer> RouteLayerBuilder::getProduct() {
  verify(m_layer);
  return m_layer;
}

void RouteLayerBuilder::onBaseHeight(real h) {
  verify(m_baseHeight == NINF); // 尚未设置
  m_baseHeight = h;
}

void RouteLayerBuilder::onThickness(real t) {
  verify(m_thickness == NINF); // 尚未设置厚度
  verify(!m_layer); // 尚未创建“层”实例
  const bool upward = (m_layerType != "Bottom");
  m_thickness = t;
  m_layer.reset(new RouteLayer(m_layerType, m_baseHeight, m_thickness, upward));
}

void RouteLayerBuilder::onPointCount(uint n) { m_layer->pointCountHint(n); }

void RouteLayerBuilder::onSegCount(uint n) { m_layer->segCountHint(n); }

void RouteLayerBuilder::newLine(euid id) {
  verify(m_trail == nullptr);
  m_line.reset(id);
  m_trail = &m_line.trail();
}

void RouteLayerBuilder::endLine() {
  m_layer->addLine(std::move(m_line));
  m_trail = nullptr;
}

void RouteLayerBuilder::newShape(euid id) {
  verify(m_trail == nullptr);
  m_shape.reset(id);
  m_shape.addTrail(); // 外边界
  m_trail = &m_shape.trail(m_shape.trailNum() - 1);
}

void RouteLayerBuilder::newHole() {
  m_shape.addTrail();
  const uint nt = m_shape.trailNum();
  verify(nt > 0); // 已添加过外边界
  m_trail = &m_shape.trail(nt - 1);
}

void RouteLayerBuilder::endHole() {
  m_trail = nullptr;
}

void RouteLayerBuilder::endShape() {
  m_layer->addShape(std::move(m_shape));
  m_trail = nullptr;
}

void RouteLayerBuilder::onTrail(uint nodeNum) {
  m_trail->resize(nodeNum);
  m_segIdx = 0;
}

void RouteLayerBuilder::onStart(real sx, real sy) {
  const V2d v{sx, sy};
  m_headVid = m_layer->addVertex(v);
}

void RouteLayerBuilder::onCircle(euid id, real width, real centerX,
                                 real centerY, real radius) {
  verify(m_trail);
  verify(m_trail->size() == 1);
  verify(m_segIdx == 0);
  const V2d vc{centerX, centerY};
  const uint centerVid = m_layer->addVertex(vc);
  m_trail->seg(m_segIdx++).toCircle(id, width, centerVid, radius);
}

void RouteLayerBuilder::onLineSeg(euid id, real width, real x, real y) {
  verify(m_trail);
  verify(m_segIdx < m_trail->size());
  const V2d vt{x, y};
  const uint tailVid = m_layer->addVertex(vt);
  m_trail->seg(m_segIdx++).toLine(id, width, m_headVid, tailVid);
  m_headVid = tailVid;
}

void RouteLayerBuilder::onArcSeg(euid id, real width, real centerX,
                                 real centerY, real x, real y, bool clockwise) {
  verify(m_trail);
  verify(m_segIdx < m_trail->size());
  const V2d vc{centerX, centerY};
  const uint centerVid = m_layer->addVertex(vc);
  const V2d vt{x, y};
  const uint tailVid = m_layer->addVertex(vt);
  m_trail->seg(m_segIdx++).toArc(id, width, m_headVid, tailVid, centerVid, clockwise);
  m_headVid = tailVid;
}

/**
 * 平面层
 */

PlaneLayerBuilder::PlaneLayerBuilder()
    : m_layer(new PlaneLayer("Plane")), m_baseHeight(NINF), m_thickness(NINF),
      m_headVid(), m_line(), m_shape(0), m_trail(), m_segIdx(), m_seg() {}

std::shared_ptr<Layer> PlaneLayerBuilder::getProduct() {
  verify(m_layer);
  return m_layer;
}

void PlaneLayerBuilder::onBaseHeight(real h) { m_baseHeight = h; }

void PlaneLayerBuilder::onThickness(real t) { m_thickness = t; }

void PlaneLayerBuilder::onPointCount(uint n) { m_layer->pointCountHint(n); }

void PlaneLayerBuilder::onSegCount(uint n) { m_layer->segCountHint(n); }

void PlaneLayerBuilder::newLine(euid) {
  verify(false); // 平面层没有布线数据
}

void PlaneLayerBuilder::endLine() {
  verify(false); // 平面层没有布线数据
}

void PlaneLayerBuilder::newShape(euid id) {
  verify(m_baseHeight > NINF); // 之前应设置过基准平面高度、区域厚度。
  verify(m_thickness > NINF);
  m_shape.reset(id);
  m_shape.addTrail();
  m_trail = &m_shape.trail(m_shape.trailNum() - 1);
}

void PlaneLayerBuilder::newHole() {
  m_shape.addTrail();
  const uint nt = m_shape.trailNum();
  verify(nt > 0); // 已添加过外边界
  m_trail = &m_shape.trail(nt - 1);
}

void PlaneLayerBuilder::endHole() {
  m_trail = nullptr;
}

void PlaneLayerBuilder::endShape() {
  verify(m_baseHeight > NINF); // 之前应设置过基准平面高度、区域厚度。
  verify(m_thickness > NINF);
  m_layer->addShape(m_baseHeight, m_thickness, std::move(m_shape));
  m_baseHeight = NINF; // 基准平面高度、区域厚度已被使用，重置。
  m_thickness = NINF;
  m_trail = nullptr;
}

void PlaneLayerBuilder::onTrail(uint nodeNum) {
  m_trail->resize(nodeNum);
  m_segIdx = 0;
}

void PlaneLayerBuilder::onStart(real sx, real sy) {
  const V2d v{sx, sy};
  m_headVid = m_layer->addVertex(v);
}

void PlaneLayerBuilder::onCircle(euid id, real width, real centerX,
                                 real centerY, real radius) {
  verify(m_trail);
  verify(m_trail->size() == 1);
  verify(m_segIdx == 0);
  const V2d vc{centerX, centerY};
  const uint centerVid = m_layer->addVertex(vc);
  m_trail->seg(m_segIdx++).toCircle(id, width, centerVid, radius);
}

void PlaneLayerBuilder::onLineSeg(euid id, real width, real x,
                                  real y) {
  verify(m_trail);
  verify(m_segIdx < m_trail->size());
  const V2d vt{x, y};
  const uint tailVid = m_layer->addVertex(vt);
  m_trail->seg(m_segIdx++).toLine(id, width, m_headVid, tailVid);
  m_headVid = tailVid;
}

void PlaneLayerBuilder::onArcSeg(euid id, real width, real centerX,
                                 real centerY, real x, real y, bool clockwise) {
  verify(m_trail);
  verify(m_segIdx < m_trail->size());
  const V2d vc{centerX, centerY};
  const uint centerVid = m_layer->addVertex(vc);
  const V2d vt{x, y};
  const uint tailVid = m_layer->addVertex(vt);
  m_trail->seg(m_segIdx++).toArc(id, width, m_headVid, tailVid, centerVid, clockwise);
  m_headVid = tailVid;
}

/**
 * 跨层
 */

MultiLayerBuilder::MultiLayerBuilder()
    : m_layer(), m_baseHeight(NINF), m_thickness(NINF), m_headVid(), m_group(),
      m_line(), m_shape(0), m_trail(), m_segIdx(), m_seg() {}

std::shared_ptr<Layer> MultiLayerBuilder::getProduct() {
  verify(m_layer);
  return m_layer;
}

void MultiLayerBuilder::onBaseHeight(real h) {
  verify(!m_layer); // 仅创建一次
  m_baseHeight = h;
  m_layer.reset(new MultiLayer("MultiLayer", m_baseHeight));
}

/**
 * @brief 对于跨层，遇到厚度意味着多了新的“子层”。
 */
void MultiLayerBuilder::onThickness(real t) {
  m_thickness = t;
  m_layer->addGroup(t);
  const uint ng = m_layer->groupNum();
  m_group = &m_layer->group(ng - 1); // 总是向最后的组添加元素
}

void MultiLayerBuilder::onPointCount(uint n) { m_layer->pointCountHint(n); }

void MultiLayerBuilder::onSegCount(uint n) { m_layer->segCountHint(n); }

void MultiLayerBuilder::newLine(euid id) {
  verify(m_trail == nullptr);
  m_line.reset(id);
  m_trail = &m_line.trail();
}

void MultiLayerBuilder::endLine() {
  m_group->addLine(std::move(m_line));
  m_trail = nullptr;
}

void MultiLayerBuilder::newShape(euid id) {
  verify(m_trail == nullptr);
  m_shape.reset(id);
  m_shape.addTrail(); // 外边界
  m_trail = &m_shape.trail(m_shape.trailNum() - 1);
}

void MultiLayerBuilder::endShape() {
  verify(m_group != nullptr); // 之前应设置过基准平面高度、区域厚度。
  m_group->addShape(std::move(m_shape));
  m_trail = nullptr;
}

void MultiLayerBuilder::newHole() {
  m_shape.addTrail();
  const uint nt = m_shape.trailNum();
  verify(nt > 0); // 已添加过外边界
  m_trail = &m_shape.trail(nt - 1);
}

void MultiLayerBuilder::endHole() {
  m_trail = nullptr;
}

void MultiLayerBuilder::onTrail(uint nodeNum) {
  m_trail->resize(nodeNum);
  m_segIdx = 0;
}

void MultiLayerBuilder::onStart(real sx, real sy) {
  const V2d v{sx, sy};
  m_headVid = m_layer->addVertex(v);
}

void MultiLayerBuilder::onCircle(euid id, real width, real centerX,
                                 real centerY, real radius) {
  verify(m_trail);
  verify(m_trail->size() == 1);
  verify(m_segIdx == 0);
  const V2d vc{centerX, centerY};
  const uint centerVid = m_layer->addVertex(vc);
  m_trail->seg(m_segIdx++).toCircle(id, width, centerVid, radius);
}

void MultiLayerBuilder::onLineSeg(euid id, real width, real x, real y) {
  verify(m_trail);
  verify(m_segIdx < m_trail->size());
  const V2d vt{x, y};
  const uint tailVid = m_layer->addVertex(vt);
  m_trail->seg(m_segIdx++).toLine(id, width, m_headVid, tailVid);
  m_headVid = tailVid;
}

void MultiLayerBuilder::onArcSeg(euid id, real width, real centerX,
                                 real centerY, real x, real y, bool clockwise) {
  verify(m_trail);
  verify(m_segIdx < m_trail->size());
  const V2d vc{centerX, centerY};
  const uint centerVid = m_layer->addVertex(vc);
  const V2d vt{x, y};
  const uint tailVid = m_layer->addVertex(vt);
  m_trail->seg(m_segIdx++).toArc(id, width, m_headVid, tailVid, centerVid, clockwise);
  m_headVid = tailVid;
}

} // namespace hwpcb