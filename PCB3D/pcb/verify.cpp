#include "verify.hpp"

#include <cstdarg>
#include <cstdio>
#include <stdexcept>
using namespace std;

namespace hwpcb {

void eprintf(const char* format, ...) {
  va_list va;
  const int L = 1024;
  char buf[L];
  va_start(va, format);
  vsnprintf(buf, L, format, va);
  va_end(va);
  throw std::runtime_error(buf);
}

void verify_failed(const char* file, int line, const char* expr) {
  eprintf("%s:%d::verify_failed: %s", file, line, expr);
}

}