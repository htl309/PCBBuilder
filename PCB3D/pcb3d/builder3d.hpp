#pragma once

#include<pcb/board.hpp>
#include<pcb3d/layer3d.hpp>

namespace hwpcb {
	class Builder3D {
		
	public:

		//析构函数
		~Builder3D();

		//输入一个Board，输出一个Board3D,用引用输出
		PCBError create_Board3D(Board board, Board3D& board3d); 
	

		//输入一个shape，拉升成一个shell，当然需要提供基线高度和最终高度
		PCBShell* create_Shape_Shell(Shape s, real baseheight, real topheight);

		//由一个圆弧膨胀拉伸成shell
		//圆弧不多，这个函数没多大调用
		PCBShell* create_Arc_Shell(Seg s, real baseheight, real topheight);

		//处理膨胀线宽大于圆弧半径的情况
		PCBShell* create_Arc_Shell2(Seg s, real baseheight, real topheight);

		//这个是放在shell里面的顶点数组
		//每创建一个shell之前要清空一下
		const V2d* vertices2d;
	private:
		
		
		//对输入的层进行三维建模
		Layer3D* create_Layer3D(Layer& layer2d);

		//处理布线层
		Layer3D* deal_RouteLayer(const RouteLayer& routelayer);

		//处理平面层
		Layer3D* deal_PlaneLayer(const PlaneLayer& planelayer);
		
		//处理跨层
		Layer3D* deal_MultiLayer(const MultiLayer& multilayer);

		
		//创建shape中的刀劈面
        PCBShell* create_FaceShell_in_Shape(euid id,Seg s, real baseheight,real topheight);
		
		//创建由Line拉伸成的shell
        PCBShell* create_Line_Shell(Seg s, real baseheight, real topheight);
		


		//处理膨胀线宽大于端点之间距离的情况
		PCBShell* create_Arc_Shell3(Seg s, real baseheight,real topheight) ;
              
		//由一个圆膨胀拉伸成shell
		PCBShell* create_Circle_Shell(Seg s, real baseheight, real topheight);

		//统计Shape中顶点的数量
		uint vertexCount(const Shape& sp);

		//这个函数是用于判断圆弧是优弧还是劣弧的
		//优弧输出1，劣弧输出0,半圆输出-1
		int ARC_TYPE(Seg s) ;
	};
}