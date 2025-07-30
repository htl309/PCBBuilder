#include<pcb3d/stepwriter.hpp>

namespace hwpcb {

	//构造函数
	StepWriter::StepWriter(){};

	//析构函数
	StepWriter::~StepWriter(){}

	//输出函数，只需要三维板和输出路径，就能输出Step文件到该路径下
	//路径name包含文件名称
	PCBError StepWriter::write_Step(Board3D& board3d, string name) {
		
		//Step标准，索引从1开始
        number = 1;

		//打开文件
		ofs.open(name, ios::out);

		//写入头文件逻辑
		write_Begin();
		try {
        ofs << "#" + to_string(number++) + " = (LENGTH_UNIT()NAMED_UNIT(*)SI_UNIT(.MILLI.,.METRE.));" << endl;
        ofs << "#" + to_string(number++) + " = (NAMED_UNIT(*)PLANE_ANGLE_UNIT()SI_UNIT($,.RADIAN.));" << endl;
		ofs << "#" + to_string(number++) + " = (NAMED_UNIT(*)SI_UNIT($,.STERADIAN.)SOLID_ANGLE_UNIT());" << endl;
		ofs << "#" + to_string(number++) + " = UNCERTAINTY_MEASURE_WITH_UNIT(LENGTH_MEASURE(0.01),#9,'DISTANCE_ACCURACY_VALUE','Maximum Tolerance applied to model');" << endl;
		ofs << "#" + to_string(number++) + " = (GEOMETRIC_REPRESENTATION_CONTEXT(3)GLOBAL_UNCERTAINTY_ASSIGNED_CONTEXT((#10))GLOBAL_UNIT_ASSIGNED_CONTEXT((#7,#8,#9))REPRESENTATION_CONTEXT('STEP 203','TOP_LEVEL_ASSEMBLY_PART'));" << endl;

		euid temp = number - 2;
		//调用函数，逐层写入            
		
		for (int i = 0,boardsize=board3d.size(); i < boardsize; i++) {
			write_Product_Definition("Layer",i + 1);

            if (!write_Layer_to_Step(board3d[i])) {
						throw StepWriterException(i);
				}

            }
       
		
			//记录一下体索引数组的长度
			uint SOLIDSIZE = MANIFOLD_SOLID_BREP.size();
		write_End();
                } 
		catch (const PCBException& e) {
                  std::cout << "Error: " << e.what() << std::endl;
                }
                return PCBError::Success;
			
	}

