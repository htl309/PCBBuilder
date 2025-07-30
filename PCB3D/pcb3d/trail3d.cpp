#include<pcb3d/trail3d.hpp>

namespace hwpcb {

    //构造函数
    Trail3D::Trail3D(euid _ID) : ID(_ID) {}
    
    //析构函数
    Trail3D::~Trail3D() {
        for(auto i:shelllist) delete i;
    }

    //获取走线中的Shell数组
    vector<PCBShell*>& Trail3D::get_ShellList() {return shelllist;}

} //namespace hwpcb