#include<pcb3d/triangulation.hpp>
#include<math.h>
namespace hwpcb {

    //析构函数
     Triangulation::~Triangulation() { shell = nullptr; }

    //对Layer离散化，将三角化之后的信息保存到成员变量中
	void Triangulation::Tri_Layer(Layer3D* layer3d) {
        //对两个数组进行清空
        PointsList.clear();
        IndexesList.clear();
      
        //获取Shell数组
        vector<PCBShell*>& ShellList = layer3d->get_ShellList();
      
        for (int i = 0, shellcount = ShellList.size(); i < shellcount; i++) {
         
            //先调用Shell离散化函数
            Tri_shell(ShellList[i]);

            //获取Points和indexes的信息
            PointsList.push_back(get_Tri_Points());
            IndexesList.push_back(get_Tri_Index());
      
        }

       //遍历Trail3D中的Shell，获取其网格信息
        for (int i = 0, trailcount = layer3d->get_TrailList().size(); i < trailcount; i++) {
            vector<PCBShell*>& SegShellList = layer3d->get_TrailList()[i]->get_ShellList();
            for (int j = 0, shellcount = SegShellList.size(); j < shellcount; j++) {
                
                //先调用Shell离散化函数
                Tri_shell(SegShellList[j]);
                PointsList.push_back(get_Tri_Points());
                IndexesList.push_back(get_Tri_Index());
            }
      
        }
    }

