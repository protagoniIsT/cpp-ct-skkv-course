#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class GameOverWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GameOverWindow(QWidget *parent = nullptr);

private:
    QLabel *messageLabel;
    QLabel *infoLabel;
    QPushButton *okButton;
};
