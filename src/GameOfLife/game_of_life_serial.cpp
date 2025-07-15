#include "game_of_life_serial.hpp"
#include <random>
#include <algorithm>
#include <iostream>
#include <cstring>

GameOfLifeSerial::GameOfLifeSerial(uint rows, uint cols)
  : rows_(rows)
  , cols_(cols)
{
    // Reserva un buffer 1D de tamaño rows_*cols_
    grid     = new ubyte[rows_ * cols_];
    nextGrid = new ubyte[rows_ * cols_];
}

GameOfLifeSerial::~GameOfLifeSerial() {
    delete[] grid;
    delete[] nextGrid;
}

void GameOfLifeSerial::initialize() {
    std::fill_n(grid,     rows_ * cols_, 0);
    std::fill_n(nextGrid, rows_ * cols_, 0);
}

void GameOfLifeSerial::initializeRandom() {
    std::mt19937 gen(123);
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (uint i = 0, N = rows_ * cols_; i < N; ++i) {
        grid[i] = (dis(gen) < 0.5f) ? 1 : 0;
    }
    std::copy(grid, grid + rows_ * cols_, nextGrid);
}


void GameOfLifeSerial::setGrid(const ubyte* gridData) {
    // Copiamos todo el buffer de golpe
    std::memcpy(
        /*dest*/ grid,
        /*src */ gridData,
        /*bytes*/ rows_ * cols_ * sizeof(ubyte)
    );
}

void GameOfLifeSerial::setCell(uint x, uint y, ubyte state) {
    if (x < cols_ && y < rows_) {
        set(x, y, state ? 1 : 0);
    }
}

void GameOfLifeSerial::step() {
    for (uint y = 0; y < rows_; ++y) {
        uint y0 = (y + rows_ - 1) % rows_;
        uint y1 = y;
        uint y2 = (y + 1)    % rows_;
        for (uint x = 0; x < cols_; ++x) {
            uint x0 = (x + cols_ - 1) % cols_;
            uint x2 = (x + 1)    % cols_;
            ubyte alive = countAliveCells(x0, x, x2, y0, y1, y2);
            nextGrid[y * cols_ + x] =
                (alive == 3 || (alive == 2 && at(x,y))) ? 1 : 0;
        }
    }
    std::swap(grid, nextGrid);
}

std::ostream &operator<<(std::ostream &os, const GameOfLifeSerial &game) {
    for (uint y = 0; y < game.rows_; ++y) {
        os << "[ ";
        for (uint x = 0; x < game.cols_; ++x) {
            os << int(game.at(x,y))
               << (x + 1 < game.cols_ ? ", " : "");
        }
        os << " ]\n";
    }
    return os;
}
