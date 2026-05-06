#include "schemaserializer.h"

QJsonObject SchemaSerializer::pointToJson(const QPointF& point)
{
    QJsonObject object;
    object["x"] = point.x();
    object["y"] = point.y();
    return object;
}

QPointF SchemaSerializer::pointFromJson(const QJsonObject& object)
{
    return QPointF(object["x"].toDouble(), object["y"].toDouble());
}

QJsonDocument SchemaSerializer::toJson(const EditorSchemaModel& model)
{
    QJsonObject root;
    root["version"] = 1;
    root["currentParentId"] = model.currentParentId;
    root["defaultNodeConfiguration"] = configurationToJson(model.defaultNodeConfiguration);
    root["topLevelStructureType"] = structureTypeToString(model.topLevelStructureType);
    root["topLevelRequiredElements"] = model.topLevelRequiredElements;

    QJsonArray cyclogram;
    for (const CyclogramStage& stage : model.cyclogram)
    {
        QJsonObject object;
        object["name"] = stage.name;
        object["mode"] = operationModeToString(stage.mode);
        object["duration"] = stage.duration;
        cyclogram.append(object);
    }
    root["cyclogram"] = cyclogram;

    QJsonArray initialProbabilities;
    for (double probability : model.initialProbabilities)
        initialProbabilities.append(probability);
    root["initialProbabilities"] = initialProbabilities;

    QJsonArray nodes;
    for (const EditorSchemaNodeData& node : model.nodes)
    {
        QJsonObject object;
        object["id"] = node.id;
        object["parentId"] = node.parentId;
        object["position"] = pointToJson(node.position);
        object["configuration"] = configurationToJson(node.configuration);
        nodes.append(object);
    }
    root["nodes"] = nodes;

    QJsonArray connections;
    for (const EditorSchemaConnectionData& connection : model.connections)
    {
        QJsonObject object;
        object["sourceNodeId"] = connection.sourceNodeId;
        object["targetNodeId"] = connection.targetNodeId;

        QJsonArray bendPoints;
        for (const QPointF& point : connection.bendPoints)
            bendPoints.append(pointToJson(point));
        object["bendPoints"] = bendPoints;

        connections.append(object);
    }
    root["connections"] = connections;

    return QJsonDocument(root);
}

bool SchemaSerializer::fromJson(const QJsonDocument& document, EditorSchemaModel& model, QString& errorMessage)
{
    if (!document.isObject())
    {
        errorMessage = "Файл схемы должен содержать JSON-объект.";
        return false;
    }

    const QJsonObject root = document.object();
    EditorSchemaModel loadedModel;
    loadedModel.currentParentId = root["currentParentId"].toInt();

    StructureType topLevelStructureType{StructureType::Series};
    const QString topLevelStructureValue = root.contains("topLevelStructureType") ? root["topLevelStructureType"].toString("series") : root["rootStructureType"].toString("series");
    if (!structureTypeFromString(topLevelStructureValue, topLevelStructureType))
    {
        errorMessage = "В файле найден неизвестный тип корневой структуры.";
        return false;
    }
    loadedModel.topLevelStructureType = topLevelStructureType;
    loadedModel.topLevelRequiredElements = root.contains("topLevelRequiredElements") ? root["topLevelRequiredElements"].toInt(1) : root["rootRequiredElements"].toInt(1);
    if (loadedModel.topLevelRequiredElements < 1)
        loadedModel.topLevelRequiredElements = 1;

    const QJsonArray cyclogram = root["cyclogram"].toArray();
    for (const QJsonValue& value : cyclogram)
    {
        const QJsonObject object = value.toObject();
        CyclogramStage stage;
        stage.name = object["name"].toString();
        if (stage.name.trimmed().isEmpty())
            stage.name = QString("Этап %1").arg(loadedModel.cyclogram.size() + 1);

        OperationMode mode{OperationMode::Functioning};
        if (!operationModeFromString(object["mode"].toString("functioning"), mode))
        {
            errorMessage = "В файле найден неизвестный режим этапа циклограммы.";
            return false;
        }
        stage.mode = mode;
        stage.duration = object["duration"].toDouble(1.0);
        if (stage.duration < 0.0)
            stage.duration = 0.0;

        loadedModel.cyclogram.append(stage);
    }

    const QJsonArray initialProbabilities = root["initialProbabilities"].toArray();
    for (const QJsonValue& value : initialProbabilities)
        loadedModel.initialProbabilities.append(value.toDouble());

    if (root.contains("defaultNodeConfiguration"))
    {
        if (!configurationFromJson(root["defaultNodeConfiguration"].toObject(), loadedModel.defaultNodeConfiguration, errorMessage)) return false;
    }

    const QJsonArray nodes = root["nodes"].toArray();
    for (const QJsonValue& value : nodes)
    {
        const QJsonObject object = value.toObject();
        EditorSchemaNodeData node;
        node.id = object["id"].toInt();
        node.parentId = object["parentId"].toInt();
        node.position = pointFromJson(object["position"].toObject());

        if (!configurationFromJson(object["configuration"].toObject(), node.configuration, errorMessage))
            return false;

        if (node.id < 1)
        {
            errorMessage = "В файле найден узел без корректного идентификатора.";
            return false;
        }

        loadedModel.nodes.append(node);
    }

    const QJsonArray connections = root["connections"].toArray();
    for (const QJsonValue& value : connections)
    {
        const QJsonObject object = value.toObject();
        EditorSchemaConnectionData connection;
        connection.sourceNodeId = object["sourceNodeId"].toInt();
        connection.targetNodeId = object["targetNodeId"].toInt();

        const QJsonArray bendPoints = object["bendPoints"].toArray();
        for (const QJsonValue& pointValue : bendPoints)
            connection.bendPoints.append(pointFromJson(pointValue.toObject()));

        if (connection.sourceNodeId < 1 || connection.targetNodeId < 1)
        {
            errorMessage = "В файле найдена связь без корректных идентификаторов узлов.";
            return false;
        }

        loadedModel.connections.append(connection);
    }

    model = loadedModel;
    return true;
}

