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
#include <QJsonArray>
#include <QJsonParseError>
#include <QMessageBox>
#include <QSignalBlocker>

#include "reliabilityscene.h"
#include "statescene.h"
#include "stateview.h"
#include "structureview.h"
#include "nodeconfigurationwidget.h"
#include "reliabilitycore.h"
#include "schemaserializer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setupCentralWidgets();
    void setupDockWidgets();

    void setupToolBar();

    void setupMenu();

    void createActions();

    void setupStatusBar();

private:
    ReliabilityScene* structureScene{nullptr};
    StateScene* stateScene{nullptr};

    StructureView* structureView{nullptr};
    StateView* stateGraphView{nullptr};

    QSplitter* splitter{nullptr};

    QDockWidget* matrixDock{nullptr};
    QDockWidget* probabilityDock{nullptr};
    QDockWidget* nodeConfigurationDock{nullptr};
    QDockWidget* cyclogramDock{nullptr};
    QDockWidget* statesDock{nullptr};
    QDockWidget* initialProbabilitiesDock{nullptr};

    QTableWidget* matrixTable{nullptr};
    QTableWidget* probabilityTable {nullptr};
    QTableWidget* cyclogramTable{nullptr};
    QTableWidget* statesTable{nullptr};
    QTableWidget* initialProbabilitiesTable{nullptr};
    QComboBox* matrixStageComboBox{nullptr};
    QComboBox* matrixTypeComboBox{nullptr};
    QComboBox* topLevelStructureTypeComboBox{nullptr};
    QSpinBox* topLevelRequiredElementsSpinBox{nullptr};
    NodeConfigurationWidget* nodeConfigurationWidget{nullptr};
    CalculationResult lastCalculationResult;
    ProbabilityVector savedInitialProbabilities;

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

private slots:
    void showNodeConfiguration();
    void showNodeConfiguration(Node* node);
    void applyNodeConfiguration(Node* node, const NodeConfiguration& configuration);
    void onConfiguredNodeAboutToBeRemoved(Node* node);
    void toggleNodeConfigurationMode(bool checked);
    void toggleModelsAddMode(bool checked);
    void toggleSelectionMode(bool checked);
    void upLevel();
    void toggleConnectionMode(bool checked);
    void toggleDeleteMode(bool checked);
    void resetEditorModes();
    void newSchema();
    void openSchema();
    void saveSchema();
    void calculate();
    void exportResults();

private:
    static constexpr int maxStateCountForCalculation = 4096;

    static QJsonArray matrixToJson(const ReliabilityMatrix& matrix);
    static QJsonArray vectorToJson(const ProbabilityVector& vector);
    static QString operationModeName(OperationMode mode);
    static QString resultSystemStateName(ReliabilitySystemState state);
    static int stateCountForNodeCount(int nodeCount);
    static int reliabilityNodeCount(const QList<SchemaNodeData>& nodes);

    void updateStructureViewInteractionMode();
    void showIntensityMatrix(const CalculationResult& result);
    void updateMatrixTable();
    void showProbabilityTable(const CalculationResult& result);
    void showStatesTable(const CalculationResult& result);
    void showInitialProbabilitiesTable(const CalculationResult& result);
    void clearCalculationResultViews();
    QString systemStateToString(ReliabilitySystemState state) const;
    QString failedNodesToString(const ReliabilityState& state, const QList<SchemaNodeData>& nodes) const;
    bool validateCalculationInput(const SchemaModel& model, const Cyclogram& cyclogram, QString& errorMessage) const;
    bool validateInitialProbabilities(const ProbabilityVector& probabilities, int stateCount, QString& errorMessage) const;
    ProbabilityVector initialProbabilitiesFromTable(int stateCount) const;
    void addCyclogramStage(OperationMode mode = OperationMode::Functioning, double duration = 1.0);
    void removeSelectedCyclogramStage();
    Cyclogram cyclogramFromTable() const;
    void setCyclogramTable(const Cyclogram& cyclogram);
    void applyTopLevelStructureConfiguration();
    void syncTopLevelStructureControlsFromScene();

signals:
    void upLevelSignal();

};
#endif // MAINWINDOW_H
