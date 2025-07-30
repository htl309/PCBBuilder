#pragma once

#include "pcb/board.hpp"
#include "pcb/layer.hpp"
#include "pcb/segment.hpp"
#include "pcb/trail.hpp"
#include <ostream>

namespace hwpcb {

/**
 * @brief 实数到流的输出，能忽略多余后缀0，又保证精度无损。
 */
void printReal(std::ostream& ost, real x);

/**
 * @brief 输出线段或弧段信息。
 */
void printSeg(std::ostream& ost, const V2d vertices[],
              const Seg& seg);

/**
 * @brief 输出轨迹信息。
 */
void printTrail(std::ostream& ost, const V2d vertices[],
                const std::string& tag, const Trail& trail);

/**
 * @brief 输出Line信息。
 */
void printLine(std::ostream& ost, const V2d vertices[],
               const Line& line);

/**
 * @brief 输出形状信息。
 */
void printShape(std::ostream& ost, const V2d vertices[],
                const Shape& shape);

/**
 * @brief 输出“层”信息。
 */
void printLayer(std::ostream& ost, const Layer& layer);

/**
 * @brief 输出电路板信息。
 */
void printBoard(std::ostream& ost, const Board& board);

} // namespace hwpcb