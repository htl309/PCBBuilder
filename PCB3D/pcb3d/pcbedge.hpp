#pragma once
#include <pcb/base.hpp>
#include<iostream>
#include<pcb3d/pcbexception.hpp>
namespace hwpcb {


	class PCBEdge {
		public:
			  enum GeoCurve {
				LineEdge,   // 线段
				ArcEdge_CW, // 顺时针圆弧
				ArcEdge_CCW,// 逆时针圆弧
				CircleEdge, // 圆
			  };

			//直线段构造函数
			PCBEdge(uint h, uint t, V3d direction);

			//圆弧构造函数
			PCBEdge(bool cw, uint h, uint t, uint c, real r);

            PCBEdge(GeoCurve cw, uint h, uint t, uint c, real r);
			//圆构造函数
			PCBEdge(uint h, uint c, real r);

			//析构函数
			~PCBEdge();

			//获取头节点索引
			const uint& head() const;

			//获取尾节点索引
			const uint& tail() const;

			//获取圆心点索引
			const uint& center() const;

			//当类型为圆或者圆弧时，获取半径
			const real& radius() const;
		
			//获取方向
			const V3d& direction() const;

			//获取几何类型
			const GeoCurve& edgetype() const;

		private:
		
			//如果是圆的话，起始点和结束点依旧存在，只是他们相同
			//截取边的头顶点的索引
			const uint m_head;
		
			//截取边的尾顶点的索引
			const uint m_tail;
		
			//截取边的圆心索引如果截取边是线，那么该值就是0
			const uint m_center;
        
			//截取边的半径如果截取边是线，那么该值就是0
			const real m_radius;
		
			//截取边的方向如果截取边是圆或圆弧，该方向为它们所在平面的法向
			//该工程情境下:该圆和圆弧的方向始终为(0,0,1)
			const V3d m_direction;

			//枚举类型：线、圆、顺时针圆弧和逆时针圆弧
			//LineEdge / CircleEdge / ArcEdge_CW / ArcEdge_CCW
			const GeoCurve m_edgetype;
	};

}




