#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QDockWidget>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QActionGroup>

#include "reliabilityscene.h"
#include "statescene.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //============================================================================================
    void setupCentralWidgets();
    void setupDockWidgets();

    void setupToolBar();

    void setupMenu();

    void createActions();

    void setupStatusBar();
    //============================================================================================

private:
    //============================================================================================
    ReliabilityScene* structureScene{nullptr};
    StateScene* stateScene{nullptr};

    QGraphicsView* structureView{nullptr};
    QGraphicsView* stateGraphView{nullptr};

    QSplitter* splitter{nullptr};

    QDockWidget* matrixDock{nullptr};
    QDockWidget* probabilityDock{nullptr};

    QTableWidget* matrixTable{nullptr};
    QTableWidget* probabilityTable {nullptr};

    QToolBar* mainToolBar{nullptr};

    QLabel* breadcrumbLabel {nullptr};

    QAction* configNodeAction{nullptr};
    QAction* addNodeAction{nullptr};
    QAction* deleteItemAction{nullptr};
    QAction* selectAction{nullptr};
    QAction* connectAction{nullptr};
    QAction* calculateAction{nullptr};
    QAction* upLevelAction{nullptr};
    QAction* newSchemaAction{nullptr};
    QAction* openSchemaAction{nullptr};
    QAction* saveSchemaAction{nullptr};
    QAction* exportResultsAction{nullptr};
    QAction* aboutAction{nullptr};

    QMenu* fileMenu{nullptr};
    QMenu* viewMenu{nullptr};
    QMenu* helpMenu{nullptr};
    //============================================================================================
private slots:
    void toggleModelsAddMode(bool toSwich);
    void toggleSelectionMode(bool checked);
    void upLevel();
    void deleteSelectedModelsNodes();
    void connectSelectedNodes();

    //============================================================================================
protected:

    //============================================================================================
signals:
    void upLevelSignal();
    void deleteSelectedModelsNodesSignal();
    void connectSelectedNodesSignal();

    //============================================================================================
};
#endif // MAINWINDOW_H