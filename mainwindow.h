#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <array>

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>

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
    void handle_open();
    void handle_solve();
    void handle_clear();
    void handle_save();

private:
    Board m_board;

    void update_board();
    void create_menus();
    void create_input_array();

    QMenu* file_menu;
    QAction* open_action;
    QAction* close_action;
    QAction* save_action;

    std::array<std::array<QLineEdit*, 9>, 9> input_array;
    QPushButton* solve_button;
    QPushButton* clear_button;
    QTextBrowser* textb_solved;
};

#endif // MAINWINDOW_H
