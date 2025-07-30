#include "pcb/layer_parser.hpp"
#include "pcb/layer_builder.hpp"
#include <unordered_map>
using namespace std;

namespace hwpcb {

/**
 * matchX函数是尝试匹配终结符，不符合可以回退。
 * parseX函数解析指定语句，不符合则抛出语法错误。
 */
class LayerParser1 : public LayerParser {
public:
  LayerParser1(std::istream& ist) : m_lex(LayerLexer::make(ist)), m_lexer(*m_lex) {
  }

  LayerParser1(const char* s) : m_lex(LayerLexer::make(s)), m_lexer(*m_lex) {
  }

  /**
   * @brief 析构函数。
   */
  virtual ~LayerParser1() {}

  /**
   * @brief 解析数据流，匹配“层”数据，构造电路板层。
   * 若数据结束（遇到EOF）则返回空指针。
   * 流如果含有多个“层”的数据，需要重复调用该函数。
   * @return 构造出的电路板“层”。
   */
  virtual std::shared_ptr<Layer> parse() override {
    const Token t = m_lexer.scanKeyword();
    if (t == Token::TK_EMPTY) {
      return nullptr;
    } else if (t != Token::KW_LAYER_BEGIN) { // 异常标识符
      eprintf("[%d, %d] syntax error: unknown lexeme '%s'", m_lexer.line(),
              m_lexer.column(), m_lexer.lexeme().c_str());
    }
    return parseLayerData();
  }

private:
  /**
   * @brief 匹配层的内容，根据类型确定后续数据。
   */
  std::shared_ptr<Layer> parseLayerData() {
    m_lexer.requireKeyword(Token::KW_LAYER_TYPE);
    m_lexer.requireComma();
    if (!m_lexer.scanLine()) { // 记号类型不符
      eprintf("[%d, %d] syntax error: expect identifier, actual '%s'",
              m_lexer.line(), m_lexer.column(), m_lexer.lexeme().c_str());
    }
    const string layerType = m_lexer.lexeme();
    if (layerType == "Plane") { // 平面层
      return parsePlaneLayer();
    } else if (layerType == "MultiLayer") { // 跨层
      return parseMultiLayer();
    } else { // 信号层的类型（名字）不固定
      return parseRouteLayer(layerType);
    }
  }

  /**
   * @brief 匹配走线层（信号层）。
   */
  std::shared_ptr<Layer> parseRouteLayer(const string& layerType) {
    auto builder = RouteLayerBuilder(layerType);
    m_lexer.requireKeyword(Token::KW_BASE_PLANE_HEIGHT);
    m_lexer.requireComma();
    const real height = m_lexer.requireF64();
    builder.onBaseHeight(height);

    m_lexer.requireKeyword(Token::KW_THICKNESS);
    m_lexer.requireComma();
    const real thickness = m_lexer.requireF64(); // 所有对象有统一的拉伸高度。
    builder.onThickness(thickness);

    m_lexer.requireKeyword(Token::KW_POINT_COUNT);
    m_lexer.requireComma();
    builder.onPointCount(m_lexer.requireU32()); // 提示顶点数量（至少）

    m_lexer.requireKeyword(Token::KW_SEG_COUNT);
    m_lexer.requireComma();
    builder.onSegCount(m_lexer.requireU32()); // 提示片段数量

    while (1) {
      if (const Token k = m_lexer.scanKeyword(); k == Token::KW_LINE_BEGIN) {
        parseLine(builder);
      } else if (k == Token::KW_SHAPE_BEGIN) {
        parseShape(builder);
      } else if (k == Token::KW_LAYER_END) {
        break;
      } else {
        eprintf("[%d, %d] expect KW_LAYEREND, actual '%s'", m_lexer.line(),
                m_lexer.column(), m_lexer.lexeme().c_str());
      }
    }
    return builder.getProduct();
  }

