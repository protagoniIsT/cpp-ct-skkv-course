#include "SettingsWindow.h"

#include <QFont>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(QWidget *parent) :
      QDialog(parent), rowsSpinBox(new QSpinBox(this)), colsSpinBox(new QSpinBox(this)), minesSpinBox(new QSpinBox(this)),
      okButton(new QPushButton("OK", this)), cancelButton(new QPushButton("Cancel", this))
{
    setStyleSheet("background-color: rgb(255, 255, 255)");
    setWindowTitle(tr("Game Settings"));
    setWindowIcon(QIcon(":/settings_window_icon.ico"));
    resize(200, 200);

    rowsSpinBox->setRange(2, 100);
    colsSpinBox->setRange(2, 100);
    minesSpinBox->setRange(1, 9999);

    rowsSpinBox->setValue(10);
    colsSpinBox->setValue(10);
    minesSpinBox->setValue(10);

    QFont font;
    font.setPointSize(10);
    rowsSpinBox->setFont(font);
    colsSpinBox->setFont(font);
    minesSpinBox->setFont(font);
    okButton->setFont(font);
    okButton->setStyleSheet("QPushButton { border: 1px solid blue; }");
    okButton->setFixedSize(100, 40);
    cancelButton->setFont(font);
    cancelButton->setStyleSheet("QPushButton { border: 1px solid blue; }");
    cancelButton->setFixedSize(100, 40);

    QLabel *rowsLabel = new QLabel(tr("Rows:"), this);
    QLabel *colsLabel = new QLabel(tr("Columns:"), this);
    QLabel *minesLabel = new QLabel(tr("Mines:"), this);

    rowsLabel->setFont(font);
    colsLabel->setFont(font);
    minesLabel->setFont(font);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(rowsLabel, rowsSpinBox);
    formLayout->addRow(colsLabel, colsSpinBox);
    formLayout->addRow(minesLabel, minesSpinBox);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    connect(okButton, &QPushButton::clicked, this, &SettingsWindow::accept);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsWindow::reject);
    connect(rowsSpinBox, QOverload< int >::of(&QSpinBox::valueChanged), this, &SettingsWindow::updateMinesRange);
    connect(colsSpinBox, QOverload< int >::of(&QSpinBox::valueChanged), this, &SettingsWindow::updateMinesRange);

    updateMinesRange();
}

int SettingsWindow::getRows() const
{
    return rowsSpinBox->value();
}

int SettingsWindow::getCols() const
{
    return colsSpinBox->value();
}

int SettingsWindow::getMines() const
{
    return minesSpinBox->value();
}

void SettingsWindow::updateMinesRange()
{
    int maxMines = rowsSpinBox->value() * colsSpinBox->value() - 1;
    minesSpinBox->setMaximum(maxMines);
    if (minesSpinBox->value() > maxMines)
    {
        minesSpinBox->setValue(maxMines);
    }
}
