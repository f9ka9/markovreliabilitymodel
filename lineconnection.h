#ifndef LINECONNECTION_H
#define LINECONNECTION_H

class Node;

class LineConnection
{
public:
    LineConnection() = default;

    Node* getSource() const;
    void setSource(Node* node);

    Node* getTarget() const;
    void setTarget(Node* node);

private:
    Node* source{nullptr};
    Node* target{nullptr};
};

#endif // LINECONNECTION_H