	//写入文件头部逻辑
	void StepWriter::write_Begin() {

		ofs << "ISO-10303-21;" << endl;
		ofs << "HEADER;" << endl;
		ofs << "FILE_DESCRIPTION (('PCB3D'), '');" << endl;
		ofs << "FILE_NAME ('', '2024-09', ('SDU'), ('SDU Software Institute'), '', 'C++', '');" << endl;
		ofs << "FILE_SCHEMA (('CONFIG_CONTROL_DESIGN'));" << endl;
		ofs << "ENDSEC;" << endl;
		ofs << endl;
		ofs << "DATA;" << endl;

		//写入一些常用的向量方向元素，减少一些冗余
		ofs << "#" + to_string(number++) + " = DIRECTION('', (1.0, 0.0, 0.0)); " << endl;
		ofs << "#" + to_string(number++) + " = DIRECTION('', (0.0, 1.0, 0.0)); " << endl;
		ofs << "#" + to_string(number++) + " = DIRECTION('', (0.0, 0.0, 1.0)); " << endl;
		ofs << "#" + to_string(number++) + " = VECTOR('',#" + to_string(number - 3) + ",1.); " << endl;
		ofs << "#" + to_string(number++) + " = VECTOR('',#" + to_string(number - 3) + ",1.); " << endl;
		ofs << "#" + to_string(number++) + " = VECTOR('',#" + to_string(number - 3) + ",1.); " << endl;
		ofs << "#" + to_string(number++) + " =CARTESIAN_POINT('',(0.,0.,0.));" << endl;
        ofs << "#" + to_string(number++) + " = AXIS2_PLACEMENT_3D('',#7,#3,#1);"<< endl;
	}
	void StepWriter::write_Product_Definition(string type, euid num) {
        

		if (type == "shell")
			type = "";
        ofs << "#" + to_string(number++) + " = APPLICATION_CONTEXT('part');"<< endl;
        ofs << "#" + to_string(number++) + " = MECHANICAL_CONTEXT('none',#"<< number - 2 << ",'component');" << endl;
		ofs << "#" + to_string(number++) + " = PRODUCT('" << type<<num<< "','" << type<<num << "','',(#" << number - 2 << "));" << endl;
        ofs << "#" + to_string(number++) + " = PRODUCT__RELATED_PRODUCT_CATEGORY('detail','',(#"<< number - 2 << "));" << endl;
        ofs << "#" + to_string(number++) + " = PRODUCT_DEFINITION_FORMATION_WITH_SPECIFIED_SOURCE('ANY','UNSPECIFIED',#"<< number - 3 << ",.NOT_KNOWN.);" << endl;
        ofs << "#" + to_string(number++) + " = APPLICATION_CONTEXT('part');"<< endl;
        ofs << "#" + to_string(number++) + " = APPLICATION_PROTOCOL_DEFINITION('international ""standard','config_control_design',1994,#"<< number - 2 << ");" << endl;
        ofs << "#" + to_string(number++) + " = DESIGN_CONTEXT('detailed',#" << number - 3 <<",'design');"<< endl;
        ofs << "#" + to_string(number++) + " = PRODUCT_DEFINITION('','UNSPECIFIED',#"<< number - 5 << ",#" << number - 2 << ");" << endl;
        ofs << "#" + to_string(number++) + " = PRODUCT_DEFINITION_SHAPE('NONE','NONE',#"<< number - 2 << ");" << endl;
        ofs << "#" + to_string(number++) + " = SHAPE_REPRESENTATION('"<< type<< num << "',(#8),#13);" << endl;
        ofs << "#" + to_string(number++) + " = SHAPE_DEFINITION_REPRESENTATION(#"<< number - 3 << ",#" << number - 2 << ");" << endl;
	}
	void StepWriter::write_Product_Add() {
          ofs << "#" + to_string(number++) + " =  ITEM_DEFINED_TRANSFORMATION('', '', #8, #8);" << endl;
          ofs << "#" + to_string(number++) + " =  (REPRESENTATION_RELATIONSHIP('','',#" << shape_shell_id<<",#"<<shape_layer_id<<")REPRESENTATION_RELATIONSHIP_WITH_TRANSFORMATION(#" << number - 2<<")SHAPE_REPRESENTATION_RELATIONSHIP());" << endl;;
          ofs << "#" + to_string(number++) + " =  NEXT_ASSEMBLY_USAGE_OCCURRENCE('', '', '', #" << shape_layer_id - 2 <<",#" <<shape_shell_id - 2 <<",$ );" << endl;
          ofs << "#" + to_string(number++) + " =  PRODUCT_DEFINITION_SHAPE('', '', #" << number - 2 <<");" << endl;
          ofs << "#" + to_string(number++) + " =  CONTEXT_DEPENDENT_SHAPE_REPRESENTATION(#" << number - 4 <<",#" << number - 2 <<");" << endl;
		}
	//逐层写入函数
    bool StepWriter::write_Layer_to_Step(Layer3D* layer3d) {
		shape_layer_id = number - 2;
		//获取Layer中包含Shell的数组
		vector<PCBShell*>& shelllist = layer3d->get_ShellList();
		vector<Trail3D*>& trail3d = layer3d->get_TrailList();
		int z = 0;
			//遍历这些数组，依次往Step里面写入Shell
			for (int i = 0,listsize=shelllist.size(); i < listsize; i++) {
                write_Product_Definition("shell", shelllist[i]->ID);
                shape_shell_id = number - 2;
				//先写入Shell中的点，因为点是统一存放在Shell中
				write_PointList_in_Shell(shelllist[i]);
				//写入Shell，需要保存一下写入Shell的索引，之后加入到产品描述中
				MANIFOLD_SOLID_BREP.push_back(write_Shell_to_Step(shelllist[i]));
				write_Product_Add();
				//清空一下数组，不能对写入的下一个Shell产生干扰
				wirte_NewShell();
			}

			//同上面一个循环，逻辑是一样的
			for (int i = 0,trialsize=trail3d.size(); i < trialsize; i++) {
				for (int j = 0,listsize=trail3d[i]->get_ShellList().size(); j < listsize;j++) {
                    write_Product_Definition("shell", trail3d[i]->get_ShellList()[j]->ID);
					shape_shell_id = number - 2;
					write_PointList_in_Shell(trail3d[i]->get_ShellList()[j]);	
					MANIFOLD_SOLID_BREP.push_back(write_Shell_to_Step(trail3d[i]->get_ShellList()[j]));
					write_Product_Add();
                    wirte_NewShell();
                }
            }
		uint SOLIDSIZE = MANIFOLD_SOLID_BREP.size();
		//将写完的Shell写入产品中
		ofs << "#" + to_string(number++) + " = ADVANCED_BREP_SHAPE_REPRESENTATION('',(#";
		if (SOLIDSIZE > 1) {
			for (int i = 0; i < SOLIDSIZE - 1; i++) ofs << to_string(MANIFOLD_SOLID_BREP[i]) + ", #";
		}
		ofs << to_string(MANIFOLD_SOLID_BREP[SOLIDSIZE - 1]) + "),#11);" << endl;
        ofs << "#" + to_string(number++) +" = SHAPE_REPRESENTATION_RELATIONSHIP('NONE','NONE', #"<< shape_layer_id << ",#" << number - 2 << ");" << endl;
        
		MANIFOLD_SOLID_BREP.clear();
		return 1;
    }

