#include "pcb/writer.hpp"
#include "verify.hpp"
#include <iomanip>
using namespace std;

namespace hwpcb {

/**
 * @brief 实数到流的输出，能忽略多余后缀0，又保证精度无损。
 */
void printReal(ostream& ost, real x) { ost << x; }

/**
 * @brief 输出线段或弧段信息。
 */
void printSeg(ostream& ost, const V2d vertices[], const Seg& seg) {
  if (seg.isLine()) {
    const V2d& vt = vertices[seg.tailVid];
    ost << "STRAIGHT," << seg.id << ',' << seg.width << ',' << vt.x << ','
        << vt.y << '\n';
  } else if (seg.isArc()) {
    const V2d& vc = vertices[seg.centerVid];
    const V2d& vt = vertices[seg.tailVid];
    // ARC,ID,线宽,centerX,centerY,x,y,方向（CW/CCW）
    ost << "ARC," << seg.id << ',' << seg.width << ',' << vc.x << ',' << vc.y
        << ',' << vt.x << ',' << vt.y << ',' << (seg.clockwise ? "CW" : "CCW") << '\n';
  } else {
    verify(seg.isCircle()); // 目前只有3种Seg
    const V2d& vc = vertices[seg.centerVid];
    // CIRCLE,ID,线宽,centerX,centerY,半径
    ost << "CIRCLE," << seg.id << ',' << seg.width << ',' << vc.x << ',' << vc.y
        << ',' << seg.radius << '\n';
  }
}

void printTrail(ostream& ost, const V2d vertices[], const string& tag,
                const Trail& trail) {
  if (trail.isCircle()) {
    ost << tag << "DATA,1\n";
    printSeg(ost, vertices, trail.seg(0));
  } else {
    const uint NS = trail.size();
    const V2d& vs = vertices[trail.seg(0).headVid];
    ost << tag << "DATA," << (NS + 1) << '\n'; // 节点数量 = 片段数量 + 1
    ost << vs.x << ',' << vs.y << '\n';
    for (uint i = 0; i < NS; ++i) {
      printSeg(ost, vertices, trail.seg(i));
    }
  }
}

/**
 * @brief 输出Line信息。
 */
void printLine(ostream& ost, const V2d vertices[], const Line& line) {
  ost << "LINEBEGIN\n";
  ost << "LINEID," << line.id() << '\n';
  printTrail(ost, vertices, "LINE", line.trail());
  ost << "LINEEND\n";
}

/**
 * @brief 数据输出到流。
 */
void printShape(ostream& ost, const V2d vertices[], const Shape& shape) {
  ost << "SHAPEBEGIN\n";
  ost << "SHAPEID," << shape.id() << '\n';
  const int NT = shape.trailNum();
  if (NT > 0) {
    printTrail(ost, vertices, "SHAPE", shape.trail(0));
  }
  for (int i = 1; i < NT; ++i) {
    printTrail(ost, vertices, "VOID", shape.trail(i));
  }
  ost << "SHAPEEND\n";
}

/**
 * @brief 输出到流。
 *
 * TODO: Line/Shape混合。"LLLSS"
 */
void printRouteLayer(ostream& ost, const RouteLayer& layer) {
  ost << "LAYERBEGIN\n";
  ost << "LAYERTYPE," << layer.name() << '\n';
  ost << "BASEPLANEHEIGHT," << layer.baseHeight() << '\n';
  ost << "THICKNESS," << layer.thickness() << '\n';
  ost << "POINTCOUNT," << layer.pointCountHint() << '\n';
  ost << "SEGCOUNT," << layer.segCountHint() << '\n';
  const V2d* const vertexArray = layer.vertexArray();
  for (uint i = 0, nl = layer.lineNum(); i < nl; ++i) {
    printLine(ost, vertexArray, layer.line(i));
  }
  for (uint i = 0, ns = layer.shapeNum(); i < ns; ++i) {
    printShape(ost, vertexArray, layer.shape(i));
  }
  ost << "LAYEREND\n";
}

/**
 * @brief 输出到流。
 */
void printPlaneLayer(ostream& ost, const PlaneLayer& layer) {
  ost << "LAYERBEGIN\n";
  ost << "LAYERTYPE," << layer.name() << '\n';
  const uint NS = layer.shapeNum();
  const V2d* const vertexArray = layer.vertexArray();
  for (uint i = 0; i < NS; ++i) {
    ost << "BASEPLANEHEIGHT," << layer.baseHeight(i) << '\n';
    ost << "THICKNESS," << layer.thickness(i) << '\n';
    ost << "POINTCOUNT," << layer.pointCountHint() << '\n';
    ost << "SEGCOUNT," << layer.segCountHint() << '\n';
    printShape(ost, vertexArray, layer.shape(i));
  }
  ost << "LAYEREND\n";
}

/**
 * @brief 输出到流。
 */
void printMultiLayer(ostream& ost, const MultiLayer& layer) {
  ost << "LAYERBEGIN\n";
  ost << "LAYERTYPE," << layer.name() << '\n';
  ost << "BASEPLANEHEIGHT," << layer.baseHeight() << '\n';
  const V2d* const vertexArray = layer.vertexArray();
  for (uint i = 0, ng = layer.groupNum(); i < ng; ++i) {
    const MultiLayer::Group& g = layer.group(i);
    ost << "THICKNESS," << g.thickness() << '\n';
    for (uint k = 0, nl = g.lineNum(); k < nl; ++k) {
      printLine(ost, vertexArray, g.line(k));
    }
    for (uint k = 0, ns = g.shapeNum(); i < ns; ++k) {
      printShape(ost, vertexArray, g.shape(k));
    }
  }
  ost << "LAYEREND\n";
}

/**
 * @brief 数据输出到流。非功能需求，仅作调试用。
 * 方案1：现行的类型判断方案。
 * 方案2：OOP做法是设计Layer::print(ostream&)纯虚函数然后三种层的子类实现，
 * 代价是需要向Layer头文件引入`<ios_fwd>`依赖。
 * 方案3：设计LayerVisitor接口。
 */
void printLayer(ostream& ost, const Layer& layer) {
  switch(layer.type()){
  case Layer::ROUTE_LAYER:
    printRouteLayer(ost, dynamic_cast<const RouteLayer&>(layer));
    break;
  case Layer::PLANE_LAYER:
    printPlaneLayer(ost, dynamic_cast<const PlaneLayer&>(layer));
    break;
  case Layer::MULTI_LAYER:
    printMultiLayer(ost, dynamic_cast<const MultiLayer&>(layer));
    break;
  default:
    verify(false);
  }
}

/**
 * @brief 将电路板输出到流。
 */
void printBoard(ostream& ost, const Board& board) {
  ost << fixed << setprecision(6);
  const int N = board.layerNum();
  for (int i = 0; i < N; ++i) {
    printLayer(ost, board.layer(i));
  }
}

} // namespace hwpcb