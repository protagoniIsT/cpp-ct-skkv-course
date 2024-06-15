#include "GameOverWindow.h"
#include "MainWindow.h"
#include "WinWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QCryptographicHash>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QSettings>
#include <QWidget>
#include <cstring>

#define UNREVEALED_CELL_COLOR "rgb(192, 192, 192)"
#define REVEALED_CELL_COLOR "rgb(90, 90, 90)"
#define MINE_CELL_COLOR "rgb(255, 0, 0)"
#define CLR_MODE_CELL_COLOR "rgb(150, 150, 150)"
#define CLR_MODE_MINE_COLOR "rgb(255, 102, 102)"
#define LAST_PRESSED_BUTTON_COLOR "rgb(255, 128, 0)"
#define FAST_REVEALED_CELL_COLOR "rgb(255, 0, 255)"

MainWindow::MainWindow(bool clearMode, QWidget *parent) :
      QMainWindow(parent), field(nullptr), rows(0), cols(0), mines(0), clearModeCheckBox(nullptr), mode(clearMode),
      lastPressedButton(nullptr), mineIcon(":/mine_icon.png"), flagIcon(":/flag_icon.png"), currentLanguage("en"),
      newGameAction(nullptr), languageMenu(nullptr), isGameFinished(false)
{
    gameSaveFile = QDir::currentPath() + "/game_state.ini";
    QSettings settings("Local", "MyApp");

    currentLanguage = settings.value("language", "en").toString();

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *vboxLayout = new QVBoxLayout(centralWidget);
    vboxLayout->setAlignment(Qt::AlignCenter);

    QHBoxLayout *hboxLayout = new QHBoxLayout();
    hboxLayout->setAlignment(Qt::AlignCenter);

    gridLayout = new QGridLayout();
    gridLayout->setSpacing(1);
    gridLayout->setMargin(0);

    hboxLayout->addLayout(gridLayout);
    vboxLayout->addLayout(hboxLayout);

    createActions();

    switchLanguage(currentLanguage);
    setLangChecked();

    QToolBar *toolBar = addToolBar(tr("Main Toolbar"));
    toolBar->addAction(newGameAction);
    toolBar->addAction(newGameAction);
    toolBar->addAction(restartGameAction);

    if (mode)
    {
        clearModeCheckBox = new QCheckBox(tr("Clear mode"), this);
        clearModeCheckBox->setFont(QFont("Sans", 12, QFont::Bold));
        clearModeCheckBox->setChecked(0);
        connect(clearModeCheckBox, &QCheckBox::toggled, this, &MainWindow::toggleClearMode);
        toolBar->addWidget(clearModeCheckBox);
    }

    if (QFile::exists(gameSaveFile) && !iniFileEmpty())
    {
        loadGame();
    }
    else
    {
        setupGame();
    }

    setWindowTitle(tr("Minesweeper"));
}

MainWindow::~MainWindow()
{
    saveGame();
    clearField();
    delete field;
}

bool MainWindow::iniFileEmpty()
{
    QSettings settings(gameSaveFile, QSettings::IniFormat);
    return settings.allKeys().isEmpty();
}

void MainWindow::setupGame()
{
    SettingsWindow dialog(this);
    dialog.setWindowTitle(tr("Game Settings"));
    dialog.resize(400, 300);
    if (dialog.exec() == QDialog::Accepted)
    {
        rows = dialog.getRows();
        cols = dialog.getCols();
        mines = dialog.getMines();
        field = new Field(rows, cols, mines, true, this);
        createButtons(rows, cols);
    }
    else
    {
        close();
    }
}

void MainWindow::restartGame()
{
    clearField();
    delete field;
    field = new Field(rows, cols, mines, true, this);
    createButtons(rows, cols);
}

