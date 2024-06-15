#include "WinWindow.h"

#include <QSizePolicy>
#include <QSpacerItem>

WinWindow::WinWindow(QWidget *parent) : QDialog(parent)
{
    setStyleSheet("background-color: rgb(255, 255, 255);");
    setWindowTitle("Congratulations!");
    setWindowIcon(QIcon(":/win_icon.ico"));

    messageLabel = new QLabel(tr("Congratulations! You win!"), this);
    QFont messageFont("Sans", 10);
    messageLabel->setFont(messageFont);

    infoLabel = new QLabel(tr("Click OK to reveal all cells."), this);
    QFont infoFont("Sans", 10);
    infoLabel->setFont(infoFont);

    okButton = new QPushButton("OK", this);
    okButton->setStyleSheet("QPushButton { border: 2px solid blue; }");
    okButton->setFixedSize(100, 40);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(messageLabel, 0, Qt::AlignCenter);
    layout->addWidget(infoLabel, 0, Qt::AlignCenter);

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->addItem(verticalSpacer);

    layout->addWidget(okButton, 0, Qt::AlignCenter);

    setLayout(layout);
    resize(400, 200);
}
