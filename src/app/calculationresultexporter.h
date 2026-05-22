#ifndef CALCULATIONRESULTEXPORTER_H
#define CALCULATIONRESULTEXPORTER_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QString>

#include "calculationresult.h"

class CalculationResultExporter
{
public:
    static QJsonDocument toJson(const CalculationResult& result);

private:
    static QJsonArray matrixToJson(const ReliabilityMatrix& matrix);
    static QJsonArray vectorToJson(const ProbabilityVector& vector);
    static QString operationModeName(OperationMode mode);
    static QString resultSystemStateName(ReliabilitySystemState state);
};

#endif // CALCULATIONRESULTEXPORTER_H
