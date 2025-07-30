#include<pcb3d/pcbedge.hpp>
#include<pcb3d/pcbexception.hpp>
namespace hwpcb {
    //直线段构造函数
PCBEdge::PCBEdge(uint h, uint t, V3d direction)
    : m_head(h), m_tail(t), m_center(0), m_radius(0), m_direction(direction),
      m_edgetype(LineEdge) {}

// 圆弧构造函数
PCBEdge::PCBEdge(bool cw, uint h, uint t, uint c, real r)
    : m_head(h), m_tail(t), m_center(c), m_radius(r), m_direction(V3d(0, 0, 1)),
      m_edgetype(cw ? ArcEdge_CW : ArcEdge_CCW) {}

PCBEdge::PCBEdge(GeoCurve cw, uint h, uint t, uint c, real r)
    : m_head(h), m_tail(t), m_center(c), m_radius(r), m_direction(V3d(0, 0, 1)),
      m_edgetype(cw ) {}
// 圆构造函数
PCBEdge::PCBEdge(uint h, uint c, real r)
    : m_head(h), m_tail(h), m_center(c), m_radius(r), m_direction(V3d(0, 0, 1)),
      m_edgetype(CircleEdge) {}

// 析构函数
PCBEdge::~PCBEdge() {}

// 获取头节点索引
const uint& PCBEdge::head() const { return m_head; }

// 获取尾节点索引
const uint& PCBEdge::tail() const { return m_tail; }

// 获取圆心点索引
const uint& PCBEdge::center() const { return m_center; }

// 当类型为圆或者圆弧时，获取半径
const real& PCBEdge::radius() const { return m_radius; }

// 获取方向
const V3d& PCBEdge::direction() const { return m_direction; }

// 获取几何类型
const PCBEdge::GeoCurve& PCBEdge::edgetype() const { return m_edgetype; }
 }