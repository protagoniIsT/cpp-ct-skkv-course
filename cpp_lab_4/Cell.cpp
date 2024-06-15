#include "Cell.h"

Cell::Cell(Type type, Status status, QObject* parent) : QObject(parent), type(type), status(status), minesNearby(0) {}

Cell::Cell(QObject* parent) : QObject(parent), type(EMPTY), status(UNREVEALED), minesNearby(0) {}

void Cell::setStatus(Status status)
{
    this->status = status;
}

Cell::Status Cell::getStatus() const
{
    return this->status;
}

void Cell::setType(Type type)
{
    this->type = type;
}

Cell::Type Cell::getType() const
{
    return this->type;
}

void Cell::setMinesNearby(int mines)
{
    this->minesNearby = mines;
}

int Cell::getMinesNearby() const
{
    return this->minesNearby;
}

void Cell::toggleFlag()
{
    if (status == UNREVEALED)
    {
        status = FLAGGED;
    }
    else if (status == FLAGGED)
    {
        status = UNREVEALED;
    }
}