	//写入Shell
    euid StepWriter::write_Shell_to_Step(PCBShell* shell) {

        // 用于记录面索引的数组
		vector<euid> ADVANCED_FACE;

        // FaceShell需要特殊处理
        if (shell->ShellType() != PCBShell::FaceShell) {

			// 获取顶面和侧面并写入
                        PCBFace* topface = shell->GetTopFace();
			ADVANCED_FACE.push_back(write_Face_to_Step(topface));
			vector<PCBFace*> sidefaces = shell->GetSideFace();
			
			//写入侧面
			for (int i = 0,sidefacesize=sidefaces.size(); i < sidefacesize; i++)
				ADVANCED_FACE.push_back(write_Face_to_Step(sidefaces[i]));

			//释放顶面和侧面的内存
            delete topface;
            for (PCBFace* i : sidefaces)  delete i;
            sidefaces.clear();
        }
            // 写入Shell的底面或FaceShell的悬挂面
		ADVANCED_FACE.push_back(write_Face_to_Step(shell->BottomFace()));

       // 将写入面依次写入Shell，因为Shell就是由面组成的
       ofs << "#" + to_string(number++) + " = CLOSED_SHELL('',(#";
       for (int i = 0,FACESIZE=ADVANCED_FACE.size(); i < FACESIZE - 1; i++)  ofs << to_string(ADVANCED_FACE[i]) + ", #";
       ofs << to_string(ADVANCED_FACE[ADVANCED_FACE.size() - 1]) + "));"<< endl;

       ofs << "#" + to_string(number++) + " = MANIFOLD_SOLID_BREP('',#" +to_string(number - 1) + ");"<< endl;
       ofs << "#" + to_string(number++) +" = ADVANCED_BREP_SHAPE_REPRESENTATION('',(#"<< number - 2 << "),#11);" << endl;
       ofs << "#" + to_string(number++) +" = SHAPE_REPRESENTATION_RELATIONSHIP('NONE','NONE', #"<< shape_shell_id << ",#" << number - 2 << ");" << endl;
       //返回Shell的索引，需要减1，因为最后一步number++了
       return number - 1;
    }

