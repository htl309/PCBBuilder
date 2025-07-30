#pragma once

#include<pcb3d/pcbshell.hpp>


namespace hwpcb {

    class Trail3D {
        public:
           //走线的ID号
           const euid ID;

           //构造函数
           Trail3D(euid _ID);

           //析构函数
           ~Trail3D();

            //获取走线中的Shell数组
            vector<PCBShell*>& get_ShellList();

        private:
            //Shell数组,包含走线中所有的Shell
            std::vector<PCBShell*> shelllist;
    };

}