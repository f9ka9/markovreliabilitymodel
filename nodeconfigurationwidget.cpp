#include "nodeconfigurationwidget.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

NodeConfigurationWidget::NodeConfigurationWidget(QWidget* parent): QWidget(parent)
{
    modeLabel = new QLabel(this);

    idEdit = new QLineEdit(this);
    idEdit->setReadOnly(true);

    nameEdit = new QLineEdit(this);
    groupEdit = new QLineEdit(this);

    for (QDoubleSpinBox*& spinBox : failureRateSpinBoxes)
    {
        spinBox = new QDoubleSpinBox(this);
        spinBox->setDecimals(10);
        spinBox->setRange(0.0, 1.0e9);
        spinBox->setSingleStep(0.000001);
        spinBox->setSuffix(" 1/h");
    }

    structureTypeComboBox = new QComboBox(this);
    structureTypeComboBox->addItem("Обычный элемент", static_cast<int>(StructureType::Element));
    structureTypeComboBox->addItem("Последовательная структура", static_cast<int>(StructureType::Series));
    structureTypeComboBox->addItem("Параллельная структура", static_cast<int>(StructureType::Parallel));
    structureTypeComboBox->addItem("k из n", static_cast<int>(StructureType::KOutOfN));

    requiredElementsSpinBox = new QSpinBox(this);
    requiredElementsSpinBox->setRange(1, 1000000);

    applyButton = new QPushButton("Применить", this);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("ID", idEdit);
    formLayout->addRow("Название", nameEdit);
    formLayout->addRow("Группа", groupEdit);
    formLayout->addRow("λ транспортирование", failureRateSpinBoxes[static_cast<int>(OperationMode::Transportation)]);
    formLayout->addRow("λ хранение", failureRateSpinBoxes[static_cast<int>(OperationMode::Storage)]);
    formLayout->addRow("λ функционирование", failureRateSpinBoxes[static_cast<int>(OperationMode::Functioning)]);
    formLayout->addRow("λ выключенное состояние", failureRateSpinBoxes[static_cast<int>(OperationMode::Off)]);
    formLayout->addRow("Тип", structureTypeComboBox);
    formLayout->addRow("k", requiredElementsSpinBox);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(modeLabel);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(applyButton);
    mainLayout->addStretch();

    connect(applyButton, &QPushButton::clicked, this, [this]()
    {
        emit configurationApplied(editedNode, configurationFromFields());
    });

    connect(structureTypeComboBox, &QComboBox::currentIndexChanged, this, &NodeConfigurationWidget::updateFieldsAvailability);
    updateFieldsAvailability();
}

void NodeConfigurationWidget::editDefaultConfiguration(const NodeConfiguration& configuration)
{
    editedNode = nullptr;
    setConfiguration("Настройки новых узлов", "auto", configuration);
}

void NodeConfigurationWidget::editSelectedNode(Node* node)
{
    if (!node) return;

    editedNode = node;
    setConfiguration("Настройки выбранного узла",
                     QString::number(node->getId()),
                     node->getConfiguration());
}

bool NodeConfigurationWidget::isEditingNode(Node* node) const
{
    return editedNode == node;
}

NodeConfiguration NodeConfigurationWidget::configurationFromFields() const
{
    NodeConfiguration configuration;
    configuration.name = nameEdit->text();
    configuration.groupName = groupEdit->text();
    configuration.failureRates = failureRatesFromFields();
    configuration.structureType = static_cast<StructureType>(structureTypeComboBox->currentData().toInt());
    configuration.requiredElements = requiredElementsSpinBox->value();
    return configuration;
}

FailureRates NodeConfigurationWidget::failureRatesFromFields() const
{
    FailureRates values{};
    for (int i = 0; i < static_cast<int>(values.size()); ++i)
        values[i] = failureRateSpinBoxes[i]->value();
    return values;
}

void NodeConfigurationWidget::setConfiguration(const QString& mode, const QString& idText, const NodeConfiguration& configuration)
{
    modeLabel->setText(mode);
    idEdit->setText(idText);
    nameEdit->setText(configuration.name);
    groupEdit->setText(configuration.groupName);

    for (int i = 0; i < static_cast<int>(configuration.failureRates.size()); ++i)
        failureRateSpinBoxes[i]->setValue(configuration.failureRates[i]);

    const int typeIndex = structureTypeComboBox->findData(static_cast<int>(configuration.structureType));
    structureTypeComboBox->setCurrentIndex(typeIndex >= 0 ? typeIndex : 0);
    requiredElementsSpinBox->setValue(configuration.requiredElements);
    updateFieldsAvailability();
}

void NodeConfigurationWidget::updateFieldsAvailability()
{
    const StructureType structureType = static_cast<StructureType>(structureTypeComboBox->currentData().toInt());
    const bool isElement = structureType == StructureType::Element;
    for (QDoubleSpinBox* spinBox : failureRateSpinBoxes)
        spinBox->setEnabled(isElement);
    requiredElementsSpinBox->setEnabled(structureType == StructureType::KOutOfN);
}
