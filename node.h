#ifndef NODE_H
#define NODE_H

#include <QString>
#include <QList>
#include <QPointF>

class Node
{
public:
    Node(Node* par = nullptr);
    ~Node();

    Node* getParent() const ;
    void setParent(Node* par);

    QPointF getPosition() const;
    void setPosition(const QPointF& pos);

    QList<Node*> getChildren() const;
    void addChild(Node* child);

private:
    Node* parent{nullptr};
    QList<Node*> children;
    // Пока, надеюсь, добавление position в node - это временное решение, так как я не хочу
    // замарачиваться с тем , как отрисовывать элементы на экране.
    QPointF position;
};

#endif // NODE_H