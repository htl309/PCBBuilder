#include<pcb3d/pcbshell.hpp>

namespace hwpcb {

        //Shell构造函数。注意！二维点数组的填充需要在创建Shell之后进行填充
        PCBShell::PCBShell(PCBShellType _shelltype, euid _ID, real _baseheight,real _topheight,PCBFace* _bottom, int pointscount)
            : m_shelltype(_shelltype), ID(_ID), m_baseheight(_baseheight),
                m_topheight(_topheight), m_bottom(_bottom), m_points(pointscount) {}

        //悬挂面构造函数
        PCBShell::PCBShell(euid _ID, real _baseheight, real _topheight, PCBFace* _bottom)
            : m_shelltype(FaceShell), ID(_ID), m_baseheight(_baseheight),
              m_topheight(_topheight), m_bottom(_bottom) {}

        //析构函数
        PCBShell::~PCBShell() {delete m_bottom;}

        //获取二维点数组的数量，该数量是shell中包含点数量的一半
        uint PCBShell::PointCount() { return m_points.size(); }

        //获取二维点数组
        vector<V2d>& PCBShell::PointList() { return m_points; }

        //获取基线高度
        const real& PCBShell::BaseHeight() const { return m_baseheight; }

        //获取顶面高度
        const real& PCBShell::TopHeight() const { return m_topheight; }

        //获取Shell的类型
        PCBShell::PCBShellType PCBShell::ShellType() { return m_shelltype; }

        //获取指向底面的指针
        PCBFace* PCBShell::BottomFace() { return m_bottom; }


        //获取侧面
        vector<PCBFace*> PCBShell::GetSideFace() {
          vector<PCBFace*> sidefaces;

          //这个是位移，因为二维点数组中就记录了一半的点，想要获取顶面的点就必须加个位移。
          auto offset = m_points.size();

          //获取底面的圈数组
          vector<PCBLoop>& bottomface = m_bottom->Face();

          //遍历圈数组
            for (int i = 0 ,bottomloopsize=bottomface.size(); i < bottomloopsize; i++) {

            PCBLoop loop = bottomface[i];
            
            //遍历某一个圈中的边
                for (int j = 0, loopsize = loop.size(); j < loopsize; j++) {

                    PCBEdge* bottomedge = loop.at(j);

              //对不同类型的边进行不同的处理
              //如果线是直线段类型
                  if (bottomedge->edgetype() == PCBEdge::LineEdge) {
                    PCBEdge* topedge = new PCBEdge(bottomedge->head() + offset,
                                                   bottomedge->tail() + offset,
                                                   bottomedge->direction());
                    //创建由直线段拉伸而成的侧面，并把它放入侧面数组中
                    sidefaces.push_back(GetSidePlaneFace(bottomedge, topedge));
                    delete topedge;
                  } else if (bottomedge->edgetype() == PCBEdge::ArcEdge_CCW) {
                    //创建由圆弧段拉伸而成的侧面，并把它放入侧面数组中
                    sidefaces.push_back(GetSideArcFace(bottomedge, offset));
                  } else if (bottomedge->edgetype() == PCBEdge::ArcEdge_CW) {
                    //创建由圆弧段拉伸而成的侧面，并把它放入侧面数组中
                    sidefaces.push_back(GetSideArcFace(bottomedge, offset));
                  } else if (bottomedge->edgetype() == PCBEdge::CircleEdge) {

                    //创建由圆拉伸而成的侧面，并把它放入侧面数组中
                    sidefaces.push_back(GetSideCircleFace(bottomedge, offset));
                  }
                }
            }
          //返回侧面数组
          return sidefaces;
        }

        //获取顶面
        PCBFace* PCBShell::GetTopFace() {
          //获取位移
          auto offset = m_points.size();
          vector<PCBLoop>& bottomface = m_bottom->Face();

          //创建顶面
          PCBFace* topface =new PCBFace(m_bottom->Origin() + offset, m_bottom->Normal());

          //获取顶面的圈数组，准备往里面填充
          vector<PCBLoop>& topfaceloop = topface->Face();


          //预留好内存，节约时间
          topfaceloop.resize(bottomface.size());

          //开始遍历底面，按照底面的拓扑关系填充顶面
          for (int i = 0,bottomloopsize=bottomface.size(); i < bottomloopsize; i++) {
            PCBLoop loop = bottomface[i];
            PCBLoop planeloop(loop.size());

            //遍历圈
            for (int j = 0, loopsize = loop.size(); j < loopsize; j++) {

              PCBEdge* bottomedge = loop[j];

              //对不同类型的边进行不同的处理
              if (bottomedge->edgetype() == PCBEdge::LineEdge) {

                //创建顶边，拓扑关系和底边一致，只是顶点的索引加上一个位移即可
                PCBEdge* topedge = new PCBEdge(bottomedge->head() + offset,bottomedge->tail() + offset, bottomedge->direction());

                //将顶边放入数组中，由于提前分配好了内存，所以这里不需要用push_back的方法
                planeloop[j] = topedge;
              } else if (bottomedge->edgetype() == PCBEdge::ArcEdge_CCW||
                  bottomedge->edgetype() == PCBEdge::ArcEdge_CW) {
                
                  //创建顶边并放入数组
                PCBEdge* topedge = new PCBEdge(bottomedge->edgetype(), bottomedge->head() + offset, bottomedge->tail() + offset,bottomedge->center() + offset, bottomedge->radius());
                planeloop[j] = topedge;
              } else if (bottomedge->edgetype() == PCBEdge::CircleEdge) {
                
                  //创建顶边并放入数组
                  PCBEdge* topedge = new PCBEdge(bottomedge->head() + offset, bottomedge->center() + offset, bottomedge->radius());
                planeloop[j] = topedge;
              }
            }

            //将创建完成的一条圈放入圈数组中，因为存在多条圈。(第一条是轮廓之后的都是洞)
            topfaceloop[i] = planeloop;
          }

          return topface;
        }

