#ifndef RELIABILITYSCENE_H
#define RELIABILITYSCENE_H

#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QHash>
#include <QList>
#include <QPainterPath>
#include <QPointF>
#include <QString>

#include "lineconnectiongraphics.h"
#include "nodegraphics.h"
#include "schemamodel.h"

class ReliabilityScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit ReliabilityScene(QObject* parent = nullptr);
    ~ReliabilityScene();

    void setModelsAddMode(bool enabled);
    void setConnectionMode(bool enabled);
    void setDeleteMode(bool enabled);
    void setDefaultNodeConfiguration(const NodeConfiguration& configuration);
    NodeConfiguration getDefaultNodeConfiguration() const;
    Node* selectedNode() const;
    void updateNodeGraphics(Node* node);
    QString currentLevelPath() const;
    int currentDepth() const;
    SchemaModel exportSchemaModel() const;
    SchemaModel exportCurrentLevelModel() const;

public slots:
    void onUpLevel();
    void onNodeDoubleClicked(Node* node);

signals:
    void editorModesResetRequested();
    void nodeAboutToBeRemoved(Node* node);
    void currentLevelChanged(const QString& path);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

private:
    bool modelsAddMode{false};
    bool connectionMode{false};
    bool deleteMode{false};
    bool connectionDrawingActive{false};

    QPointF lastCursorScenePos;
    NodeGraphics* connectionStartNodeGraphics{nullptr};
    Node* selectedParentNode{nullptr};
    QList<Node*> rootNodes;
    QList<LineConnection*> connections;
    QList<QPointF> draftConnectionPoints;
    QHash<Node*, NodeGraphics*> nodeGraphicsByNode;
    QHash<LineConnection*, LineConnectionGraphics*> connectionGraphicsByConnection;
    QGraphicsPathItem* tempConnectionPath{nullptr};
    NodeConfiguration defaultNodeConfiguration;

    void addRootNode(Node* node);
    NodeGraphics* addNodeToScene(Node* node);
    LineConnectionGraphics* addConnectionToScene(LineConnection* connection);
    void clearSceneItems();
    void deleteAllEditorObjects();
    void refreshCurrentLevel();

    void createConnection(NodeGraphics* first, NodeGraphics* second, const QList<QPointF>& bendPoints);
    void removeConnection(LineConnection* connection);
    void removeConnectionsForNode(Node* node);
    void removeNode(NodeGraphics* nodeGraphics);
    void emitNodeAboutToBeRemovedForSubtree(Node* node);
    void removeNodeGraphicsForSubtree(Node* node);

    void startConnectionDrawing(NodeGraphics* nodeGraphics, const QPointF& cursorPos);
    void finishConnectionDrawing(NodeGraphics* nodeGraphics);
    void addDraftConnectionPoint(const QPointF& scenePos);
    void cancelConnectionDrawing();
    void resetEditorModes();

    QPointF calculateOrthogonalPoint(const QPointF& from, const QPointF& target) const;
    QPainterPath calculateRubberBandPath() const;
    bool canPlaceNodeAt(const QPointF& scenePos) const;
    bool isNodeInCurrentLevel(Node* node) const;
    bool nodeContains(Node* root, Node* candidate) const;
    void appendNodeToSchemaModel(Node* node, SchemaModel& model, int depth) const;
};

#endif // RELIABILITYSCENE_H
