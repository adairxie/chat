#ifndef MUDUO_EXAMPLES_SUDOKU_SUDOKU_H
#define MUDUO_EXAMPLES_SUDOKU_SUDOKU_H


#include "../../Types.h"
#include "../../StringPiece.h"

std::string solveSudoku(const StringPiece& puzzle);
const int kCells = 81;
extern const char kNoSolution[];

#endif
