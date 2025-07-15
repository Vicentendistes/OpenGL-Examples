// game_of_life_serial.hpp
#ifndef GAME_OF_LIFE_SERIAL_H
#define GAME_OF_LIFE_SERIAL_H

#include "game_of_life.hpp"
#include <cstdint>
#include <ostream>

using ubyte = uint8_t;
using uint  = unsigned int;

class GameOfLifeSerial : public GameOfLife {
private:
    uint rows_, cols_;
    ubyte *grid, *nextGrid;

    inline ubyte at(uint x, uint y) const {
        return grid[y * cols_ + x];
    }
    inline void set(uint x, uint y, ubyte v) {
        grid[y * cols_ + x] = v;
    }

public:
    GameOfLifeSerial(uint rows, uint cols);
    ~GameOfLifeSerial() override;

    void initialize() override;
    void initializeRandom() override;
    void setCell(uint x, uint y, ubyte state) override;
    void step() override;

    inline ubyte countAliveCells(uint x0, uint x1, uint x2,
                                 uint y0, uint y1, uint y2) const
    {
        return at(x0,y0) + at(x1,y0) + at(x2,y0)
             + at(x0,y1)           + at(x2,y1)
             + at(x0,y2) + at(x1,y2) + at(x2,y2);
    }

    ubyte* getGrid() override {
        return grid;
    }
    void setGrid(const ubyte* grid) override;

    friend std::ostream &operator<<(std::ostream &os,
                                    const GameOfLifeSerial &game);
};

#endif // GAME_OF_LIFE_SERIAL_H