  /**
   * @brief 匹配平面层。
   * 平面层可有多组数据
   * 1个BASEPLANEHEIGHT+1个THICKNESS+1个shape为一组。
   */
  std::shared_ptr<Layer> parsePlaneLayer() {
    auto builder = PlaneLayerBuilder();
    while (1) { // {高度, 厚度, Shape}
      if (Token t = m_lexer.scanKeyword(); t == Token::KW_BASE_PLANE_HEIGHT) {
        m_lexer.requireComma();
        const real height = m_lexer.requireF64();
        builder.onBaseHeight(height);

        m_lexer.requireKeyword(Token::KW_THICKNESS);
        m_lexer.requireComma();
        const real thickness = m_lexer.requireF64();
        builder.onThickness(thickness);

        m_lexer.requireKeyword(Token::KW_POINT_COUNT);
        m_lexer.requireComma();
        builder.onPointCount(m_lexer.requireU32());

        m_lexer.requireKeyword(Token::KW_SEG_COUNT);
        m_lexer.requireComma();
        builder.onSegCount(m_lexer.requireU32());
      } else {
        verify(t == Token::KW_LAYER_END);
        break;
      }

      m_lexer.requireKeyword(Token::KW_SHAPE_BEGIN);
      parseShape(builder);
    }
    return builder.getProduct();
  }

  /**
   * @brief 匹配跨层。
   */
  std::shared_ptr<Layer> parseMultiLayer() {
    auto builder = MultiLayerBuilder();
    m_lexer.requireKeyword(Token::KW_BASE_PLANE_HEIGHT);
    m_lexer.requireComma();
    const real height = m_lexer.requireF64();
    builder.onBaseHeight(height);

    for (bool loop = true; loop;) { // 每个厚度对应若干Line和Shape。
      if (const Token t = m_lexer.scanKeyword(); t == Token::KW_THICKNESS) {
        m_lexer.requireComma();
        const real thickness = m_lexer.requireF64();
        builder.onThickness(thickness);
      } else {
        verify(t == Token::KW_LAYER_END);
        break;
      }

      m_lexer.requireKeyword(Token::KW_POINT_COUNT);
      m_lexer.requireComma();
      builder.onPointCount(m_lexer.requireU32());

      m_lexer.requireKeyword(Token::KW_SEG_COUNT);
      m_lexer.requireComma();
      builder.onSegCount(m_lexer.requireU32());

      while (loop) {
        if (const Token k = m_lexer.scanKeyword(); k == Token::KW_LINE_BEGIN) {
          parseLine(builder);
        } else if (k == Token::KW_SHAPE_BEGIN) {
          parseShape(builder);
        } else {
          m_lexer.undo(); // LayerEnd或Thickness
          break;
        }
      }
    }
    return builder.getProduct();
  }

  void parseLine(LayerBuilder& builder) {
    builder.newLine(requireEuid(Token::KW_LINE_ID));

    m_lexer.requireKeyword(Token::KW_LINE_DATA);
    m_lexer.requireComma();
    const uint nv = m_lexer.requireU32();
    builder.onTrail(nv);

    if (nv == 1) {
      parseCircle(builder);
    } else {
      real sx = m_lexer.requireF64(); // start point
      m_lexer.requireComma();
      real sy = m_lexer.requireF64();
      builder.onStart(sx, sy);

      while (matchSegList(builder)) {
        // 无需处理，数据已保存
      }
    }

    m_lexer.requireKeyword(Token::KW_LINE_END);
    builder.endLine();
  }

  /**
   * @brief 匹配Shape记录。
   */
  void parseShape(LayerBuilder& builder) {
    builder.newShape(requireEuid(Token::KW_SHAPE_ID));

    // 外框
    m_lexer.requireKeyword(Token::KW_SHAPE_DATA);
    m_lexer.requireComma();
    const uint nv = m_lexer.requireU32();
    builder.onTrail(nv);

    if (nv == 1) {
      parseCircle(builder);
    } else {
      real sx = m_lexer.requireF64(); // start point
      m_lexer.requireComma();
      real sy = m_lexer.requireF64();
      builder.onStart(sx, sy);

      while (matchSegList(builder)) {
        // 继续
      }
    }
    while (matchVoid(builder)) {
      // 继续
    }

    m_lexer.requireKeyword(Token::KW_SHAPE_END);
    builder.endShape();
  }

