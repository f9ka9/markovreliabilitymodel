#ifndef MATRIXBUILDER_H
#define MATRIXBUILDER_H

#include <QList>
#include <QVector>

#include "schemamodel.h"
#include "transitionbuilder.h"

using ReliabilityMatrix = QVector<QVector<double>>;

class MatrixBuilder
{
public:
    ReliabilityMatrix buildIntensityMatrix(const QList<ReliabilityState>& states, const QList<ReliabilityTransition>& transitions, const QList<SchemaNodeData>& nodes, OperationMode mode) const;

private:
    ReliabilityMatrix createZeroMatrix(int size) const;
    double failureRateForNode(const QList<SchemaNodeData>& nodes, int nodeId, OperationMode mode) const;
};

#endif // MATRIXBUILDER_H
