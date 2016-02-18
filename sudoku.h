#ifndef SUDOKU_H
#define SUDOKU_H

#include <array>

/*
 * Represent sudoku grid as N by N array of ints, with 0 representing an
 * unfilled square.
 */

constexpr int N = 9;
using Grid_t = std::array<std::array<int, N>, N>;

class Board {
public:
    Board();
    Board(const Grid_t &grid);

    Grid_t grid() const { return m_grid; }
    Grid_t original_grid() const { return m_original_grid; }

    // Returns false if grid is unsolvable (doesn't determine if a solution
    // is unique)
    bool solve();

    // Clears class data
    void clear();

    // Checks if the original grid has any immediate contradictions (the same
    // numbers sharing rows, columns, or squares)
    bool contradictory();
private:
    // Checks whether an entry is valid in a given sudoku board
    bool valid(const Grid_t& grid, int row, int col, int entry);

    // Returns false if a grid is unsolvable
    // Recursive function
    bool solve(Grid_t grid);

    // Starting grid
    Grid_t m_original_grid;
    // Working/finished grid
    Grid_t m_grid;
};

std::istream& operator>>(std::istream& is, Board& b);
std::ostream& operator<<(std::ostream& os, const Board& b);

Board read_from_file(std::string file_name);
std::string print(const Board& b);

#endif // SUDOKU_H
