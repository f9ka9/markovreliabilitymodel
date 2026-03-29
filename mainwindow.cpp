#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),

    structureScene{nullptr},
    stateScene{nullptr},
    structureView{nullptr},
    stateGraphView{nullptr},

    matrixDock{nullptr},
    probabilityDock{nullptr},

    matrixTable{nullptr},
    probabilityTable {nullptr},

    mainToolBar{nullptr},

    breadcrumbLabel{nullptr},

    configNodeAction{nullptr},
    addNodeAction{nullptr},
    connectAction{nullptr},
    calculateAction{nullptr},
    upLevelAction{nullptr},

    fileMenu{nullptr},
    viewMenu{nullptr},
    helpMenu{nullptr}
{
    //============================================================================================
    setWindowTitle("Markov Reliability Model");


    setupCentralWidgets();
    setupDockWidgets();

    createActions();
    setupToolBar();

    setupMenu();

    setupStatusBar();

    //============================================================================================
    connect(this, &MainWindow::upLevelSignal, structureScene, &ReliabilityScene::onUpLevel);
    connect(this, &MainWindow::deleteSelectedModelsNodesSignal, structureScene, &ReliabilityScene::onDeleteSelectedModelsNodes);

    //============================================================================================
}
//============================================================================================
void MainWindow::setupCentralWidgets()
{
    structureScene = new ReliabilityScene(this);
    structureView = new QGraphicsView(this);
    structureView->setScene(structureScene);
    structureView->setDragMode(QGraphicsView::ScrollHandDrag);

    stateScene = new StateScene(this);
    stateGraphView = new QGraphicsView(this);
    stateGraphView->setScene(stateScene);
    stateGraphView->setDragMode(QGraphicsView::ScrollHandDrag);

    splitter = new QSplitter(this);
    splitter->addWidget(structureView);
    splitter->addWidget(stateGraphView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

}

void MainWindow::setupDockWidgets()
{
    matrixDock = new QDockWidget("Матрицы Q(k) / P(k)", this);
    matrixTable = new QTableWidget(5,5,this);
    matrixDock->setWidget(matrixTable);
    addDockWidget(Qt::RightDockWidgetArea, matrixDock);

    probabilityDock = new QDockWidget("Вероятности состояний p(i)", this);
    probabilityTable = new QTableWidget(5,5,this);
    probabilityDock->setWidget(probabilityTable);
    addDockWidget(Qt::BottomDockWidgetArea, probabilityDock);

}

void MainWindow::createActions()
{

    //Можно сделать вариант с изображением-иконками на усмотрение куратора, пока оставлю так
    configNodeAction = new QAction("⚙ Конфигурация", this);
    configNodeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Q));
    configNodeAction->setStatusTip("Выбор конфигурации узла на сцене редактора структурной схемы надёжности");
    //connect (configNodeAction, &QAction::triggered, this, &MainWindow::configureNode) ;

    addNodeAction = new QAction("＋ Добавить узлы",this);
    addNodeAction->setCheckable(true);
    addNodeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    addNodeAction->setStatusTip("Добавить новые узелы на сцену редактора структурной схемы надежности");
    connect(addNodeAction, &QAction::triggered, this, &MainWindow::toggleModelsAddMode);

    deleteItemAction = new QAction("✖ Удалить узел" , this);
    deleteItemAction->setShortcut(QKeySequence(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_X)));
    deleteItemAction->setStatusTip("Удалить выбранный узел со сцены");
    connect(deleteItemAction, &QAction::triggered, this, &MainWindow::deleteSelectedModelsNodes);

    selectAction = new QAction("🔲 Выделение", this);
    selectAction->setCheckable(true);
    connect(selectAction, &QAction::toggled, this, &MainWindow::toggleSelectionMode);

    connectAction = new QAction("🔗 Соединить",this);
    connectAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    connectAction->setStatusTip("Соединение линиями между собой выбранных двух узлов на сцене редактора структурной схемы надёжности");
    //connect(connectAction, &QAction::triggered, this, &MainWindow::connectSelectedNodes);

    calculateAction = new QAction("∑ Расчитать",this);
    calculateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    calculateAction->setStatusTip("Выполнить расчет надежности для текущей структурной схемы");
    //connect(calculateAction, &QAction::triggered, this, &MainWindow::calculate);

    upLevelAction = new QAction("⬆ На уровень выше",this);
    upLevelAction->setShortcut(QKeySequence(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z)));
    upLevelAction->setStatusTip("Перейти на один уровень вверх в иерархической структуре");
    connect(upLevelAction, &QAction::triggered, this, &MainWindow::upLevel);

    newSchemaAction = new QAction("📄 Новая схема",this);
    newSchemaAction->setStatusTip("Создать новый файл структурной схемы");
    //connect(newSchemaAction, &QAction::triggered, this, &MainWindow::newSchema);

    openSchemaAction = new QAction("📂 Открыть...",this);
    openSchemaAction->setStatusTip("Открыть существующий файл структурной схемы");
    //connect(openSchemAction, &QAction::triggered, this, &MainWindow::openSchema);

    saveSchemaAction = new QAction("💾 Сохранить",this);
    saveSchemaAction->setShortcut(QKeySequence(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S)));
    saveSchemaAction->setStatusTip("Сохранить текущую схему в файл");
    //connect(saveSchemaAction, &QAction::triggered, this, &MainWindow::saveSchema);

    exportResultsAction = new QAction("📤 Экспорт результатов",this);
    exportResultsAction->setStatusTip("Экспортировать результаты в файл");
    //connect(exportResultsAction, &QAction::triggered, this, &MainWindow::exportResults);

    aboutAction = new QAction("❓  О программе",this);
    aboutAction->setStatusTip("Сведения о приложении Markov Reliability Model");
    //connect(aboutAction, &QAction::triggered, this, &MainWindow::about);


}

void MainWindow::setupToolBar()
{
    mainToolBar = new QToolBar ("Main Toolbar", this);
    addToolBar(Qt::TopToolBarArea,  mainToolBar);

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

    viewMenu->addAction(matrixDock->toggleViewAction());
    viewMenu->addAction(probabilityDock->toggleViewAction());

    helpMenu = new QMenu("Справка", this);
    menuBar()->addMenu(helpMenu);

    helpMenu->addAction(aboutAction);

}

void MainWindow::setupStatusBar ()
{
    statusBar()->showMessage("Готово", 5000);
}
//============================================================================================
void MainWindow::toggleModelsAddMode(bool toSwich)
{
    if (structureScene)
        structureScene->setModelsAddMode(toSwich);
}

void MainWindow::toggleSelectionMode(bool checked)
{

    if (checked)
    {
        structureView->setDragMode(QGraphicsView::RubberBandDrag);
        if(addNodeAction->isChecked())
            addNodeAction->setChecked(false);
            structureScene->setModelsAddMode(false);
    } else
    {
        structureView->setDragMode(QGraphicsView::ScrollHandDrag);
    }
}

void MainWindow::upLevel() {emit upLevelSignal();}

void MainWindow::deleteSelectedModelsNodes() {emit deleteSelectedModelsNodesSignal();}
//============================================================================================

//============================================================================================
MainWindow::~MainWindow()
{}