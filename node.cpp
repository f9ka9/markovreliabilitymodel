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

NodeConfiguration Node::getConfiguration() const {return configuration;}

void Node::setConfiguration(const NodeConfiguration& value)
{
    configuration = value;
    if (configuration.requiredElements < 1)
        configuration.requiredElements = 1;
}

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

QString Node::getName() const {return configuration.name;}
void Node::setName(const QString& value){configuration.name = value;}

QString Node::getGroupName() const {return configuration.groupName;}
void Node::setGroupName(const QString& value){configuration.groupName = value;}

FailureRates Node::getFailureRates() const {return configuration.failureRates;}
void Node::setFailureRates(const FailureRates& values){configuration.failureRates = values;}

double Node::getFailureRate(OperationMode mode) const
{
    return configuration.failureRates[static_cast<int>(mode)];
}

void Node::setFailureRate(OperationMode mode, double value)
{
    configuration.failureRates[static_cast<int>(mode)] = value;
}

Node::StructureType Node::getStructureType() const {return configuration.structureType;}
void Node::setStructureType(StructureType value){configuration.structureType = value;}

int Node::getRequiredElements() const {return configuration.requiredElements;}

void Node::setRequiredElements(int value)
{
    configuration.requiredElements = value < 1 ? 1 : value;
}
