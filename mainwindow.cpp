#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QKeyEvent>
#include <QShortcut>
#include <QKeySequence>
#include <QTextBrowser>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QMessageBox>
#include <QMenuBar>
#include <QLabel>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QPen>

#include <fstream>
#include <sstream>
#include <chrono>

#include "mainwindow.h"
#include "sudoku.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->resize(580, 380);
    this->setMinimumSize(580, 380);

    create_menus();
    create_output_view();
    create_buttons();
    create_input_array();
    create_labels();

    create_shortcuts();
}

MainWindow::~MainWindow()
{
    delete solve_button;
}

// ----------------------------------------------------------------------------

void MainWindow::handle_open()
{
    QString file_name = QFileDialog::getOpenFileName(this,
                            tr("Open sudoku puzzle from file"));
    if (file_name == "") return;

    std::ifstream file(file_name.toStdString());

    if (!(file >> m_in_board)) {
        alert("Error reading file");
    } else {
        for (std::size_t row = 0; row < 9; ++row) {
            for (std::size_t col = 0; col < 9; ++col) {
                int num = m_in_board[row][col];
                std::string text = std::to_string(num);
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
    
    if (m_in_board.contradictory()) {
        alert("Puzzle input is incorrect");
        return;
    }

    m_out_board = m_in_board;
    
    std::chrono::steady_clock::time_point begin_time =
        std::chrono::steady_clock::now();

    if (!m_out_board.solve()) {
        alert("Unsolvable puzzle");
    } else {
        std::chrono::steady_clock::time_point end_time =
            std::chrono::steady_clock::now();

        unsigned long int milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>
            (end_time - begin_time).count();

        print_output(milliseconds);
    }
}

void MainWindow::handle_save()
{
    if (!update_board()) {
        alert("Bad input");
        return;
    }

    QString file = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                                QString(),
                                                tr("All Files (*)"));
    std::ofstream ofs(file.toStdString().c_str());
    ofs << m_in_board.str();
}

void MainWindow::handle_clear()
{
    m_in_board.clear();
    m_out_board.clear();

    clear_output();

    // Clear input
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            input_array[row][col]->setText("");
        }
    }
}

void MainWindow::handle_copy_input_board()
{
    copy_board(true);
}

void MainWindow::handle_copy_output_board()
{
    copy_board(false);
}

// ----------------------------------------------------------------------------

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj->property("input") == true) {
        std::size_t row = obj->property("row").toInt();
        std::size_t col = obj->property("col").toInt();

        if (event->type() == QEvent::FocusIn) {
            input_array[row][col]->selectAll();
        }
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* key_event = static_cast<QKeyEvent*>(event);

            if (key_event->key() == Qt::Key_Up) {
                if (row == 0) row = 9;
                input_array[row-1][col]->setFocus();
                return true;
            } else if (key_event->key() == Qt::Key_Down) {
                if (row == 8) row = -1;
                input_array[row+1][col]->setFocus();
                return true;
            } else if (key_event->key() == Qt::Key_Left) {
                if (col == 0) col = 9;
                input_array[row][col-1]->setFocus();
                return true;
            } else if (key_event->key() == Qt::Key_Right) {
                if (col == 8) col = -1;
                input_array[row][col+1]->setFocus();
                return true;
            }
        }
        return false;
    }
    return QMainWindow::eventFilter(obj, event);
}

// ----------------------------------------------------------------------------

bool MainWindow::update_board()
{
    m_in_board.clear();

    std::array<std::array<int, 9>, 9> grid;
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            std::string input = input_array[row][col]->text().toStdString();

            if (input.empty()) input = "0";
            if (input.length() > 1 || !std::isdigit(input[0])) {
                return false;
            }

            int num_input = std::stoi(input);
            grid[row][col] = num_input;
        }
    }

    m_in_board = Board(grid);
    return true;
}

