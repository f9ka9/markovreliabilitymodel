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
void Node::addChild(Node* child)
{
    if (!child) return;
    children.append(child);
}

void Node::removeChild(Node* child)
{
    if (!child)
        return;

    children.removeOne(child);
    child->setParent(nullptr);
}

QList<Node*> Node::getNeighborNodes() const {return neighborNodes;}
void Node::addNeighborNodes(Node* node){if (!neighborNodes.contains(node)) neighborNodes.append(node);}
