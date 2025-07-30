#pragma once

#include <CDT/CDT.h>
#include <pcb3d/pcbshell.hpp>
#include <pcb3d/layer3d.hpp>

namespace hwpcb {

	class Triangulation {

		public:
	  
		  //保存Layer3D离散化之后的信息
		  //点数组
		  vector<vector<V3d>> PointsList;
	  
		  //索引数组
		  vector<vector<uint>> IndexesList;

		  //析构函数
		  ~Triangulation() ;

		  //对Layer离散化，将三角化之后的信息保存到成员变量中
		  void Tri_Layer(Layer3D* layer3d);
	 
		  //对Shell进行离散化
		  void Tri_shell(PCBShell* _shell);

		  //获取Shell的点数组，调用这个函数之前，先得调用Tri_shell
		  vector<V3d> get_Tri_Points();

		  //获取Shell的索引数组，调用这个函数之前，先得调用Tri_shell
		  vector<uint> get_Tri_Index(int baseindex=0);

          vector< V2d >& get_Vertices() { return vertices; }
		private:

		  //保存某个Shell离散化之后的数据
		  //点数组
		  vector<V2d> vertices;

		  //边数组
		  vector<V2i> edges;

		  //记录一下需要离散化的Shell
		  PCBShell* shell;

		  //基数，将新离散化好的线添加到多边形中起到重要的作用
		  int number;
	  
		  //如果是正常的几何体，离散化所得的点就是底面的轮廓线
		  //如果不是，那离散化所得的点只是一条线上的点
		  bool discret_Shell(PCBShell* shell);
	 
		  //离散化圆弧和圆
		  bool discret_Edge(PCBEdge* edge, vector<V2d>& pointslist);
	  
		  //离散化圆弧，将点依次放到vertexs里面，第一个点和最后一个点是圆弧的起始点和结束点
		  void discret_ARC(PCBEdge* edge, int n, int base, vector<V2d>& pointslist);

		  //n是离散化的段数，angle_begin必须要小于angle_end
		  void discret(V2d Arc_center, double r, double angle_begin, double angle_end,int n, int base);

		  //离散化平面，将一个包含圆弧的二维闭合形状，变成一个多边形
		  void discret_Face(PCBFace* face, vector<V2d>& pointslist);
	  
		  //计算角度函数，角度值限制在0~2*pi
		  double angle(int _center, int _point, double r, vector<V2d>& pointslist);

		  //离散化新的Shell之前需要清空一下前一个shell保存的信息
		  void clear();
	
	};

} 