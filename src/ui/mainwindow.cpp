#include "mainwindow.h"

#include <cmath>
#include <limits>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "compositereliabilitycalculator.h"
#include "stategenerator.h"

QJsonArray MainWindow::matrixToJson(const ReliabilityMatrix& matrix)
{
    QJsonArray rows;
    for (const QVector<double>& row : matrix)
    {
        QJsonArray values;
        for (double value : row)
            values.append(value);
        rows.append(values);
    }
    return rows;
}

QJsonArray MainWindow::vectorToJson(const ProbabilityVector& vector)
{
    QJsonArray values;
    for (double value : vector)
        values.append(value);
    return values;
}

QString MainWindow::operationModeName(OperationMode mode)
{
    switch (mode)
    {
    case OperationMode::Transportation:
        return "transportation";
    case OperationMode::Storage:
        return "storage";
    case OperationMode::Functioning:
        return "functioning";
    case OperationMode::Off:
        return "off";
    }

    return "functioning";
}

QString MainWindow::resultSystemStateName(ReliabilitySystemState state)
{
    switch (state)
    {
    case ReliabilitySystemState::Working:
        return "working";
    case ReliabilitySystemState::PartialFailure:
        return "partialFailure";
    case ReliabilitySystemState::Failure:
        return "failure";
    }

    return "unknown";
}

int MainWindow::reliabilityNodeCount(const QList<SchemaNodeData>& nodes)
{
    int count = 0;
    for (const SchemaNodeData& node : nodes)
    {
        if (node.nodeKind == NodeKind::Normal)
            ++count;
    }
    return count;
}

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle("Марковская модель надежности");


    setupCentralWidgets();
    setupDockWidgets();

    createActions();
    setupToolBar();

    setupMenu();

    setupStatusBar();

    connect(this, &MainWindow::upLevelSignal, structureScene, &ReliabilityScene::onUpLevel);
    connect(structureScene, &ReliabilityScene::editorModesResetRequested, this, &MainWindow::resetEditorModes);
    connect(structureScene, &ReliabilityScene::nodeAboutToBeRemoved, this, &MainWindow::onConfiguredNodeAboutToBeRemoved);
    connect(structureScene, &ReliabilityScene::nodeConfigurationRequested, this, QOverload<Node*>::of(&MainWindow::showNodeConfiguration));
    connect(structureScene, &ReliabilityScene::currentLevelChanged, breadcrumbLabel, &QLabel::setText);
    connect(structureScene, &ReliabilityScene::currentLevelChanged, this, [this]() { clearCalculationResultViews(); });
    breadcrumbLabel->setText(structureScene->currentLevelPath());

}

