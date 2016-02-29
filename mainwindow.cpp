#include <QFileDialog>
#include <QThread>
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

    m_solver = nullptr;
    m_solver_thread = nullptr;
}

MainWindow::~MainWindow()
{
    delete solve_button;
}

// ----------------------------------------------------------------------------

void MainWindow::handle_open()
{
    handle_clear();

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

    m_solver_thread = new QThread(this);
    m_solver = new Solver(m_in_board);
    m_solver->moveToThread(m_solver_thread);

    connect(m_solver_thread, SIGNAL(started()), m_solver, SLOT(solve()));
    connect(m_solver, SIGNAL(finished()), m_solver_thread, SLOT(quit()));
    connect(m_solver, SIGNAL(finished()), this, SLOT(handle_finish_solve()));

    print_waiting();
    m_solver_thread->start();
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
    if (m_solver != nullptr) {
        m_solver->cancel();
    }

    m_in_board.clear();
    m_out_board.clear();

    clear_output();
    print_grid();

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

void MainWindow::handle_finish_solve()
{
    if (m_solver->cancelled()) {
        return;
    }

    m_out_board = m_solver->board();

    if (!m_solver->solvable()) {
        clear_output();
        alert("Unsolvable");
    } else {
        print_output(m_solver->milliseconds());
    }

    delete m_solver;
    delete m_solver_thread;
    m_solver = nullptr;
    m_solver_thread = nullptr;
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
    print_grid();

    QString timer_text = timer_label->property("display_text").toString() +
                         QString::number(milliseconds) + " ms";
    timer_label->setText(timer_text);

    QString count_text = count_label->property("display_text").toString() +
                         QString::number(m_out_board.count());
    count_label->setText(count_text);

    // Print the numbers in the grid
    for (std::size_t row = 0; row < 9; ++row) {
        for (std::size_t col = 0; col < 9; ++col) {
            bool original = (m_in_board[row][col] == m_out_board[row][col]);

            QString output = QString::number(m_out_board[row][col]);
            QGraphicsTextItem* text_item = output_scene->addText(output);

            if (original) {
                output = "<font color=\"red\"><b>" + output + "</b></font>";
                text_item->setHtml(output);
            }

            QFont font;
            font.setPixelSize(m_size*0.7/9);
            text_item->setFont(font);

            text_item->setPos(m_size*col/9, m_size*0.98*row/9);
        }
    }
}

void MainWindow::print_waiting()
{
    clear_output();
    QGraphicsTextItem* text_item = output_scene->addText("Waiting...");
    text_item->setPos(0.8 * m_size / 2, m_size / 2);
}

void MainWindow::print_grid()
{
    QPen pen = QPen();
    pen.setWidth(m_big_width);

    output_scene->addRect(QRectF(0, 0, m_size, m_size), pen);

    for (int i = 1; i < 9; ++i) {
        if (i % 3 == 0) pen.setWidth(m_big_width);
        else pen.setWidth(m_small_width);

        output_scene->addLine(QLineF(m_size*i/9, 0, m_size*i/9, m_size), pen);
        output_scene->addLine(QLineF(0, m_size*i/9, m_size, m_size*i/9), pen);
    }
}

void MainWindow::clear_output()
{
    timer_label->setText(timer_label->property("display_text").toString());
    count_label->setText(count_label->property("display_text").toString());

    for (auto i : output_scene->items()) {
        output_scene->removeItem(i);
        delete i;
    }
    /*
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
    */
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
    output_scene = new QGraphicsScene();

    output_view = new QGraphicsView(output_scene, this);
    output_view->setGeometry(300, 50, m_size+m_big_width*2, m_size+m_big_width*2);

    print_grid();
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

