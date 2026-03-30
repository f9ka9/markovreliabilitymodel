#include "node.h"

Node::Node(Node* par): parent(par)
{}
Node::~Node()
{
    qDeleteAll(children);
    children.clear();
}

Node* Node::getParent() const {return parent;}
void Node::setParent(Node* par){parent = par;}

QPointF Node::getPosition() const {return position;}
void Node::setPosition(const QPointF& pos){position = pos;}

QPointF Node::getPreviousPosition() const {return previousPosition;}
void Node::setPreviousPosition(const QPointF& pos){previousPosition = pos;}

QList<Node*> Node::getChildren() const {return children;}
void Node::addChild(Node* child) {children.append(child);}
void Node::deleteChildren()
{
    QList<Node*> copyNodesList = children;
    for(Node* node: copyNodesList)
    {
        if(!node->getChildren().isEmpty())
        {
            node->deleteChildren();
        }
        node->setParent(nullptr);
        delete node;
    }
    children.clear();
}

void Node::removeChild(Node* child)
{
    children.removeOne(child);
}

QList<Node*> Node::getNeighborNodes() const {return neighborNodes;}
void Node::addNeighborNodes(Node* node) {neighborNodes.append(node);}
