#include <QFileDialog>
#include <QTextBrowser>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QMessageBox>
#include <QMenuBar>

#include <fstream>
#include <sstream>

#include "mainwindow.h"
#include "sudoku.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->resize(600, 400);
    create_menus();

    solve_button = new QPushButton(this);
    solve_button->setText("Solve");
    solve_button->setGeometry(30, 310, 242, 20);
    connect(solve_button, SIGNAL(clicked()), this, SLOT(handle_solve()));

    clear_button = new QPushButton(this);
    clear_button->setText("Clear");
    clear_button->setGeometry(30, 350, 242, 20);
    connect(clear_button, SIGNAL(clicked()), this,  SLOT(handle_clear()));

    textb_solved = new QTextBrowser(this);
    textb_solved->setGeometry(320, 50, 192, 242);
    textb_solved->setFontFamily("monospace");

    create_input_array();

    //connect(actionOpen, SIGNAL(triggered()), this, SLOT(handleOpen()));
    //connect(actionClose, SIGNAL(triggered()), this, SLOT(close()));
}

MainWindow::~MainWindow()
{
    delete solve_button;
}

void MainWindow::handle_open()
{
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open sudoku puzzle from file"));
    std::ifstream file(file_name.toStdString());
    if (!(file >> m_board)) {
        QMessageBox message_box;
        message_box.setText("Error reading file");
        message_box.exec();
    } else {
        for (std::size_t row = 0; row < 9; ++row) {
            for (std::size_t col = 0; col < 9; ++col) {
                std::string text = std::to_string(m_board.original_grid()[row][col]);
                if (text == "0") text = "";
                input_array[row][col]->setText(text.c_str());
            }
        }
        std::stringstream ss;
        ss << m_board;
        QString board_display = ss.str().c_str();
        //textBrowser->setText(board_display);
    }
}

void MainWindow::handle_solve()
{
    update_board();

    m_board = Board(grid);
    if (m_board.contradictory()) {
        QMessageBox message_box;
        message_box.setText("Puzzle input is incorrect");
        message_box.exec();
        return;
    }

    QString old_textb_contents = textb_solved->document()->toPlainText();
    textb_solved->setText("Working...");
    textb_solved->repaint();

    if (!m_board.solve()) {
        QMessageBox message_box;
        message_box.setText("Unsolvable puzzle");
        message_box.exec();

        textb_solved->setText(old_textb_contents);
        return;
    }
    std::stringstream ss;
    ss << m_board;
    QString board_display = ss.str().c_str();
    textb_solved->setText(board_display);
}

void MainWindow::handle_clear()
{
    m_board.clear();
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            input_array[row][col]->setText("");
        }
    }
    textb_solved->setText("");
}

void MainWindow::handle_save()
{
    update_board();

    QString file = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                                QString(),
                                                tr("All Files (*)"));
    std::ofstream ofs(file.toStdString().c_str());
    ofs << m_board;
}

void MainWindow::update_board()
{
    m_board.clear();
    Grid_t grid;
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            std::string input = input_array[row][col]->text().toStdString();
            if (input.empty()) input = "0";
            if (input.length() > 1 || !std::isdigit(input[0])) {
                QMessageBox message_box;
                message_box.setText("Bad input");
                message_box.exec();
                return;
            }
            int number_input = std::stoi(input);
            grid[row][col] = number_input;
        }
    }
}

void MainWindow::create_menus()
{
    open_action = new QAction(tr("&Open a sudoku file"), this);
    close_action = new QAction(tr("Close"), this);
    save_action = new QAction(tr("&Save this puzzle"), this);

    file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(open_action);
    file_menu->addAction(close_action);
    file_menu->addAction(save_action);

    connect(open_action, SIGNAL(triggered()), this, SLOT(handle_open()));
    connect(close_action, SIGNAL(triggered()), this, SLOT(close()));
    connect(save_action, SIGNAL(triggered()), this, SLOT(handle_save()));
}

void MainWindow::create_input_array()
{
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            input_array[row][col] = new QLineEdit(this);
            int x = 30 + (col * 26) + 5 * (col / 3);
            int y = 50 + (row * 26) + 5 * (row / 3);
            input_array[row][col]->setGeometry(x, y, 24, 24);
        }
    }
}