	//将Shell中的点写入Step文件
	void StepWriter::write_PointList_in_Shell(PCBShell* shell) {

		//获取点数组
		vector<V2d>& pointslist = shell->PointList();
		//依次写入点
		//需要注意：点数组中的x，y和shell包含的高度值组成一个三维点进行写入
		//先和基线高度组合，进行写入
		for (int i = 0,listsize=pointslist.size(); i < listsize; i++) {
			V2d v = pointslist[i];
			string str = "#" + to_string(number) +
                             " = CARTESIAN_POINT ('', (" + to_string(v.x) +
                             "," + to_string(v.y) + "," +
                             to_string(shell->BaseHeight()) + "));";
            ofs << str << endl;
            //记录索引
			CARTESIAN_POINT.push_back(number);
            number++;
        }
		//与顶面高度组合进行写入
        for (int i = 0, listsize = pointslist.size(); i < listsize; i++) {
			V2d v = pointslist[i];
			string str = "#" + to_string(number) +
                             " = CARTESIAN_POINT ('', (" + to_string(v.x) +
                             "," + to_string(v.y) + "," +
                             to_string(shell->TopHeight()) + "));";
			ofs << str << endl;
			//记录索引
			CARTESIAN_POINT.push_back(number);
			number++;
        }
		//写入向量点，之后可能会用
        for (int i = 0, POINTSIZE= CARTESIAN_POINT.size(); i < POINTSIZE; i++) {
			string str = "#" + to_string(number) +
                             " = VERTEX_POINT ('', #" +
                             to_string(CARTESIAN_POINT[i]) + ");";
			ofs << str << endl;
			//记录索引
			VERTEX_POINT.push_back(number);
			number++;
        }
    }

	//将Face写入Step文件
	euid StepWriter::write_Face_to_Step(PCBFace* pcbface) {
        
		//获取Face最重要的两个几何属性：原点和法向
		euid CARTESIAN_POINT_ID = CARTESIAN_POINT[pcbface->Origin()];
		euid DIRECTION = write_DIRECTION(pcbface->Normal());

		////写入面所在的平面，以及所需要的3D坐标系
		ofs << "#" + to_string(number++) + " = AXIS2_PLACEMENT_3D('', #" + to_string(CARTESIAN_POINT_ID) + ", #" + to_string(DIRECTION) + ", " + "#" + to_string((DIRECTION == 1) + 1) + ");" << endl;

		//根据面的类型不同，在Step里写入不同拓扑属性的面，平面和侧面
        if (pcbface->FaceType() == PCBFace::PlaneFace)
			ofs << "#" + to_string(number++) + " = PLANE('', #" + to_string(number - 1) + ");" << endl;
        else 
			ofs << "#" + to_string(number++) + " = CYLINDRICAL_SURFACE('', #" + to_string(number - 1) + "," + to_string(pcbface->Radius()) + ");" << endl;
        
		//保留平面的stp索引，后续写入ADVANCED_FACE需要用到
		euid PLANE = number - 1;
        
		vector<PCBLoop> thisface = pcbface->Face();
		//记录面的索引，这里的面不仅仅指轮廓线，也包括洞
		vector<euid> FACE_BOUND;
		for (int i = 0,facesize = thisface.size(); i < facesize; i++) {

			//***写入每一条边
			//保存边的ORIENTED_EDGE索引
			vector<euid> ORIENTED_EDGE;
			for (int j = 0,faceisize=thisface[i].size(); j < faceisize; j++) ORIENTED_EDGE.push_back(write_Edge_to_Step(thisface[i][j]));

			//***写入loop
			ofs << "#" + to_string(number++) + " = EDGE_LOOP('', (#";
			//要是是个圆的话，就只有一条边，就不需要执行循环中的东西了
			if (ORIENTED_EDGE.size() > 1) {
				//将保存在ORIENTED_EDGE里的有向边索引依次写入该loop中
				for (int j = 0,faceisize=thisface[i].size(); j < faceisize - 1; j++)  ofs << to_string(ORIENTED_EDGE[j]) + ", #";
			}
			ofs << to_string(ORIENTED_EDGE[ORIENTED_EDGE.size() - 1]) + "));" << endl;

			//记录一下FACE_BOUND的索引，为了之后写ADVANCED_FACE使用
			FACE_BOUND.push_back(number);
			//***写入FACE_BOUND，第二条以及之后的都是洞，圆的方向要变成F
            //ORIENTED_EDGE.size() ==1就证明是圆
	             if (i > 0 && ORIENTED_EDGE.size() ==1)
                 ofs << "#" + to_string(number++) + " = FACE_BOUND('',#" + to_string(number - 1) + ",.F.); " << endl;
                  else
                 ofs << "#" + to_string(number++) + " = FACE_BOUND('',#" + to_string(number - 1) + ",.T.); " << endl;
		
		}
          
		//整合上面的循环写入的loop，将loop全写入FACE_BOUND中
		ofs << "#" + to_string(number++) + " = ADVANCED_FACE('', (#";
		//有洞的话就进入循环进行写入，没有就不需要写入了
		if (FACE_BOUND.size() > 1) {
			for (int i = 0 ,BOUNDSIZE=FACE_BOUND.size(); i < BOUNDSIZE - 1; i++) ofs << to_string(FACE_BOUND[i]) + ", #";
		}
		ofs << to_string(FACE_BOUND[FACE_BOUND.size() - 1]) + "), #" + to_string(PLANE) + ", .T.);" << endl;
              
		return number - 1;
	}
	
