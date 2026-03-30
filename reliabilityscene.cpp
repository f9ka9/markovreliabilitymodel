#include "reliabilityscene.h"

ReliabilityScene::ReliabilityScene(QObject* parent): QGraphicsScene(parent)
{
    setSceneRect(-5000, -5000, 10000, 10000);
}

void ReliabilityScene::setModelsAddMode(bool toSwich){modelsAddMode = toSwich;}

void ReliabilityScene::onUpLevel()
{

    clearNodes(items());
    if(selectedParentNode && selectedParentNode->getParent())
    {
        selectedParentNode = selectedParentNode->getParent();
        for(Node* child : selectedParentNode->getChildren()) addNodeToScene(child);
    }

    else
    {
        selectedParentNode = nullptr;
        for(Node* rootNode : rootNodes) addNodeToScene(rootNode);
    }
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
        clearNodes(items());
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
        if(!node->getChildren().isEmpty()) node->deleteChildren();
        if (node->getParent()) node->getParent()->removeChild(node);
        else rootNodes.removeOne(node);
        removeItem(item);
        delete item;
        delete node;
    }
}

void ReliabilityScene::onConnectSelectedNodes()
{
    if (selectedItems().size() != 2) return;
    NodeGraphics* first = dynamic_cast<NodeGraphics*> (selectedItems()[0]);
    NodeGraphics* second = dynamic_cast<NodeGraphics*> (selectedItems()[1]);
    LineConnectionGraphics* line = new LineConnectionGraphics(first, second);
    addItem(line);
    first->getModelNode()->addNeighborNodes(second->getModelNode());
    second->getModelNode()->addNeighborNodes(first->getModelNode());
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

        qDebug() << node->getParent();

        NodeGraphics* modelNode = new NodeGraphics(node);
        connect(modelNode, &NodeGraphics::nodeDoubleClicked, this, &ReliabilityScene::onNodeDoubleClicked);
        modelNode->setPos(x,y);
        addItem(modelNode);
    }
    QGraphicsScene::mousePressEvent(event);
}

void ReliabilityScene::clearNodes(const QList<QGraphicsItem*>& items)
{

    for(QGraphicsItem* item : items)
    {
        //спросить про это у куратора*
        NodeGraphics* mNode = dynamic_cast<NodeGraphics*> (item);
        removeItem(item);
    }
}

void ReliabilityScene::createConnections()
{
    //это временное решение , я потом переделаю !!!!!!!!
    QList<NodeGraphics*> nodeGraphicsList;

    for (QGraphicsItem* item : items())
        if (NodeGraphics* node = dynamic_cast<NodeGraphics*>(item))
            nodeGraphicsList.append(node);

    for (NodeGraphics* node : nodeGraphicsList)
    {
        Node* model = node->getModelNode();
        for (Node* neighbor : model->getNeighborNodes())
        {
            for (NodeGraphics* other : nodeGraphicsList)
            {
                if (other->getModelNode() == neighbor)
                {
                    if (node < other)
                    {
                        LineConnectionGraphics* line = new LineConnectionGraphics(node, other);

                        addItem(line);
                    }
                }
            }
        }
    }
}