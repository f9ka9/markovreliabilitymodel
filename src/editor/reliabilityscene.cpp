#include "reliabilityscene.h"

ReliabilityScene::ReliabilityScene(QObject* parent): QGraphicsScene(parent)
{
    setSceneRect(-5000, -5000, 10000, 10000);
    createTempConnectionPath();
}

ReliabilityScene::~ReliabilityScene()
{
    deleteAllEditorObjects();
}

void ReliabilityScene::setModelsAddMode(bool enabled)
{
    modelsAddMode = enabled;
}

void ReliabilityScene::setConnectionMode(bool enabled)
{
    connectionMode = enabled;
    if (!enabled) cancelConnectionDrawing();
}

void ReliabilityScene::setDeleteMode(bool enabled)
{
    deleteMode = enabled;
    if (enabled) cancelConnectionDrawing();
}

void ReliabilityScene::setConfigurationMode(bool enabled)
{
    configurationMode = enabled;
    if (enabled) cancelConnectionDrawing();
}

void ReliabilityScene::setDefaultNodeConfiguration(const NodeConfiguration& configuration)
{
    defaultNodeConfiguration = configuration;
}

NodeConfiguration ReliabilityScene::getDefaultNodeConfiguration() const {return defaultNodeConfiguration;}

void ReliabilityScene::setTopLevelStructureConfiguration(StructureType structureType, int requiredElements)
{
    topLevelStructureType = structureType;
    topLevelRequiredElements = requiredElements < 1 ? 1 : requiredElements;
}

StructureType ReliabilityScene::getTopLevelStructureType() const {return topLevelStructureType;}

int ReliabilityScene::getTopLevelRequiredElements() const {return topLevelRequiredElements;}

Node* ReliabilityScene::selectedNode() const
{
    const QList<QGraphicsItem*> selected = selectedItems();
    if (selected.size() != 1) return nullptr;

    const NodeGraphics* nodeGraphics = dynamic_cast<NodeGraphics*>(selected.first());
    return nodeGraphics ? nodeGraphics->getModelNode() : nullptr;
}

void ReliabilityScene::updateNodeGraphics(Node* node)
{
    if (NodeGraphics* graphics = nodeGraphicsByNode.value(node, nullptr))
        graphics->update();
}

QString ReliabilityScene::currentLevelPath() const
{
    if (!selectedParentNode) return "Корень";

    QStringList path;
    for (Node* node = selectedParentNode; node; node = node->getParent())
        path.prepend(QString("%1 (#%2)").arg(node->getName()).arg(node->getId()));

    path.prepend("Корень");
    return path.join(" > ");
}

int ReliabilityScene::currentDepth() const
{
    int depth = 1;
    for (Node* node = selectedParentNode; node; node = node->getParent())
        ++depth;
    return depth;
}


SchemaModel ReliabilityScene::exportCurrentLevelModel() const
{
    SchemaModel model;
    model.currentParentId = selectedParentNode ? selectedParentNode->getId() : 0;
    model.structureType = selectedParentNode ? selectedParentNode->getStructureType() : topLevelStructureType;
    model.requiredElements = selectedParentNode ? selectedParentNode->getRequiredElements() : topLevelRequiredElements;

    const QList<Node*> nodes = selectedParentNode ? selectedParentNode->getChildren() : rootNodes;
    for (Node* node : nodes)
    {
        if (!node) continue;

        model.nodes.append(createNodeData(node, currentDepth()));
    }

    for (LineConnection* connection : connections)
    {
        if (!connection || !isNodeInCurrentLevel(connection->getSource()) || !isNodeInCurrentLevel(connection->getTarget()))
            continue;

        model.connections.append(createConnectionData(connection));
    }

    return model;
}

EditorSchemaModel ReliabilityScene::exportEditorSchemaModel() const
{
    EditorSchemaModel model;
    model.currentParentId = selectedParentNode ? selectedParentNode->getId() : 0;
    model.defaultNodeConfiguration = defaultNodeConfiguration;
    model.topLevelStructureType = topLevelStructureType;
    model.topLevelRequiredElements = topLevelRequiredElements;

    for (Node* node : rootNodes)
        appendNodeToEditorSchemaModel(node, model);

    for (LineConnection* connection : connections)
    {
        if (!connection || !connection->getSource() || !connection->getTarget()) continue;

        EditorSchemaConnectionData connectionData;
        connectionData.sourceNodeId = connection->getSource()->getId();
        connectionData.targetNodeId = connection->getTarget()->getId();

        if (const LineConnectionGraphics* graphics = connectionGraphicsByConnection.value(connection, nullptr))
            connectionData.bendPoints = graphics->getBendPoints();

        model.connections.append(connectionData);
    }

    return model;
}

