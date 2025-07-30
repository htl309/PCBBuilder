#pragma once

#include <iostream>
#include <stdexcept>
#include <string>

namespace hwpcb {
enum class PCBError : uint64_t {
		Success = 0,
		Build3DError = 1,
		StepWriteError = 2,
		DiscretError = 3 
		
	};
}
// 定义一个通用的异常基类，继承自 std::exception
class PCBException : public std::exception {
protected:
  std::string message; // 异常信息

public:
  // 构造函数，接收异常信息
  PCBException(const std::string& msg) : message(msg) {}

  // 重写 what() 方法，用于获取异常信息
  virtual const char* what() const noexcept override { return message.c_str(); }
};

//三维建模过程失败
class Build3DException : public PCBException {
public:
  Build3DException(const std::uint64_t i): PCBException("layer3d can't be build, layerindex: " + std::to_string(i)) {}
};


//step文件输出失败
class StepWriterException : public PCBException {
public:
  StepWriterException(const std::uint64_t i): PCBException("Step File can't be Written ,layerindex: " + std::to_string(i)){}
};

//离散化失败
class DiscretException : public PCBException {
public:
  DiscretException(const std::uint64_t i): PCBException("Geometry can't Discret Correct, Shellindex: " +std::to_string(i)) {}
};

