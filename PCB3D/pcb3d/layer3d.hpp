#pragma once


#include<pcb3d/pcbshell.hpp>
#include<pcb/layer.hpp>
#include<pcb3d/trail3d.hpp>

namespace hwpcb {

	//层
	class Layer3D {

	public: 
		
		//构造函数
		Layer3D(Layer::Type type);

		//析构函数
         ~Layer3D();
		
		//获取ShapeShell数组
         vector<PCBShell*>& get_ShellList();

		//获取走线数组
        vector<Trail3D*>& get_TrailList() ;

		//获取层类型
        Layer::Type layertype() const;

	private:
		
		//存放ShapeShell的数组
		vector<PCBShell*> m_shelllist;
        
		//存放走线Trail3D的数组
		vector<Trail3D*> m_traillist;

		//Layer的类型，有三种类型和二维数据结构对应
		const Layer::Type m_layertype;
	};
	
	//板
	using Board3D = vector<Layer3D *>;
}