bool ReliabilityScene::importEditorSchemaModel(const EditorSchemaModel& model, QString& errorMessage)
{
    QHash<int, Node*> nodesById;
    QHash<int, int> parentById;

    for (const EditorSchemaNodeData& nodeData : model.nodes)
    {
        if (nodesById.contains(nodeData.id))
        {
            errorMessage = "В файле схемы найден повторяющийся идентификатор узла.";
            qDeleteAll(nodesById);
            return false;
        }

        Node* node = new Node;
        node->setIdForLoading(nodeData.id);
        node->setConfiguration(nodeData.configuration);
        nodesById.insert(nodeData.id, node);
        parentById.insert(nodeData.id, nodeData.parentId);
    }

    QList<Node*> loadedRootNodes;
    for (const EditorSchemaNodeData& nodeData : model.nodes)
    {
        if (nodeData.parentId == 0)
        {
            loadedRootNodes.append(nodesById.value(nodeData.id));
            continue;
        }

        if (!nodesById.contains(nodeData.parentId))
        {
            errorMessage = "В файле схемы узел ссылается на несуществующего родителя.";
            qDeleteAll(nodesById);
            return false;
        }

        int depth = 1;
        int parentId = nodeData.parentId;
        QList<int> visitedNodeIds{nodeData.id};
        while (parentId != 0)
        {
            if (visitedNodeIds.contains(parentId))
            {
                errorMessage = "В файле схемы обнаружен цикл вложенности узлов.";
                qDeleteAll(nodesById);
                return false;
            }

            visitedNodeIds.append(parentId);
            ++depth;
            if (depth > 4)
            {
                errorMessage = "В файле схемы глубина вложенности больше 4 уровней.";
                qDeleteAll(nodesById);
                return false;
            }

            parentId = parentById.value(parentId, 0);
        }
    }

    for (const EditorSchemaConnectionData& connectionData : model.connections)
    {
        if (!nodesById.contains(connectionData.sourceNodeId) || !nodesById.contains(connectionData.targetNodeId))
        {
            errorMessage = "В файле схемы связь ссылается на несуществующий узел.";
            qDeleteAll(nodesById);
            return false;
        }
        if (parentById.value(connectionData.sourceNodeId) != parentById.value(connectionData.targetNodeId))
        {
            errorMessage = "В файле схемы связь соединяет узлы разных уровней.";
            qDeleteAll(nodesById);
            return false;
        }
    }

    for (const EditorSchemaNodeData& nodeData : model.nodes)
    {
        if (nodeData.parentId == 0) continue;

        Node* parent = nodesById.value(nodeData.parentId);
        Node* node = nodesById.value(nodeData.id);
        parent->addChild(node);
    }

    clearSchema();
    defaultNodeConfiguration = model.defaultNodeConfiguration;
    topLevelStructureType = model.topLevelStructureType;
    topLevelRequiredElements = model.topLevelRequiredElements < 1 ? 1 : model.topLevelRequiredElements;
    rootNodes = loadedRootNodes;

    for (const EditorSchemaNodeData& nodeData : model.nodes)
    {
        Node* node = nodesById.value(nodeData.id, nullptr);
        NodeGraphics* graphics = createNodeGraphics(node);
        if (graphics)
            graphics->setPos(GridSettings::snapToGrid(nodeData.position));
    }

    for (const EditorSchemaConnectionData& connectionData : model.connections)
    {
        LineConnection* connection = new LineConnection;
        connection->setSource(nodesById.value(connectionData.sourceNodeId));
        connection->setTarget(nodesById.value(connectionData.targetNodeId));
        connections.append(connection);
        createConnectionGraphics(connection, connectionData.bendPoints);
    }

    selectedParentNode = nodesById.value(model.currentParentId, nullptr);
    refreshCurrentLevel();
    emit currentLevelChanged(currentLevelPath());
    return true;
}

