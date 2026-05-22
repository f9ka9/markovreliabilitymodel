#include "calculationservice.h"

#include <cmath>

#include "compositereliabilitycalculator.h"
#include "reliabilitycore.h"
#include "stategenerator.h"

CalculationService::CalculationService(int maxStateCountForCalculation):
    maxStateCountForCalculation(maxStateCountForCalculation)
{}

CalculationPreparation CalculationService::prepare(const EditorSchemaModel& editorModel, const Cyclogram& cyclogram) const
{
    CalculationPreparation preparation;
    preparation.cyclogram = cyclogram;

    CompositeReliabilityCalculator compositeCalculator;
    preparation.model = compositeCalculator.buildEffectiveModel(editorModel, editorModel.currentParentId);

    if (!validateModelAndCyclogram(preparation.model, preparation.cyclogram, preparation.stateCount, preparation.errorMessage))
        return preparation;

    preparation.ok = true;
    return preparation;
}

CalculationRun CalculationService::calculate(const CalculationPreparation& preparation, const ProbabilityVector& initialProbabilities) const
{
    CalculationRun run;
    if (!preparation.ok)
    {
        run.errorMessage = preparation.errorMessage;
        return run;
    }

    if (!validateInitialProbabilities(initialProbabilities, preparation.stateCount, run.errorMessage))
        return run;

    ReliabilityCore core;
    run.result = core.calculateCyclogram(preparation.model, preparation.model.structureType, preparation.model.requiredElements, preparation.cyclogram, initialProbabilities);
    run.ok = true;
    return run;
}

int CalculationService::reliabilityNodeCount(const QList<SchemaNodeData>& nodes)
{
    int count = 0;
    for (const SchemaNodeData& node : nodes)
    {
        if (node.nodeKind == NodeKind::Normal)
            ++count;
    }
    return count;
}

bool CalculationService::validateModelAndCyclogram(const SchemaModel& model, const Cyclogram& cyclogram, int& stateCount, QString& errorMessage) const
{
    if (model.nodes.isEmpty())
    {
        errorMessage = "На текущем уровне нет узлов.";
        return false;
    }

    StateGenerator stateGenerator;
    const int reliableNodes = reliabilityNodeCount(model.nodes);
    stateCount = stateGenerator.generate(model.nodes, model.connections, model.structureType, model.requiredElements, maxStateCountForCalculation).size();
    if (stateCount > maxStateCountForCalculation)
    {
        errorMessage = QString("На текущем уровне слишком много расчетных узлов: %1. Двоичный граф состояний содержит больше %2 состояний. Откройте вложенный блок и рассчитайте меньший уровень.").arg(reliableNodes).arg(maxStateCountForCalculation);
        return false;
    }

    if (model.connections.isEmpty() && model.structureType == StructureType::KOutOfN && model.requiredElements > reliableNodes)
    {
        errorMessage = QString("Для структуры k из n значение k должно быть <= n. Сейчас k=%1, n=%2.").arg(model.requiredElements).arg(reliableNodes);
        return false;
    }

    if (cyclogram.isEmpty())
    {
        errorMessage = "Циклограмма не содержит этапов.";
        return false;
    }

    for (int i = 0; i < cyclogram.size(); ++i)
    {
        if (cyclogram[i].duration <= 0.0 || !std::isfinite(cyclogram[i].duration))
        {
            errorMessage = QString("У этапа %1 некорректная длительность. Длительность должна быть > 0.").arg(i + 1);
            return false;
        }
    }

    for (const SchemaNodeData& node : model.nodes)
    {
        if (node.nodeKind != NodeKind::Normal)
            continue;

        for (double lambda : node.failureRates)
        {
            if (lambda < 0.0 || !std::isfinite(lambda))
            {
                errorMessage = QString("Узел %1 содержит некорректную интенсивность отказа.").arg(node.name.isEmpty() ? QString("#%1").arg(node.id) : node.name);
                return false;
            }
        }
    }

    return true;
}

bool CalculationService::validateInitialProbabilities(const ProbabilityVector& probabilities, int stateCount, QString& errorMessage)
{
    if (probabilities.isEmpty())
        return true;

    if (probabilities.size() != stateCount)
    {
        errorMessage = "Вектор начальных вероятностей имеет неверный размер.";
        return false;
    }

    double sum = 0.0;
    for (double probability : probabilities)
    {
        if (probability < 0.0 || !std::isfinite(probability))
        {
            errorMessage = "Начальные вероятности должны быть конечными и неотрицательными.";
            return false;
        }
        sum += probability;
    }

    if (std::abs(sum - 1.0) > 1.0e-6)
    {
        errorMessage = QString("Сумма начальных вероятностей должна быть равна 1. Сейчас сумма = %1.").arg(sum, 0, 'g', 10);
        return false;
    }

    return true;
}
