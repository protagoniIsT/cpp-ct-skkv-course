#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class WinWindow : public QDialog
{
    Q_OBJECT

public:
    explicit WinWindow(QWidget *parent = nullptr);

private:
    QLabel *messageLabel;
    QLabel *infoLabel;
    QPushButton *okButton;
};