	//写入边
    euid StepWriter::write_Edge_to_Step(PCBEdge* edge) {
		// 根据不同类型的边，采用不同的写入策略
		if (edge->edgetype() == PCBEdge::LineEdge)
			return write_LineEdge_to_Step(edge);
		else if (edge->edgetype() == PCBEdge::ArcEdge_CCW || 
			edge->edgetype() == PCBEdge::ArcEdge_CW)
			return write_ArcEdge_to_Step(edge);
		else if (edge->edgetype() == PCBEdge::CircleEdge)
			return write_CircleEdge_to_Step(edge);
        else
          verify(false); // 不可能。改用switch更好。
          return 0;
    }

	//写入LineEdge
	euid StepWriter::write_LineEdge_to_Step(PCBEdge* lineedge) {

		//获取边的头顶点和尾顶点的索引
		euid VERTEX_POINT_Start_ID = VERTEX_POINT[lineedge->head()];
		euid VERTEX_POINT_End_ID = VERTEX_POINT[lineedge->tail()];

		//获取边的方向并写入
		V3d direction = lineedge->direction();
		euid DIRECTION = write_DIRECTION(direction);
		euid VECTOR = write_VECTOR(DIRECTION);
		//输出这条边所在的LINE
		ofs << "#" + to_string(number++) + " = LINE('', #" + to_string(CARTESIAN_POINT[lineedge->head()]) + ", #" + to_string(VECTOR) + ");" << endl;

		//输出EDGE_CURVE
		ofs << "#" + to_string(number++) + " = EDGE_CURVE('', #" + to_string(VERTEX_POINT_Start_ID) +
			", #" + to_string(VERTEX_POINT_End_ID) +
			", #" + to_string(number - 1) + ", .T.);" << endl;

		//输出ORIENTED_EDGE
		ofs << "#" + to_string(number++) + " = ORIENTED_EDGE('', *, *, #" + to_string(number - 1) + ", .T.);" << endl;

		return number - 1;
	}