        //平面侧面
        //由一条顶边和一条底边创建侧面，这里创建的是平面侧面
        PCBFace* PCBShell::GetSidePlaneFace(PCBEdge* edge1, PCBEdge* edge2) {

            //创建圈
		    PCBLoop loop;       
			
            //加上两条侧边
            //四条边必须满足首位相接
			PCBEdge* bottomedge = new PCBEdge(edge1->head(), edge1->tail(), edge1->direction());
			PCBEdge* side_edge1 = new PCBEdge(edge1->tail(), edge2->tail(), V3d(0, 0, 1));
			PCBEdge* topedge    = new PCBEdge(edge2->tail(), edge2->head(), -edge2->direction());
			PCBEdge* side_edge2 = new PCBEdge(edge2->head(), edge1->head(), V3d(0, 0, 1));

            //将边依次加入到圈中
			loop.push_back(bottomedge);
			loop.push_back(side_edge1);
			loop.push_back(topedge);
			loop.push_back(side_edge2);
               
            //创建顶面
			PCBFace* face = new PCBFace(edge1->head(), edge1->direction().cross(  V3d(0, 0, 1)));
			vector<PCBLoop>& loops = face->Face(); 
            
            //侧面没有洞，因此只有一条圈，直接push_back即可
            loops.push_back(loop);
                     
            //顶面指针
			return face;
		}

        //圆弧侧面
        //由一条顶边和一个位移创建侧面，这里创建的是圆弧柱面拉升而成的侧面
		PCBFace* PCBShell::GetSideArcFace(PCBEdge* edge1, uint offset) {
                 
            //创建侧面
            //按照创建平面侧面的逻辑进行圆弧侧面的创建
			PCBFace* face = new PCBFace(edge1->center(), edge1->radius());
            PCBLoop loop ;
			
            // 创建四条首尾相连的边
            // 注意圆弧的顺时针和逆时针
            // 这里之所以要对圆弧的类型进行判断，而不是直接用构造函数赋值
 
            PCBEdge* bottomedge= new PCBEdge( edge1->edgetype() ,edge1->head(), edge1->tail(), edge1->center(), edge1->radius());;
          
            PCBEdge* side_edge1 = new PCBEdge(edge1->tail(), edge1->tail() + offset, V3d(0, 0, 1));
 
            PCBEdge* topedge =
                new PCBEdge(edge1->edgetype() == PCBEdge::ArcEdge_CCW?PCBEdge::ArcEdge_CW:PCBEdge::ArcEdge_CCW ,
                            edge1->tail() + offset, edge1->head() + offset,
                            edge1->center() + offset, edge1->radius());
           
			PCBEdge* side_edge2 =new PCBEdge(edge1->head() + offset, edge1->head(), V3d(0, 0, 1));
           
            //将边依次加入到圈中
            loop.push_back(bottomedge);
		    loop.push_back(side_edge1);
            loop.push_back(topedge);
            loop.push_back(side_edge2);
            face->Face().push_back(loop);

            //侧面指针
			return  face;
		}

        //圆柱侧面
        //由一条顶边和一条底边创建侧面，这里创建的是圆柱侧面
		PCBFace* PCBShell::GetSideCircleFace(PCBEdge* edge1, uint offset) {

            //创建圆柱侧面，原点与半径与底边相同
			PCBFace* face = new PCBFace(edge1->center(), edge1->radius());

            PCBEdge* bottomedge = new PCBEdge(edge1->head(),edge1->center(),edge1->radius());
            PCBEdge* topedge = new PCBEdge(edge1->head()+offset,edge1->center()+ offset,edge1->radius());

            //这里无需创建侧边
            //圆柱侧面将底面看作轮廓线，将顶面看作洞，也就是说顶面和底面两个圆的顺逆时针情况都不同
            //这是按照Step标准设计的
            PCBLoop loop1 ;
            PCBLoop loop2 ;
            loop1.push_back(bottomedge);
            loop2.push_back(topedge);
            face->Face().push_back(loop1);
            face->Face().push_back(loop2);

            //侧面指针
			return  face;
		}
    }