#include "reliabilityscene.h"

ReliabilityScene::ReliabilityScene(QObject* parent): QGraphicsScene(parent)
{
    setSceneRect(-5000, -5000, 10000, 10000);
}

void ReliabilityScene::setModelsAddMode(bool toSwich){modelsAddMode = toSwich;}

void ReliabilityScene::onUpLevel()
{
    clearNodes();
    if (selectedParentNode) selectedParentNode = selectedParentNode->getParent();
    if (selectedParentNode) {for (Node* child : selectedParentNode->getChildren()) addNodeToScene(child);}
    else{for (Node* rootNode : rootNodes) addNodeToScene(rootNode);}
    createConnections();
}

void ReliabilityScene::addNodeToScene(Node* node)
{
    NodeGraphics* modelNode = new NodeGraphics(node);
    connect(modelNode, &NodeGraphics::nodeDoubleClicked, this, &ReliabilityScene::onNodeDoubleClicked);
    modelNode->setPos(node->getPosition());
    addItem(modelNode);
}

void ReliabilityScene::setSelectedParentNode(Node* node) {selectedParentNode = node;}
Node* ReliabilityScene::getSelectedParentNode () const {return selectedParentNode;}

void ReliabilityScene::addRootNode(Node* node) {rootNodes.append(node);}
QList<Node*> ReliabilityScene::getRootNodes () const {return rootNodes;}

void ReliabilityScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    int step = 50;
    painter->setPen(QPen(Qt::lightGray, 2));
    int left = floor(rect.left()/step)*step;
    int top = floor(rect.top()/step)*step;
    for(int x = left; x <= rect.right(); x+=step)
        for(int y = top; y <= rect.bottom(); y+=step)
            painter->drawPoint(x,y);
}

void ReliabilityScene::onNodeDoubleClicked(Node* node)
{
    if (node)
    {
        clearNodes();
        selectedParentNode = node;
        for(Node* child : node->getChildren()) addNodeToScene(child);
        createConnections();
    }
}

void ReliabilityScene::onDeleteSelectedModelsNodes()
{
    QList<QGraphicsItem*> itemsToDelete = selectedItems();
    if (itemsToDelete.isEmpty()) return;
    for(QGraphicsItem* item : itemsToDelete)
    {
        NodeGraphics* nodeModel = dynamic_cast<NodeGraphics*>(item);
        if (!nodeModel) continue;
        Node* node = nodeModel->getModelNode();
        if (!node) continue;
        if (node->getParent()) node->getParent()->removeChild(node);
        else rootNodes.removeOne(node);
        removeItem(item);
        delete item;
        delete node;
    }
}

void ReliabilityScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(modelsAddMode && event->button() == Qt::LeftButton)
    {
        if (!items(event->scenePos()).isEmpty()) return;
        int step = 50;
        int x = round(event->scenePos().x()/step)*step;
        int y = round(event->scenePos().y()/step)*step;
        Node* node = new Node();
        node->setPosition(QPointF(x,y));

        if (selectedParentNode == nullptr) addRootNode(node);
        else
        {
            selectedParentNode->addChild(node);
            node->setParent(selectedParentNode);
        }
        addNodeToScene(node);
    }
    QGraphicsScene::mousePressEvent(event);
}

void ReliabilityScene::clearNodes()
{
    QList<QGraphicsItem*> itemsCopy = items();
    for (QGraphicsItem* item : itemsCopy)
    {
        removeItem(item);
        delete item;
    }
}

void ReliabilityScene::createConnections()
{}

ReliabilityScene::~ReliabilityScene()
{
    qDeleteAll(rootNodes);
    rootNodes.clear();
}

void ReliabilityScene::onConnectSelectedNodes()
{
}