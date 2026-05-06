#include "matrixbuilder.h"

ReliabilityMatrix MatrixBuilder::buildIntensityMatrix(const QList<ReliabilityState>& states, const QList<ReliabilityTransition>& transitions, const QList<SchemaNodeData>& nodes, OperationMode mode) const
{
    ReliabilityMatrix matrix = createZeroMatrix(states.size());

    for (const ReliabilityTransition& transition : transitions)
    {
        if (transition.sourceStateId < 0 || transition.sourceStateId >= matrix.size())
            continue;
        if (transition.targetStateId < 0 || transition.targetStateId >= matrix.size())
            continue;

        const double lambda = failureRateForNode(nodes, transition.failedNodeId, mode);
        matrix[transition.sourceStateId][transition.targetStateId] += lambda;
    }

    for (int row = 0; row < matrix.size(); ++row)
    {
        double rowSum = 0.0;
        for (int column = 0; column < matrix[row].size(); ++column)
        {
            if (row == column) continue;
            rowSum += matrix[row][column];
        }
        matrix[row][row] = -rowSum;
    }

    return matrix;
}

ReliabilityMatrix MatrixBuilder::createZeroMatrix(int size) const
{
    ReliabilityMatrix matrix(size);
    for (QVector<double>& row : matrix)
        row = QVector<double>(size, 0.0);
    return matrix;
}

double MatrixBuilder::failureRateForNode(const QList<SchemaNodeData>& nodes, int nodeId, OperationMode mode) const
{
    for (const SchemaNodeData& node : nodes)
    {
        if (node.id != nodeId)
            continue;

        return node.failureRates[static_cast<int>(mode)];
    }

    return 0.0;
}
