#pragma once

#include <QObject>

class Cell : public QObject
{
    Q_OBJECT

public:
    enum Status
    {
        UNREVEALED,
        REVEALED,
        FLAGGED
    };

    enum Type
    {
        EMPTY,
        MINE
    };

private:
    Type type;
    Status status;
    int minesNearby;

public:
    explicit Cell(Type type = EMPTY, Status status = UNREVEALED, QObject* parent = nullptr);
    explicit Cell(QObject* parent = nullptr);

    void setStatus(Status status);
    Status getStatus() const;

    void setType(Type type);
    Type getType() const;

    void setMinesNearby(int mines);
    int getMinesNearby() const;

    void toggleFlag();

    Cell(const Cell&) = delete;
    Cell& operator=(const Cell&) = delete;
};
