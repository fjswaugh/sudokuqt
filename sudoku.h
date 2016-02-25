#ifndef SUDOKU_H
#define SUDOKU_H

#include <array>

/*
 * Represent sudoku grid as 9 by 9 array of ints, with 0 representing an
 * unfilled square.
 */

class Board {
public:
    Board();
    Board(const std::array<std::array<int, 9>, 9>& grid);

    std::array<int, 9>& operator[](int row);
    const std::array<int, 9>& operator[](int row) const;

    std::string str() const;
    // Returns the count of how many numbers were tested overall before
    // finding the correct number everywhere
    unsigned long int count() const { return m_count; }

    // Returns false if grid is unsolvable (doesn't determine if a solution
    // is unique)
    bool solve();

    // Clears class data
    void clear();

    // Checks if the grid has any immediate contradictions (the same numbers
    // sharing rows, columns, or squares)
    bool contradictory();
private:
    // Checks whether an entry is valid in a given sudoku board
    bool valid(const std::array<std::array<int, 9>, 9>& grid,
               int row, int col, int entry);

    // Returns false if a grid is unsolvable
    // Recursive function
    bool solve(std::array<std::array<int, 9>, 9> grid);

    std::array<std::array<int, 9>, 9> m_grid;

    unsigned long int m_count;
};

std::istream& operator>>(std::istream& is, Board& b);
std::ostream& operator<<(std::ostream& os, const Board& b);

#endif // SUDOKU_H
