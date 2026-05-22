#ifndef RELIABILITYSCENE_H
#define RELIABILITYSCENE_H

#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QHash>
#include <QList>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QTransform>
#include <QtMath>

#include "editorschemamodel.h"
#include "gridsettings.h"
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
    void setConfigurationMode(bool enabled);

    void setDefaultNodeConfiguration(const NodeConfiguration& configuration);
    NodeConfiguration getDefaultNodeConfiguration() const;

    void setTopLevelStructureConfiguration(StructureType structureType, int requiredElements);
    StructureType getTopLevelStructureType() const;
    int getTopLevelRequiredElements() const;

    Node* selectedNode() const;
    void updateNodeGraphics(Node* node);

    QString currentLevelPath() const;
    int currentDepth() const;

    SchemaModel exportCurrentLevelModel() const;
    EditorSchemaModel exportEditorSchemaModel() const;
    bool importEditorSchemaModel(const EditorSchemaModel& model, QString& errorMessage);

    void clearSchema();
    void removeNodeChildren(Node* node);

signals:
    void editorModesResetRequested();
    void nodeAboutToBeRemoved(Node* node);
    void nodeConfigurationRequested(Node* node);
    void currentLevelChanged(const QString& path);

public slots:
    void onUpLevel();
    void onNodeDoubleClicked(Node* node);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void addRootNode(Node* node);
    NodeGraphics* addNodeToScene(Node* node);
    LineConnectionGraphics* addConnectionToScene(LineConnection* connection);

    NodeGraphics* createNodeGraphics(Node* node);
    LineConnectionGraphics* createConnectionGraphics(LineConnection* connection, const QList<QPointF>& bendPoints);

    void clearSceneItems();
    void deleteAllEditorObjects();
    void refreshCurrentLevel();

    void createTempConnectionPath();

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
    SchemaNodeData createNodeData(Node* node, int depth) const;
    SchemaConnectionData createConnectionData(LineConnection* connection) const;
    void appendNodeToEditorSchemaModel(Node* node, EditorSchemaModel& model) const;

    bool modelsAddMode{false};
    bool connectionMode{false};
    bool deleteMode{false};
    bool configurationMode{false};
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
    StructureType topLevelStructureType{StructureType::Series};
    int topLevelRequiredElements{1};
};

#endif // RELIABILITYSCENE_H
