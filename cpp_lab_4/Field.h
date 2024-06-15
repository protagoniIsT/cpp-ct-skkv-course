#pragma once

#include "Cell.h"

#include <QObject>
#include <QVector>

class Field : public QObject
{
    Q_OBJECT

private:
    int rows_cnt;
    int cols_cnt;
    int mines_cnt;
    bool firstMove;
    bool mode;
    QVector< QVector< Cell* > > field;

    void placeMines(int firstRow, int firstCol);
    void calcNearbyCnt();

public:
    explicit Field(int rows_cnt = 10, int cols_cnt = 10, int mines_cnt = 10, bool firstMove = true, QObject* parent = nullptr);
    ~Field();

    int getRows() const;
    int getCols() const;
    int getMinesCnt() const;

    QVector< QVector< Cell* > >& getField();

    void revealCell(int row, int col);
    void toggleFlag(int row, int col);
};
