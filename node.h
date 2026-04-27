#ifndef NODE_H
#define NODE_H

#include <array>

#include <QList>
#include <QString>

using FailureRates = std::array<double, 4>;

enum class StructureType
{
    Element,
    Series,
    Parallel,
    KOutOfN
};

enum class OperationMode
{
    Transportation = 0,
    Storage = 1,
    Functioning = 2,
    Off = 3
};

enum class LambdaInputType
{
    Fixed,
    Expression
};

struct LambdaDefinition
{
    LambdaInputType inputType{LambdaInputType::Fixed};
    double value{0.0};
    double multiplier{1.0};
    OperationMode referenceMode{OperationMode::Functioning};
};

using LambdaDefinitions = std::array<LambdaDefinition, 4>;

LambdaDefinitions defaultLambdaDefinitions();
LambdaDefinitions zeroLambdaDefinitions();
FailureRates resolvedFailureRates(const LambdaDefinitions& definitions);

struct NodeConfiguration
{
    QString name{"Node"};
    QString groupName;
    LambdaDefinitions lambdaDefinitions{defaultLambdaDefinitions()};
    StructureType structureType{StructureType::Element};
    int requiredElements{1};
};

class Node
{
public:
    using StructureType = ::StructureType;
    using OperationMode = ::OperationMode;

    explicit Node(Node* par = nullptr);
    ~Node();

    int getId() const;
    void setIdForLoading(int value);

    NodeConfiguration getConfiguration() const;
    void setConfiguration(const NodeConfiguration& value);

    Node* getParent() const;
    void setParent(Node* par);

    QList<Node*> getChildren() const;
    void addChild(Node* child);
    void removeChild(Node* child);

    QString getName() const;
    void setName(const QString& value);

    QString getGroupName() const;
    void setGroupName(const QString& value);

    FailureRates getFailureRates() const;
    void setFailureRates(const FailureRates& values);
    LambdaDefinitions getLambdaDefinitions() const;
    void setLambdaDefinitions(const LambdaDefinitions& values);
    double getFailureRate(OperationMode mode) const;
    void setFailureRate(OperationMode mode, double value);

    StructureType getStructureType() const;
    void setStructureType(StructureType value);

    int getRequiredElements() const;
    void setRequiredElements(int value);

private:
    static int nextId;

    int id{0};
    Node* parent{nullptr};
    QList<Node*> children;
    NodeConfiguration configuration;
};

#endif // NODE_H
