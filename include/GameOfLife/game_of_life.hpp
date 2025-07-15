// game_of_life.hpp
#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#include <cstddef>
#include <ostream>
#include <cstdint>

using ubyte = uint8_t;
using uint  = unsigned int;

class GameOfLife {
public:
    virtual ~GameOfLife() = default;

    virtual void initialize() = 0;
    virtual void initializeRandom() = 0;
    virtual void setCell(uint i, uint j, ubyte state) = 0;
    virtual ubyte*   getGrid() = 0;
    virtual void     setGrid(const ubyte* grid) = 0;
    virtual void     step() = 0;
    virtual void     copyGridToHost() {}
};

#endif // GAME_OF_LIFE_H
