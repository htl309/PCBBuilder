#include"pcb3d/pcbshell.hpp"
#include"pcb3d/layer3d.hpp"
#include<fstream>

namespace hwpcb {

	class StepWriter {
	
		public:

			//构造函数
			StepWriter();

			//析构函数
			~StepWriter();

			//输出函数，只需要三维板和输出路径，就能输出Step文件到该路径下
			//路径name包含文件名称
			PCBError write_Step(Board3D& board3d, string name);
		
		private:
			//stp文件中的索引数字,很重要
			euid number;
            euid shape_layer_id;
            euid shape_shell_id;
			//文件流
			ofstream ofs;

			//记录索引的数组这三个数组贯穿整个类
			//记录点索引
			vector < euid > CARTESIAN_POINT;
			//记录向量点索引
			vector < euid > VERTEX_POINT;
			//记录体索引
			vector < euid > MANIFOLD_SOLID_BREP;
		
			//写入文件头部逻辑
			void write_Begin();
		
			//逐层写入函数
			bool write_Layer_to_Step(Layer3D* layer3d);
		
			//写入Shell
			euid write_Shell_to_Step(PCBShell* shell);

			//将Shell中的点写入Step文件
			void write_PointList_in_Shell(PCBShell* shell);

			//将Face写入Step文件
			euid write_Face_to_Step(PCBFace* planeface);

			//写入边
			euid write_Edge_to_Step(PCBEdge* edge) ;

			//写入LineEdge
			euid write_LineEdge_to_Step(PCBEdge* lineedge);

			//写入ArcEdge
			euid write_ArcEdge_to_Step(PCBEdge* arcedge);

			//写入CircleEdge
			euid write_CircleEdge_to_Step(PCBEdge* circleedge);

			//写入方向
			euid write_DIRECTION(V3d direction);

			//根据方向写入向量
			euid write_VECTOR(euid DIRECTION);
		
			//写入新Shell的准备工作
			void wirte_NewShell() ;
            void write_Product_Definition(string type,euid num);
            void write_Product_Add();
			//文件尾部逻辑
			void write_End();
		
	};
}
