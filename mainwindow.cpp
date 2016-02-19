#include <QFileDialog>
#include <QTextBrowser>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QMessageBox>
#include <QMenuBar>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QPen>

#include <fstream>
#include <sstream>

#include "mainwindow.h"
#include "sudoku.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->resize(800, 400);
    create_menus();

    create_output_view();

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
    }
}

void MainWindow::handle_solve()
{
    update_board();

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
    } else {
        print_grid_in_output();
    }
}

void MainWindow::print_grid_in_output()
{
    clear_output();

    std::stringstream ss;
    ss << m_board;
    QString board_display = ss.str().c_str();
    textb_solved->setText(board_display);

    constexpr int view_size = 150;
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            QString text_str = std::to_string(m_board.grid()[row][col]).c_str();
            QGraphicsTextItem* text_item = output_scene->addText(text_str);
            text_item->setPos(view_size*col/9, view_size*row/9);
        }
    }
}

void MainWindow::clear_output()
{
    constexpr int view_size = 150;
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            QGraphicsItem* text_item = output_scene->itemAt(view_size*col/9, view_size*row/9, QTransform());
    
            while (text_item && dynamic_cast<QGraphicsTextItem*>(text_item)) {
                output_scene->removeItem(text_item);
                delete text_item;
                text_item = output_scene->itemAt(view_size*col/9, view_size*row/9, QTransform());
            }

        }
    }
    textb_solved->setText("");
}

void MainWindow::handle_clear()
{
    m_board.clear();

    clear_output();

    // Clear input
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            input_array[row][col]->setText("");
        }
    }
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

    m_board = Board(grid);
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

void MainWindow::create_output_view()
{
    QPen pen = QPen();
    pen.setWidth(2);

    output_scene = new QGraphicsScene();

    constexpr int size = 150;

    output_scene->addRect(QRectF(0, 0, size, size), pen);
    for (int i = 1; i < 9; ++i) {
        if (i % 3 == 0) pen.setWidth(2);
        else pen.setWidth(1);

        output_scene->addLine(QLineF(size*i/9, 0, size*i/9, size), pen);
        output_scene->addLine(QLineF(0, size*i/9, size, size*i/9), pen);
    }

    output_view = new QGraphicsView(output_scene, this);
    output_view->setGeometry(530, 50, 200, 200);

}

