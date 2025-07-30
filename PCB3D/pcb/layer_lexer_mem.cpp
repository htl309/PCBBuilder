#include "pcb/layer_lexer.hpp"
#include "pcb/base.hpp"
#include "fast_float.h"
#include "verify.hpp"
using namespace std;

/**
 * @brief 标识符首字符（字母、下划线）。标准库也类似做法，但用了位压缩。
 */
static constexpr bool id_head[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

/**
 * @brief 标识符字符（字母、数字、下划线）
 */
static constexpr bool id_body[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

/**
 * @brief 分隔符（空\x00、换行0x0A、空格0x20、逗号0x2C、行注释# 0x23）
 */
static constexpr bool delimiters[256] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
};

/**
 * @brief 浮点字面量首字符：数字（0~9）、正负号
 */
static constexpr bool fp_head[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

/**
 * @brief 判断空白（空格或制表）。
 * 不包括换行、回车、垂直制表、换页符。
 * 标准库的 isblank() 依赖locale，可能接受其他字符，不妥。
 * TODO: 很想拒绝制表符。
 */
static inline bool isBlank(char c) { return c == ' ' || c == '\n'; }

/**
 * @brief 判断换行符。
 */
static inline bool isNewline(char c) { return c == '\n'; }

/**
 * @brief 判断正负号字符。
 */
static inline bool isSign(char c) { return c == '+' || c == '-'; }

/**
 * @brief 判断数字。标准库isdigit(int)稍慢，可能因为locale和参数转型原因？
 */
static inline bool isDigit(char c) { return c >= '0' && c <= '9'; }

/**
 * @brief 判断浮点字面量首字符（数字、正负号）
 */
static inline bool isFpHead(char c) {
#if 1
  return fp_head[(unsigned char)c];
#else
  return (c >= '0' && c <= '9') || c == '+' || c == '-';
#endif
}

/**
 * @brief 标识符首字符（字母、下划线）。标准库也类似做法，但用了位压缩。
 */
static inline bool isIdHead(char c) { return id_head[(unsigned char)c]; }

/**
 * @brief 标识符字符（字母、数字、下划线）
 */
static inline bool isIdBody(char c) { return id_body[(unsigned char)c]; }

/**
 * @brief 判断分隔符（换行、空格、逗号、#号），作为标识符、数值字面量的结束。
 */
static inline bool isDelimiter(char c) { return delimiters[(unsigned char)c]; }

/**
 * @brief 对字符进行必要的转义。调试用。
 */
static string escape(char c) {
  switch (c) {
  case '\t':
    return "\\t";
  case '\n':
    return "\\n";
  case '\r':
    return "\\r";
  case '\a':
    return "\\a";
  case '\b':
    return "\\b";
  case '\f':
    return "\\f";
  case '\v':
    return "\\v";
  default:
    if(c >= ' ' && c < 127) {
      return string(1, c);
    } else {
      char buf[8];
      snprintf(buf, 8, "\\x%02X", (unsigned char)c);
      return buf;
    }
    break;
  }
}


namespace hwpcb {

class LayerLexerMem : public LayerLexer {
  static const TokenTrie keywordTrie; // 关键字的“字典树”

  LayerLexerMem(LayerLexerMem&) = delete;
  LayerLexerMem& operator=(LayerLexerMem&) = delete;

public:
  /**
   * @brief 创建解析器。
   * 文件输入用ifstream。内存数据可用istringstream。
   */
  LayerLexerMem(const char* s);

  virtual Token scan() override;
  virtual void undo() override;

  virtual bool scanLine() override;
  virtual Token scanKeyword() override;

  virtual void requireKeyword(Token kw) override;
  virtual uint64_t requireU64() override;
  virtual real requireF64() override;
  virtual void requireComma() override;

  virtual Token token() const override;
  virtual real getF64() const override;
  virtual uint64_t getU64() const override;
  virtual std::string lexeme() const override;
  virtual uint line() const override;
  virtual uint column() const override;

private:
  const char* m_base; // 基指针
  int m_idx;          // 字符索引
  uint m_line;        // 行号
  uint m_column;      // 列号
  uint m_preCol;      // 前一行的最大行宽。
  uint m_len;         // 词素长度，不包括串结束符。
  Token m_token;      // 语法记号
  real m_f64;         // 浮点字面量值
  uint64_t m_u64;     // 整型字面量值
  bool m_undo;        // 回退标志

  enum { LEXEME_SIZE = 255 }; // 词素最大长度。
  char m_lexeme[LEXEME_SIZE +
                1]; // 词素（关键字、标识符、整型字面量、浮点字面量）。

  /**
   * @brief 扫描行注释。
   * 之前应当已读取#字符，扫描到换行或EOF。
   */
  void scanLineComment();

  /**
   * @brief 回退一个字符，并调整行号、列号。
   */
  void unget();

  /**
   * @brief 解析浮点字面量。
   */
  void parseF64();

  /**
   * @brief 解析无符号整型字面量。
   */
  void parseU64();

  /**
   * @brief 响应换行。
   */
  inline void newline() {
    m_preCol = m_column; // 记录列号，后续如果回退字符要用到
    m_column = 1;        // 行首的列是0，读入字符后才递增
    ++m_line;
  }

  /**
   * @brief 抛出无效字符异常。
   */
  void invalidChar(const char* tag, char c) const;
};

const TokenTrie LayerLexerMem::keywordTrie; // 关键字的“字典树”

LayerLexerMem::LayerLexerMem(const char* s)
    : m_base(s), m_idx(), m_line(1), m_column(1),
      m_preCol(0), m_len(), m_token(), m_f64(),
      m_u64(), m_undo(false), m_lexeme() {
}

enum class State {
  START,      // 初始状态
  IDENTIFIER, // 标识符，关键字
  UNSIGNED,   // \d+
  SIGN,       // [+-] 不完整状态
  SIGNED,     // [+-]\d+ 遇到符号就认为是浮点数。
  DEC_DOT, // [+-]?\d+\. 一旦遇到小数点，就认为是浮点数。不完整状态。
  DECIMAL,      // [+-]?\d+\.\d+
  FP_E,         // [+-]?\d+\.\d+[Ee] 不完整状态。
  FP_E_SIGN,    // [+-]?\d+\.\d+[Ee][+-] 不完整状态。
  FP_EXP,       // [+-]?\d+\.\d+[Ee][+-]?\d+
  LINE_COMMENT, // 行注释
};

Token LayerLexerMem::scan() {
  if (m_undo) { // 调用过undo，那么m_token就是上次扫描的记号，直接返回即可。
    m_undo = false; // 只允许回退一步。
    return m_token;
  }

  m_token = Token::TK_EMPTY;
  m_len = 0;
  m_lexeme[m_len] = 0;
  State state = State::START; // 开始状态

  while (1) {
    const char c = m_base[m_idx++]; // 读入字符
    if (isNewline(c)) {
      newline();
    } else {
      ++m_column; /// 普通字符，列号加1。FIXME: 中文等多字节文字会有偏差。
    }

    switch (state) {
    case State::START:
      if (isIdHead(c)) { // 标识符
        state = State::IDENTIFIER;
        m_token = Token::TK_IDENTIFIER;
        m_lexeme[m_len++] = c;
      } else if (isDigit(c)) {   // 数值
        state = State::UNSIGNED; // 先视为无符号数，后续如有小数点或指数则转浮点类别
        m_token = Token::TK_UNSIGNED;
        m_lexeme[m_len++] = c; // 需要保存，因为可能是浮点值
      } else if (c == ',') {   // 逗号
        m_token = Token::TK_COMMA;
        m_lexeme[m_len++] = ',';
        return Token::TK_COMMA; // 逗号属于分隔符，是独立的记号。
      } else if (isBlank(c)) {  // 略过空白字符，空格非常少。
        continue;
      } else if (isSign(c)) { // 符号 ~ 浮点值
        state = State::SIGN;
        m_token = Token::TK_FLOATING;
        m_lexeme[m_len++] = c;
      } else if (c == '#') { // 行注释，概率低。
        state = State::LINE_COMMENT;
      } else if (c == 0) {
        return Token::TK_EMPTY;
      } else {
        invalidChar("token", c);
      }
      break;

    case State::IDENTIFIER:
      if (isIdBody(c)) {
        m_lexeme[m_len++] = c;
      } else if (isDelimiter(c)) {
        this->unget();
        m_lexeme[m_len] = 0;
        return m_token = keywordTrie.find(m_lexeme, m_len);
      } else {
        invalidChar("#0 identifier", c);
      }
      break;

    case State::UNSIGNED: // [0-9]+
      if (isDigit(c)) {
        m_lexeme[m_len++] = c;
      } else if (isDelimiter(c)) {
        this->unget();
        this->parseU64();
        return Token::TK_UNSIGNED;
      } else if (c == '.') { // 0.
        state = State::DEC_DOT;
        m_lexeme[m_len++] = c;
      } else if (c == 'E' || c == 'e') { // 0e
        state = State::FP_E;
        m_lexeme[m_len++] = c;
      } else {
        invalidChar("unsigned literal", c);
      }
      break;

    case State::SIGN: // [+-]
      if (isDigit(c)) {
        state = State::SIGNED;
        m_lexeme[m_len++] = c;
      } else { // 符号之后必须有数字
        invalidChar("#1 signed literal", c);
      }
      break;
    case State::SIGNED: // [+-]\d+
      if (isDigit(c)) {
        m_lexeme[m_len++] = c;
      } else if (isDelimiter(c)) {
        m_token = Token::TK_FLOATING; // 有符号数当作浮点数。
        this->unget();
        this->parseF64();
        return Token::TK_FLOATING;
      } else if (c == '.') { // 小数点
        state = State::DEC_DOT;
        m_lexeme[m_len++] = c;
      } else if (c == 'E' || c == 'e') { // 指数
        state = State::FP_E;
        m_lexeme[m_len++] = c;
      } else {
        invalidChar("#2 signed literal", c);
      }
      break;
    case State::DEC_DOT: // [+-]?\d+\.
      if (isDigit(c)) {
        state = State::DECIMAL;
        m_lexeme[m_len++] = c;
      } else { // 小数点后面必须有数字，不想支持形如.1或1.的简写。
        invalidChar("#1 decimal", c);
      }
      break;
    case State::DECIMAL: // [+-]?\d+\.\d+
      if (isDigit(c)) {
        m_lexeme[m_len++] = c;
      } else if (isDelimiter(c)) { // 小数当作浮点数。
        m_token = Token::TK_FLOATING;
        this->unget();
        this->parseF64();
        return Token::TK_FLOATING;
      } else if (c == 'E' || c == 'e') {
        state = State::FP_E; // 小数接指数，科学计数法
        m_lexeme[m_len++] = c;
      } else {
        invalidChar("#2 decimal", c);
      }
      break;
    case State::FP_E: // [+-]?\d+\.\d+[Ee]
      if (isDigit(c)) {
        m_lexeme[m_len++] = c;
        state = State::FP_EXP;
      } else if (isSign(c)) {
        state = State::FP_E_SIGN;
        m_lexeme[m_len++] = c;
      } else { // 指数标记E后面必须有数字。
        invalidChar("#1 floating-point", c);
      }
      break;
    case State::FP_E_SIGN: // [+-]?\d+\.\d+[Ee][+-]
      if (isDigit(c)) {
        m_lexeme[m_len++] = c;
        state = State::FP_EXP;
      } else { // 指数符号后面必须有数字，不支持简写。
        invalidChar("#2 floating-point", c);
      }
      break;
    case State::FP_EXP: // [+-]?\d+\.\d+[Ee][+-]?\d+
      if (isDigit(c)) {
        m_lexeme[m_len++] = c;
      } else if (isDelimiter(c)) {
        this->unget();
        this->parseF64();
        return m_token = Token::TK_FLOATING;
      } else {
        invalidChar("#3 floating-point", c);
      }
      break;
    case State::LINE_COMMENT:
      if (isNewline(c)) { // 从#到换行都视为注释，忽略。
        state = State::START;
      } else if (c == 0) { // EOS
        return Token::TK_EMPTY;
      }
      break;
    default:
      verify(false);
    }
  }

  switch (state) { // 流结束的情况
  case State::START:
  case State::LINE_COMMENT:
    break;
  case State::IDENTIFIER:
    m_lexeme[m_len] = 0;
    return m_token = keywordTrie.find(m_lexeme, m_len);
  case State::UNSIGNED:
    this->parseU64();
    return Token::TK_UNSIGNED; // 这几种情况记号的类别已设置，可以接受。
  case State::SIGNED:
  case State::DECIMAL:
  case State::FP_EXP:
    this->parseF64();
    return m_token = Token::TK_FLOATING;
  default:
    m_lexeme[m_len] = 0;
    eprintf("[%d:%d] incomplete token '%s'", m_line, m_column, m_lexeme);
    break;
  }
  return Token::TK_EMPTY;
}

bool LayerLexerMem::scanLine() {
  if (m_undo) { // 调用过undo，那么m_token就是上次扫描的记号，直接返回即可。
    m_undo = false; // 只允许回退一步。
    if (m_token >= Token::TK_IDENTIFIER) {
      return true;
    } else if (m_token == Token::TK_EMPTY) {
      return false;
    } else { // 之前回退的不是标识符
      invalidChar("#1 scanLine", m_lexeme[m_len - 1]);
    }
  }

  m_len = 0;
  while (1) { // 标识符头
    const char c = m_base[m_idx++];
    if (c > ' ' && c < 127) { // 暂不允许非ASCII字符。
      m_lexeme[m_len++] = c;
      ++m_column;
      break;
    } else if (c == ' ') {
      ++m_column;
    } else if (c == 0) {
      m_token = Token::TK_EMPTY;
      m_lexeme[0] = 0;
      return false;
    } else {
      invalidChar("#2 scanLine", c);
    }
  }

  while (1) { // 标识符体
    const char c = m_base[m_idx++];
    if (isDelimiter(c)) {
      --m_idx;
      break;
    } else if (c > ' ' && c < 127) {
      m_lexeme[m_len++] = c;
      ++m_column;
    } else if (c == 0) {
      break;
    } else { // 不允许控制字符或非ASCII字符。
      invalidChar("#3 scanLine", c);
    }
  }
  m_token = Token::TK_IDENTIFIER;
  m_lexeme[m_len] = 0; // 结束符
  return true;
}

Token LayerLexerMem::scanKeyword() {
  if (m_undo) { // 调用过undo，那么m_token就是上次扫描的记号，直接返回即可。
    m_undo = false; // 只允许回退一步。
    if (m_token > Token::TK_IDENTIFIER || m_token == Token::TK_EMPTY) {
      return m_token;
    } else { // 之前回退的不是标识符
      invalidChar("#1 scanKeyword", m_lexeme[m_len - 1]);
    }
  }

  m_len = 0;
  while (1) { // 标识符头
    const char c = m_base[m_idx++];
    if (isIdHead(c)) {
      m_lexeme[m_len++] = c;
      ++m_column;
      break;
    } else if (isNewline(c)) {
      newline();
    } else if (c == ' ') {
      ++m_column;
    } else if (c == '#') {
      scanLineComment();
    } else if (c == 0) {
      m_token = Token::TK_EMPTY;
      m_lexeme[0] = 0;
      return m_token;
    } else {
      invalidChar("#2 scanKeyword", c);
    }
  }

  while (1) { // 标识符体
    const char c = m_base[m_idx++];
    if (isIdBody(c)) {
      m_lexeme[m_len++] = c;
      ++m_column;
    } else if (isDelimiter(c)) {
      --m_idx;
      break;
    } else if (c == 0) {
      break;
    } else {
      invalidChar("#3 scanKeyword", c);
    }
  }
  m_lexeme[m_len] = 0; // 结束符
  m_token = keywordTrie.find(m_lexeme, m_len);
  return m_token;
}

void LayerLexerMem::requireKeyword(Token kw) {
  if(Token tk = scanKeyword(); tk != kw) {
    eprintf("[%d:%d] keyword '%s' mismatch, actual: '%s'", m_line, m_column,
            getTokenName(kw).c_str(), m_lexeme);
  }
}

uint64_t LayerLexerMem::requireU64() {
  uint64_t u = 0;
  while(1){
    const char c = m_base[m_idx++];
    if (isDigit(c)) {
      u = (c & 0x0F); // (c - '0');
      ++m_column;
      break;
    } else if (c == ' ') {
      ++m_column;
    } else if (isNewline(c)) {
      newline();
    } else if (c == '#') {
      scanLineComment();
    } else if (c == 0) {
      eprintf("[%d, %d] missing unsigned number.", m_line, m_column);
    } else {
      invalidChar("#1 U64", c);
    }
  }

  while (1) {
    const char c = m_base[m_idx++];
    if (isDigit(c)) {
      u = u * 10 + (c & 0x0F); // (c - '0');
      ++m_column;
    } else if(isDelimiter(c)) {
      --m_idx;
      break;
    } else if (c == 0) {
      break;
    } else {
      invalidChar("#2 U64", c);
    }
  }
  return u;
}

real LayerLexerMem::requireF64() {
  for (char c; !isFpHead(m_base[m_idx]); ++m_idx) {
    c = m_base[m_idx];
    if (c == ' ') {
      ++m_column;
    } else if (isNewline(c)) {
      newline();
    } else if (c == '#') {
      scanLineComment();
    } else if (c == 0) {
      eprintf("[%d, %d] missing floating-point literal.", m_line, m_column);
    } else {
      invalidChar("#1 F64", c);
    }
  }
  auto rst = fast_float::from_chars(m_base + m_idx, (const char*)0, m_f64);
  if(rst.ec != std::errc()) {
    eprintf("[%d, %d] invalid floating-point literal.", m_line, m_column);
  }
  int off = static_cast<int>(rst.ptr - (m_base + m_idx));
  m_idx += off;
  m_column += static_cast<int>(off);
  return m_f64;
}

void LayerLexerMem::requireComma() {
  while (1) {
    const char c = m_base[m_idx++];
    if (c == ',') {
      ++m_column;
      return;
    } else if (c == ' ') {
      ++m_column;
      continue;
    } else if (isNewline(c)) {
      newline();
    } else if (c == '#') {
      scanLineComment();
    } else {
      eprintf("[%d, %d] missing comma.", m_line, m_column);
    }
  }
}

/**
 * @brief 扫描行注释。
 */
void LayerLexerMem::scanLineComment() {
  while (1) {
    const char c = m_base[m_idx++];
    if (isNewline(c)) {
      newline();
      return;
    } else if (c == 0) { // EOS
      break;
    } else { // 忽略注释内容，无需更新行号。
    }
  }
}

/**
 * @brief 转换浮点类型值。
 * 热点函数。
 */
void LayerLexerMem::parseF64() {
  // https://github.com/fastfloat/fast_float
  m_lexeme[m_len] = 0;
  auto rst = fast_float::from_chars(m_lexeme, m_lexeme + m_len, m_f64);
  verify(rst.ec == std::errc());
}

/**
 * @brief 转换64位无符号整数值。
 */
void LayerLexerMem::parseU64() {
  uint64_t u = 0;
  const char* const s = m_lexeme;
  for (uint i = 0; i < m_len; ++i) {
    u = u * 10 + (s[i] & 0x0F); // (s[i] - '0');
  }
  m_u64 = u;
}

/**
 * @brief 回退一个字符。如果是换行还要恢复行号、列号。
 */
void LayerLexerMem::unget() {
  --m_idx;
  if (m_column == 1) { // 某行的首字符
    --m_line;
    m_column = m_preCol;
  } else {
    --m_column;
  }
}

/**
 * @brief 回退最后获取的记号。
 * 之前必须成功扫描过一个记号。之后扫描的结果一定是被退回的记号。
 * 修改Parser的话，能否不回退？
 */
void LayerLexerMem::undo() {
  if (m_undo) {
    throw runtime_error("Lexer::undo can only one step.");
  } else { // m_token, m_lexeme保存了前一个记号，回退只设标志。不改变行列号。
    m_undo = true;
  }
}

Token LayerLexerMem::token() const { return m_token; }

real LayerLexerMem::getF64() const { return m_f64; }

uint64_t LayerLexerMem::getU64() const { return m_u64; }

std::string LayerLexerMem::lexeme() const {
  return std::string(m_lexeme, m_len);
}

uint LayerLexerMem::line() const { return m_line; }

uint LayerLexerMem::column() const { return m_column; }

void LayerLexerMem::invalidChar(const char* tag, char c) const {
  eprintf("[%d, %d] %s invalid char '%s'", m_line, m_column, tag,
          escape(c).c_str());
}

shared_ptr<LayerLexer> LayerLexer::make(const char* s) {
  return shared_ptr<LayerLexer>(new LayerLexerMem(s));
}

} // namespace hwpcb