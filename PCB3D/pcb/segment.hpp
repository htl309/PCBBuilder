#pragma once

#include "pcb/base.hpp"

namespace hwpcb {

/**
 * @brief 片段，可表示线段、弧段。
 * Seg设计为基础类型，成员全部公开，没有虚函数，不期望被继承。
 * Seg定长32字节（8+8+4+4+4+1+1，8字节对齐，2字节填充）
 * 确保长度是2的幂，并且能放到缓存线里。按目前设计不会多线程竞争同一个Seg。
 */
struct Seg {
  /**
   * @brief 片段类型。
   */
  enum Type : char {
    LINE_SEG,  // 线段
    ARC_SEG,   // 弧段
    CIRCLE_SEG // 圆形
  };

  /**
   * @brief 实体ID
   */
  euid id;

  /**
   * @brief 线宽。
   */
  real width;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // 抑制“匿名结构体”相关警告
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
  union {
    /**
     * @brief 圆形半径。
     */
    real radius;

    struct {
      /**
       * @brief 起点索引。点坐标数据保存在Layer中。
       */
      uint headVid;

      /**
       * @brief 终点索引。
       */
      uint tailVid;
    };
  };
#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif

  /**
   * @brief 圆心索引。弧段或圆形使用。
   */
  uint centerVid;

  /**
   * @brief 类型标识。
   */
  Type type;

  /**
   * @brief 旋向（逆时针、顺时针），仅弧段适用。
   */
  bool clockwise;

  /**
   * @brief 默认构造函数。0长度、0宽度的线段，点索引全0。
   */
  Seg() : id(), width(), radius(), centerVid(), type(LINE_SEG), clockwise() {}

  /**
   * @brief 构造线段。
   * @param eid 实体ID。
   * @param segWidth 线宽
   * @param head 起点索引（顶点坐标保存在所属Layer中）
   * @param tail 终点索引（顶点坐标保存在所属Layer中）
   */
  Seg(euid eid, real segWidth, uint head, uint tail)
      : id(eid), width(segWidth), headVid(head), tailVid(tail), centerVid(),
        type(LINE_SEG), clockwise() {}

  /**
   * @brief 构造弧段。
   * @param eid 实体ID。
   * @param segWidth 线宽
   * @param head 起点索引（顶点坐标保存在所属Layer中）
   * @param tail 终点索引
   * @param center 圆心索引
   * @param cw 顺时针方向（默认false）
   */
  Seg(euid eid, real segWidth, uint head, uint tail, uint center,
      bool cw = false)
      : id(eid), width(segWidth), headVid(head), tailVid(tail),
        centerVid(center), type(ARC_SEG), clockwise(cw) {}

  /**
   * @brief 构造圆形。
   * @param eid 实体ID。
   * @param segWidth 线宽
   * @param center 圆心顶点索引
   * @param r 半径
   */
  Seg(euid eid, real segWidth, uint center, real r)
      : id(eid), width(segWidth), radius(r), centerVid(center),
        type(CIRCLE_SEG), clockwise() {}

  /**
   * @brief 构为线段。
   * @param eid 实体ID。
   * @param segWidth 线宽
   * @param head 起点索引（顶点坐标保存在所属Layer中）
   * @param tail 终点索引（顶点坐标保存在所属Layer中）
   */
  void toLine(euid eid, real segWidth, uint head, uint tail) {
    id = eid;
    width = segWidth;
    headVid = head;
    tailVid = tail;
    centerVid = 0;
    type = LINE_SEG;
    clockwise = false;
  }

  /**
   * @brief 设为弧段。
   * @param eid 实体ID。
   * @param segWidth 线宽
   * @param head 起点索引（顶点坐标保存在所属Layer中）
   * @param tail 终点索引
   * @param center 圆心索引
   * @param cw 顺时针方向（默认false）
   */
  void toArc(euid eid, real segWidth, uint head, uint tail, uint center, bool cw=false) {
    id = eid;
    width = segWidth;
    headVid = head;
    tailVid = tail;
    centerVid = center;
    type = ARC_SEG;
    clockwise = cw;
  }

  /**
   * @brief 设为圆形。
   * @param eid 实体ID。
   * @param segWidth 线宽
   * @param center 圆心顶点索引
   * @param r 半径
   */
  void toCircle(euid eid, real segWidth, uint center, real r) {
    id = eid;
    width = segWidth;
    centerVid = center;
    radius = r;
    type = CIRCLE_SEG;
  }

  /**
  * @brief 是否线段。
  */
  bool isLine() const { return type == LINE_SEG; }

  /**
  * @brief 是否弧段。
  */
  bool isArc() const { return type == ARC_SEG; }

  /**
  * @brief 是否圆形。
  */
  bool isCircle() const { return type == CIRCLE_SEG; }
};

} // namespace hwpcb
