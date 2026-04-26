#include "node.h"

int Node::nextId = 1;

Node::Node(Node* par): id(nextId++), parent(par)
{}

Node::~Node()
{
    qDeleteAll(children);
    children.clear();
}

int Node::getId() const {return id;}

Node* Node::getParent() const {return parent;}
void Node::setParent(Node* par){parent = par;}

QList<Node*> Node::getChildren() const {return children;}

void Node::addChild(Node* child)
{
    if (!child) return;
    children.append(child);
}

void Node::removeChild(Node* child)
{
    if (!child) return;

    children.removeOne(child);
    child->setParent(nullptr);
}

QString Node::getName() const {return name;}
void Node::setName(const QString& value){name = value;}

QString Node::getGroupName() const {return groupName;}
void Node::setGroupName(const QString& value){groupName = value;}

FailureRates Node::getFailureRates() const {return failureRates;}
void Node::setFailureRates(const FailureRates& values){failureRates = values;}

double Node::getFailureRate(OperationMode mode) const
{
    return failureRates[static_cast<int>(mode)];
}

void Node::setFailureRate(OperationMode mode, double value)
{
    failureRates[static_cast<int>(mode)] = value;
}

Node::StructureType Node::getStructureType() const {return structureType;}
void Node::setStructureType(StructureType value){structureType = value;}

int Node::getRequiredElements() const {return requiredElements;}

void Node::setRequiredElements(int value)
{
    requiredElements = value < 1 ? 1 : value;
}
