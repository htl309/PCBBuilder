#pragma once

#include<pcb3d/pcbedge.hpp>
#include<vector>

namespace hwpcb {
	using namespace std;

	//包围圈
    using PCBLoop = vector<PCBEdge*>;

	//面
	class PCBFace {
		public:
			  enum GeoSurface {
				PlaneFace,   // 平面
				CylinderFace,// 圆柱面	
			  };
          
		public:
	
            //这里的点都是索引下标
			//平面构造函数
			PCBFace(uint o, V3d n);

			//柱面构造函数
			PCBFace(uint o, real r);


			//析构函数
			~PCBFace();

			//获取面原点索引
			const uint& Origin() const;

			//获取面的法向
			const V3d& Normal() const;

			//获取圆柱面的半径
			//如果是平面则返回值为0
			const real& Radius() const ;

			//获取面的类型
			const GeoSurface& FaceType()  const;

		   //获取面的包围圈数组
			vector<PCBLoop>& Face() ;
		private:

			//平面的原点,或圆的圆心,面的圆心和底面的圆弧的圆心是同一个点
			const uint m_origin;

			//平面的法向量，或圆柱面的延伸方向
			const V3d m_normal;

			//圆柱面半径，如果截取边是线，那么该值就是0
			const real m_radius;
        
			//圈数组，第一条是轮廓，之后是洞
			//轮廓是逆时针连接，而洞是顺时针连接
			vector<PCBLoop> m_loops;

			//面的几何类型：平面和圆柱面
			const GeoSurface m_facetype;
	};

}