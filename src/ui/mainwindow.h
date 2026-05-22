#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QTableWidget>
#include <QLabel>
#include <QSplitter>
#include <QDockWidget>
#include <QAction>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QToolBar>
#include <QMenuBar>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QHash>
#include <QJsonParseError>
#include <QMessageBox>
#include <QSignalBlocker>

#include "reliabilityscene.h"
#include "statescene.h"
#include "stateview.h"
#include "structureview.h"
#include "nodeconfigurationwidget.h"
#include "calculationresult.h"
#include "schemaserializer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void upLevelSignal();

private slots:
    void showNodeConfiguration();
    void showNodeConfiguration(Node* node);
    void toggleNodeConfigurationMode(bool checked);
    void applyNodeConfiguration(Node* node, const NodeConfiguration& configuration);
    void onConfiguredNodeAboutToBeRemoved(Node* node);

    void toggleModelsAddMode(bool checked);
    void toggleSelectionMode(bool checked);
    void toggleConnectionMode(bool checked);
    void toggleDeleteMode(bool checked);

    void upLevel();
    void resetEditorModes();

    void newSchema();
    void openSchema();
    void saveSchema();
    void calculate();
    void exportResults();

private:
    static constexpr int maxStateCountForCalculation = 4096;

    void setupCentralWidgets();
    void setupDockWidgets();
    void createActions();
    void setupToolBar();
    void setupMenu();
    void setupStatusBar();

    void updateStructureViewInteractionMode();

    void showIntensityMatrix(const CalculationResult& result);
    void updateMatrixTable();
    void showProbabilityTable(const CalculationResult& result);
    void showStatesTable(const CalculationResult& result);
    void showInitialProbabilitiesTable(const CalculationResult& result);
    void clearCalculationResultViews();

    QString systemStateToString(ReliabilitySystemState state) const;
    QHash<int, QString> createNodeNamesById(const QList<SchemaNodeData>& nodes) const;
    QString failedNodesToString(const ReliabilityState& state, const QHash<int, QString>& nodeNamesById) const;

    ProbabilityVector initialProbabilitiesFromTable(int stateCount) const;

    void addCyclogramStage(OperationMode mode = OperationMode::Functioning, double duration = 1.0);
    void removeSelectedCyclogramStage();
    Cyclogram cyclogramFromTable() const;
    void setCyclogramTable(const Cyclogram& cyclogram);
    void applyTopLevelStructureConfiguration();
    void syncTopLevelStructureControlsFromScene();

    ReliabilityScene* structureScene{nullptr};
    StateScene* stateScene{nullptr};

    StructureView* structureView{nullptr};
    StateView* stateGraphView{nullptr};

    QSplitter* splitter{nullptr};

    QDockWidget* matrixDock{nullptr};
    QDockWidget* probabilityDock{nullptr};
    QDockWidget* statesDock{nullptr};
    QDockWidget* initialProbabilitiesDock{nullptr};
    QDockWidget* nodeConfigurationDock{nullptr};
    QDockWidget* cyclogramDock{nullptr};

    QTableWidget* matrixTable{nullptr};
    QTableWidget* probabilityTable{nullptr};
    QTableWidget* statesTable{nullptr};
    QTableWidget* initialProbabilitiesTable{nullptr};
    QTableWidget* cyclogramTable{nullptr};

    QComboBox* matrixStageComboBox{nullptr};
    QComboBox* matrixTypeComboBox{nullptr};
    QComboBox* topLevelStructureTypeComboBox{nullptr};

    QSpinBox* topLevelRequiredElementsSpinBox{nullptr};

    NodeConfigurationWidget* nodeConfigurationWidget{nullptr};

    QToolBar* mainToolBar{nullptr};
    QLabel* breadcrumbLabel{nullptr};

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
    QAction* stateGraphAction{nullptr};

    QMenu* fileMenu{nullptr};
    QMenu* viewMenu{nullptr};
    QMenu* helpMenu{nullptr};

    CalculationResult lastCalculationResult;
    ProbabilityVector savedInitialProbabilities;

};
#endif // MAINWINDOW_H
