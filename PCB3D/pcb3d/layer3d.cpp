#include<pcb3d/layer3d.hpp>

namespace hwpcb {

    //构造函数
	Layer3D::Layer3D(Layer::Type type) : m_layertype(type){};

    //析构函数
     Layer3D::~Layer3D() {
        for (auto i : m_shelllist) {
            delete i;
        }
        for (auto i : m_traillist) {
            delete i;
        }
     }

    //获取ShapeShell数组
    vector<PCBShell*>& Layer3D::get_ShellList() {return m_shelllist;}

    //获取走线数组
     vector<Trail3D*>& Layer3D::get_TrailList() { return m_traillist; }

    //获取层类型
     Layer::Type Layer3D::layertype() const { return m_layertype; }
}