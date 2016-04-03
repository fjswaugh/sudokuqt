SudokuQt
========

This is a simple Sudoku solving program with a Qt graphical interface.

Prerequisites
-------------

- Qt5
- qmake
- make
- g++

Build instructions
------------------

Enter the directory from the terminal and type

```bash
qmake && make
```

These commands will not install anything on the system, just produce an
executable in the current directory. To run, type

```bash
./sudokuqt
```

Program usage
-------------

The program can import sudoku boards from text files formatted as either...

```
+-----------------------+
| 0 0 2 | 0 4 0 | 8 0 0 | 
| 4 7 0 | 0 0 0 | 0 5 3 | 
| 0 3 0 | 2 0 6 | 0 4 7 | 
|-------+-------+-------|
| 0 0 0 | 0 0 0 | 0 0 0 | 
| 5 0 0 | 7 0 3 | 0 0 2 | 
| 0 6 0 | 4 0 8 | 0 1 0 | 
|-------+-------+-------|
| 0 0 7 | 0 9 0 | 5 0 0 | 
| 3 0 0 | 8 0 5 | 0 0 1 | 
| 0 2 0 | 0 0 0 | 0 8 0 | 
+-----------------------+
```

or

```
002040800
470000053
030206047
000000000
500703002
060408010
007090500
300805001
020000080
```
