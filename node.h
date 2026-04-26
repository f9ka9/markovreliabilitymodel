#ifndef NODE_H
#define NODE_H

#include <array>

#include <QList>
#include <QString>

using FailureRates = std::array<double, 4>;

class Node
{
public:
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

    explicit Node(Node* par = nullptr);
    ~Node();

    int getId() const;

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
    QString name{"Node"};
    QString groupName;
    FailureRates failureRates{};
    StructureType structureType{StructureType::Element};
    int requiredElements{1};
};

#endif // NODE_H