void MainWindow::setupCentralWidgets()
{
    structureScene = new ReliabilityScene(this);
    structureView = new StructureView(this);
    structureView->setScene(structureScene);

    stateScene = new StateScene(this);
    stateGraphView = new StateView(this);
    stateGraphView->setScene(stateScene);


    splitter = new QSplitter(this);
    splitter->addWidget(structureView);
    splitter->addWidget(stateGraphView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    stateGraphView->hide();

    setCentralWidget(splitter);

}

void MainWindow::setupDockWidgets()
{
    matrixDock = new QDockWidget("Матрицы Q(k) / P(k)", this);
    QWidget* matrixWidget = new QWidget(this);
    QVBoxLayout* matrixLayout = new QVBoxLayout(matrixWidget);
    QHBoxLayout* matrixControlsLayout = new QHBoxLayout;
    matrixStageComboBox = new QComboBox(matrixWidget);
    matrixTypeComboBox = new QComboBox(matrixWidget);
    matrixTypeComboBox->addItem("Q", "Q");
    matrixTypeComboBox->addItem("P", "P");
    matrixControlsLayout->addWidget(new QLabel("Этап", matrixWidget));
    matrixControlsLayout->addWidget(matrixStageComboBox);
    matrixControlsLayout->addWidget(new QLabel("Матрица", matrixWidget));
    matrixControlsLayout->addWidget(matrixTypeComboBox);
    matrixTable = new QTableWidget(5, 5, matrixWidget);
    matrixLayout->addLayout(matrixControlsLayout);
    matrixLayout->addWidget(matrixTable);
    matrixDock->setWidget(matrixWidget);
    addDockWidget(Qt::RightDockWidgetArea, matrixDock);
    matrixDock->hide();

    connect(matrixStageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { updateMatrixTable(); });
    connect(matrixTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { updateMatrixTable(); });

    probabilityDock = new QDockWidget("Вероятности состояний p(i)", this);
    probabilityTable = new QTableWidget(5, 5, this);
    probabilityDock->setWidget(probabilityTable);
    addDockWidget(Qt::BottomDockWidgetArea, probabilityDock);
    probabilityDock->hide();

    statesDock = new QDockWidget("Состояния", this);
    statesTable = new QTableWidget(0, 3, this);
    statesTable->setHorizontalHeaderLabels({"Состояние", "Статус системы", "Отказавшие узлы"});
    statesDock->setWidget(statesTable);
    addDockWidget(Qt::BottomDockWidgetArea, statesDock);
    statesDock->hide();

    initialProbabilitiesDock = new QDockWidget("Начальные вероятности p(0)", this);
    initialProbabilitiesTable = new QTableWidget(0, 2, this);
    initialProbabilitiesTable->setHorizontalHeaderLabels({"Состояние", "Вероятность"});
    initialProbabilitiesDock->setWidget(initialProbabilitiesTable);
    addDockWidget(Qt::BottomDockWidgetArea, initialProbabilitiesDock);
    initialProbabilitiesDock->hide();

    nodeConfigurationDock = new QDockWidget("Конфигурация узла", this);
    nodeConfigurationWidget = new NodeConfigurationWidget(this);
    nodeConfigurationDock->setWidget(nodeConfigurationWidget);
    addDockWidget(Qt::LeftDockWidgetArea, nodeConfigurationDock);
    nodeConfigurationDock->hide();

    connect(nodeConfigurationWidget, &NodeConfigurationWidget::configurationApplied, this, &MainWindow::applyNodeConfiguration);
    connect(nodeConfigurationDock, &QDockWidget::visibilityChanged, this, [this](bool visible) { if (!visible && configNodeAction && configNodeAction->isChecked()) configNodeAction->setChecked(false); });

    cyclogramDock = new QDockWidget("Циклограмма", this);
    QWidget* cyclogramWidget = new QWidget(this);
    QVBoxLayout* cyclogramLayout = new QVBoxLayout(cyclogramWidget);

    QHBoxLayout* topLevelStructureLayout = new QHBoxLayout;
    topLevelStructureTypeComboBox = new QComboBox(cyclogramWidget);
    topLevelStructureTypeComboBox->addItem("Последовательная", static_cast<int>(StructureType::Series));
    topLevelStructureTypeComboBox->addItem("Параллельная", static_cast<int>(StructureType::Parallel));
    topLevelStructureTypeComboBox->addItem("k из n", static_cast<int>(StructureType::KOutOfN));
    topLevelRequiredElementsSpinBox = new QSpinBox(cyclogramWidget);
    topLevelRequiredElementsSpinBox->setRange(1, 1000000);
    topLevelStructureLayout->addWidget(new QLabel("Критерий уровня без связей", cyclogramWidget));
    topLevelStructureLayout->addWidget(topLevelStructureTypeComboBox);
    topLevelStructureLayout->addWidget(new QLabel("k", cyclogramWidget));
    topLevelStructureLayout->addWidget(topLevelRequiredElementsSpinBox);

    cyclogramTable = new QTableWidget(0, 3, cyclogramWidget);
    cyclogramTable->setHorizontalHeaderLabels({"Этап", "Режим", "Длительность"});

    QPushButton* addStageButton = new QPushButton("Добавить этап", cyclogramWidget);
    QPushButton* removeStageButton = new QPushButton("Удалить этап", cyclogramWidget);
    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(addStageButton);
    buttonsLayout->addWidget(removeStageButton);

    cyclogramLayout->addLayout(topLevelStructureLayout);
    cyclogramLayout->addWidget(cyclogramTable);
    cyclogramLayout->addLayout(buttonsLayout);
    cyclogramDock->setWidget(cyclogramWidget);
    addDockWidget(Qt::LeftDockWidgetArea, cyclogramDock);
    cyclogramDock->hide();

    connect(addStageButton, &QPushButton::clicked, this, [this]() { addCyclogramStage(); });
    connect(removeStageButton, &QPushButton::clicked, this, &MainWindow::removeSelectedCyclogramStage);
    connect(topLevelStructureTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { applyTopLevelStructureConfiguration(); });
    connect(topLevelRequiredElementsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { applyTopLevelStructureConfiguration(); });
    syncTopLevelStructureControlsFromScene();
    addCyclogramStage(OperationMode::Functioning, 1.0);
}
void MainWindow::createActions()
{
    configNodeAction = new QAction("Конфигурация", this);
    configNodeAction->setCheckable(true);
    configNodeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Q));
    configNodeAction->setStatusTip("Открыть настройки выбранного узла или настройки новых узлов");
    connect(configNodeAction, &QAction::toggled, this, &MainWindow::toggleNodeConfigurationMode);

    addNodeAction = new QAction("Добавить узел", this);
    addNodeAction->setCheckable(true);
    addNodeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    addNodeAction->setStatusTip("Перейти в режим добавления узлов");
    connect(addNodeAction, &QAction::triggered, this, &MainWindow::toggleModelsAddMode);

    deleteItemAction = new QAction("Удалить", this);
    deleteItemAction->setCheckable(true);
    deleteItemAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_X));
    deleteItemAction->setStatusTip("Перейти в режим удаления узлов и связей");
    connect(deleteItemAction, &QAction::toggled, this, &MainWindow::toggleDeleteMode);

    selectAction = new QAction("Выделение", this);
    selectAction->setCheckable(true);
    selectAction->setStatusTip("Перейти в режим выделения объектов");
    connect(selectAction, &QAction::toggled, this, &MainWindow::toggleSelectionMode);

    connectAction = new QAction("Соединить", this);
    connectAction->setCheckable(true);
    connectAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    connectAction->setStatusTip("Перейти в режим соединения узлов");
    connect(connectAction, &QAction::triggered, this, &MainWindow::toggleConnectionMode);

    calculateAction = new QAction("Рассчитать", this);
    calculateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    calculateAction->setStatusTip("Выполнить расчет для текущего уровня схемы");
    connect(calculateAction, &QAction::triggered, this, &MainWindow::calculate);

    upLevelAction = new QAction("На уровень выше", this);
    upLevelAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
    upLevelAction->setStatusTip("Перейти на уровень выше");
    connect(upLevelAction, &QAction::triggered, this, &MainWindow::upLevel);

    newSchemaAction = new QAction("Новая схема", this);
    newSchemaAction->setStatusTip("Создать новую структурную схему");
    connect(newSchemaAction, &QAction::triggered, this, &MainWindow::newSchema);

    openSchemaAction = new QAction("Открыть...", this);
    openSchemaAction->setStatusTip("Открыть файл структурной схемы");
    connect(openSchemaAction, &QAction::triggered, this, &MainWindow::openSchema);

    saveSchemaAction = new QAction("Сохранить", this);
    saveSchemaAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    saveSchemaAction->setStatusTip("Сохранить текущую схему в файл");
    connect(saveSchemaAction, &QAction::triggered, this, &MainWindow::saveSchema);

    exportResultsAction = new QAction("Экспорт результатов", this);
    exportResultsAction->setStatusTip("Сохранить результаты расчета в файл");
    connect(exportResultsAction, &QAction::triggered, this, &MainWindow::exportResults);

    aboutAction = new QAction("О программе", this);
    aboutAction->setStatusTip("Сведения о программе");
}
void MainWindow::setupToolBar()
{
    mainToolBar = new QToolBar("Основная панель", this);
    addToolBar(Qt::TopToolBarArea, mainToolBar);

    breadcrumbLabel = new QLabel("", this);

    mainToolBar->addAction(configNodeAction);
    mainToolBar->addAction(addNodeAction);
    mainToolBar->addAction(deleteItemAction);
    mainToolBar->addAction(selectAction);
    mainToolBar->addAction(connectAction);
    mainToolBar->addAction(calculateAction);

    mainToolBar->addSeparator();

    mainToolBar->addAction(upLevelAction);

    mainToolBar->addSeparator();

    mainToolBar->addWidget(breadcrumbLabel);
}
void MainWindow::setupMenu()
{
    fileMenu = new QMenu("Файл", this);
    menuBar()->addMenu(fileMenu);

    fileMenu->addAction(newSchemaAction);
    fileMenu->addAction(openSchemaAction);
    fileMenu->addAction(saveSchemaAction);

    fileMenu->addSeparator();

    fileMenu->addAction(exportResultsAction);

    viewMenu = new QMenu("Вид", this);
    menuBar()->addMenu(viewMenu);

    stateGraphAction = viewMenu->addAction("Граф состояний");
    stateGraphAction->setCheckable(true);
    stateGraphAction->setChecked(false);
    connect(stateGraphAction, &QAction::toggled, this, [this](bool checked) { stateGraphView->setVisible(checked); if (checked) splitter->setSizes({1, 1}); });

    viewMenu->addSeparator();

    viewMenu->addAction(matrixDock->toggleViewAction());
    viewMenu->addAction(probabilityDock->toggleViewAction());
    viewMenu->addAction(statesDock->toggleViewAction());
    viewMenu->addAction(initialProbabilitiesDock->toggleViewAction());
    viewMenu->addAction(nodeConfigurationDock->toggleViewAction());
    viewMenu->addAction(cyclogramDock->toggleViewAction());

    helpMenu = new QMenu("Справка", this);
    menuBar()->addMenu(helpMenu);

    helpMenu->addAction(aboutAction);
}
void MainWindow::setupStatusBar ()
{
    statusBar()->showMessage("Готово", 5000);
}
void MainWindow::showNodeConfiguration()
{
    showNodeConfiguration(structureScene->selectedNode());
}

