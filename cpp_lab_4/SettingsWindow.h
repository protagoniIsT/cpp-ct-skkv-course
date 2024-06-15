#pragma once

#include <QDialog>
#include <QPushButton>
#include <QSpinBox>

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent = nullptr);
    int getRows() const;
    int getCols() const;
    int getMines() const;

private slots:
    void updateMinesRange();

private:
    QSpinBox *rowsSpinBox;
    QSpinBox *colsSpinBox;
    QSpinBox *minesSpinBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
};
