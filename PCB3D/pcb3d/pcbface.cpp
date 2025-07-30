#include<pcb3d/pcbface.hpp>

namespace hwpcb {

    //平面构造函数
    PCBFace::PCBFace(uint o, V3d n)
        : m_facetype(PlaneFace), m_origin(o), m_normal(n), m_radius(0) {}


    //柱面构造函数
    PCBFace::PCBFace(uint o, real r)
        : m_facetype(CylinderFace), m_origin(o), m_normal(V3d(0, 0, 1)),
          m_radius(r) {}

    //析构函数
    PCBFace::~PCBFace() {
      for (auto loop : m_loops) {
        for (auto edge : loop)
              delete edge;
      }
    }

    //获取面原点索引
    const uint& PCBFace::Origin() const { return m_origin; }

    //获取面的法向
    const V3d& PCBFace::Normal() const { return m_normal; }

    //获取圆柱面的半径
    //如果是平面则返回值为0
    const real& PCBFace::Radius() const { return m_radius; }

    //获取面的类型
    const PCBFace::GeoSurface& PCBFace::FaceType() const { return m_facetype; } 
    
    //获取面的包围圈数组
    vector<PCBLoop>& PCBFace::Face() { return m_loops; }

}