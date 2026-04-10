#include "reliabilityscene.h"

ReliabilityScene::ReliabilityScene(QObject* parent): QGraphicsScene(parent)
{
    setSceneRect(-5000, -5000, 10000, 10000);
    tempConnectionPath = new QGraphicsPathItem;
    tempConnectionPath->setZValue(-1.0);                     // рисуем под узлами
    tempConnectionPath->setPen(QPen(Qt::blue, 2, Qt::DashLine));
    tempConnectionPath->setVisible(false);
    addItem(tempConnectionPath);
}

void ReliabilityScene::setModelsAddMode(bool toSwich){modelsAddMode = toSwich;}

void ReliabilityScene::onUpLevel()
{
    clearNodes();
    if (selectedParentNode) selectedParentNode = selectedParentNode->getParent();
    if (selectedParentNode) {for (Node* child : selectedParentNode->getChildren()) addNodeToScene(child);}
    else{for (Node* rootNode : rootNodes) addNodeToScene(rootNode);}
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
    if(connectionMode && event->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        NodeGraphics* nodeItem =  dynamic_cast<NodeGraphics*> (item);
        if (!connectionDrawingActive)
        {
            if (!nodeItem) return;
            connectionStartNode = nodeItem->getModelNode();
            connectionDrawingActive = true;
            lastCursorScenePos = event->scenePos();
            tempConnectionPath->setPath(calculateRubberBandPath());
            tempConnectionPath->setVisible(true);
            event->accept();
            return;
        }
        else
        {
            if(nodeItem)
            {
                connectionStartNode = nullptr;
                connectionDrawingActive = false;
                connectionStartNode = nullptr;
                tempConnectionPath->setVisible(false);
                tempConnectionPath->setPath(QPainterPath());
                event->accept();
                return;
            }
        }
    }
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

void ReliabilityScene::setConnectionMode(bool enabled)
{
    connectionMode = enabled;
    if(!enabled)
    {
        connectionDrawingActive = false;
        connectionStartNode = nullptr;
        tempConnectionPath->setVisible(false);
        tempConnectionPath->setPath(QPainterPath());
    }
}



ReliabilityScene::~ReliabilityScene()
{
    qDeleteAll(rootNodes);
    rootNodes.clear();
}


void ReliabilityScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (connectionMode && connectionDrawingActive) {
        lastCursorScenePos = event->scenePos();
        tempConnectionPath->setPath(calculateRubberBandPath());
        event->accept();
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

QPainterPath ReliabilityScene::calculateRubberBandPath() const
{
    QPainterPath path;
    if (!connectionStartNode) return path;

    QPointF start = connectionStartNode->getPosition();
    QPointF end = lastCursorScenePos;

    QPointF corner(end.x(), start.y());
    path.moveTo(start);
    path.lineTo(corner);
    path.lineTo(end);
    return path;
}