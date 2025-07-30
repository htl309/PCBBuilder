#pragma once

#include "math/vec2.hpp"
#include "math/vec3.hpp"

#include <cstdint>

namespace hwpcb {

/**
 * @brief 无符号数用于顶点索引、片段数量等。
 */
using uint = std::uint32_t;

/**
 * @brief 基础数据类型。
 *
 * 坐标、尺寸、高度等都以fp64表示。
 */
using real = double;

/**
 * @brief 实体唯一ID。
 *
 * 回避UUID、GUID的名字，以防与系统冲突。
 */
using euid = std::uint64_t;

/**
 * @brief 二维向量。
 */
using hwcad::V2d;

/**
 * @brief 三维向量。
 */
using hwcad::V3d;

/**
 * @brief 二维整型向量。
 */
using hwcad::V2i;

} // namespace hwpcb
