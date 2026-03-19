#include "reliabilityscene.h"

ReliabilityScene::ReliabilityScene(QObject* parent): QGraphicsScene(parent)
{
    setSceneRect(-5000, -5000, 10000, 10000);
}

void ReliabilityScene::setModelsAddMode(bool toSwich){modelsAddMode = toSwich;}

void ReliabilityScene::onUpLevel()
{
    clear();

    if(selectedParentNode && selectedParentNode->getParent())
    {
        selectedParentNode = selectedParentNode->getParent();
        QList<Node*> children = selectedParentNode->getChildren();
        for(Node* child : children)
        {
            NodeGraphics* modelNode = new NodeGraphics(child);
            connect(modelNode, &NodeGraphics::nodeDoubleClicked, this, &ReliabilityScene::onNodeDoubleClicked);
            modelNode->setPos(child->getPosition());
            addItem(modelNode);
        }
    }
    else
    {
        QList<Node*> rootNodes = getRootNodes();
        for(Node* rootNode : rootNodes)
        {
            NodeGraphics* modelNode = new NodeGraphics(rootNode);
            connect(modelNode, &NodeGraphics::nodeDoubleClicked, this, &ReliabilityScene::onNodeDoubleClicked);
            modelNode->setPos(rootNode->getPosition());
            addItem(modelNode);
        }
    }
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
    clear();
    selectedParentNode = node;
    QList<Node*> children = node->getChildren();
    for(Node* child : children)
    {
        NodeGraphics* modelNode = new NodeGraphics(child);
        connect(modelNode, &NodeGraphics::nodeDoubleClicked, this, &ReliabilityScene::onNodeDoubleClicked);
        modelNode->setPos(child->getPosition());
        addItem(modelNode);
    }
}

void ReliabilityScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(modelsAddMode && event->button() == Qt::LeftButton)
    {
        int step = 50;
        int x = round(event->scenePos().x()/step)*step;
        int y = round(event->scenePos().y()/step)*step;
        Node* node = new Node();
        node->setPosition(QPointF(x,y));

        if (selectedParentNode == nullptr) addRootNode(node);
        else selectedParentNode->addChild(node);

        NodeGraphics* modelNode = new NodeGraphics(node);
        connect(modelNode, &NodeGraphics::nodeDoubleClicked, this, &ReliabilityScene::onNodeDoubleClicked);
        modelNode->setPos(x,y);
        addItem(modelNode);
    }
    QGraphicsScene::mousePressEvent(event);
}