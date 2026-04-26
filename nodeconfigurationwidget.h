#ifndef NODECONFIGURATIONWIDGET_H
#define NODECONFIGURATIONWIDGET_H

#include <QWidget>

#include "node.h"

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;

class NodeConfigurationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NodeConfigurationWidget(QWidget* parent = nullptr);

    void editDefaultConfiguration(const QString& name, const QString& groupName, const FailureRates& failureRates, Node::StructureType structureType, int requiredElements);
    void editSelectedNode(int id, const QString& name, const QString& groupName, const FailureRates& failureRates, Node::StructureType structureType, int requiredElements);

signals:
    void configurationApplied(const QString& name, const QString& groupName, const FailureRates& failureRates, Node::StructureType structureType, int requiredElements);

private:
    FailureRates failureRatesFromFields() const;
    void setConfiguration(const QString& mode, const QString& idText, const QString& name, const QString& groupName, const FailureRates& failureRates, Node::StructureType structureType, int requiredElements);
    void updateFieldsAvailability();

    QLabel* modeLabel{nullptr};
    QLineEdit* idEdit{nullptr};
    QLineEdit* nameEdit{nullptr};
    QLineEdit* groupEdit{nullptr};
    std::array<QDoubleSpinBox*, 4> failureRateSpinBoxes{};
    QComboBox* structureTypeComboBox{nullptr};
    QSpinBox* requiredElementsSpinBox{nullptr};
    QPushButton* applyButton{nullptr};
};

#endif // NODECONFIGURATIONWIDGET_H
