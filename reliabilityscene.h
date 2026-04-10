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
    void setConnectionMode(bool enabled);


    void setSelectedParentNode(Node* node);
    Node* getSelectedParentNode () const;

    void addRootNode(Node* node);
    QList<Node*> getRootNodes () const;

    void addNodeToScene(Node* node);
    void clearNodes();

    void createConnections();
    QPainterPath calculateRubberBandPath() const;


public slots:
    void onUpLevel();
    void onNodeDoubleClicked(Node* node);
    void onDeleteSelectedModelsNodes();

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool modelsAddMode{false};
    bool connectionMode{false};
    bool connectionDrawingActive{false};
    QPointF lastCursorScenePos;
    Node* connectionStartNode {nullptr};
    Node* selectedParentNode{nullptr};
    QList<Node*> rootNodes;
    QGraphicsPathItem* tempConnectionPath;
};
#endif // RELIABILITYSCENE_H