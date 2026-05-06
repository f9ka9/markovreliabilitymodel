#ifndef CALCULATIONRESULT_H
#define CALCULATIONRESULT_H

#include <QList>

#include "cyclogram.h"
#include "matrixbuilder.h"
#include "probabilitycalculator.h"
#include "reliabilitystate.h"
#include "transitionbuilder.h"

struct StageCalculationResult
{
    CyclogramStage stage;
    ReliabilityMatrix intensityMatrix;
    ReliabilityMatrix transitionMatrix;
    ProbabilityVector probabilities;
};

struct CalculationResult
{
    QList<SchemaNodeData> nodes;
    QList<ReliabilityState> states;
    QList<ReliabilityTransition> transitions;
    ReliabilityMatrix intensityMatrix;
    ReliabilityMatrix transitionMatrix;
    ProbabilityVector initialProbabilities;
    QList<StageCalculationResult> stages;
};

#endif // CALCULATIONRESULT_H
