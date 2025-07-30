#pragma once

#include <limits>
#include <sstream>
#include <stdexcept>

namespace hwpcb {

/**
 * @brief 格式化异常信息，并抛出runtime_error。
 */
void eprintf(const char* format, ...);

/**
 * @brief 抛出带有验证失败原因的运行时异常。
 *
 * 内联是为了方便使用，无需CPP文件。抛异常而不是直接终止程序，给外层处理机会。
 *
 * @param file 源文件名。
 * @param line 源码行号。
 * @param expr 验证失败的表达式。
 */
void verify_failed(const char* file, int line, const char* expr);

/**
 * @brief 验证宏。总是启用，验证失败则抛出运行时异常。
 */
#define verify(x) if(x) {} else verify_failed(__FILE__, __LINE__, #x)

} // namespace hwpcb