void MainWindow::print_output(unsigned long int milliseconds)
{
    clear_output();

    std::string text;

    text = timer_label->property("display_text").toString().toStdString();
    text += std::to_string(milliseconds);
    text += " ms";
    timer_label->setText(text.c_str());

    text = count_label->property("display_text").toString().toStdString();
    text += std::to_string(m_out_board.count());
    count_label->setText(text.c_str());

    const int view_size = output_scene->width();

    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            int out_num = m_out_board[row][col];
            bool original = (m_in_board[row][col] == out_num);
            std::string out_text = std::to_string(out_num);

            QGraphicsTextItem* text_item =
                output_scene->addText(out_text.c_str());

            if (original) {
                out_text = "<font color=\"red\"><b>" +
                           out_text +
                           "</b></font>";
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
    timer_label->setText(timer_label->property("display_text").toString());
    count_label->setText(count_label->property("display_text").toString());

    const int view_size = output_scene->width();
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            QGraphicsItem* text_item = output_scene->itemAt(view_size*col/9,
                                                            view_size*row/9,
                                                            QTransform());
    
            while (text_item && dynamic_cast<QGraphicsTextItem*>(text_item)) {
                output_scene->removeItem(text_item);
                delete text_item;

                text_item = output_scene->itemAt(view_size*col/9,
                                                 view_size*0.98*row/9,
                                                 QTransform());
            }

        }
    }
}

void MainWindow::alert(const std::string& message)
{
    QMessageBox message_box;
    message_box.setText(message.c_str());
    message_box.exec();
}

void MainWindow::copy_board(bool input_board)
{
    if (!update_board()) {
        alert("Bad input");
        return;
    }

    QClipboard* clipboard = QApplication::clipboard();
    Board& b = (input_board ? m_in_board : m_out_board);
    clipboard->setText(b.str().c_str());
}

// ----------------------------------------------------------------------------

void MainWindow::create_menus()
{
    open_action = new QAction(tr("&Open a sudoku file"), this);
    save_action = new QAction(tr("&Save this puzzle"), this);
    close_action = new QAction(tr("&Exit"), this);

    file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(open_action);
    file_menu->addAction(save_action);
    file_menu->addAction(close_action);

    connect(open_action, SIGNAL(triggered()), this, SLOT(handle_open()));
    connect(close_action, SIGNAL(triggered()), this, SLOT(close()));
    connect(save_action, SIGNAL(triggered()), this, SLOT(handle_save()));

    copy_input_board_action = new QAction(tr("&Copy input sudoku board"),
                                          this);
    copy_output_board_action = new QAction(tr("Copy &output sudoku board"),
                                           this);

    edit_menu = menuBar()->addMenu(tr("&Edit"));
    edit_menu->addAction(copy_input_board_action);
    edit_menu->addAction(copy_output_board_action);

    connect(copy_input_board_action,
            SIGNAL(triggered()),
            this,
            SLOT(handle_copy_input_board()));
    connect(copy_output_board_action,
            SIGNAL(triggered()),
            this,
            SLOT(handle_copy_output_board()));
}

void MainWindow::create_input_array()
{
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            input_array[row][col] = new QLineEdit(this);

            int x = 30 + (col * 26) + 5 * (col / 3);
            int y = 50 + (row * 26) + 5 * (row / 3);
            input_array[row][col]->setGeometry(x, y, 24, 24);

            input_array[row][col]->setProperty("input", true);
            input_array[row][col]->setProperty("row", (int)row);
            input_array[row][col]->setProperty("col", (int)col);

            connect(input_array[row][col],
                    SIGNAL(returnPressed()),
                    this,
                    SLOT(handle_solve()));

            input_array[row][col]->installEventFilter(this);
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

void MainWindow::create_buttons()
{
    solve_button = new QPushButton(this);
    solve_button->setText("Solve");
    solve_button->setGeometry(30, 310, 242, 20);
    connect(solve_button, SIGNAL(clicked()), this, SLOT(handle_solve()));

    clear_button = new QPushButton(this);
    clear_button->setText("Clear");
    clear_button->setGeometry(30, 340, 242, 20);
    connect(clear_button, SIGNAL(clicked()), this,  SLOT(handle_clear()));
}

void MainWindow::create_labels()
{
    timer_label = new QLabel(this);
    QString display_text = "Time taken: ";
    timer_label->setProperty("display_text" , display_text);
    timer_label->setText(display_text);

    const int x_pos = 300;
    const int y_pos = output_view->geometry().height() + 50 + 15;
    const int x_size = output_view->geometry().width();
    timer_label->setGeometry(x_pos, y_pos, x_size, 20);

    count_label = new QLabel(this);
    display_text = "Number of numbers tried: ";
    count_label->setProperty("display_text", display_text);
    count_label->setText(display_text);

    count_label->setGeometry(x_pos, y_pos + 30, x_size, 20);
}

void MainWindow::create_shortcuts()
{
    open_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
    connect(open_shortcut, SIGNAL(activated()), this, SLOT(handle_open()));

    save_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this);
    connect(save_shortcut, SIGNAL(activated()), this, SLOT(handle_save()));
}

