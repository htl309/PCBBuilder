#pragma once


#include<pcb3d/pcbface.hpp> 
#include<iostream>

namespace hwpcb {

	class PCBShell {
		public:
		enum PCBShellType {
			LineShell,   // 由线段生成的shell
			ArcShell,    // 由圆弧生成的shell
			CircleShell, // 由圆生成的shell
			ShapeShell,  // 由shape拉伸而成的shell
			FaceShell, // 只有一个刀劈面的shell，这时，bottom指针就直线这个刀劈面
		};

		public:

			//ID号
			const euid ID;

			//Shell构造函数。注意！二维点数组的填充需要在创建Shell之后进行填充
			PCBShell(PCBShellType _shelltype, euid _ID, real _baseheight, real _topheight,PCBFace* _bottom ,int pointscount);
		
			//悬挂面构造函数
			PCBShell(euid _ID, real _baseheight, real _topheight, PCBFace* _bottom);

			//析构函数
			~PCBShell();
		
			//获取二维点数组的数量，该数量是shell中包含点数量的一半
			uint  PointCount() ;

			//获取二维点数组
			vector<V2d>& PointList() ;
	
			//获取基线高度
			const real& BaseHeight() const ;

			//获取顶面高度
			const real& TopHeight() const;

			//获取Shell的类型
			PCBShellType ShellType();

			//获取指向底面的指针
			PCBFace* BottomFace();

			//获取侧面
			vector<PCBFace*> GetSideFace();

			//获取顶面
			PCBFace* GetTopFace();

			//由一条顶边和一条底边拉成的面
			//实际上就是补全面的两个侧边
			//当然要是圆的话就不需要补全侧边了
			//注意，edge1的head与edge2的head对应，tail与tail对应，也就是差了一个高度
			PCBFace* GetSidePlaneFace(PCBEdge* edge1, PCBEdge* edge2);

			//获取由圆弧拉伸而成的侧面
			PCBFace* GetSideArcFace(PCBEdge* edge1, uint offset);

			//获取由圆拉伸而成的侧面
			PCBFace* GetSideCircleFace(PCBEdge* edge1, uint offset);


            private:

			//底面的指针
			PCBFace* m_bottom;

			//二维点数组
			vector<V2d> m_points;

			//Shell的类型,有五种：
			//由线段膨胀生成的Shell、由圆弧膨胀生成的Shell、由圆膨胀生成的Shell、由Shape拉伸生成的Shell和悬挂面
			//ShellLineShell/ArcShell/CircleShell/ShapeShell/FaceShell
			const PCBShellType m_shelltype;

			//基线高度
			const real m_baseheight;

			//顶面高度，在Bottom层中顶面高度小于基线高度
			const real m_topheight;

			PCBShell(const PCBShell&) = delete; // 抑制
			PCBShell& operator=(const PCBShell&) = delete; // 抑制
	};

}