    //对Shell进行离散化
    void Triangulation::Tri_shell(PCBShell* _shell) {
      shell = _shell;
      //不同类型的Shell有不同的离散化方法
      if (shell->ShellType() == PCBShell::FaceShell) {
      
        try {
            if (!discret_Edge(shell->BottomFace()->Face()[0][0],shell->PointList()))
                  throw DiscretException(shell->ID);
        } catch (PCBException& e) {
                std::cout << "Error: " << e.what() << std::endl;
        }
      }    
      else {
        try {
            if (!discret_Shell(shell))
                throw DiscretException(shell->ID);
        } catch (PCBException& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
      }
    }

    //获取Shell的点数组，调用这个函数之前，先得调用Tri_shell
    vector<V3d> Triangulation::get_Tri_Points() {
        int verticesize = vertices.size();

        //预分配内存
        vector<V3d> points(2 * verticesize);

        //填充顶点数组
        for (int i = 0; i < verticesize; i++) {
            points[i] = V3d(vertices[i].x, vertices[i].y, shell->BaseHeight());
            points[i + verticesize] = V3d(vertices[i].x, vertices[i].y, shell->TopHeight());
        }

        return points;
    }
    
    //获取Shell的索引数组，调用这个函数之前，先得调用Tri_shell
    vector<uint> Triangulation::get_Tri_Index(int baseindex) {

        //处理FaceShell的情况
        if (shell->ShellType() == PCBShell::FaceShell) {

            vector<uint> index(vertices.size() * 6);
            int verticesize = vertices.size();
            //填充侧面网格索引
            for (int i = 0, edgessize = edges.size(); i < edgessize; i++) {
                index[6 * i] = edges[i].x+ baseindex;
                index[6 * i + 1] = edges[i].y + baseindex;
                index[6 * i + 2] = edges[i].x + verticesize + baseindex;
                index[6 * i + 3] = edges[i].y + baseindex;
                index[6 * i + 4] = edges[i].y + verticesize + baseindex;
                index[6 * i + 5] = edges[i].x + verticesize + baseindex;
            }

            return index;
        }

        //调用外部函数库进行三角化
        CDT::Triangulation<double> cdt;
        try {
            cdt.insertVertices(vertices.begin(), vertices.end(), [](const V2d& v) { return v.x; },[](const V2d& v) { return v.y; });
            cdt.insertEdges(edges.begin(), edges.end(), [](const V2i& e) { return e.x; },[](const V2i& e) { return e.y; });
            cdt.eraseOuterTrianglesAndHoles();
        }catch (const std::exception& e) {
            std::clog << "Geometry can't Generate Tri-Mesh, Shellindex: "+to_string(shell->ID) <<endl;
        }
        //记录三角形数量
        int trisize = cdt.triangles.size();

        //记录顶点数量
        int verticesize = vertices.size();

        //预分配内存
        vector<uint> index(6 * trisize + 6 * verticesize);

        //顶面的网格索引
        for (int i = 0; i < trisize; i++) {
            index[3 * i] = cdt.triangles[i].vertices[0] + baseindex;
            index[3 * i + 1] = cdt.triangles[i].vertices[1] + baseindex;
            index[3 * i + 2] = cdt.triangles[i].vertices[2] + baseindex;
        }
        //底面的网格索引
        for (int i = 0; i < trisize; i++) {
            index[3 * i + 3 * trisize] = cdt.triangles[i].vertices[0] + verticesize + baseindex;
            index[3 * i + 3 * trisize+1] = cdt.triangles[i].vertices[1] + verticesize + baseindex;
            index[3 * i + 3 * trisize+2] = cdt.triangles[i].vertices[2] + verticesize + baseindex;
        }

        //填充侧面网格索引
        for (int i = 0, edgessize = edges.size() ; i < edgessize; i++) {
            int basenumber = trisize + i; 
            index[6 * basenumber] = edges[i].x + baseindex;
            index[6 * basenumber + 1] = edges[i].y + baseindex;
            index[6 * basenumber + 2] = edges[i].x + verticesize + baseindex;
            index[6 * basenumber + 3] = edges[i].y + baseindex;
            index[6 * basenumber + 4] = edges[i].y + verticesize + baseindex;
            index[6 * basenumber + 5] = edges[i].x + verticesize + baseindex;
        }

        return index;
    }

    //如果是正常的几何体，离散化所得的点就是底面的轮廓线
    //如果不是，那离散化所得的点只是一条线上的点
    bool Triangulation::discret_Shell(PCBShell* shell) {

        clear();
        vector<V2d>& pointslist = shell->PointList();
        discret_Face(shell->BottomFace(), pointslist);
        return 1;
    }

    //离散化圆弧和圆
    bool Triangulation::discret_Edge(PCBEdge* edge, vector<V2d>& pointslist) {
        clear();
        if (edge->edgetype() == PCBEdge::ArcEdge_CCW ||
            edge->edgetype() == PCBEdge::ArcEdge_CW) {
            discret_ARC(edge, 20, 0, pointslist);
        } else if (edge->edgetype() == PCBEdge::CircleEdge) {
            discret(pointslist[edge->center()], edge->radius(), 0, 2 * M_PI, 50, 0);
            edges.pop_back();
            edges.push_back(V2i(49, 0));
        }
        return 1;
    }

    //离散化圆弧，将点依次放到vertexs里面，第一个点和最后一个点是圆弧的起始点和结束点
    void Triangulation::discret_ARC(PCBEdge* edge, int n, int base, vector<V2d>& pointslist) {

        double r = edge->radius();
        double angle_begin = angle(edge->center(), edge->head(), r, pointslist);
        double angle_end = angle(edge->center(), edge->tail(), r, pointslist);

        if (edge->edgetype() == PCBEdge::ArcEdge_CW) {

            //如果结束角度小于起始角度，那么该顺时针圆弧必然不通过x轴正方向
            if (angle_end < angle_begin) {
                discret(pointslist[edge->center()], r, angle_begin, angle_end, n, base);
            } else if (angle_end > angle_begin) {
                
                //如果起始点角度是0
                if (angle_begin == 0) {
                    discret(pointslist[edge->center()], r, 2 * M_PI, angle_end, n, base);

                //如果结束点角度是2*pi
                }else if (angle_end > 6.18318) {
                    discret(pointslist[edge->center()], r, angle_begin, 0, n, base);

                //不是以上两种情况
                } else {
                    discret(pointslist[edge->center()], r, angle_begin, 0, n / 2, base);
                    discret(pointslist[edge->center()], r, 2 * M_PI, angle_end, n / 2,base + n / 2);
                }
            }
        //处理逆时针圆弧的情况
        } else if (edge->edgetype() == PCBEdge::ArcEdge_CCW) {
            if (angle_end > angle_begin) {
                discret(pointslist[edge->center()], r, angle_begin, angle_end, n, base);
            } else if (angle_end < angle_begin) {
                if (angle_end == 0) {
                    discret(pointslist[edge->center()], r, angle_begin, 2 * M_PI, n, base);
                } else if (angle_begin > 6.18318) {
                    discret(pointslist[edge->center()], r, 0, angle_end, n, base);
                } else {
                    discret(pointslist[edge->center()], r, angle_begin, 2 * M_PI, n / 2,base);
                    discret(pointslist[edge->center()], r, 0, angle_end, n / 2,base + n / 2);
                }
            }
        }
        //加上最后一个顶点
        vertices.push_back(pointslist[edge->tail()]);
    }

    //n是离散化的段数，angle_begin必须要小于angle_end
    void Triangulation::discret(V2d Arc_center, double r, double angle_begin, double angle_end,int n, int base) {
       
        //角度间隔，根据离散化的份数计算角度步长
        double small_angle = (angle_end - angle_begin) / n;
        //保留头节点、不保留尾部的节点
        for (int i = 0; i < n; i++) {
            //计算顶点位置，一小步小步的往前走
            V2d p(Arc_center.x + r * std::cos(angle_begin + small_angle * i),
                  Arc_center.y + r * std::sin(angle_begin + small_angle * i));

            //放入顶点
            vertices.push_back(p);
            //添加边
            edges.push_back(V2i(base + i, base + i + 1));
        }
    }

    //离散化平面，将一个包含圆弧的二维闭合形状，变成一个多边形
    void Triangulation::discret_Face(PCBFace* face, vector<V2d>& pointslist) {

    vector<PCBLoop>& planeface = face->Face();
    for (int i = 0; i < planeface.size(); i++) {

      int edgenumber = 0;
      for (int j = 0; j < planeface[i].size(); j++) {
        PCBEdge* edge = planeface[i][j];
        if (edge->edgetype() == PCBEdge::LineEdge) {
          // 只添加每条边的头节点
          vertices.push_back(pointslist[edge->head()]);

          edges.push_back(V2i(edgenumber + number, edgenumber + number + 1));
          edgenumber += 1;
        } else if (edge->edgetype() == PCBEdge::ArcEdge_CCW ||
                   edge->edgetype() == PCBEdge::ArcEdge_CW) {
          discret_ARC(edge, 20, edgenumber + number, pointslist);
          vertices.pop_back();
          edgenumber = edgenumber + 20;

        } else if (edge->edgetype() == PCBEdge::CircleEdge) {
          //将圆弧离散化为24面体
          discret(pointslist[edge->center()], edge->radius(), 0, 2 * M_PI, 24,number + edgenumber);
          edgenumber = edgenumber + 24;
        }
      }
      // 最后一条边的索引错了，所以修改一下
      // 错在最后一个顶点没有，最后一个顶点实际上是第一个
      edges.pop_back();
      edges.push_back(V2i(number + edgenumber - 1, number));

      // number从planeface[i].size()后面开始计数
      number = number + edgenumber;
    }
  }

    //计算角度函数，角度值限制在0~2*pi
    double Triangulation::angle(int _center, int _point, double r, vector<V2d>& pointslist) {
        V2d center = pointslist[_center];
        V2d point = pointslist[_point];
   
        //计算正弦值
        double sin = (point.y - center.y) / r;
    
        //处理精度，因为精度问题 sin的值可能为1.000001导致程序崩溃
        if (sin > 1) sin = 1;
        else if (sin < -1) sin = -1;

        double theangle = asin(sin);

        //根据正弦值，将角度划分到不同的象限中
        //第一象限不用处理
        if ((point.x - center.x) >= 0) {
          // 第四象限处理
          if (theangle < 0)
            theangle = theangle + M_PI * 2;

          //第二,第三象限处理方式相同
        } else {
            theangle = M_PI - theangle; 
        }

        return theangle;
    }

    //离散化新的Shell之前需要清空一下前一个shell保存的信息
    void Triangulation::clear() {
        vertices.clear();
        edges.clear();
        number = 0;
    }

}