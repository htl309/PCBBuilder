#pragma once

#include "pcb/base.hpp"
#include "verify.hpp"

#include <istream>
#include <limits>
#include <memory>

namespace hwpcb {

/**
 * @brief 语法记号。
 * 标识符的词素（文本）从词法扫描器获取。
 */
enum class Token {
  TK_EMPTY,      // 空记号，说明数据结束。
  TK_UNSIGNED,   // 无符号型字面量。
  TK_FLOATING,   // 浮点型字面量（有小数或指数）。
  TK_COMMA,      // 逗号，作为分隔符。
  TK_IDENTIFIER, // 标识符。字母或下划线开头，字母数字下划线。这个枚举项必须紧接着以下关键字。
  KW_LAYER_BEGIN,
  KW_LAYER_END,
  KW_LAYER_TYPE,
  KW_BASE_PLANE_HEIGHT,
  KW_THICKNESS,
  KW_POINT_COUNT,
  KW_SEG_COUNT,
  KW_LINE_BEGIN,
  KW_LINE_DATA,
  KW_LINE_ID,
  KW_LINE_END,
  KW_SHAPE_BEGIN,
  KW_SHAPE_ID,
  KW_SHAPE_DATA,
  KW_SHAPE_END,
  KW_VOID_DATA,
  KW_STRAIGHT,
  KW_ARC,
  KW_CIRCLE,
  KW_CW,
  KW_CCW,
};

struct TokenName {
  Token token;
  std::string name;
};

extern const TokenName g_tokens[];

const std::string& getTokenName(Token token);

/**
 * @brief 电路版层词法分析器。
 * 仅识别：关键字、标识符、无符号整数U64、浮点数F64、逗号。
 * 如遇EOF且未读到有效字符则返回空标记（TK_EMPTY）。
 * 仅支持UTF-8编码。
 * 标记最长不超过63字符，包括串结束符则是64字符。
 *
 * 样例数据文件：3.4MB, 660k tokens.
 * v0: 每次识别一个记号。0.3s(rel), 0.7s(dbg).
 * v1: 一次扫描全部记号，返回记号列表。0.26s(rel), 0.60s(dbg).
 * v2: 批量全部扫描不利于语法回退，也浪费空间。
 *     改为每次扫描一个记号，并在构造时绑定输入流。
 *     如需预先扫描全部符号也容易实现：
 */
class LayerLexer {
public:
  /**
   * @brief 构造词法分析器并绑定到输入流。
   * 在Lexer析构之前输入流不能重用。
   */
  static std::shared_ptr<LayerLexer> make(std::istream& ist);

  /**
   * @brief 内存数据解析接口设计
   * @param s PCB一“层”的数据，即应以LAYERBEGIN开始、以LAYEREND结束，并且一定有串结束符（NUL）。
   * 数据最后要是能添加一个“哨兵”字符就好了
   * 方案1：(const char* pb); 推荐，甚至可与C语言兼容。
   * 方案2：(const std::string& s); 推荐，便于多线程处理，string::c_str()返回的指针确保连续，末尾隐含NUL结束符可作哨兵。
   * 方案3：(const char* pb, const char* pe); 符合C++惯例，但判断结束比哨兵方式慢（多少？5%？）
   * 方案4：(const std::vector<char>& s); 没有隐含的'\0'结束符，参数是常量也不能添加。
   */
  static std::shared_ptr<LayerLexer> make(const char* s);

  virtual ~LayerLexer() {}

  /**
   * @brief 扫描下一个记号。
   * 如输入结束则记号为TK_EMPTY。如遇到词法错误则抛出异常。
   * 该函数比下述requireX()系列函数更为通用，速度相对稍慢。
   *
   * 2024-08-02: LayerType可能有`L4_Sig-h`, `3.3V`, `Pwr1(split)`等形式，不能视为标识符，
   *   此时不能用scan()，而应用scanLine()。
   *   即如果读到KW_LAYER_TYPE，下个符号应当是逗号，之后必须用scanLine()读取到换行，全部内容作为层类型。
   */
  virtual Token scan() = 0;

  /**
   * @brief 扫描读入直到换行，目前仅用于读入LayerType。
   * 若读到标识符返回true，标识符用lexeme()获取。
   * 若数据结束返回false。
   */
  virtual bool scanLine() = 0;

  /**
   * @brief 扫描关键字。
   */
  virtual Token scanKeyword() = 0;

  /**
   * @brief 要求关键字。匹配后不能回退。失配则抛出异常。
   */
  virtual void requireKeyword(Token kw) = 0;

  /**
   * @brief 要求64位无符号数。匹配后不能回退。失配则抛出异常。
   */
  virtual uint64_t requireU64() = 0;

  /**
   * @brief 要求32位无符号数。匹配后不能回退。失配则抛出异常。
   */
  virtual uint32_t requireU32() {
    auto u = requireU64();
    verify(u <= std::numeric_limits<uint32_t>::max());
    return static_cast<uint32_t>(u);
  }

  /**
   * @brief 扫描64位无符号数。必须匹配。
   * FP64最多由符号、16位精度、E、指数符号、3位指数构成，不超过22字符。
   */
  virtual real requireF64() = 0;

  /**
   * @brief 要求匹配逗号。匹配后不能回退。失配则抛出异常。
   */
  virtual void requireComma() = 0;

  /**
   * @brief 回退最近一次的扫描。不能连续调用undo()。
   * 回退后下次调用scan()的内容不变。
   */
  virtual void undo() = 0;

  /**
   * @brief 获取记号类别。
   */
  virtual Token token() const = 0;

  /**
   * @brief 获取浮点值。仅当token == TK_FLOATING时可用。
   */
  virtual real getF64() const = 0;

  /**
   * @brief 获取无符号整数值。仅当token == TK_UNSIGNED时可用。
   */
  virtual uint64_t getU64() const = 0;

  /**
   * @brief 获取词素。仅用于标识符或关键字。
   */
  virtual std::string lexeme() const = 0;

  /**
   * @brief 待扫描字符所在行号，从1开始。
   * 打开文件，开始就应该是第1行、第1列。
   */
  virtual uint line() const = 0;

  /**
   * @brief 待扫描字符所在列号，从1开始。
   */
  virtual uint column() const = 0;
};

/**
 * @brief 字典树，用于区分关键字和标识符。
 * 
 * 比散列表或映射表更快。关键字树节点不超过200项，但经试验用byte或short更慢。
 */
class TokenTrie {
  static constexpr uint N = 256;
  Token m_tokens[N];
  uint m_nodes[N][26];
  uint m_size;
  // std::unordered_map<std::string, Token> m_tokenMap;

public:
  TokenTrie();

  /**
   * @brief 添加一项关键字。
   */
  void add(const std::string& s, Token token);

  /**
   * @brief 查找字符串s对应关键字的语法记号。若非关键字则返回TK_IDENTIFIER。
   * @param s 以NUL结尾的字符串。
   */
  Token find(const char* const s, const uint len) const;
};

} // namespace hwpcb