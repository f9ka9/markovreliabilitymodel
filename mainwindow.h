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
#include "stateview.h"
#include "structureview.h"
#include "nodeconfigurationwidget.h"

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

    StructureView* structureView{nullptr};
    StateView* stateGraphView{nullptr};

    QSplitter* splitter{nullptr};

    QDockWidget* matrixDock{nullptr};
    QDockWidget* probabilityDock{nullptr};
    QDockWidget* nodeConfigurationDock{nullptr};

    QTableWidget* matrixTable{nullptr};
    QTableWidget* probabilityTable {nullptr};
    NodeConfigurationWidget* nodeConfigurationWidget{nullptr};

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

    Node* configuredNode{nullptr};
    QString defaultNodeName{"Node"};
    QString defaultNodeGroupName;
    FailureRates defaultNodeFailureRates{};
    Node::StructureType defaultNodeStructureType{Node::StructureType::Element};
    int defaultNodeRequiredElements{1};
    //============================================================================================
private slots:
    void showNodeConfiguration();
    void applyNodeConfiguration(const QString& name, const QString& groupName, const FailureRates& failureRates, Node::StructureType structureType, int requiredElements);
    void onConfiguredNodeAboutToBeRemoved(Node* node);
    void toggleModelsAddMode(bool checked);
    void toggleSelectionMode(bool checked);
    void upLevel();
    void toggleConnectionMode(bool checked);
    void toggleDeleteMode(bool checked);
    void resetEditorModes();

    //============================================================================================
private:
    void updateStructureViewInteractionMode();

protected:

    //============================================================================================
signals:
    void upLevelSignal();

    //============================================================================================
};
#endif // MAINWINDOW_H