void MainWindow::showNodeConfiguration(Node* node)
{
    if (node)
        nodeConfigurationWidget->editSelectedNode(node);
    else
        nodeConfigurationWidget->editDefaultConfiguration(structureScene->getDefaultNodeConfiguration());

    nodeConfigurationDock->show();
    nodeConfigurationDock->raise();
}

void MainWindow::toggleNodeConfigurationMode(bool checked)
{
    structureScene->setConfigurationMode(checked);
    if (checked)
    {
        if(selectAction->isChecked()) selectAction->setChecked(false);
        if(addNodeAction->isChecked()) addNodeAction->setChecked(false);
        if(connectAction->isChecked()) connectAction->setChecked(false);
        if(deleteItemAction->isChecked()) deleteItemAction->setChecked(false);
        structureScene->setModelsAddMode(false);
        structureScene->setConnectionMode(false);
        structureScene->setDeleteMode(false);
        showNodeConfiguration();
    }
    else
    {
        nodeConfigurationDock->hide();
    }
    updateStructureViewInteractionMode();
}

void MainWindow::applyNodeConfiguration(Node* node, const NodeConfiguration& configuration)
{
    NodeConfiguration normalizedConfiguration = configuration;
    normalizedConfiguration.name = normalizedConfiguration.name.trimmed();
    if (normalizedConfiguration.name.isEmpty())
    {
        if (normalizedConfiguration.nodeKind == NodeKind::Input)
            normalizedConfiguration.name = "Вход";
        else if (normalizedConfiguration.nodeKind == NodeKind::Output)
            normalizedConfiguration.name = "Выход";
        else
            normalizedConfiguration.name = "Узел";
    }
    normalizedConfiguration.groupName = normalizedConfiguration.groupName.trimmed();
    if (normalizedConfiguration.requiredElements < 1)
        normalizedConfiguration.requiredElements = 1;
    if (normalizedConfiguration.structureType != StructureType::KOutOfN)
        normalizedConfiguration.requiredElements = 1;
    if (normalizedConfiguration.structureType != StructureType::Element)
        normalizedConfiguration.lambdaDefinitions = LambdaDefinitions{};
    if (normalizedConfiguration.nodeKind != NodeKind::Normal)
    {
        normalizedConfiguration.groupName.clear();
        normalizedConfiguration.lambdaDefinitions = LambdaDefinitions{};
        normalizedConfiguration.structureType = StructureType::Element;
        normalizedConfiguration.requiredElements = 1;
    }

    if (node)
    {
        if (normalizedConfiguration.nodeKind != NodeKind::Normal || normalizedConfiguration.structureType == StructureType::Element)
            structureScene->removeNodeChildren(node);
        node->setConfiguration(normalizedConfiguration);
        structureScene->updateNodeGraphics(node);
        return;
    }

    structureScene->setDefaultNodeConfiguration(normalizedConfiguration);
}

