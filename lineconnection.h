#ifndef _LINECONNECTION_H
#define _LINECONNECTION_H

#include <QPoint>
#include "node.h"

class Node;

class LineConnection
{
public:
    LineConnection() = default;
    ~LineConnection() = default;

    Node* getFirst() const;
    void setFirst(Node* node);

    Node* getSecond() const;
    void setSecond(Node* node);

private:
    Node* first{nullptr};
    Node* second{nullptr};
};

#endif // _LINECONNECTION_H
