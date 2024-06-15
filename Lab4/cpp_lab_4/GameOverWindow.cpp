#include "GameOverWindow.h"

#include <QSizePolicy>
#include <QSpacerItem>

GameOverWindow::GameOverWindow(QWidget *parent) : QDialog(parent)
{
    setStyleSheet("background-color: rgb(255, 255, 255);");
    setWindowTitle(tr("Game Over"));
    setWindowIcon(QIcon(":/sad_icon.ico"));

    messageLabel = new QLabel(tr("Oh no! You hit a mine!"), this);
    QFont messageFont("Sans", 10);
    messageLabel->setFont(messageFont);

    okButton = new QPushButton("OK", this);
    okButton->setStyleSheet("QPushButton { border: 1px solid blue; }");
    okButton->setFixedSize(100, 40);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QSpacerItem *topSpacer = new QSpacerItem(20, 60, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->addItem(topSpacer);

    layout->addWidget(messageLabel, 0, Qt::AlignCenter);

    QSpacerItem *bottomSpacer = new QSpacerItem(20, 60, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->addItem(bottomSpacer);

    layout->addWidget(okButton, 0, Qt::AlignCenter);

    setLayout(layout);
    resize(400, 200);
}
