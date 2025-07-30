#pragma once

#include <pcb/layer.hpp>
#include <pcb/layer_lexer.hpp>
#include <istream>
#include <memory>

namespace hwpcb {

/**
 * @brief 电路板“层”数据解析器。
 *
 * 终结符：
 *    KW_开头的关键字
 *    OP_开头的符号。目前只有逗号。
 *    标识符。例如`Plane`, `L4_Sig-h`等，目前仅出现在LayerType中。
 *    无符号数。用于实体唯一ID。
 *    浮点数。用于线宽、顶点坐标。
 * 非终结符：
 *    下文小写变量。
 * 产生式：
 * layer -> KW_LAYERBEGIN KW_LAYERTYPE TK_COMMA layer_data KW_LAYEREND
 * 
 * layer_data -> plane_layer | multi_layer | bottom_layer | normal_layer
 * 
 *  plane_layer -> KW_Plane plane_layer_data
 *  multi_layer -> KW_MultiLayer base_height sublayer_list
 * bottom_layer -> KW_Bottom route_layer_data
 * normal_layer -> TK_IDENTIFIER route_layer_data
 *
 * plane_layer_data -> plane_layer_data | eps
 *
 * plane_group -> base_height thickness shape
 *
 * sublayer_list -> sublayer_list + sublayer
 *
 * sublayer -> thickness line_list shape_list
 *
 * route_layer_data -> base_height thickness line_list shape_list
 *
 * base_height -> KW_BASEPLANEHEIGHT TK_COMMA numeric
 *
 * thickness -> KW_THICKNESS TK_COMMA numeric
 *
 * line_list -> line_list line | eps
 * shape_list -> shape_list shape | eps
 *
 * line -> KW_LINEBEGIN line_id line_data trail KW_LINEEND
 * line_id -> KW_LINEID TK_COMMA euid
 * line_data -> KW_LINEDATA TK_COMMA TK_UNSIGNED
 *
 * shape -> KW_SHAPEBEGIN shape_id shape_data start_point trail hole_list
 * shape_id -> KW_SHAPEID TK_COMMA euid
 * shape_data -> KW_SHAPE_DATA TK_COMMA TK_UNSIGNED
 *
 * hole_list -> hole_list hole | hole
 * hole -> KW_VOIDDATA TK_UNSIGNED trail
 *
 * trail -> seg_list | circle
 *
 * seg_list ->  seg_list | seg
 * seg -> line_seg | arc_seg
 * 
 * line_seg -> KW_STRAIGHT TK_COMM euid TK_COMMA width TK_COMMA x TK_COMMA y
 * circle -> KW_CIRCLE TK_COMMA euid TK_COMMA 
 *
 * start_point -> real TK_COMMA real
 *
 * euid -> TK_UNSIGNED
 * numeric -> TK_UNSIGNED | TK_FLOATING
 */
class LayerParser {
public:
  /**
   * @brief 创建解析器。绑定在输入流对象。
   * 可以连续读取多个Layer（如果有数据的话）。
   * 遇到EOF之后解析器完成任务，应释放。
   */
  static std::shared_ptr<LayerParser> make(std::istream& ist);

  /**
   * @brief 内存数据解析接口。
   * @param s PCB一“层”的数据，即应以LAYERBEGIN开始、以LAYEREND结束，并且一定有串结束符（NUL）。
   */
  static std::shared_ptr<LayerParser> make(const char* s);
 
  /**
   * @brief 析构函数。功能由派生类实现，因此设计为虚函数。
   */
  virtual ~LayerParser() {}

  /**
   * @brief 解析数据流，构造电路板层。
   * 流如果含有多个“层”的数据，需要重复调用该函数。
   * @return 构造出的电路板“层”。
   */
  virtual std::shared_ptr<Layer> parse() = 0;
};

} // namespace hwpcb