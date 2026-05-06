#ifndef NODECONFIGURATIONWIDGET_H
#define NODECONFIGURATIONWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QShortcut>
#include <QSpinBox>
#include <QVBoxLayout>

#include "node.h"

class NodeConfigurationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NodeConfigurationWidget(QWidget* parent = nullptr);

    void editDefaultConfiguration(const NodeConfiguration& configuration);
    void editSelectedNode(Node* node);
    bool isEditingNode(Node* node) const;

signals:
    void configurationApplied(Node* node, const NodeConfiguration& configuration);

private:
    NodeConfiguration configurationFromFields() const;
    LambdaDefinitions lambdaDefinitionsFromFields() const;
    void applyConfiguration();
    void setConfiguration(const QString& mode, const QString& idText, const NodeConfiguration& configuration);
    QWidget* createLambdaEditor(OperationMode mode);
    void fillReferenceModeComboBox(QComboBox* comboBox, OperationMode editedMode);
    void updateFieldsAvailability();
    void updateLambdaFieldsAvailability();

    QLabel* modeLabel{nullptr};
    QLineEdit* idEdit{nullptr};
    QLineEdit* nameEdit{nullptr};
    QLineEdit* groupEdit{nullptr};
    std::array<QComboBox*, 4> lambdaInputTypeComboBoxes{};
    std::array<QDoubleSpinBox*, 4> lambdaValueSpinBoxes{};
    std::array<QDoubleSpinBox*, 4> lambdaMultiplierSpinBoxes{};
    std::array<QComboBox*, 4> lambdaReferenceModeComboBoxes{};
    QComboBox* nodeKindComboBox{nullptr};
    QComboBox* structureTypeComboBox{nullptr};
    QSpinBox* requiredElementsSpinBox{nullptr};
    QPushButton* applyButton{nullptr};
    Node* editedNode{nullptr};
};

#endif // NODECONFIGURATIONWIDGET_H
