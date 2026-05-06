#ifndef PROBABILITYCALCULATOR_H
#define PROBABILITYCALCULATOR_H

#include <QVector>
#include <QtMath>

#include "matrixbuilder.h"

using ProbabilityVector = QVector<double>;

class ProbabilityCalculator
{
public:
    ReliabilityMatrix buildTransitionMatrix(const ReliabilityMatrix& intensityMatrix, double duration) const;
    ProbabilityVector initialProbabilities(int stateCount) const;
    ProbabilityVector normalizedInitialProbabilities(const ProbabilityVector& probabilities, int stateCount) const;
    ProbabilityVector multiply(const ProbabilityVector& probabilities, const ReliabilityMatrix& matrix) const;

private:
    ReliabilityMatrix identityMatrix(int size) const;
    ReliabilityMatrix multiplyMatrices(const ReliabilityMatrix& left, const ReliabilityMatrix& right) const;
    void addScaledMatrix(ReliabilityMatrix& target, const ReliabilityMatrix& source, double scale) const;
    double maxExitRate(const ReliabilityMatrix& intensityMatrix) const;
};

#endif // PROBABILITYCALCULATOR_H