void MainWindow::onConfiguredNodeAboutToBeRemoved(Node* node)
{
    if (!nodeConfigurationWidget->isEditingNode(node)) return;

    nodeConfigurationWidget->editDefaultConfiguration(structureScene->getDefaultNodeConfiguration());
}

void MainWindow::toggleModelsAddMode(bool checked)
{
    structureScene->setModelsAddMode(checked);
    if (checked)
    {
        if(configNodeAction->isChecked()) configNodeAction->setChecked(false);
        if(selectAction->isChecked()) selectAction->setChecked(false);
        if(connectAction->isChecked()) connectAction->setChecked(false);
        if(deleteItemAction->isChecked()) deleteItemAction->setChecked(false);
        structureScene->setConfigurationMode(false);
        structureScene->setConnectionMode(false);
        structureScene->setDeleteMode(false);
    }
    updateStructureViewInteractionMode();
}

void MainWindow::toggleSelectionMode(bool checked)
{

    if (checked)
    {
        if(configNodeAction->isChecked()) configNodeAction->setChecked(false);
        if(addNodeAction->isChecked()) addNodeAction->setChecked(false);
        if(connectAction->isChecked()) connectAction->setChecked(false);
        if(deleteItemAction->isChecked()) deleteItemAction->setChecked(false);
        structureScene->setConfigurationMode(false);
        structureScene->setModelsAddMode(false);
        structureScene->setConnectionMode(false);
        structureScene->setDeleteMode(false);
    }
    updateStructureViewInteractionMode();
}

void MainWindow::toggleConnectionMode (bool checked)
{
    structureScene->setConnectionMode(checked);
    if (checked)
    {
        if(configNodeAction->isChecked()) configNodeAction->setChecked(false);
        if(selectAction->isChecked()) selectAction->setChecked(false);
        if(addNodeAction->isChecked()) addNodeAction->setChecked(false);
        if(deleteItemAction->isChecked()) deleteItemAction->setChecked(false);
        structureScene->setConfigurationMode(false);
        structureScene->setModelsAddMode(false);
        structureScene->setDeleteMode(false);
    }
    updateStructureViewInteractionMode();
}

void MainWindow::toggleDeleteMode(bool checked)
{
    structureScene->setDeleteMode(checked);
    if (checked)
    {
        if(configNodeAction->isChecked()) configNodeAction->setChecked(false);
        if(selectAction->isChecked()) selectAction->setChecked(false);
        if(addNodeAction->isChecked()) addNodeAction->setChecked(false);
        if(connectAction->isChecked()) connectAction->setChecked(false);
        structureScene->setConfigurationMode(false);
        structureScene->setModelsAddMode(false);
        structureScene->setConnectionMode(false);
    }
    updateStructureViewInteractionMode();
}

void MainWindow::upLevel() {emit upLevelSignal();}

void MainWindow::resetEditorModes()
{
    const QSignalBlocker configBlocker(configNodeAction);
    const QSignalBlocker addBlocker(addNodeAction);
    const QSignalBlocker deleteBlocker(deleteItemAction);
    const QSignalBlocker selectBlocker(selectAction);
    const QSignalBlocker connectBlocker(connectAction);

    configNodeAction->setChecked(false);
    addNodeAction->setChecked(false);
    deleteItemAction->setChecked(false);
    selectAction->setChecked(false);
    connectAction->setChecked(false);

    structureScene->setConfigurationMode(false);
    structureScene->setModelsAddMode(false);
    structureScene->setConnectionMode(false);
    structureScene->setDeleteMode(false);
    nodeConfigurationDock->hide();
    updateStructureViewInteractionMode();
}

void MainWindow::updateStructureViewInteractionMode()
{
    const bool selectionMode = selectAction->isChecked();
    const bool editorModeActive = addNodeAction->isChecked() || deleteItemAction->isChecked() || configNodeAction->isChecked() || selectionMode || connectAction->isChecked();

    structureView->setDragMode(selectionMode ? QGraphicsView::RubberBandDrag : QGraphicsView::NoDrag);
    structureView->setPanningEnabled(!editorModeActive);
    structureView->setCursor(Qt::ArrowCursor);
    structureView->viewport()->setCursor(Qt::ArrowCursor);
}

void MainWindow::newSchema()
{
    const QMessageBox::StandardButton answer = QMessageBox::question( this, "Новая схема", "Очистить текущую структурную схему?");
    if (answer != QMessageBox::Yes) return;

    resetEditorModes();
    structureScene->clearSchema();
    syncTopLevelStructureControlsFromScene();
    setCyclogramTable({});
    savedInitialProbabilities.clear();
    initialProbabilitiesTable->setRowCount(0);
    nodeConfigurationWidget->editDefaultConfiguration(structureScene->getDefaultNodeConfiguration());
    statusBar()->showMessage("Создана новая схема", 5000);
}

