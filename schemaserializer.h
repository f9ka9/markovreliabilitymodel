#ifndef SCHEMASERIALIZER_H
#define SCHEMASERIALIZER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QPair>

#include "editorschemamodel.h"

class SchemaSerializer
{
public:
    static QJsonDocument toJson(const EditorSchemaModel& model);
    static bool fromJson(const QJsonDocument& document, EditorSchemaModel& model, QString& errorMessage);

private:
    static QJsonObject configurationToJson(const NodeConfiguration& configuration);
    static bool configurationFromJson(const QJsonObject& object, NodeConfiguration& configuration, QString& errorMessage);
    static QJsonObject lambdaDefinitionToJson(const LambdaDefinition& definition);
    static bool lambdaDefinitionFromJson(const QJsonObject& object, LambdaDefinition& definition, QString& errorMessage);
    static QString lambdaInputTypeToString(LambdaInputType type);
    static bool lambdaInputTypeFromString(const QString& value, LambdaInputType& type);
    static QString operationModeToString(OperationMode mode);
    static bool operationModeFromString(const QString& value, OperationMode& mode);
    static QString structureTypeToString(StructureType type);
    static bool structureTypeFromString(const QString& value, StructureType& type);
};

#endif // SCHEMASERIALIZER_H
