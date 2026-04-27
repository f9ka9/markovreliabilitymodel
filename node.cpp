#include "node.h"

int Node::nextId = 1;

namespace
{
int modeIndex(OperationMode mode)
{
    return static_cast<int>(mode);
}

double resolveLambda(const LambdaDefinitions& definitions, OperationMode mode, std::array<bool, 4>& resolving)
{
    const int index = modeIndex(mode);
    const LambdaDefinition& definition = definitions[index];

    if (definition.inputType == LambdaInputType::Fixed)
        return definition.value;

    if (resolving[index])
        return 0.0;

    resolving[index] = true;
    const double referenceValue = resolveLambda(definitions, definition.referenceMode, resolving);
    resolving[index] = false;
    return definition.multiplier * referenceValue;
}
}

LambdaDefinitions defaultLambdaDefinitions()
{
    return LambdaDefinitions{};
}

LambdaDefinitions zeroLambdaDefinitions()
{
    LambdaDefinitions definitions{};
    for (LambdaDefinition& definition : definitions)
        definition = LambdaDefinition{};
    return definitions;
}

FailureRates resolvedFailureRates(const LambdaDefinitions& definitions)
{
    FailureRates values{};
    for (int i = 0; i < static_cast<int>(values.size()); ++i)
    {
        std::array<bool, 4> resolving{};
        values[i] = resolveLambda(definitions, static_cast<OperationMode>(i), resolving);
    }
    return values;
}

Node::Node(Node* par): id(nextId++), parent(par)
{}

Node::~Node()
{
    qDeleteAll(children);
    children.clear();
}

int Node::getId() const {return id;}

void Node::setIdForLoading(int value)
{
    if (value < 1) return;

    id = value;
    if (nextId <= value)
        nextId = value + 1;
}

NodeConfiguration Node::getConfiguration() const {return configuration;}

void Node::setConfiguration(const NodeConfiguration& value)
{
    configuration = value;
    if (configuration.requiredElements < 1)
        configuration.requiredElements = 1;
    if (configuration.structureType != StructureType::Element)
        configuration.lambdaDefinitions = zeroLambdaDefinitions();
}

Node* Node::getParent() const {return parent;}
void Node::setParent(Node* par){parent = par;}

QList<Node*> Node::getChildren() const {return children;}

void Node::addChild(Node* child)
{
    if (!child) return;
    child->setParent(this);
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

FailureRates Node::getFailureRates() const {return resolvedFailureRates(configuration.lambdaDefinitions);}

void Node::setFailureRates(const FailureRates& values)
{
    LambdaDefinitions definitions = defaultLambdaDefinitions();
    for (int i = 0; i < static_cast<int>(values.size()); ++i)
    {
        definitions[i].inputType = LambdaInputType::Fixed;
        definitions[i].value = values[i];
    }
    configuration.lambdaDefinitions = definitions;
}

LambdaDefinitions Node::getLambdaDefinitions() const {return configuration.lambdaDefinitions;}

void Node::setLambdaDefinitions(const LambdaDefinitions& values)
{
    configuration.lambdaDefinitions = values;
}

double Node::getFailureRate(OperationMode mode) const
{
    return getFailureRates()[static_cast<int>(mode)];
}

void Node::setFailureRate(OperationMode mode, double value)
{
    LambdaDefinition& definition = configuration.lambdaDefinitions[static_cast<int>(mode)];
    definition.inputType = LambdaInputType::Fixed;
    definition.value = value;
}

Node::StructureType Node::getStructureType() const {return configuration.structureType;}
void Node::setStructureType(StructureType value){configuration.structureType = value;}

int Node::getRequiredElements() const {return configuration.requiredElements;}

void Node::setRequiredElements(int value)
{
    configuration.requiredElements = value < 1 ? 1 : value;
}
