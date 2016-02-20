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
        alert("Error reading file");
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
    if (!update_board()) {
        alert("Bad input");
        return;
    }

    if (m_board.contradictory()) {
        alert("Puzzle input is incorrect");
        return;
    }

    if (!m_board.solve()) {
        alert("Unsolvable puzzle");
    } else {
        print_output();
    }
}

void MainWindow::print_output()
{
    clear_output();

    const int view_size = output_scene->width();
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            int out_num = m_board.grid()[row][col];
            bool original = (m_board.original_grid()[row][col] == out_num);
            std::string out_text = std::to_string(out_num);

            QGraphicsTextItem* text_item = output_scene->addText(out_text.c_str());

            if (original) {
                out_text = "<font color=\"red\"><b>" + out_text + "</b></font>";
                text_item->setHtml(out_text.c_str());
            }

            QFont font = QFont();
            font.setPixelSize(view_size*0.7/9);
            text_item->setFont(font);
            text_item->setPos(view_size*col/9, view_size*0.98*row/9);
        }
    }
}

void MainWindow::clear_output()
{
    const int view_size = output_scene->width();
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            QGraphicsItem* text_item = output_scene->itemAt(view_size*col/9, view_size*row/9, QTransform());
    
            while (text_item && dynamic_cast<QGraphicsTextItem*>(text_item)) {
                output_scene->removeItem(text_item);
                delete text_item;
                text_item = output_scene->itemAt(view_size*col/9, view_size*0.98*row/9, QTransform());
            }

        }
    }
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

void MainWindow::alert(const std::string& message)
{
    QMessageBox message_box;
    message_box.setText(message.c_str());
    message_box.exec();
}

void MainWindow::handle_save()
{
    if (!update_board()) {
        alert("Bad input");
    }

    QString file = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                                QString(),
                                                tr("All Files (*)"));
    std::ofstream ofs(file.toStdString().c_str());
    ofs << m_board;
}

bool MainWindow::update_board()
{
    m_board.clear();
    Grid_t grid;
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            std::string input = input_array[row][col]->text().toStdString();
            if (input.empty()) input = "0";
            if (input.length() > 1 || !std::isdigit(input[0])) {
                return false;
            }
            int number_input = std::stoi(input);
            grid[row][col] = number_input;
        }
    }

    m_board = Board(grid);
    return true;
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
    constexpr int big_width = 2;
    constexpr int small_width = 1;
    QPen pen = QPen();
    pen.setWidth(big_width);

    output_scene = new QGraphicsScene();

    constexpr int size = 240;

    output_scene->addRect(QRectF(0, 0, size, size), pen);
    for (int i = 1; i < 9; ++i) {
        if (i % 3 == 0) pen.setWidth(big_width);
        else pen.setWidth(small_width);

        output_scene->addLine(QLineF(size*i/9, 0, size*i/9, size), pen);
        output_scene->addLine(QLineF(0, size*i/9, size, size*i/9), pen);
    }

    output_view = new QGraphicsView(output_scene, this);
    output_view->setGeometry(300, 50, size+big_width*2, size+big_width*2);
}

