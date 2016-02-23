#include <string>
#include <sstream>
#include <array>

#include "sudoku.h"

Board::Board()
{
    for (std::size_t row = 0; row < N; ++row) {
        for (std::size_t col = 0; col < N; ++col) {
            m_original_grid[row][col] = 0;
            m_grid[row][col] = 0;
        }
    }
}

Board::Board(const Grid_t& grid)
    : m_original_grid(grid), m_grid(grid)
{}

std::string Board::str(bool original_grid) const
{
    std::stringstream ss;

    const Grid_t& grid = (original_grid ? m_original_grid : m_grid);

    for (std::size_t row = 0; row < N; ++row) {
        if (row == 0) ss << "+-----------------------+\n";

        for (std::size_t col = 0; col < N; ++col) {
            if (col == 0) ss << "| ";
            if (grid[row][col] == 0) {
                ss << grid[row][col] << ' ';
            } else {
                ss /*<< "\033[1;31m"*/ << grid[row][col] /*<< "\033[0m "*/ << " ";
            }
            if (col != 0 && (col+1)%3 == 0) ss << "| ";
        }
        ss << '\n';

        if (row != 0 && row != N-1 && (row+1)%3 == 0) {
            ss << "|-------+-------+-------|\n";
        }
        if (row == N-1) ss << "+-----------------------+\n";
    }

    return ss.str();
}

bool Board::solve()
{
    if (contradictory()) {
        return false;
    }
    return solve(m_original_grid);
}

bool Board::contradictory()
{
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            if (m_original_grid[row][col] != 0 &&
                    !valid(m_original_grid, row, col,
                           m_original_grid[row][col])) {
                return true;
            }
        }
    }
    return false;
}

bool Board::valid(const Grid_t& grid, int row, int col, int entry)
{
    // Test rows and columns
    bool valid = true;
    for (int i = 0; i < N; ++i) {
        if ((grid[row][i] == entry && i != col) ||
            (grid[i][col] == entry && i != row)) {
            valid = false;
            break;
        }
    }

    // Test groups of nine
    int row_group = row / 3;
    int col_group = col / 3;
    for (int i_row = 0; i_row < 3; ++i_row) {
        if (!valid) break;
        for (int i_col = 0; i_col < 3; ++i_col) {
            if (col_group*3+i_col == col || row_group*3+i_row == row) continue;
            if (!valid) break;
            if (grid[row_group*3+i_row][col_group*3+i_col] == entry) {
                valid = false;
            }
        }
    }

    return valid;
}

void Board::clear()
{
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            m_original_grid[row][col] = 0;
            m_grid[row][col] = 0;
        }
    }
}

bool Board::solve(Grid_t grid)
{
    int row, col;
    bool found = false;
    for (row = 0; row < N; ++row) {
        for (col = 0; col < N; ++col) {
            if (grid[row][col] == 0) found = true;
            if (found) break;
        }
        if (found) break;
    }
    if (!found) {
        m_grid = grid;
        return true;
    }

    for (int entry = 1; entry < 10; ++entry) {
        if (valid(grid, row, col, entry)) {
            grid[row][col] = entry;
            if (solve(grid)) return true;
        }
    }
    return false;
}

std::istream& operator>>(std::istream& is, Board& b)
{
    Grid_t grid;
    std::string line;
    int row = 0;
    while (std::getline(is, line)) {
        // Accept that some lines will be dashes for formatting.
        if (line[1] == '-') continue;

        // Only accept N lines of actual input.
        if (row > N-1) break;

        // Otherwise, read in digits (as characters, so that 123 is read as
        // three separate numbers).
        std::istringstream ss(line);
        char ch;
        int column = 0;
        while (ss >> ch) {
            // Ignore whitespace or other formatting.
            if (ch == ' ' || ch == '|') continue;

            // Don't accept more that N columns of actual input, nor anything
            // that isn't formatting or numbers.
            if (/*column > N-1 || */!std::isdigit(ch)) {
                is.setstate(std::ios::failbit);
                return is;
            }

            grid[row][column] = ch - '0';
            column++;
        }
        row++;
    }

    b = Board(grid);
    is.clear(std::ios_base::goodbit);
    return is;
}

std::ostream& operator<<(std::ostream& os, const Board& b)
{
    return os << b.str(false);
}
