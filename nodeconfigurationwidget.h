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

    void editDefaultConfiguration(const NodeConfiguration& configuration);
    void editSelectedNode(Node* node);
    bool isEditingNode(Node* node) const;

signals:
    void configurationApplied(Node* node, const NodeConfiguration& configuration);

private:
    NodeConfiguration configurationFromFields() const;
    FailureRates failureRatesFromFields() const;
    void setConfiguration(const QString& mode, const QString& idText, const NodeConfiguration& configuration);
    void updateFieldsAvailability();

    QLabel* modeLabel{nullptr};
    QLineEdit* idEdit{nullptr};
    QLineEdit* nameEdit{nullptr};
    QLineEdit* groupEdit{nullptr};
    std::array<QDoubleSpinBox*, 4> failureRateSpinBoxes{};
    QComboBox* structureTypeComboBox{nullptr};
    QSpinBox* requiredElementsSpinBox{nullptr};
    QPushButton* applyButton{nullptr};
    Node* editedNode{nullptr};
};

#endif // NODECONFIGURATIONWIDGET_H