QJsonObject SchemaSerializer::configurationToJson(const NodeConfiguration& configuration)
{
    QJsonObject object;
    object["name"] = configuration.name;
    object["groupName"] = configuration.groupName;
    object["nodeKind"] = nodeKindToString(configuration.nodeKind);
    object["structureType"] = structureTypeToString(configuration.structureType);
    object["requiredElements"] = configuration.requiredElements;

    QJsonObject lambdaDefinitions;
    lambdaDefinitions["transportation"] = lambdaDefinitionToJson(configuration.lambdaDefinitions[static_cast<int>(OperationMode::Transportation)]);
    lambdaDefinitions["storage"] = lambdaDefinitionToJson(configuration.lambdaDefinitions[static_cast<int>(OperationMode::Storage)]);
    lambdaDefinitions["functioning"] = lambdaDefinitionToJson(configuration.lambdaDefinitions[static_cast<int>(OperationMode::Functioning)]);
    lambdaDefinitions["off"] = lambdaDefinitionToJson(configuration.lambdaDefinitions[static_cast<int>(OperationMode::Off)]);
    object["lambdaDefinitions"] = lambdaDefinitions;

    return object;
}

bool SchemaSerializer::configurationFromJson(const QJsonObject& object, NodeConfiguration& configuration, QString& errorMessage)
{
    configuration.name = object["name"].toString("Узел").trimmed();
    if (configuration.name.isEmpty())
        configuration.name = "Узел";
    configuration.groupName = object["groupName"].toString().trimmed();

    NodeKind nodeKind{NodeKind::Normal};
    if (!nodeKindFromString(object["nodeKind"].toString("normal"), nodeKind))
    {
        errorMessage = "В файле найден неизвестный тип узла.";
        return false;
    }
    configuration.nodeKind = nodeKind;

    StructureType type{StructureType::Element};
    if (!structureTypeFromString(object["structureType"].toString("element"), type))
    {
        errorMessage = "В файле найден неизвестный тип структуры узла.";
        return false;
    }
    configuration.structureType = type;

    configuration.requiredElements = object["requiredElements"].toInt(1);
    if (configuration.requiredElements < 1)
        configuration.requiredElements = 1;

    configuration.lambdaDefinitions = defaultLambdaDefinitions();
    if (object.contains("lambdaDefinitions"))
    {
        const QJsonObject lambdaDefinitions = object["lambdaDefinitions"].toObject();
        const QList<QPair<QString, OperationMode>> fields{{"transportation", OperationMode::Transportation}, {"storage", OperationMode::Storage}, {"functioning", OperationMode::Functioning}, {"off", OperationMode::Off}};

        for (const auto& field : fields)
        {
            LambdaDefinition definition;
            if (!lambdaDefinitionFromJson(lambdaDefinitions[field.first].toObject(), definition, errorMessage))
                return false;
            configuration.lambdaDefinitions[static_cast<int>(field.second)] = definition;
        }
    }
    else
    {
        const QJsonArray rates = object["failureRates"].toArray();
        FailureRates failureRates{};
        for (int i = 0; i < rates.size() && i < static_cast<int>(failureRates.size()); ++i)
            failureRates[i] = rates[i].toDouble();

        LambdaDefinitions definitions = defaultLambdaDefinitions();
        for (int i = 0; i < static_cast<int>(failureRates.size()); ++i)
        {
            definitions[i].inputType = LambdaInputType::Fixed;
            definitions[i].value = failureRates[i];
        }
        configuration.lambdaDefinitions = definitions;
    }

    if (configuration.nodeKind != NodeKind::Normal)
    {
        configuration.groupName.clear();
        configuration.lambdaDefinitions = LambdaDefinitions{};
        configuration.structureType = StructureType::Element;
        configuration.requiredElements = 1;
    }

    return true;
}

