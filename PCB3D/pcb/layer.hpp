#pragma once

#include "pcb/line.hpp"
#include "pcb/shape.hpp"
#include <vector>

namespace hwpcb {

/**
 * @brief PCB的“层”
 * 连续、密集存放所含子层、Line、Shape、Seg的顶点数据。
 * 仅有Seg/Line/Shape对象的话，不知道所属的Layer也就无法获取顶点数据，无法独立IO。
 * 因此只有Layer对象能直接输出到流。
 */
class Layer {
public:
  /**
   * @brief 层面类型。这种设计违背OOP，只是比较明确。
   * 顶层、底层可以放器件。
   */
  enum Type {
    ROUTE_LAYER, // 布线层：统一的基线平面高度，统一的厚度，若干Line和Shape。
    PLANE_LAYER, // 平面层：每个Shape有对应的基线平面高度和厚度。
    MULTI_LAYER, // 复合层：统一的基线平面高度，不同厚度的子层。每个子层有若干Line和Shape。
  };

  /**
   * @brief 构造函数。
   * 顶点数组长度至少为1，保留第[0]个顶点未用，确保有效的顶点索引应大于0。
   * TODO: 删除name属性。层名字可以在测试时保存在map的key中。
   */
  Layer(const std::string& name)
      : m_name(name), m_pointCountHint(), m_segCountHint(), m_vertices(1) {}

  /**
   * @brief 析构函数。因有派生类，需要设计为虚函数。
   */
  virtual ~Layer() {}

  /**
   * @brief 获取层的名称。
   * @return 名称（Plane/MultiLayer/Bottom/Gnd01等）
   * 数据源称为LayerType，有些是自定义名称。
   */
  virtual const std::string& name() const { return m_name; }

  /**
   * @brief 设置名称。
   * @param name 层的名称。
   */
  virtual void setName(const std::string& name) { m_name = name; }

  /**
   * @brief 获取类型。比用RTTI方便一点。
   */
  virtual Type type() const = 0;

  /**
   * @brief 提示顶点数量，所有Line，但不包括Shape。
   */
  virtual void pointCountHint(uint n) { m_pointCountHint = n; }

  /**
   * @brief 获取顶点数量提示，所有Line，但不包括Shape。
   */
  virtual uint pointCountHint() const { return m_pointCountHint; }

  /**
  * @brief 提示片段数量，所有Line，但不包括Shape。
  */
  virtual void segCountHint(uint n) { m_segCountHint = n; }

  /**
   * @brief 获取片段数量提示，所有Line，但不包括Shape。
   */
  virtual uint segCountHint() const { return m_segCountHint; }

  /**
   * @brief 添加顶点，返回顶点的索引。
   * @param v 顶点位置向量。
   */
  uint addVertex(const V2d& v) {
    m_vertices.push_back(v);
    return static_cast<uint>(m_vertices.size() - 1);
  }

  /**
   * @brief 获取顶点数量。
   */
  uint pointCount() { return static_cast<uint>(m_vertices.size()); }

  /**
   * @brief 获取顶点的常性引用。
   * @param i 顶点索引，uint32。
   */
  const V2d& vertex(uint i) const { return m_vertices[i]; }

  /**
   * @brief 获取顶点的引用。
   * @param i 顶点索引，uint32。
   */
  V2d& vertex(uint i) { return m_vertices[i]; }

  /**
   * @brief 获取顶点数组。
   */
  const V2d* vertexArray() const { return m_vertices.data(); }

private:
  std::string m_name;
  uint m_pointCountHint;
  uint m_segCountHint;
  std::vector<V2d> m_vertices;
};

/**
 * @brief 布线层。
 */
class RouteLayer : public Layer {
public:
  /**
   * @brief 构造函数。
   */
  RouteLayer(const std::string& name, real baseHeight, real thickness,
             bool upward)
      : Layer(name), m_baseHeight(baseHeight), m_thickness(thickness),
        m_upward(upward), m_lines(), m_shapes() {}

  /**
   * @brief 查询类型。
   */
  virtual Type type() const { return Type::ROUTE_LAYER; }

  /**
   * @brief 朝向
   */
  virtual bool upward() const { return m_upward; }

  /**
   * @brief 基线平面高度。整层相对这个高度。
   */
  real baseHeight() const { return m_baseHeight; }

  /**
   * @brief 获取厚度。所有对象有统一的拉伸高度。
   */
  real thickness() const { return m_thickness; }

  /**
   * @brief 添加走线。
   */
  void addLine(Line&& line) { m_lines.emplace_back(line); }

  /**
   * @brief 获取走线数量。
   */
  uint lineNum() const { return static_cast<uint>(m_lines.size()); }

  /**
   * @brief 获取走线的常性引用。
   */
  const Line& line(uint i) const { return m_lines[i]; }

  /**
   * @brief 添加形状。
   */
  void addShape(Shape&& shape) { m_shapes.emplace_back(shape); }

  /**
   * @brief 获取形状数量。
   */
  uint shapeNum() const { return static_cast<uint>(m_shapes.size()); }

  /**
   * @brief 获取形状的常性引用。
   */
  const Shape& shape(uint i) const { return m_shapes[i]; }

