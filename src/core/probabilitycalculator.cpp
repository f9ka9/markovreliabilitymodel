#include "probabilitycalculator.h"


ReliabilityMatrix ProbabilityCalculator::buildTransitionMatrix(const ReliabilityMatrix& intensityMatrix,  double duration) const
{
    const int size = intensityMatrix.size();
    if (size == 0) return {};
    if (duration <= 0.0) return identityMatrix(size);

    const double rate = maxExitRate(intensityMatrix);
    if (qFuzzyIsNull(rate)) return identityMatrix(size);

    ReliabilityMatrix embedded = identityMatrix(size);
    for (int row = 0; row < size; ++row)
    {
        for (int column = 0; column < size; ++column)
            embedded[row][column] += intensityMatrix[row][column] / rate;
    }

    ReliabilityMatrix result(size);
    for (QVector<double>& row : result)
        row = QVector<double>(size, 0.0);

    ReliabilityMatrix power = identityMatrix(size);
    const double mean = rate * duration;
    double poissonWeight = qExp(-mean);
    addScaledMatrix(result, power, poissonWeight);

    constexpr int maxIterations = 10000;
    constexpr double tolerance = 1.0e-14;

    for (int iteration = 1; iteration <= maxIterations; ++iteration)
    {
        power = multiplyMatrices(power, embedded);
        poissonWeight *= mean / iteration;
        addScaledMatrix(result, power, poissonWeight);

        if (poissonWeight < tolerance)
            break;
    }

    return result;
}

ProbabilityVector ProbabilityCalculator::initialProbabilities(int stateCount) const
{
    ProbabilityVector probabilities(stateCount, 0.0);
    if (!probabilities.isEmpty())
        probabilities[0] = 1.0;
    return probabilities;
}

ProbabilityVector ProbabilityCalculator::normalizedInitialProbabilities(const ProbabilityVector& probabilities, int stateCount) const
{
    if (probabilities.size() != stateCount)
        return initialProbabilities(stateCount);

    double sum = 0.0;
    for (double value : probabilities)
        sum += value;

    if (qFuzzyIsNull(sum))
        return initialProbabilities(stateCount);

    ProbabilityVector normalized = probabilities;
    for (double& value : normalized)
        value /= sum;
    return normalized;
}

ProbabilityVector ProbabilityCalculator::multiply(const ProbabilityVector& probabilities, const ReliabilityMatrix& matrix) const
{
    ProbabilityVector result(matrix.size(), 0.0);
    for (int column = 0; column < matrix.size(); ++column)
    {
        double value = 0.0;
        for (int row = 0; row < probabilities.size() && row < matrix.size(); ++row)
            value += probabilities[row] * matrix[row][column];
        result[column] = value;
    }
    return result;
}

ReliabilityMatrix ProbabilityCalculator::identityMatrix(int size) const
{
    ReliabilityMatrix matrix(size);
    for (int row = 0; row < size; ++row)
    {
        matrix[row] = QVector<double>(size, 0.0);
        matrix[row][row] = 1.0;
    }
    return matrix;
}

ReliabilityMatrix ProbabilityCalculator::multiplyMatrices(const ReliabilityMatrix& left, const ReliabilityMatrix& right) const
{
    const int size = left.size();
    ReliabilityMatrix result(size);
    for (QVector<double>& row : result)
        row = QVector<double>(size, 0.0);

    for (int row = 0; row < size; ++row)
    {
        for (int column = 0; column < size; ++column)
        {
            double value = 0.0;
            for (int inner = 0; inner < size; ++inner)
                value += left[row][inner] * right[inner][column];
            result[row][column] = value;
        }
    }

    return result;
}

void ProbabilityCalculator::addScaledMatrix(ReliabilityMatrix& target, const ReliabilityMatrix& source, double scale) const
{
    for (int row = 0; row < target.size(); ++row)
    {
        for (int column = 0; column < target[row].size(); ++column)
            target[row][column] += source[row][column] * scale;
    }
}

double ProbabilityCalculator::maxExitRate(const ReliabilityMatrix& intensityMatrix) const
{
    double rate = 0.0;
    for (int row = 0; row < intensityMatrix.size(); ++row)
        rate = qMax(rate, -intensityMatrix[row][row]);
    return rate;
}
