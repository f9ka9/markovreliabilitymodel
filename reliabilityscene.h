#ifndef RELIABILITYSCENE_H
#define RELIABILITYSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

#include "nodegraphics.h"

class ReliabilityScene : public QGraphicsScene
{
    Q_OBJECT
public:
    ReliabilityScene(QObject* parent = nullptr);
    ~ReliabilityScene();

    void setModelsAddMode(bool toSwitch);

    void setSelectedParentNode(Node* node);
    Node* getSelectedParentNode () const;

    void addRootNode(Node* node);
    QList<Node*> getRootNodes () const;

    void addNodeToScene(Node* node);
    void clearNodes();

    void createConnections();


public slots:
    void onUpLevel();
    void onNodeDoubleClicked(Node* node);
    void onDeleteSelectedModelsNodes();
    void onConnectSelectedNodes();

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool modelsAddMode = false;
    Node* selectedParentNode{nullptr};
    QList<Node*> rootNodes;
};
#endif // RELIABILITYSCENE_H