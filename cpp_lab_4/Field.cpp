#include "Field.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>

Field::Field(int rows_cnt, int cols_cnt, int mines_cnt, bool initializeMines, QObject* parent) :
      QObject(parent), rows_cnt(rows_cnt), cols_cnt(cols_cnt), mines_cnt(mines_cnt), firstMove(initializeMines),
      field(rows_cnt, QVector< Cell* >(cols_cnt))
{
    for (int i = 0; i < rows_cnt; ++i)
    {
        for (int j = 0; j < cols_cnt; ++j)
        {
            field[i][j] = new Cell(Cell::EMPTY, Cell::UNREVEALED, this);
        }
    }
    if (!initializeMines)
    {
        placeMines(-1, -1);
    }
}

Field::~Field()
{
    for (int i = 0; i < rows_cnt; ++i)
    {
        for (int j = 0; j < cols_cnt; ++j)
        {
            delete field[i][j];
        }
    }
    field.clear();
}

int Field::getRows() const
{
    return rows_cnt;
}

int Field::getCols() const
{
    return cols_cnt;
}

int Field::getMinesCnt() const
{
    return mines_cnt;
}

QVector< QVector< Cell* > >& Field::getField()
{
    return field;
}

void Field::placeMines(int firstRow, int firstCol)
{
    std::srand(std::time(nullptr));
    int placed_cnt = 0;
    while (placed_cnt < mines_cnt)
    {
        int row = std::rand() % rows_cnt;
        int col = std::rand() % cols_cnt;
        if ((row != firstRow || col != firstCol) && field[row][col]->getType() != Cell::MINE)
        {
            field[row][col]->setType(Cell::MINE);
            ++placed_cnt;
        }
    }
    calcNearbyCnt();
}

void Field::calcNearbyCnt()
{
    for (int i = 0; i < rows_cnt; ++i)
    {
        for (int j = 0; j < cols_cnt; ++j)
        {
            if (field[i][j]->getType() == Cell::MINE)
                continue;
            int count = 0;
            for (int x = -1; x <= 1; ++x)
            {
                for (int y = -1; y <= 1; ++y)
                {
                    int newRow = i + x;
                    int newCol = j + y;
                    if (newRow >= 0 && newRow < rows_cnt && newCol >= 0 && newCol < cols_cnt &&
                        field[newRow][newCol]->getType() == Cell::MINE)
                    {
                        ++count;
                    }
                }
            }
            field[i][j]->setMinesNearby(count);
        }
    }
}

void Field::revealCell(int row, int col)
{
    if (firstMove)
    {
        placeMines(row, col);
        firstMove = false;
    }
    if (field[row][col]->getStatus() != Cell::UNREVEALED)
        return;
    field[row][col]->setStatus(Cell::REVEALED);
    if (field[row][col]->getMinesNearby() == 0)
    {
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                int newRow = row + x;
                int newCol = col + y;
                if (newRow >= 0 && newRow < rows_cnt && newCol >= 0 && newCol < cols_cnt)
                {
                    revealCell(newRow, newCol);
                }
            }
        }
    }
}

void Field::toggleFlag(int row, int col)
{
    if (field[row][col]->getStatus() == Cell::UNREVEALED)
    {
        field[row][col]->setStatus(Cell::FLAGGED);
    }
    else if (field[row][col]->getStatus() == Cell::FLAGGED)
    {
        field[row][col]->setStatus(Cell::UNREVEALED);
    }
}
