#pragma once

#include "Field.h"
#include "SettingsWindow.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QToolBar>
#include <QTranslator>
#include <QVector>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(bool clearModeEnabled, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleLMB();
    void handleRMB(QPushButton *button);
    void handleMMB(QPushButton *button);
    void checkWin();
    void revealAllCells();
    void toggleClearMode(bool checked);
    void startNewGame();
    void saveGame();
    void loadGame();
    bool iniFileEmpty();
    void switchLanguage(const QString &language);
    void setLangChecked();
    void restartGame();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void createTranslationAction(const QString &languageName, const QString &languageCode, QMenu *languageMenu);
    void updateTranslations();
    QByteArray calculateFileHash(const QString &filePath);

private:
    void setupGame();
    void createButtons(int rows, int cols);
    void clearField();
    void reveal(int row, int col);
    QString setNumColor(int num);
    QString gameSaveFile;
    Field *field;
    int rows;
    int cols;
    int mines;
    QVector< QVector< QPushButton * > > buttons;
    QGridLayout *gridLayout;
    QCheckBox *clearModeCheckBox;
    bool mode;
    QPushButton *lastPressedButton;
    QToolBar *toolbar;
    QIcon mineIcon;
    QIcon flagIcon;
    int buttonSize;

    void saveFieldState(QSettings &settings);
    void loadFieldState(QSettings &settings);

    void createActions();
    QTranslator translator;
    QActionGroup *languageActionGroup;
    QString currentLanguage;
    QAction *newGameAction;
    QMenu *languageMenu;
    QByteArray previousHash;
    QSet< QPushButton * > highlightedCells;
    QMap< QPushButton *, QString > originalStyles;
    QAction *restartGameAction;
    bool isGameFinished;
};
