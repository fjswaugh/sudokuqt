#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <array>

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    // Event handlers
    void handle_open();
    void handle_solve();
    void handle_clear();
    void handle_save();
    void handle_copy_input_board();
    void handle_copy_output_board();

private:
    // Event Filter
    bool eventFilter(QObject* obj, QEvent* event);

    // Utility functions
    bool update_board();
    void print_output(unsigned long int milliseconds);
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
};

#endif // MAINWINDOW_H