  /**
   * @brief 获取形状的引用，用于添加片段等。
   */
  Shape& shape(uint i) { return m_shapes[i]; }

private:
  /**
   * @brief 基线平面高度。
   * 设计范围：
   */
  real m_baseHeight;

  /**
   * @brief 厚度。
   * 设计范围：
   */
  real m_thickness;

  /**
   * @brief 朝向。底层为false。
   */
  bool m_upward;

  /**
   * @brief 走线列表。
   */
  std::vector<Line> m_lines;

  /**
   * @brief 形状列表。
   */
  std::vector<Shape> m_shapes;
};

/**
 * @brief 平面层，例如电源层（Power Plane）、接地层（Ground Plane）。
 * 通常大面积铺铜，以提高接地性能、减小电磁干扰。
 * 基材（绿色部分）、Surface层。
 * Plane每个区域有自己的基线平面高度，有自己的厚度
 * 单层2D图形+基线高度+厚度，在基线高度上拉伸生成模型体。
 */
class PlaneLayer : public Layer {
public:
  /**
   * @brief 构造函数。
   */
  PlaneLayer(const std::string& name)
      : Layer(name), m_baseHeights(), m_thickness(), m_shapes() {}

  /**
   * @brief 查询类型。
   */
  virtual Type type() const { return Type::PLANE_LAYER; }

  /**
   * @brief 添加形状和基线平面高度。
   * 1个BASEPLANEHEIGHT+1个THICKNESS+1个shape为一组。
   */
  void addShape(real baseHeight, real thickness, Shape&& shape) {
    m_baseHeights.push_back(baseHeight);
    m_thickness.push_back(thickness);
    m_shapes.emplace_back(shape);
  }

  /**
   * @brief 获取形状数量。
   */
  uint shapeNum() const { return static_cast<uint>(m_shapes.size()); }

  /**
   * @brief 获取基线平面高度。
   */
  real baseHeight(uint i) const { return m_baseHeights[i]; }

  /**
   * @brief 获取厚度。
   */
  real thickness(uint i) const { return m_thickness[i]; }

  /**
   * @brief 获取形状的常性引用。
   */
  const Shape& shape(uint i) const { return m_shapes[i]; }

  /**
   * @brief 获取形状的引用，用于添加片段等。
   */
  Shape& shape(uint i) { return m_shapes[i]; }

private:
  /**
   * @brief 每个区域有自己的基线平面高度。
   */
  std::vector<real> m_baseHeights;

  /**
   * @brief 每个区域有自己的厚度。
   */
  std::vector<real> m_thickness;

  /**
   * @brief 形状列表。
   */
  std::vector<Shape> m_shapes;
};

/**
 * @brief 放置各种跨层对象，比如过孔、焊盘等，用于void/镀层场景。
 * 这里面所有数据都在同一个基准平面；
 * 但有多种高度规格，每种高度会有多个数据。
 * 1个高度+多个几何为1组数据，可以以THICKNESS为不同厚度的起点关键字。
 * 每个Shape有个对应高度。
 * 层不多，高度有复用的可能，并不是任意高度。
 */
class MultiLayer : public Layer {
public:
  /**
   * @brief 构造函数
   */
  MultiLayer(const std::string& name, real baseHeight)
      : Layer(name), m_baseHeight(baseHeight), m_groups() {}

  /**
   * @brief 查询类型。
   */
  virtual Type type() const { return Type::MULTI_LAYER; }

  /**
   * @brief 获取基线平面高度。
   */
  real baseHeight() const { return m_baseHeight; }

  /**
   * @brief 子层。
   */
  class Group {
  public:
    Group(real thickness) : m_thickness(thickness), m_lines(), m_shapes() {}

    /**
     * @brief 添加走线。
     */
    void addLine(Line&& line) { m_lines.emplace_back(line); }

    /**
     * @brief 向第i层添加形状。
     */
    void addShape(Shape&& shape) { m_shapes.emplace_back(shape); }

    real thickness() const { return m_thickness;  }

    uint lineNum() const { return static_cast<uint>(m_lines.size()); }

    uint shapeNum() const { return static_cast<uint>(m_shapes.size()); }

    const Line& line(uint i) const { return m_lines.at(i); }

    const Shape& shape(uint i) const { return m_shapes.at(i); }

  private:
    real m_thickness;
    std::vector<Line> m_lines;
    std::vector<Shape> m_shapes;
  };

  /**
   * @brief 添加子层。
   */
  void addGroup(real thickness) { m_groups.emplace_back(Group{thickness}); }

  /**
   * @brief 获取子层数量。
   */
  uint groupNum() const { return static_cast<uint>(m_groups.size()); }

  /**
   * @brief 获取子层。
   */
  const Group& group(uint i) const { return m_groups.at(i); }

  /**
   * @brief 获取子层。
   */
  Group& group(uint i) { return m_groups.at(i); }

private:
  /**
   * @brief 基线平面高度
   */
  real m_baseHeight;

  /**
   * @brief 厚度对应的子层。
   */
  std::vector<Group> m_groups;
};

} // namespace hwpcb