  bool matchVoid(LayerBuilder& builder) {
    if (m_lexer.scanKeyword() != Token::KW_VOID_DATA) {
      m_lexer.undo();
      return false;
    }
    builder.newHole();

    m_lexer.requireComma();
    const uint nv = m_lexer.requireU32();
    builder.onTrail(nv);
    if (nv == 1) {
      parseCircle(builder);
    } else {
      real sx = m_lexer.requireF64(); // start point
      m_lexer.requireComma();
      real sy = m_lexer.requireF64();
      builder.onStart(sx, sy);

      while (matchSegList(builder)) {
        // 继续
      }
    }
    builder.endHole();
    return true;
  }

  /**
   * @brief 匹配片段列表。
   */
  bool matchSegList(LayerBuilder& builder) {
    if (auto t = m_lexer.scanKeyword(); t == Token::KW_STRAIGHT) {
      parseLineSeg(builder);
    } else if (t == Token::KW_ARC) {
      parseArcSeg(builder);
    } else {
      m_lexer.undo();
      return false;
    }
    return true;
  }

  /**
   * @brief 匹配线段。
   */
  void parseLineSeg(LayerBuilder& builder) {
    m_lexer.requireComma();
    euid id = m_lexer.requireU64();
    m_lexer.requireComma();
    real w = m_lexer.requireF64();
    m_lexer.requireComma();
    real x = m_lexer.requireF64();
    m_lexer.requireComma();
    real y = m_lexer.requireF64();
    builder.onLineSeg(id, w, x, y);
  }

  /**
   * @brief 匹配弧段。
   */
  void parseArcSeg(LayerBuilder& builder) {
    m_lexer.requireComma();
    euid id = m_lexer.requireU64();
    m_lexer.requireComma();
    real w = m_lexer.requireF64();
    m_lexer.requireComma();
    real cx = m_lexer.requireF64();
    m_lexer.requireComma();
    real cy = m_lexer.requireF64();
    m_lexer.requireComma();
    real x = m_lexer.requireF64();
    m_lexer.requireComma();
    real y = m_lexer.requireF64();
    m_lexer.requireComma();
    bool cw = false;
    if (Token t = m_lexer.scanKeyword(); t == Token::KW_CW) {
      cw = true;
    } else { // 否则必须是CCW
      verify(t == Token::KW_CCW);
    }
    builder.onArcSeg(id, w, cx, cy, x, y, cw);
  }

  /**
   * @brief 匹配圆形。
   */
  void parseCircle(LayerBuilder& builder) {
    m_lexer.requireKeyword(Token::KW_CIRCLE);
    m_lexer.requireComma();
    euid id = m_lexer.requireU64();
    m_lexer.requireComma();
    real w = m_lexer.requireF64();
    m_lexer.requireComma();
    real cx = m_lexer.requireF64();
    m_lexer.requireComma();
    real cy = m_lexer.requireF64();
    m_lexer.requireComma();
    real r = m_lexer.requireF64();
    builder.onCircle(id, w, cx, cy, r);
  }

  /**
   * @brief 要求关键字对应的实体ID。
   * 例如“LINEID,<ID>”
   */
  inline euid requireEuid(Token kw) {
    m_lexer.requireKeyword(kw);
    m_lexer.requireComma();
    return m_lexer.requireU64();
  }

  std::shared_ptr<LayerLexer> m_lex;

  /**
   * @brief 词法分析器。
   */
  LayerLexer& m_lexer;
};

/**
 * @brief 创建解析器。
 */
std::shared_ptr<LayerParser> LayerParser::make(std::istream& ist) {
  return std::shared_ptr<LayerParser1>(new LayerParser1(ist));
}

std::shared_ptr<LayerParser> LayerParser::make(const char* s) {
  return std::shared_ptr<LayerParser1>(new LayerParser1(s));
}

} // namespace hwpcb