void ReliabilityScene::clearSchema()
{
    deleteAllEditorObjects();
    emit currentLevelChanged(currentLevelPath());
}

void ReliabilityScene::onUpLevel()
{
    resetEditorModes();
    if (selectedParentNode) selectedParentNode = selectedParentNode->getParent();
    refreshCurrentLevel();
    emit editorModesResetRequested();
    emit currentLevelChanged(currentLevelPath());
}

void ReliabilityScene::onNodeDoubleClicked(Node* node)
{
    if (!node) return;
    if (node->getNodeKind() != NodeKind::Normal) return;
    if (node->getStructureType() == StructureType::Element) return;
    if (currentDepth() >= 4) return;

    resetEditorModes();
    selectedParentNode = node;
    refreshCurrentLevel();
    emit editorModesResetRequested();
    emit currentLevelChanged(currentLevelPath());
}

void ReliabilityScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->setPen(QPen(Qt::lightGray, 2));

    const int left = qFloor(rect.left() / GridSettings::step) * GridSettings::step;
    const int top = qFloor(rect.top() / GridSettings::step) * GridSettings::step;

    for(int x = left; x <= rect.right(); x += GridSettings::step)
        for(int y = top; y <= rect.bottom(); y += GridSettings::step)
            painter->drawPoint(x, y);
}

void ReliabilityScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if(configurationMode && event->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        if (NodeGraphics* nodeItem = dynamic_cast<NodeGraphics*>(item))
        {
            emit nodeConfigurationRequested(nodeItem->getModelNode());
            event->accept();
            return;
        }
    }

    if(deleteMode && event->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());

        if (NodeGraphics* nodeItem = dynamic_cast<NodeGraphics*>(item))
        {
            removeNode(nodeItem);
            event->accept();
            return;
        }

        if (LineConnectionGraphics* lineItem = dynamic_cast<LineConnectionGraphics*>(item))
        {
            removeConnection(lineItem->getConnection());
            event->accept();
            return;
        }
    }

    if(connectionMode && event->button() == Qt::RightButton)
    {
        cancelConnectionDrawing();
        event->accept();
        return;
    }

    if(connectionMode && event->button() == Qt::LeftButton)
    {
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        NodeGraphics* nodeItem = dynamic_cast<NodeGraphics*>(item);

        if (!connectionDrawingActive)
        {
            if (!nodeItem) return;
            startConnectionDrawing(nodeItem, event->scenePos());
            event->accept();
            return;
        }

        if (nodeItem) finishConnectionDrawing(nodeItem);
        else addDraftConnectionPoint(event->scenePos());

        event->accept();
        return;
    }

    if(modelsAddMode && event->button() == Qt::LeftButton)
    {
        const QPointF nodePos = GridSettings::snapToGrid(event->scenePos());
        if (!canPlaceNodeAt(nodePos)) return;

        Node* node = new Node(selectedParentNode);
        node->setConfiguration(defaultNodeConfiguration);

        if (selectedParentNode) selectedParentNode->addChild(node);
        else addRootNode(node);

        NodeGraphics* graphics = addNodeToScene(node);
        if (graphics) graphics->setPos(nodePos);
        event->accept();
        return;
    }

    QGraphicsScene::mousePressEvent(event);
}

void ReliabilityScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (connectionMode && connectionDrawingActive)
    {
        lastCursorScenePos = event->scenePos();
        tempConnectionPath->setPath(calculateRubberBandPath());
        event->accept();
        return;
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void ReliabilityScene::addRootNode(Node* node)
{
    if (node) rootNodes.append(node);
}

NodeGraphics* ReliabilityScene::addNodeToScene(Node* node)
{
    if (!node) return nullptr;

    NodeGraphics* nodeGraphics = createNodeGraphics(node);

    if (!nodeGraphics->scene())
        addItem(nodeGraphics);

    return nodeGraphics;
}

LineConnectionGraphics* ReliabilityScene::addConnectionToScene(LineConnection* connection)
{
    if (!connection) return nullptr;

    LineConnectionGraphics* connectionGraphics = createConnectionGraphics(connection, {});
    if (!connectionGraphics) return nullptr;

    if (!connectionGraphics->scene())
        addItem(connectionGraphics);

    connectionGraphics->updateGeometry();
    return connectionGraphics;
}

NodeGraphics* ReliabilityScene::createNodeGraphics(Node* node)
{
    if (!node) return nullptr;

    NodeGraphics* nodeGraphics = nodeGraphicsByNode.value(node, nullptr);
    if (nodeGraphics) return nodeGraphics;

    nodeGraphics = new NodeGraphics(node);
    nodeGraphicsByNode.insert(node, nodeGraphics);
    connect(nodeGraphics, &NodeGraphics::nodeDoubleClicked, this, &ReliabilityScene::onNodeDoubleClicked);
    return nodeGraphics;
}

LineConnectionGraphics* ReliabilityScene::createConnectionGraphics(LineConnection* connection, const QList<QPointF>& bendPoints)
{
    if (!connection) return nullptr;

    LineConnectionGraphics* connectionGraphics = connectionGraphicsByConnection.value(connection, nullptr);
    if (connectionGraphics) return connectionGraphics;

    NodeGraphics* first = nodeGraphicsByNode.value(connection->getSource(), nullptr);
    NodeGraphics* second = nodeGraphicsByNode.value(connection->getTarget(), nullptr);
    if (!first || !second) return nullptr;

    connectionGraphics = new LineConnectionGraphics(connection, first, second, bendPoints);
    connectionGraphicsByConnection.insert(connection, connectionGraphics);
    connect(first, &NodeGraphics::positionChanged, connectionGraphics, &LineConnectionGraphics::updateGeometry);
    connect(second, &NodeGraphics::positionChanged, connectionGraphics, &LineConnectionGraphics::updateGeometry);
    return connectionGraphics;
}

void ReliabilityScene::clearSceneItems()
{
    const QList<QGraphicsItem*> itemsCopy = items();
    for (QGraphicsItem* item : itemsCopy)
    {
        if (item == tempConnectionPath) continue;
        if (!dynamic_cast<NodeGraphics*>(item) && !dynamic_cast<LineConnectionGraphics*>(item)) continue;
        removeItem(item);
    }
}

void ReliabilityScene::deleteAllEditorObjects()
{
    cancelConnectionDrawing();

    const auto connectionGraphics = connectionGraphicsByConnection.values();
    for (LineConnectionGraphics* graphics : connectionGraphics)
    {
        if (!graphics) continue;
        if (graphics->scene()) removeItem(graphics);
        delete graphics;
    }
    connectionGraphicsByConnection.clear();

    qDeleteAll(connections);
    connections.clear();

    const auto nodeGraphics = nodeGraphicsByNode.values();
    for (NodeGraphics* graphics : nodeGraphics)
    {
        if (!graphics) continue;
        if (graphics->scene()) removeItem(graphics);
        delete graphics;
    }
    nodeGraphicsByNode.clear();

    qDeleteAll(rootNodes);
    rootNodes.clear();
    selectedParentNode = nullptr;
    topLevelStructureType = StructureType::Series;
    topLevelRequiredElements = 1;
}

void ReliabilityScene::refreshCurrentLevel()
{
    clearSceneItems();

    const QList<Node*> nodes = selectedParentNode ? selectedParentNode->getChildren() : rootNodes;
    for(Node* node : nodes)
        addNodeToScene(node);

    for (LineConnection* connection : connections)
    {
        if (isNodeInCurrentLevel(connection->getSource()) && isNodeInCurrentLevel(connection->getTarget()))
            addConnectionToScene(connection);
    }
}

void ReliabilityScene::createTempConnectionPath()
{
    tempConnectionPath = new QGraphicsPathItem;

    tempConnectionPath->setZValue(-1.0);
    tempConnectionPath->setPen(QPen(Qt::blue, 2, Qt::DashLine));
    tempConnectionPath->setVisible(false);

    addItem(tempConnectionPath);
}

void ReliabilityScene::createConnection(NodeGraphics* first, NodeGraphics* second, const QList<QPointF>& bendPoints)
{
    if (!first || !second || first == second) return;

    Node* firstNode = first->getModelNode();
    Node* secondNode = second->getModelNode();
    if (!firstNode || !secondNode) return;
    if (!isNodeInCurrentLevel(firstNode) || !isNodeInCurrentLevel(secondNode)) return;

    for (LineConnection* connection : connections)
    {
        const bool sameDirection = connection->getSource() == firstNode && connection->getTarget() == secondNode;
        const bool reverseDirection = connection->getSource() == secondNode && connection->getTarget() == firstNode;
        if (sameDirection || reverseDirection) return;
    }

    LineConnection* connection = new LineConnection;
    connection->setSource(firstNode);
    connection->setTarget(secondNode);
    connections.append(connection);

    if (LineConnectionGraphics* connectionGraphics = createConnectionGraphics(connection, bendPoints))
        addItem(connectionGraphics);
}

void ReliabilityScene::removeConnection(LineConnection* connection)
{
    if (!connection) return;

    if (LineConnectionGraphics* graphics = connectionGraphicsByConnection.take(connection))
    {
        removeItem(graphics);
        delete graphics;
    }

    connections.removeOne(connection);
    delete connection;
}

void ReliabilityScene::removeConnectionsForNode(Node* node)
{
    if (!node) return;

    QList<LineConnection*> toDelete;
    for (LineConnection* connection : connections)
    {
        if (nodeContains(node, connection->getSource()) || nodeContains(node, connection->getTarget()))
            toDelete.append(connection);
    }

    for (LineConnection* connection : toDelete)
        removeConnection(connection);
}

void ReliabilityScene::removeNode(NodeGraphics* nodeGraphics)
{
    if (!nodeGraphics) return;

    Node* node = nodeGraphics->getModelNode();
    if (!node) return;

    removeConnectionsForNode(node);
    if (node->getParent()) node->getParent()->removeChild(node);
    else rootNodes.removeOne(node);

    emitNodeAboutToBeRemovedForSubtree(node);
    removeNodeGraphicsForSubtree(node);
    delete node;
}

void ReliabilityScene::emitNodeAboutToBeRemovedForSubtree(Node* node)
{
    if (!node) return;

    emit nodeAboutToBeRemoved(node);
    for (Node* child : node->getChildren())
        emitNodeAboutToBeRemovedForSubtree(child);
}

void ReliabilityScene::removeNodeGraphicsForSubtree(Node* node)
{
    if (!node) return;

    for (Node* child : node->getChildren())
        removeNodeGraphicsForSubtree(child);

    if (NodeGraphics* graphics = nodeGraphicsByNode.take(node))
    {
        removeItem(graphics);
        delete graphics;
    }
}

void ReliabilityScene::startConnectionDrawing(NodeGraphics* nodeGraphics, const QPointF& cursorPos)
{
    if (!nodeGraphics) return;

    connectionStartNodeGraphics = nodeGraphics;
    connectionDrawingActive = true;
    draftConnectionPoints.clear();
    draftConnectionPoints.append(nodeGraphics->pos());
    lastCursorScenePos = cursorPos;
    tempConnectionPath->setPath(calculateRubberBandPath());
    tempConnectionPath->setVisible(true);
}

void ReliabilityScene::finishConnectionDrawing(NodeGraphics* nodeGraphics)
{
    if (nodeGraphics && nodeGraphics != connectionStartNodeGraphics)
    {
        QList<QPointF> bendPoints = draftConnectionPoints;
        const QPointF corner = calculateOrthogonalPoint(bendPoints.last(), nodeGraphics->pos());
        if (corner != bendPoints.last() && corner != nodeGraphics->pos())
            bendPoints.append(corner);
        if (!bendPoints.isEmpty())
            bendPoints.removeFirst();

        createConnection(connectionStartNodeGraphics, nodeGraphics, bendPoints);
    }

    cancelConnectionDrawing();
}

void ReliabilityScene::addDraftConnectionPoint(const QPointF& scenePos)
{
    if (draftConnectionPoints.isEmpty()) return;

    const QPointF nextPoint = calculateOrthogonalPoint(draftConnectionPoints.last(), scenePos);
    if (nextPoint != draftConnectionPoints.last())
        draftConnectionPoints.append(nextPoint);

    lastCursorScenePos = scenePos;
    tempConnectionPath->setPath(calculateRubberBandPath());
}

void ReliabilityScene::cancelConnectionDrawing()
{
    connectionDrawingActive = false;
    connectionStartNodeGraphics = nullptr;
    draftConnectionPoints.clear();
    tempConnectionPath->setVisible(false);
    tempConnectionPath->setPath(QPainterPath());
}

void ReliabilityScene::resetEditorModes()
{
    modelsAddMode = false;
    connectionMode = false;
    deleteMode = false;
    configurationMode = false;
    cancelConnectionDrawing();
}

QPointF ReliabilityScene::calculateOrthogonalPoint(const QPointF& from, const QPointF& target) const
{
    const QPointF snappedTarget = GridSettings::snapToGrid(target);
    const qreal dx = snappedTarget.x() - from.x();
    const qreal dy = snappedTarget.y() - from.y();

    if (qAbs(dx) >= qAbs(dy))
        return QPointF(snappedTarget.x(), from.y());
    return QPointF(from.x(), snappedTarget.y());
}

QPainterPath ReliabilityScene::calculateRubberBandPath() const
{
    QPainterPath path;
    if (draftConnectionPoints.isEmpty()) return path;

    path.moveTo(draftConnectionPoints.first());
    for (int i = 1; i < draftConnectionPoints.size(); ++i)
        path.lineTo(draftConnectionPoints[i]);

    path.lineTo(calculateOrthogonalPoint(draftConnectionPoints.last(), lastCursorScenePos));
    return path;
}

bool ReliabilityScene::canPlaceNodeAt(const QPointF& scenePos) const
{
    const QRectF newNodeRect = NodeGraphics::defaultBoundingRect().translated(scenePos);

    for (QGraphicsItem* item : items(newNodeRect))
    {
        NodeGraphics* nodeGraphics = dynamic_cast<NodeGraphics*>(item);
        if (!nodeGraphics) continue;

        return false;
    }

    return true;
}

bool ReliabilityScene::isNodeInCurrentLevel(Node* node) const
{
    if (!node) return false;
    return node->getParent() == selectedParentNode;
}

bool ReliabilityScene::nodeContains(Node* root, Node* candidate) const
{
    if (!root || !candidate) return false;
    if (root == candidate) return true;

    for (Node* child : root->getChildren())
    {
        if (nodeContains(child, candidate))
            return true;
    }

    return false;
}

void ReliabilityScene::appendNodeToSchemaModel(Node* node, SchemaModel& model, int depth) const
{
    if (!node) return;

    model.nodes.append(createNodeData(node, depth));

    for (Node* child : node->getChildren())
        appendNodeToSchemaModel(child, model, depth + 1);
}

SchemaNodeData ReliabilityScene::createNodeData(Node* node, int depth) const
{
    SchemaNodeData nodeData;

    nodeData.id = node->getId();
    nodeData.parentId = node->getParent() ? node->getParent()->getId() : 0;
    nodeData.depth = depth;
    nodeData.name = node->getName();
    nodeData.groupName = node->getGroupName();
    nodeData.nodeKind = node->getNodeKind();
    nodeData.failureRates = node->getFailureRates();
    nodeData.structureType = node->getStructureType();
    nodeData.requiredElements = node->getRequiredElements();

    return nodeData;
}

SchemaConnectionData ReliabilityScene::createConnectionData(LineConnection* connection) const
{
    SchemaConnectionData connectionData;
    if (!connection || !connection->getSource() || !connection->getTarget())
        return connectionData;

    connectionData.sourceNodeId = connection->getSource()->getId();
    connectionData.targetNodeId = connection->getTarget()->getId();
    return connectionData;
}

void ReliabilityScene::appendNodeToEditorSchemaModel(Node* node, EditorSchemaModel& model) const
{
    if (!node) return;

    EditorSchemaNodeData nodeData;
    nodeData.id = node->getId();
    nodeData.parentId = node->getParent() ? node->getParent()->getId() : 0;
    nodeData.configuration = node->getConfiguration();

    if (const NodeGraphics* graphics = nodeGraphicsByNode.value(node, nullptr))
        nodeData.position = graphics->pos();

    model.nodes.append(nodeData);

    for (Node* child : node->getChildren())
        appendNodeToEditorSchemaModel(child, model);
}