QJsonObject SchemaSerializer::lambdaDefinitionToJson(const LambdaDefinition& definition)
{
    QJsonObject object;
    object["inputType"] = lambdaInputTypeToString(definition.inputType);
    object["value"] = definition.value;
    object["multiplier"] = definition.multiplier;
    object["referenceMode"] = operationModeToString(definition.referenceMode);
    return object;
}

bool SchemaSerializer::lambdaDefinitionFromJson(const QJsonObject& object, LambdaDefinition& definition, QString& errorMessage)
{
    LambdaInputType inputType{LambdaInputType::Fixed};
    if (!lambdaInputTypeFromString(object["inputType"].toString("fixed"), inputType))
    {
        errorMessage = "В файле найден неизвестный способ задания λ.";
        return false;
    }

    OperationMode referenceMode{OperationMode::Functioning};
    if (!operationModeFromString(object["referenceMode"].toString("functioning"), referenceMode))
    {
        errorMessage = "В файле найден неизвестный режим λ.";
        return false;
    }

    definition.inputType = inputType;
    definition.value = object["value"].toDouble();
    definition.multiplier = object["multiplier"].toDouble(1.0);
    definition.referenceMode = referenceMode;
    return true;
}

QString SchemaSerializer::lambdaInputTypeToString(LambdaInputType type)
{
    switch (type)
    {
    case LambdaInputType::Expression:
        return "expression";
    case LambdaInputType::Fixed:
        return "fixed";
    }

    return "fixed";
}

bool SchemaSerializer::lambdaInputTypeFromString(const QString& value, LambdaInputType& type)
{
    if (value == "fixed")
    {
        type = LambdaInputType::Fixed;
        return true;
    }
    if (value == "expression")
    {
        type = LambdaInputType::Expression;
        return true;
    }

    return false;
}

QString SchemaSerializer::operationModeToString(OperationMode mode)
{
    switch (mode)
    {
    case OperationMode::Transportation:
        return "transportation";
    case OperationMode::Storage:
        return "storage";
    case OperationMode::Functioning:
        return "functioning";
    case OperationMode::Off:
        return "off";
    }

    return "functioning";
}

bool SchemaSerializer::operationModeFromString(const QString& value, OperationMode& mode)
{
    if (value == "transportation")
    {
        mode = OperationMode::Transportation;
        return true;
    }
    if (value == "storage")
    {
        mode = OperationMode::Storage;
        return true;
    }
    if (value == "functioning")
    {
        mode = OperationMode::Functioning;
        return true;
    }
    if (value == "off")
    {
        mode = OperationMode::Off;
        return true;
    }

    return false;
}

QString SchemaSerializer::nodeKindToString(NodeKind kind)
{
    switch (kind)
    {
    case NodeKind::Input:
        return "input";
    case NodeKind::Output:
        return "output";
    case NodeKind::Normal:
        return "normal";
    }

    return "normal";
}

bool SchemaSerializer::nodeKindFromString(const QString& value, NodeKind& kind)
{
    if (value == "normal")
    {
        kind = NodeKind::Normal;
        return true;
    }
    if (value == "input")
    {
        kind = NodeKind::Input;
        return true;
    }
    if (value == "output")
    {
        kind = NodeKind::Output;
        return true;
    }

    return false;
}

QString SchemaSerializer::structureTypeToString(StructureType type)
{
    switch (type)
    {
    case StructureType::Series:
        return "series";
    case StructureType::Parallel:
        return "parallel";
    case StructureType::KOutOfN:
        return "kOutOfN";
    case StructureType::Element:
        return "element";
    }

    return "element";
}

bool SchemaSerializer::structureTypeFromString(const QString& value, StructureType& type)
{
    if (value == "element")
    {
        type = StructureType::Element;
        return true;
    }
    if (value == "series")
    {
        type = StructureType::Series;
        return true;
    }
    if (value == "parallel")
    {
        type = StructureType::Parallel;
        return true;
    }
    if (value == "kOutOfN")
    {
        type = StructureType::KOutOfN;
        return true;
    }

    return false;
}
