#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <array>
#include <chrono>

#include <QThread>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QShortcut>
#include <QLabel>

#include <QGraphicsView>
#include <QGraphicsScene>

#include "sudoku.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

class Solver : public QObject {
Q_OBJECT
public:
    Solver()
        : m_board(Board()), m_milliseconds(0),
          m_solvable(true), m_solving(false), m_cancelled(false)
    {}

    Board board() const { return m_board; }

    unsigned long int milliseconds() const { return m_milliseconds; }

    bool solvable() const { return m_solvable; }
    bool solving() const { return m_solving; }
    bool cancelled() const { return m_cancelled; }

    void cancel()
    {
        m_cancelled = true;
        
        // Cancel the board. This means that any attempt to call
        // m_board.solve() will simply return without actually solving
        // anything. Any currently running solve attempt should also exit
        // immediately.
        m_board.cancel();
    }

    void set_board(Board board)
    {
        m_board = board;
    }
public slots:
    void solve()
    {
        m_cancelled = false;
        m_solving = true;
        m_solvable = true;
        m_milliseconds = 0;

        std::chrono::steady_clock::time_point begin_time =
            std::chrono::steady_clock::now();

        m_solvable = m_board.solve();

        std::chrono::steady_clock::time_point end_time =
            std::chrono::steady_clock::now();

        m_milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>
            (end_time - begin_time).count();

        m_solving = false;
        emit finished();
    }
signals:
    void finished();
private:
    Board m_board;
    unsigned long int m_milliseconds;
    bool m_solvable;
    bool m_solving;
    bool m_cancelled;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    // Signal to send solver object in solving thread to prompt it to solve
    // current sudoku
    void solve();

private slots:
    // UI handlers
    void handle_open();
    void handle_solve();
    void handle_clear();
    void handle_save();
    void handle_copy_input_board();
    void handle_copy_output_board();

    // Handler for when the solving of the puzzle ends (not connected to UI
    // directly)
    void handle_finish_solve();
private:
    // Custom eventFilter function to add handling for arrow and enter keys in
    // input fields
    bool eventFilter(QObject* obj, QEvent* event);

    // Utility functions
    bool update_board();
    void print_output(unsigned long int milliseconds);
    void print_waiting();
    void print_grid();
    void clear_output();
    void alert(const std::string& message);
    void copy_board(bool input_board);

    // Functions to create elements of UI
    void create_menus();
    void create_input_array();
    void create_output_view();
    void create_buttons();
    void create_labels();
    void create_shortcuts();

    // UI elements
    QMenu* file_menu;
    QAction* open_action;
    QAction* close_action;
    QAction* save_action;
    QMenu* edit_menu;
    QAction* copy_input_board_action;
    QAction* copy_output_board_action;

    QPushButton* solve_button;
    QPushButton* clear_button;
    QGraphicsView* output_view;
    QLabel* timer_label;
    QLabel* count_label;
    QGraphicsScene* output_scene;
    std::array<std::array<QLineEdit*, 9>, 9> input_array;

    // Shortcuts
    QShortcut* open_shortcut;   
    QShortcut* save_shortcut;

    // Sudoku boards
    Board m_in_board;
    Board m_out_board;

    // Solving sudokus should be done in another thread to avoid hanging the
    // ui if the sudoku takes a long time to solve
    Solver* m_solver;
    QThread* m_solver_thread;

    // Some constants related to the window
    static constexpr int m_big_width = 2;
    static constexpr int m_small_width = 1;
    static constexpr int m_size = 240;
};

#endif // MAINWINDOW_H