void MainWindow::createButtons(int rows, int cols)
{
    buttons.resize(rows);

    QFont buttonFont;
    buttonFont.setPointSize(16);

    int buttonSize = 45;

    for (int i = 0; i < rows; ++i)
    {
        buttons[i].resize(cols);
        for (int j = 0; j < cols; ++j)
        {
            QPushButton *button = new QPushButton(this);
            button->setFixedSize(buttonSize, buttonSize);
            button->setFont(buttonFont);
            button->setStyleSheet("background-color: " UNREVEALED_CELL_COLOR ";");
            buttons[i][j] = button;
            gridLayout->addWidget(button, i, j);
            connect(button, &QPushButton::clicked, this, &MainWindow::handleLMB);
            button->installEventFilter(this);
            if (mode)
            {
                Cell *cell = field->getField()[i][j];
                if (cell->getType() == Cell::MINE)
                {
                    button->setIcon(mineIcon);
                    button->setIconSize(QSize(buttonSize - 5, buttonSize - 5));
                }
                else if (cell->getMinesNearby() > 0)
                {
                    button->setText(QString::number(cell->getMinesNearby()));
                    QFont sansFont("Sans", 16, QFont::Bold);
                    button->setFont(sansFont);
                }
            }
        }
    }
}

void MainWindow::clearField()
{
    for (int i = 0; i < buttons.size(); ++i)
    {
        for (int j = 0; j < buttons[i].size(); ++j)
        {
            delete buttons[i][j];
        }
    }
    buttons.clear();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast< QMouseEvent * >(event);
        if (mouseEvent->button() == Qt::RightButton)
        {
            QPushButton *button = qobject_cast< QPushButton * >(watched);
            if (button)
            {
                handleRMB(button);
                return true;
            }
        }
        else if (mouseEvent->button() == Qt::MiddleButton)
        {
            QPushButton *button = qobject_cast< QPushButton * >(watched);
            if (button)
            {
                handleMMB(button);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

QString MainWindow::setNumColor(int num)
{
    QString color;
    switch (num)
    {
    case 1:
        color = "rgb(176, 184, 255)";
        break;
    case 2:
        color = "rgb(0, 255, 25)";
        break;
    case 3:
        color = "rgb(255, 255, 130)";
        break;
    case 4:
        color = "rgb(245, 0, 255)";
        break;
    case 5:
        color = "rgb(0, 255, 255)";
        break;
    default:
        color = "rgb(255, 255, 255)";
        break;
    }
    return color;
}

void MainWindow::reveal(int row, int col)
{
    field->revealCell(row, col);
    for (int i = 0; i < field->getRows(); ++i)
    {
        for (int j = 0; j < field->getCols(); ++j)
        {
            Cell *cell = field->getField()[i][j];
            if (cell->getStatus() == Cell::REVEALED)
            {
                buttons[i][j]->setEnabled(false);
                buttons[i][j]->setStyleSheet("background-color: " REVEALED_CELL_COLOR ";");
                if (cell->getMinesNearby() > 0)
                {
                    QString color = setNumColor(cell->getMinesNearby());
                    buttons[i][j]->setText(QString::number(cell->getMinesNearby()));
                    buttons[i][j]->setStyleSheet(QString("color: %1; background-color: " REVEALED_CELL_COLOR ";").arg(color));
                    QFont sansFont("Sans", 16, QFont::Bold);
                    buttons[i][j]->setFont(sansFont);
                }
                else
                {
                    buttons[i][j]->setText("");
                }
            }
        }
    }
}

void MainWindow::revealAllCells()
{
    for (int i = 0; i < field->getRows(); ++i)
    {
        for (int j = 0; j < field->getCols(); ++j)
        {
            Cell *cell = field->getField()[i][j];
            if (cell->getStatus() == Cell::FLAGGED)
            {
                cell->setStatus(Cell::UNREVEALED);
                buttons[i][j]->setIcon(QIcon());
            }
            if (cell->getStatus() == Cell::UNREVEALED || cell->getStatus() == Cell::FLAGGED)
            {
                cell->setStatus(Cell::REVEALED);
                buttons[i][j]->setEnabled(false);
                buttons[i][j]->setStyleSheet("background-color: " CLR_MODE_CELL_COLOR ";");
                if (cell->getType() == Cell::MINE)
                {
                    buttons[i][j]->setIcon(mineIcon);
                    buttons[i][j]->setIconSize(QSize(35, 35));
                    buttons[i][j]->setStyleSheet("background-color: " MINE_CELL_COLOR ";");
                }
                else if (cell->getMinesNearby() > 0)
                {
                    QString color = setNumColor(cell->getMinesNearby());
                    buttons[i][j]->setText(QString::number(cell->getMinesNearby()));
                    buttons[i][j]->setStyleSheet(QString("color: %1; background-color: " CLR_MODE_CELL_COLOR ";").arg(color));
                    QFont sansFont("Sans", 16, QFont::Bold);
                    buttons[i][j]->setFont(sansFont);
                }
            }
        }
    }
}

void MainWindow::handleLMB()
{
    QPushButton *button = qobject_cast< QPushButton * >(sender());
    if (!button)
        return;
    lastPressedButton = button;
    int row = -1;
    int col = -1;
    for (int i = 0; i < buttons.size(); ++i)
    {
        for (int j = 0; j < buttons[i].size(); ++j)
        {
            if (buttons[i][j] == button)
            {
                row = i;
                col = j;
                break;
            }
        }
        if (row != -1)
            break;
    }
    if (row != -1 && col != -1)
    {
        Cell *cell = field->getField()[row][col];
        if (cell->getStatus() == Cell::FLAGGED)
            return;
        if (cell->getType() == Cell::MINE)
        {
            button->setIcon(mineIcon);
            button->setIconSize(QSize(35, 35));
            revealAllCells();
            isGameFinished = true;
            GameOverWindow gameOverWindow(this);
            gameOverWindow.exec();
            button->setStyleSheet("background-color: " LAST_PRESSED_BUTTON_COLOR ";");
        }
        else
        {
            reveal(row, col);
            checkWin();
        }
        button->setEnabled(false);
    }
}

void MainWindow::handleRMB(QPushButton *button)
{
    if (!button)
        return;

    int row = -1;
    int col = -1;
    for (int i = 0; i < buttons.size(); ++i)
    {
        for (int j = 0; j < buttons[i].size(); ++j)
        {
            if (buttons[i][j] == button)
            {
                row = i;
                col = j;
                break;
            }
        }
        if (row != -1)
            break;
    }
    if (row != -1 && col != -1)
    {
        field->toggleFlag(row, col);
        Cell *cell = field->getField()[row][col];
        if (cell->getStatus() == Cell::FLAGGED)
        {
            button->setIcon(flagIcon);
            button->setIconSize(QSize(35, 35));
        }
        else if (cell->getStatus() == Cell::UNREVEALED)
        {
            button->setIcon(QIcon());
            if (mode && cell->getType() == Cell::MINE)
            {
                button->setIcon(mineIcon);
                button->setIconSize(QSize(35, 35));
            }
        }
    }
}

void MainWindow::handleMMB(QPushButton *button)
{
    if (!button)
        return;

    int row = -1;
    int col = -1;
    for (int i = 0; i < buttons.size(); ++i)
    {
        for (int j = 0; j < buttons[i].size(); ++j)
        {
            if (buttons[i][j] == button)
            {
                row = i;
                col = j;
                break;
            }
        }
        if (row != -1)
            break;
    }
    if (row == -1 || col == -1)
        return;

    Cell *cell = field->getField()[row][col];
    if (cell->getStatus() != Cell::REVEALED || cell->getMinesNearby() == 0)
        return;

    int flagsCount = 0;
    int minesNearby = cell->getMinesNearby();
    QVector< QPushButton * > neighbors;

    for (int i = row - 1; i <= row + 1; ++i)
    {
        for (int j = col - 1; j <= col + 1; ++j)
        {
            if (i >= 0 && i < field->getRows() && j >= 0 && j < field->getCols() && (i != row || j != col))
            {
                Cell *neighbor = field->getField()[i][j];
                if (neighbor->getStatus() == Cell::FLAGGED && neighbor->getType() == Cell::MINE)
                {
                    flagsCount++;
                }
                else if (neighbor->getStatus() == Cell::UNREVEALED)
                {
                    neighbors.append(buttons[i][j]);
                }
            }
        }
    }

    if (flagsCount == minesNearby)
    {
        for (QPushButton *neighborButton : neighbors)
        {
            neighborButton->click();
        }
    }
    else
    {
        bool allHighlighted = true;
        for (QPushButton *neighborButton : neighbors)
        {
            if (neighborButton->styleSheet() != QString("background-color: " FAST_REVEALED_CELL_COLOR ";"))
            {
                allHighlighted = false;
                break;
            }
        }

        QString newColor = allHighlighted ? UNREVEALED_CELL_COLOR : FAST_REVEALED_CELL_COLOR;
        for (QPushButton *neighborButton : neighbors)
        {
            if (allHighlighted)
            {
                if (originalStyles.contains(neighborButton))
                {
                    neighborButton->setStyleSheet(originalStyles[neighborButton]);
                    originalStyles.remove(neighborButton);
                }
            }
            else
            {
                if (!originalStyles.contains(neighborButton))
                {
                    originalStyles[neighborButton] = neighborButton->styleSheet();
                }
                neighborButton->setStyleSheet(QString("background-color: %1;").arg(FAST_REVEALED_CELL_COLOR));
            }
        }
    }
}

void MainWindow::checkWin()
{
    bool won = true;
    for (int i = 0; i < field->getRows(); ++i)
    {
        for (int j = 0; j < field->getCols(); ++j)
        {
            Cell *cell = field->getField()[i][j];
            if (cell->getType() == Cell::EMPTY && cell->getStatus() != Cell::REVEALED)
            {
                won = false;
                break;
            }
        }
        if (!won)
            break;
    }
    if (won)
    {
        isGameFinished = true;
        WinWindow winWindow(this);
        winWindow.exec();
        revealAllCells();
        lastPressedButton->setStyleSheet("background-color: " LAST_PRESSED_BUTTON_COLOR ";");
    }
}

void MainWindow::toggleClearMode(bool checked)
{
    mode = checked;
    for (int i = 0; i < field->getRows(); ++i)
    {
        for (int j = 0; j < field->getCols(); ++j)
        {
            QPushButton *button = buttons[i][j];
            Cell *cell = field->getField()[i][j];
            if (mode)
            {
                if (cell->getType() == Cell::MINE)
                {
                    button->setIcon(mineIcon);
                    button->setIconSize(QSize(35, 35));
                    button->setStyleSheet("background-color: " CLR_MODE_MINE_COLOR ";");
                }
                else if (cell->getMinesNearby() > 0)
                {
                    QString color = setNumColor(cell->getMinesNearby());
                    button->setIcon(QIcon());
                    button->setText(QString::number(cell->getMinesNearby()));
                    if (cell->getStatus() != Cell::REVEALED)
                    {
                        button->setStyleSheet(QString("color: %1; background-color: " CLR_MODE_CELL_COLOR ";").arg(color));
                    }
                    QFont sansFont("Sans", 16, QFont::Bold);
                    button->setFont(sansFont);
                }
            }
            else
            {
                if (cell->getStatus() == Cell::UNREVEALED)
                {
                    button->setText("");
                    button->setIcon(QIcon());
                    button->setStyleSheet("background-color: " UNREVEALED_CELL_COLOR ";");
                }
                else if (cell->getStatus() == Cell::FLAGGED)
                {
                    button->setText("");
                    button->setIcon(flagIcon);
                    button->setIconSize(QSize(35, 35));
                    button->setStyleSheet("background-color: " UNREVEALED_CELL_COLOR ";");
                }
                else if (cell->getStatus() == Cell::REVEALED)
                {
                    if (cell->getType() == Cell::MINE)
                    {
                        button->setIcon(mineIcon);
                        button->setIconSize(QSize(35, 35));
                        button->setStyleSheet("background-color: " MINE_CELL_COLOR ";");
                    }
                    else if (cell->getMinesNearby() > 0)
                    {
                        QString color = setNumColor(cell->getMinesNearby());
                        button->setText(QString::number(cell->getMinesNearby()));
                        button->setStyleSheet(QString("color: %1; background-color: " REVEALED_CELL_COLOR ";").arg(color));
                    }
                    else
                    {
                        button->setText("");
                        button->setStyleSheet("background-color: " REVEALED_CELL_COLOR ";");
                    }
                }
            }
        }
    }
}

void MainWindow::startNewGame()
{
    clearField();
    delete field;
    setupGame();
}

void MainWindow::saveGame()
{
    QSettings settings(gameSaveFile, QSettings::IniFormat);
    settings.clear();

    if (isGameFinished)
    {
        return;
    }

    settings.beginGroup("Field");
    settings.setValue("Rows", rows);
    settings.setValue("Cols", cols);
    settings.setValue("Mines", mines);
    settings.setValue("Mode", mode);
    settings.setValue("Language", currentLanguage);
    saveFieldState(settings);
    settings.endGroup();

    settings.sync();
}

void MainWindow::saveFieldState(QSettings &settings)
{
    settings.beginGroup("Cells");
    for (int i = 0; i < field->getRows(); ++i)
    {
        for (int j = 0; j < field->getCols(); ++j)
        {
            Cell *cell = field->getField()[i][j];
            settings.beginGroup(QString("Cell%1_%2").arg(i).arg(j));
            settings.setValue("Status", cell->getStatus());
            settings.setValue("Type", cell->getType());
            settings.setValue("MinesNearby", cell->getMinesNearby());
            settings.setValue("IsMine", cell->getType() == Cell::MINE);
            settings.setValue("IsFlagged", cell->getStatus() == Cell::FLAGGED);
            settings.endGroup();
        }
    }

    if (lastPressedButton)
    {
        for (int i = 0; i < buttons.size(); ++i)
        {
            for (int j = 0; j < buttons[i].size(); ++j)
            {
                if (buttons[i][j] == lastPressedButton)
                {
                    settings.setValue("LastPressedButtonRow", i);
                    settings.setValue("LastPressedButtonCol", j);
                    break;
                }
            }
        }
    }
    settings.endGroup();
}

void MainWindow::loadGame()
{
    QSettings settings(gameSaveFile, QSettings::IniFormat);

    settings.beginGroup("Field");
    rows = settings.value("Rows").toInt();
    cols = settings.value("Cols").toInt();
    mines = settings.value("Mines").toInt();
    mode = settings.value("Mode").toBool();
    currentLanguage = settings.value("Language", "en").toString();
    switchLanguage(currentLanguage);
    field = new Field(rows, cols, mines, false, this);
    createButtons(rows, cols);
    loadFieldState(settings);
    settings.endGroup();
}

void MainWindow::loadFieldState(QSettings &settings)
{
    settings.beginGroup("Cells");

    int cellCount = 0;
    int mineCount = 0;
    int unrevealedCount = 0;
    int revealedCount = 0;

    for (int i = 0; i < field->getRows(); ++i)
    {
        for (int j = 0; j < field->getCols(); ++j)
        {
            cellCount++;
            Cell *cell = field->getField()[i][j];
            settings.beginGroup(QString("Cell%1_%2").arg(i).arg(j));
            cell->setStatus(static_cast< Cell::Status >(settings.value("Status").toInt()));
            cell->setType(static_cast< Cell::Type >(settings.value("Type").toInt()));
            cell->setMinesNearby(settings.value("MinesNearby").toInt());
            if (settings.value("IsMine").toBool())
            {
                cell->setType(Cell::MINE);
                mineCount++;
            }
            if (settings.value("IsFlagged").toBool())
            {
                cell->setStatus(Cell::FLAGGED);
            }
            if (cell->getStatus() == Cell::REVEALED)
            {
                revealedCount++;
            }
            else
            {
                unrevealedCount++;
            }
            settings.endGroup();

            QPushButton *button = buttons[i][j];
            button->setEnabled(true);
            button->setIcon(QIcon());
            button->setText("");
            button->setStyleSheet("background-color: " UNREVEALED_CELL_COLOR ";");

            if (cell->getStatus() == Cell::REVEALED)
            {
                button->setEnabled(false);
                button->setStyleSheet("background-color: " REVEALED_CELL_COLOR ";");
                if (cell->getType() == Cell::MINE)
                {
                    button->setIcon(mineIcon);
                    button->setIconSize(QSize(35, 35));
                    button->setStyleSheet("background-color: " MINE_CELL_COLOR ";");
                }
                else if (cell->getMinesNearby() > 0)
                {
                    QString color = setNumColor(cell->getMinesNearby());
                    button->setText(QString::number(cell->getMinesNearby()));
                    button->setStyleSheet(QString("color: %1; background-color: " REVEALED_CELL_COLOR ";").arg(color));
                    QFont sansFont("Sans", 16, QFont::Bold);
                    button->setFont(sansFont);
                }
            }
            else if (cell->getStatus() == Cell::FLAGGED)
            {
                button->setIcon(flagIcon);
                button->setIconSize(QSize(35, 35));
            }
            else
            {
                button->setText("");
                button->setIcon(QIcon());
                button->setStyleSheet("background-color: " UNREVEALED_CELL_COLOR ";");
            }
        }
    }

    if (cellCount != rows * cols || mineCount != mines || unrevealedCount + revealedCount != rows * cols)
    {
        QMessageBox::critical(this, tr("Error"), tr("Cannot load game: config file was edited externally or no game saved."), QMessageBox::Ok);
        startNewGame();
        return;
    }

    int lastPressedRow = settings.value("LastPressedButtonRow", -1).toInt();
    int lastPressedCol = settings.value("LastPressedButtonCol", -1).toInt();
    if (lastPressedRow != -1 && lastPressedCol != -1)
    {
        lastPressedButton = buttons[lastPressedRow][lastPressedCol];
        lastPressedButton->setStyleSheet("background-color: " LAST_PRESSED_BUTTON_COLOR ";");
    }
    settings.endGroup();
}

void MainWindow::setLangChecked()
{
    for (QAction *action : languageActionGroup->actions())
    {
        if (action->data().toString() == currentLanguage)
        {
            action->setChecked(true);
            break;
        }
    }
}

void MainWindow::switchLanguage(const QString &language)
{
    currentLanguage = language;
    qApp->removeTranslator(&translator);
    if (translator.load(":/translations/language_" + language + ".qm"))
    {
        qApp->installTranslator(&translator);
    }
    if (clearModeCheckBox)
    {
        clearModeCheckBox->setText(tr("Clear mode"));
    }
    if (newGameAction)
    {
        newGameAction->setText(tr("New Game"));
    }
    if (restartGameAction)
    {
        restartGameAction->setText(tr("Restart"));
    }
    if (languageMenu)
    {
        languageMenu->setTitle(tr("Language"));
    }
    setLangChecked();

    QSettings settings("Local", "MyApp");
    settings.setValue("language", language);
}

void MainWindow::updateTranslations()
{
    setWindowTitle(tr("Minesweeper"));
    if (clearModeCheckBox)
    {
        clearModeCheckBox->setText(tr("Clear mode"));
    }
    if (newGameAction)
    {
        newGameAction->setText(tr("New Game"));
    }
    if (restartGameAction)
    {
        restartGameAction->setText(tr("Restart"));
    }
    setLangChecked();
}

void MainWindow::createActions()
{
    languageMenu = menuBar()->addMenu(tr("Language"));

    languageActionGroup = new QActionGroup(this);

    createTranslationAction("English", "en", languageMenu);
    createTranslationAction("Español", "es", languageMenu);
    createTranslationAction("Deutsch", "de", languageMenu);

    connect(languageActionGroup,
            &QActionGroup::triggered,
            [this](QAction *action) { switchLanguage(action->data().toString()); });

    newGameAction = new QAction(tr("New Game"), this);
    newGameAction->setFont(QFont("Sans", 12, QFont::Bold));
    connect(newGameAction, &QAction::triggered, this, &MainWindow::startNewGame);

    restartGameAction = new QAction(tr("Restart"), this);
    restartGameAction->setFont(QFont("Sans", 12, QFont::Bold));
    connect(restartGameAction, &QAction::triggered, this, &MainWindow::restartGame);

    setLangChecked();
}

void MainWindow::createTranslationAction(const QString &languageName, const QString &languageCode, QMenu *languageMenu)
{
    QAction *action = new QAction(languageName, this);
    action->setCheckable(true);
    action->setData(languageCode);
    languageMenu->addAction(action);
    languageActionGroup->addAction(action);
    if (currentLanguage == languageCode)
    {
        action->setChecked(true);
    }
}