void MainWindow::openSchema()
{
    const QString fileName = QFileDialog::getOpenFileName( this, "Открыть схему", QString(), "JSON (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Открыть схему", "Не удалось открыть файл для чтения.");
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        QMessageBox::warning(this, "Открыть схему", "Ошибка JSON: " + parseError.errorString());
        return;
    }

    EditorSchemaModel model;
    QString errorMessage;
    if (!SchemaSerializer::fromJson(document, model, errorMessage) || !structureScene->importEditorSchemaModel(model, errorMessage))
    {
        QMessageBox::warning(this, "Открыть схему", errorMessage);
        return;
    }

    resetEditorModes();
    syncTopLevelStructureControlsFromScene();
    setCyclogramTable(model.cyclogram);
    savedInitialProbabilities = model.initialProbabilities;
    initialProbabilitiesTable->setRowCount(0);
    nodeConfigurationWidget->editDefaultConfiguration(structureScene->getDefaultNodeConfiguration());
    statusBar()->showMessage("Схема загружена: " + fileName, 5000);
}

void MainWindow::saveSchema()
{
    const QString fileName = QFileDialog::getSaveFileName(this, "Сохранить схему", "schema.json", "JSON (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QMessageBox::warning(this, "Сохранить схему", "Не удалось открыть файл для записи.");
        return;
    }

    EditorSchemaModel model = structureScene->exportEditorSchemaModel();
    model.cyclogram = cyclogramFromTable();
    if (initialProbabilitiesTable->rowCount() > 0)
        model.initialProbabilities = initialProbabilitiesFromTable(initialProbabilitiesTable->rowCount());
    else
        model.initialProbabilities = savedInitialProbabilities;
    const QJsonDocument document = SchemaSerializer::toJson(model);
    file.write(document.toJson(QJsonDocument::Indented));
    statusBar()->showMessage("Схема сохранена: " + fileName, 5000);
}

void MainWindow::calculate()
{
    applyTopLevelStructureConfiguration();
    const EditorSchemaModel editorModel = structureScene->exportEditorSchemaModel();
    CompositeReliabilityCalculator compositeCalculator;
    const SchemaModel model = compositeCalculator.buildEffectiveModel(editorModel, editorModel.currentParentId);
    const Cyclogram cyclogram = cyclogramFromTable();

    QString errorMessage;
    if (!validateCalculationInput(model, cyclogram, errorMessage))
    {
        QMessageBox::warning(this, "Расчет", errorMessage);
        return;
    }
    StateGenerator stateGenerator;
    const int stateCount = stateGenerator.generate(model.nodes, model.connections, model.structureType, model.requiredElements).size();
    const ProbabilityVector initialProbabilities = initialProbabilitiesFromTable(stateCount);
    if (!validateInitialProbabilities(initialProbabilities, stateCount, errorMessage))
    {
        QMessageBox::warning(this, "Расчет", errorMessage);
        return;
    }

    ReliabilityCore core;
    const CalculationResult result = core.calculateCyclogram(model, model.structureType, model.requiredElements, cyclogram, initialProbabilities);
    savedInitialProbabilities = result.initialProbabilities;

    showIntensityMatrix(result);
    showProbabilityTable(result);
    showStatesTable(result);
    showInitialProbabilitiesTable(result);
    stateScene->showCalculationResult(result);
    statusBar()->showMessage(QString("Расчет выполнен: состояний %1, переходов %2").arg(result.states.size()).arg(result.transitions.size()), 5000);
}