	//写入ArcEdge
	euid StepWriter::write_ArcEdge_to_Step(PCBEdge* arcedge) {
        
		//获取头尾顶点的索引
		euid VERTEX_POINT_Start_ID = VERTEX_POINT[arcedge->head()];
		euid VERTEX_POINT_End_ID = VERTEX_POINT[arcedge->tail()];
		euid CARTESIAN_POINT_Center_ID = CARTESIAN_POINT[arcedge->center()];

		//写入AXIS2_PLACEMENT_3D，局部坐标系
		ofs << "#" + to_string(number++) + " = AXIS2_PLACEMENT_3D('', #" + to_string(CARTESIAN_POINT_Center_ID) + ", #3, #1);" << endl;
		//写入CIRCLE
		ofs << "#" + to_string(number++) + " = CIRCLE( '', #" + to_string(number - 1) + ", " + to_string(arcedge->radius()) + ");" << endl;

		//截取边，根据不同的连接方式采取不同的写入策略
         if (arcedge->edgetype() == PCBEdge::ArcEdge_CCW)
			ofs << "#" + to_string(number++) + " = EDGE_CURVE('CCW', #" + to_string(VERTEX_POINT_Start_ID) +
			", #" + to_string(VERTEX_POINT_End_ID) +
			", #" + to_string(number - 1) + ", .T.);" << endl;
		else
			ofs << "#" + to_string(number++) + " = EDGE_CURVE('CW', #" + to_string(VERTEX_POINT_Start_ID) +
			", #" + to_string(VERTEX_POINT_End_ID) +
			", #" + to_string(number - 1) + ", .F.);" << endl;

		ofs << "#" + to_string(number++) + " = ORIENTED_EDGE('', *, *, #" + to_string(number - 1) + ", .T.);" << endl;
		
		return number - 1;
	}

	//写入CircleEdge
	euid StepWriter::write_CircleEdge_to_Step(PCBEdge* circleedge) {

		//获取起始点和结束点的索引
		euid VERTEX_POINT_Start_ID = VERTEX_POINT[circleedge->head()];
		euid VERTEX_POINT_End_ID = VERTEX_POINT[circleedge->tail()];

		//获取圆心索引
		euid CARTESIAN_POINT_Center_ID = CARTESIAN_POINT[circleedge->center()];

		//写入AXIS2_PLACEMENT_3D
		ofs << "#" + to_string(number++) + " = AXIS2_PLACEMENT_3D('', #" + to_string(CARTESIAN_POINT_Center_ID) + ", #3, #1);" << endl;
		//写入CIRCLE
		ofs << "#" + to_string(number++) + " = CIRCLE( '', #" + to_string(number - 1) + ", " + to_string(circleedge->radius()) + ");" << endl;

		ofs << "#" + to_string(number++) + " = EDGE_CURVE('', #" + to_string(VERTEX_POINT_Start_ID) +
				", #" + to_string(VERTEX_POINT_End_ID) +
				", #" + to_string(number - 1) + ", .T.);" << endl;

		ofs << "#" + to_string(number++) + " = ORIENTED_EDGE('', *, *, #" + to_string(number - 1) + ", .T.);" << endl;
		
		//返回索引
		return number - 1;
	}
	
	//写入方向
	euid StepWriter::write_DIRECTION(V3d direction) {
		//为了节约空间，要是方向是前三个里面的一个，那就不用重复写入了
		if (direction.y == 0 && direction.z == 0) return 1;
		else if (direction.x == 0 && direction.z == 0) return 2;
		else if (direction.x == 0 && direction.y == 0) return 3;
		else {
			string str = "#" + to_string(number++) + " = DIRECTION ('', (" + to_string(direction.x) + "," + to_string(direction.y) + "," + to_string(direction.z) + "));";
			ofs << str << endl;
			return number - 1;
		}

	}
	
	//根据方向写入向量
	euid StepWriter::write_VECTOR(euid DIRECTION) {
		//如果方向是前三个的其中一个，那就不需要重复写入了
		//减少冗余
		if (DIRECTION < 4) return DIRECTION + 3;
		else {
			string str = "#" + to_string(number++) + " = VECTOR ('', #" + to_string(DIRECTION) + ",1.);";
			ofs << str << endl;
			return  number - 1;
		}

	}
	
	//写入新Shell的准备工作
	void StepWriter::wirte_NewShell() {
		//清空索引数组，以免影响到下一个Shell的写入
         CARTESIAN_POINT.clear();
         VERTEX_POINT.clear();
    }

	//文件尾部逻辑
	void StepWriter::write_End() {
		ofs << "ENDSEC;" << endl;
		ofs << "END-ISO-10303-21;" << endl;
		//关闭文件
		ofs.close();
		//清空数组
        MANIFOLD_SOLID_BREP.clear();
        CARTESIAN_POINT.clear();
        VERTEX_POINT.clear();
	}
	
}