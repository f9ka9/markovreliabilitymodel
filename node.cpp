#include "node.h"

Node::Node(Node* par): parent(par)
{}
Node::~Node(){}

Node* Node::getParent() const {return parent;}
void Node::setParent(Node* par){parent = par;}

QPointF Node::getPosition() const {return position;}
void Node::setPosition(const QPointF& pos){position = pos;};

QList<Node*> Node::getChildren() const {return children;}
void Node::addChild(Node* child) {children.append(child);}