void MainWindow::exportResults()
{
    if (lastCalculationResult.states.isEmpty())
    {
        QMessageBox::information(this, "Экспорт результатов", "Сначала выполните расчет.");
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(this, "Экспорт результатов", "calculation_results.json", "JSON (*.json)");
    if (fileName.isEmpty()) return;

    QJsonObject root;
    root["version"] = 1;

    QJsonArray nodes;
    for (const SchemaNodeData& node : lastCalculationResult.nodes)
    {
        QJsonObject object;
        object["id"] = node.id;
        object["name"] = node.name;
        object["groupName"] = node.groupName;
        nodes.append(object);
    }
    root["nodes"] = nodes;

    QJsonArray states;
    for (const ReliabilityState& state : lastCalculationResult.states)
    {
        QJsonObject object;
        object["id"] = state.id;
        object["name"] = state.name;
        object["systemState"] = resultSystemStateName(state.systemState);

        QJsonArray failedNodeIds;
        for (int nodeId : state.failedNodeIds)
            failedNodeIds.append(nodeId);
        object["failedNodeIds"] = failedNodeIds;
        states.append(object);
    }
    root["states"] = states;

    QJsonArray transitions;
    for (const ReliabilityTransition& transition : lastCalculationResult.transitions)
    {
        QJsonObject object;
        object["sourceStateId"] = transition.sourceStateId;
        object["targetStateId"] = transition.targetStateId;
        object["changedNodeId"] = transition.changedNodeId;
        object["fromState"] = "working";
        object["toState"] = "failed";
        transitions.append(object);
    }
    root["transitions"] = transitions;

    root["initialProbabilities"] = vectorToJson(lastCalculationResult.initialProbabilities);

    QJsonArray stages;
    for (const StageCalculationResult& stageResult : lastCalculationResult.stages)
    {
        QJsonObject object;
        object["name"] = stageResult.stage.name;
        object["mode"] = operationModeName(stageResult.stage.mode);
        object["duration"] = stageResult.stage.duration;
        object["Q"] = matrixToJson(stageResult.intensityMatrix);
        object["P"] = matrixToJson(stageResult.transitionMatrix);
        object["probabilities"] = vectorToJson(stageResult.probabilities);
        stages.append(object);
    }
    root["stages"] = stages;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QMessageBox::warning(this, "Экспорт результатов", "Не удалось открыть файл для записи.");
        return;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    statusBar()->showMessage("Результаты экспортированы: " + fileName, 5000);
}

void MainWindow::showIntensityMatrix(const CalculationResult& result)
{
    lastCalculationResult = result;

    const QSignalBlocker blocker(matrixStageComboBox);
    matrixStageComboBox->clear();
    for (int i = 0; i < result.stages.size(); ++i)
    {
        const CyclogramStage& stage = result.stages[i].stage;
        matrixStageComboBox->addItem(QString("%1: %2").arg(i + 1).arg(stage.name), i);
    }

    updateMatrixTable();
}

void MainWindow::updateMatrixTable()
{
    if (lastCalculationResult.stages.isEmpty())
    {
        matrixTable->clear();
        matrixTable->setRowCount(0);
        matrixTable->setColumnCount(0);
        return;
    }

    int stageIndex = matrixStageComboBox->currentData().toInt();
    if (stageIndex < 0 || stageIndex >= lastCalculationResult.stages.size())
        stageIndex = 0;

    const StageCalculationResult& stageResult = lastCalculationResult.stages[stageIndex];
    const bool showTransitionMatrix = matrixTypeComboBox->currentData().toString() == "P";
    const ReliabilityMatrix& matrix = showTransitionMatrix ? stageResult.transitionMatrix : stageResult.intensityMatrix;

    const int size = matrix.size();
    matrixTable->clear();
    if (size == 0)
    {
        matrixTable->setRowCount(1);
        matrixTable->setColumnCount(1);
        matrixTable->setHorizontalHeaderLabels({""});
        matrixTable->setVerticalHeaderLabels({""});
        matrixTable->setItem(0, 0, new QTableWidgetItem("Матрица не отображается для большого графа состояний."));
        matrixTable->resizeColumnsToContents();
        matrixTable->resizeRowsToContents();
        return;
    }

    matrixTable->setRowCount(size);
    matrixTable->setColumnCount(size);

    QStringList headers;
    for (int i = 0; i < size; ++i)
        headers.append(QString("S%1").arg(i));

    matrixTable->setHorizontalHeaderLabels(headers);
    matrixTable->setVerticalHeaderLabels(headers);

    for (int row = 0; row < size; ++row)
    {
        for (int column = 0; column < matrix[row].size(); ++column)
        {
            const double value = matrix[row][column];
            matrixTable->setItem(row, column, new QTableWidgetItem(QString::number(value, 'g', 10)));
        }
    }

    matrixTable->resizeColumnsToContents();
    matrixTable->resizeRowsToContents();
}

void MainWindow::showProbabilityTable(const CalculationResult& result)
{
    const int rowCount = result.states.size();
    const int columnCount = result.stages.size() + 2;

    probabilityTable->clear();
    probabilityTable->setRowCount(rowCount);
    probabilityTable->setColumnCount(columnCount);

    QStringList headers;
    headers.append("Состояние");
    headers.append("p(0)");
    for (int i = 0; i < result.stages.size(); ++i)
        headers.append(QString("p(%1)").arg(i + 1));
    probabilityTable->setHorizontalHeaderLabels(headers);

    for (int row = 0; row < result.states.size(); ++row)
    {
        QTableWidgetItem* stateItem = new QTableWidgetItem(QString("S%1").arg(result.states[row].id));
        stateItem->setFlags(stateItem->flags() & ~Qt::ItemIsEditable);
        probabilityTable->setItem(row, 0, stateItem);

        if (row < result.initialProbabilities.size())
        {
            probabilityTable->setItem(row, 1, new QTableWidgetItem(QString::number(result.initialProbabilities[row], 'g', 10)));
        }

        for (int stageIndex = 0; stageIndex < result.stages.size(); ++stageIndex)
        {
            const ProbabilityVector& probabilities = result.stages[stageIndex].probabilities;
            if (row >= probabilities.size()) continue;

            probabilityTable->setItem(row, stageIndex + 2, new QTableWidgetItem(QString::number(probabilities[row], 'g', 10)));
        }
    }

    probabilityTable->resizeColumnsToContents();
    probabilityTable->resizeRowsToContents();
}

void MainWindow::showStatesTable(const CalculationResult& result)
{
    statesTable->clear();
    statesTable->setRowCount(result.states.size());
    statesTable->setColumnCount(3);
    statesTable->setHorizontalHeaderLabels({"Состояние", "Статус системы", "Отказавшие узлы"});
    const QHash<int, QString> nodeNamesById = createNodeNamesById(result.nodes);

    for (int row = 0; row < result.states.size(); ++row)
    {
        const ReliabilityState& state = result.states[row];
        statesTable->setItem(row, 0, new QTableWidgetItem(QString("S%1").arg(state.id)));
        statesTable->setItem(row, 1, new QTableWidgetItem(systemStateToString(state.systemState)));
        statesTable->setItem(row, 2, new QTableWidgetItem(failedNodesToString(state, nodeNamesById)));
    }

    statesTable->resizeColumnsToContents();
    statesTable->resizeRowsToContents();
}

void MainWindow::showInitialProbabilitiesTable(const CalculationResult& result)
{
    const bool canReuseExistingValues = initialProbabilitiesTable->rowCount() == result.states.size();
    ProbabilityVector previousProbabilities;
    if (canReuseExistingValues)
        previousProbabilities = initialProbabilitiesFromTable(result.states.size());

    initialProbabilitiesTable->clear();
    initialProbabilitiesTable->setRowCount(result.states.size());
    initialProbabilitiesTable->setColumnCount(2);
    initialProbabilitiesTable->setHorizontalHeaderLabels({"Состояние", "Вероятность"});

    for (int row = 0; row < result.states.size(); ++row)
    {
        QTableWidgetItem* stateItem = new QTableWidgetItem(QString("S%1").arg(result.states[row].id));
        stateItem->setFlags(stateItem->flags() & ~Qt::ItemIsEditable);
        initialProbabilitiesTable->setItem(row, 0, stateItem);

        const double probability = canReuseExistingValues && row < previousProbabilities.size() ? previousProbabilities[row] : result.initialProbabilities[row];
        initialProbabilitiesTable->setItem(row, 1, new QTableWidgetItem(QString::number(probability, 'g', 10)));
    }

    initialProbabilitiesTable->resizeColumnsToContents();
    initialProbabilitiesTable->resizeRowsToContents();
}

void MainWindow::clearCalculationResultViews()
{
    lastCalculationResult = CalculationResult{};
    savedInitialProbabilities.clear();
    stateScene->clear();

    matrixStageComboBox->clear();
    matrixTable->clear();
    matrixTable->setRowCount(0);
    matrixTable->setColumnCount(0);

    probabilityTable->clear();
    probabilityTable->setRowCount(0);
    probabilityTable->setColumnCount(0);

    statesTable->clear();
    statesTable->setRowCount(0);
    statesTable->setColumnCount(3);
    statesTable->setHorizontalHeaderLabels({"Состояние", "Статус системы", "Отказавшие узлы"});

    initialProbabilitiesTable->clear();
    initialProbabilitiesTable->setRowCount(0);
    initialProbabilitiesTable->setColumnCount(2);
    initialProbabilitiesTable->setHorizontalHeaderLabels({"Состояние", "Вероятность"});
}

QString MainWindow::systemStateToString(ReliabilitySystemState state) const
{
    switch (state)
    {
    case ReliabilitySystemState::Working:
        return "Работает";
    case ReliabilitySystemState::PartialFailure:
        return "Частичный отказ";
    case ReliabilitySystemState::Failure:
        return "Отказ";
    }

    return "Неизвестно";
}

QHash<int, QString> MainWindow::createNodeNamesById(const QList<SchemaNodeData>& nodes) const
{
    QHash<int, QString> result;
    result.reserve(nodes.size());
    for (const SchemaNodeData& node : nodes)
        result.insert(node.id, node.name);
    return result;
}

QString MainWindow::failedNodesToString(const ReliabilityState& state, const QHash<int, QString>& nodeNamesById) const
{
    if (state.failedNodeIds.isEmpty())
        return "-";

    QStringList names;
    for (int nodeId : state.failedNodeIds)
    {
        const QString name = nodeNamesById.value(nodeId);
        names.append(name.isEmpty() ? QString("узел #%1").arg(nodeId) : name);
    }

    return names.join(", ");
}

bool MainWindow::validateCalculationInput(const SchemaModel& model, const Cyclogram& cyclogram, QString& errorMessage) const
{
    if (model.nodes.isEmpty())
    {
        errorMessage = "На текущем уровне нет узлов.";
        return false;
    }

    StateGenerator stateGenerator;
    const int reliableNodes = reliabilityNodeCount(model.nodes);
    const int stateCount = stateGenerator.generate(model.nodes, model.connections, model.structureType, model.requiredElements, maxStateCountForCalculation).size();
    if (stateCount > maxStateCountForCalculation)
    {
        errorMessage = QString("На текущем уровне слишком много расчетных узлов: %1. Двоичный граф состояний содержит больше %2 состояний. Откройте вложенный блок и рассчитайте меньший уровень.").arg(reliableNodes).arg(maxStateCountForCalculation);
        return false;
    }

    if (model.connections.isEmpty() && model.structureType == StructureType::KOutOfN && model.requiredElements > reliableNodes)
    {
        errorMessage = QString("Для структуры k из n значение k должно быть <= n. Сейчас k=%1, n=%2.").arg(model.requiredElements).arg(reliableNodes);
        return false;
    }

    if (cyclogram.isEmpty())
    {
        errorMessage = "Циклограмма не содержит этапов.";
        return false;
    }

    for (int i = 0; i < cyclogram.size(); ++i)
    {
        if (cyclogram[i].duration <= 0.0 || !std::isfinite(cyclogram[i].duration))
        {
            errorMessage = QString("У этапа %1 некорректная длительность. Длительность должна быть > 0.").arg(i + 1);
            return false;
        }
    }

    for (const SchemaNodeData& node : model.nodes)
    {
        if (node.nodeKind != NodeKind::Normal)
            continue;

        for (double lambda : node.failureRates)
        {
            if (lambda < 0.0 || !std::isfinite(lambda))
            {
                errorMessage = QString("Узел %1 содержит некорректную интенсивность отказа.").arg(node.name.isEmpty() ? QString("#%1").arg(node.id) : node.name);
                return false;
            }
        }
    }

    return true;
}

bool MainWindow::validateInitialProbabilities(const ProbabilityVector& probabilities, int stateCount, QString& errorMessage) const
{
    if (probabilities.isEmpty())
        return true;

    if (probabilities.size() != stateCount)
    {
        errorMessage = "Вектор начальных вероятностей имеет неверный размер.";
        return false;
    }

    double sum = 0.0;
    for (double probability : probabilities)
    {
        if (probability < 0.0 || !std::isfinite(probability))
        {
            errorMessage = "Начальные вероятности должны быть конечными и неотрицательными.";
            return false;
        }
        sum += probability;
    }

    if (std::abs(sum - 1.0) > 1.0e-6)
    {
        errorMessage = QString("Сумма начальных вероятностей должна быть равна 1. Сейчас сумма = %1.").arg(sum, 0, 'g', 10);
        return false;
    }

    return true;
}

ProbabilityVector MainWindow::initialProbabilitiesFromTable(int stateCount) const
{
    if (!initialProbabilitiesTable || initialProbabilitiesTable->rowCount() != stateCount)
    {
        if (savedInitialProbabilities.size() == stateCount)
            return savedInitialProbabilities;
        return {};
    }

    ProbabilityVector probabilities;
    probabilities.reserve(stateCount);
    for (int row = 0; row < stateCount; ++row)
    {
        bool ok = false;
        const double value = initialProbabilitiesTable->item(row, 1) ? initialProbabilitiesTable->item(row, 1)->text().toDouble(&ok) : 0.0;
        probabilities.append(ok ? value : std::numeric_limits<double>::quiet_NaN());
    }

    return probabilities;
}

void MainWindow::addCyclogramStage(OperationMode mode, double duration)
{
    const int row = cyclogramTable->rowCount();
    cyclogramTable->insertRow(row);
    cyclogramTable->setItem(row, 0, new QTableWidgetItem(QString("Этап %1").arg(row + 1)));

    QComboBox* modeComboBox = new QComboBox(cyclogramTable);
    modeComboBox->addItem("Транспортирование", static_cast<int>(OperationMode::Transportation));
    modeComboBox->addItem("Хранение", static_cast<int>(OperationMode::Storage));
    modeComboBox->addItem("Функционирование", static_cast<int>(OperationMode::Functioning));
    modeComboBox->addItem("Выключен", static_cast<int>(OperationMode::Off));
    const int modeIndex = modeComboBox->findData(static_cast<int>(mode));
    modeComboBox->setCurrentIndex(modeIndex >= 0 ? modeIndex : 0);
    cyclogramTable->setCellWidget(row, 1, modeComboBox);

    QDoubleSpinBox* durationSpinBox = new QDoubleSpinBox(cyclogramTable);
    durationSpinBox->setRange(0.0, 1.0e9);
    durationSpinBox->setDecimals(3);
    durationSpinBox->setValue(duration);
    durationSpinBox->setSuffix(" ч");
    cyclogramTable->setCellWidget(row, 2, durationSpinBox);

    cyclogramTable->resizeColumnsToContents();
}

void MainWindow::removeSelectedCyclogramStage()
{
    const int row = cyclogramTable->currentRow();
    if (row < 0 || cyclogramTable->rowCount() <= 1) return;

    cyclogramTable->removeRow(row);
}

Cyclogram MainWindow::cyclogramFromTable() const
{
    Cyclogram cyclogram;
    for (int row = 0; row < cyclogramTable->rowCount(); ++row)
    {
        CyclogramStage stage;
        stage.name = cyclogramTable->item(row, 0) ? cyclogramTable->item(row, 0)->text() : QString("Этап %1").arg(row + 1);

        if (const QComboBox* modeComboBox = qobject_cast<QComboBox*>(cyclogramTable->cellWidget(row, 1)))
            stage.mode = static_cast<OperationMode>(modeComboBox->currentData().toInt());

        if (const QDoubleSpinBox* durationSpinBox = qobject_cast<QDoubleSpinBox*>(cyclogramTable->cellWidget(row, 2)))
            stage.duration = durationSpinBox->value();

        cyclogram.append(stage);
    }

    if (cyclogram.isEmpty())
        cyclogram.append({"Этап 1", OperationMode::Functioning, 1.0});

    return cyclogram;
}

void MainWindow::setCyclogramTable(const Cyclogram& cyclogram)
{
    cyclogramTable->setRowCount(0);

    if (cyclogram.isEmpty())
    {
        addCyclogramStage(OperationMode::Functioning, 1.0);
        return;
    }

    for (const CyclogramStage& stage : cyclogram)
    {
        addCyclogramStage(stage.mode, stage.duration);
        const int row = cyclogramTable->rowCount() - 1;
        if (QTableWidgetItem* item = cyclogramTable->item(row, 0))
            item->setText(stage.name);
    }
}

void MainWindow::applyTopLevelStructureConfiguration()
{
    if (!topLevelStructureTypeComboBox || !topLevelRequiredElementsSpinBox) return;

    const StructureType structureType = static_cast<StructureType>(topLevelStructureTypeComboBox->currentData().toInt());
    structureScene->setTopLevelStructureConfiguration(structureType, topLevelRequiredElementsSpinBox->value());
    topLevelRequiredElementsSpinBox->setEnabled(structureType == StructureType::KOutOfN);
}

void MainWindow::syncTopLevelStructureControlsFromScene()
{
    if (!topLevelStructureTypeComboBox || !topLevelRequiredElementsSpinBox) return;

    const QSignalBlocker typeBlocker(topLevelStructureTypeComboBox);
    const QSignalBlocker kBlocker(topLevelRequiredElementsSpinBox);

    const int typeIndex = topLevelStructureTypeComboBox->findData(static_cast<int>(structureScene->getTopLevelStructureType()));
    topLevelStructureTypeComboBox->setCurrentIndex(typeIndex >= 0 ? typeIndex : 0);
    topLevelRequiredElementsSpinBox->setValue(structureScene->getTopLevelRequiredElements());
    topLevelRequiredElementsSpinBox->setEnabled(structureScene->getTopLevelStructureType() == StructureType::KOutOfN);
}

MainWindow::~MainWindow() = default;
