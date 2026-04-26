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
    structureTypeComboBox->addItem("Обычный элемент", static_cast<int>(Node::StructureType::Element));
    structureTypeComboBox->addItem("Последовательная структура", static_cast<int>(Node::StructureType::Series));
    structureTypeComboBox->addItem("Параллельная структура", static_cast<int>(Node::StructureType::Parallel));
    structureTypeComboBox->addItem("k из n", static_cast<int>(Node::StructureType::KOutOfN));

    requiredElementsSpinBox = new QSpinBox(this);
    requiredElementsSpinBox->setRange(1, 1000000);

    applyButton = new QPushButton("Применить", this);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("ID", idEdit);
    formLayout->addRow("Название", nameEdit);
    formLayout->addRow("Группа", groupEdit);
    formLayout->addRow("λ транспортирование", failureRateSpinBoxes[static_cast<int>(Node::OperationMode::Transportation)]);
    formLayout->addRow("λ хранение", failureRateSpinBoxes[static_cast<int>(Node::OperationMode::Storage)]);
    formLayout->addRow("λ функционирование", failureRateSpinBoxes[static_cast<int>(Node::OperationMode::Functioning)]);
    formLayout->addRow("λ выключенное состояние", failureRateSpinBoxes[static_cast<int>(Node::OperationMode::Off)]);
    formLayout->addRow("Тип", structureTypeComboBox);
    formLayout->addRow("k", requiredElementsSpinBox);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(modeLabel);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(applyButton);
    mainLayout->addStretch();

    connect(applyButton, &QPushButton::clicked, this, [this]()
    {
        const Node::StructureType structureType = static_cast<Node::StructureType>(structureTypeComboBox->currentData().toInt());
        emit configurationApplied(nameEdit->text(), groupEdit->text(), failureRatesFromFields(), structureType, requiredElementsSpinBox->value());
    });

    connect(structureTypeComboBox, &QComboBox::currentIndexChanged, this, &NodeConfigurationWidget::updateFieldsAvailability);
    updateFieldsAvailability();
}

void NodeConfigurationWidget::editDefaultConfiguration(const QString& name, const QString& groupName, const FailureRates& failureRates, Node::StructureType structureType, int requiredElements)
{
    setConfiguration("Настройки новых узлов", "auto", name, groupName, failureRates, structureType, requiredElements);
}

void NodeConfigurationWidget::editSelectedNode(int id, const QString& name, const QString& groupName, const FailureRates& failureRates, Node::StructureType structureType, int requiredElements)
{
    setConfiguration("Настройки выбранного узла", QString::number(id), name, groupName, failureRates, structureType, requiredElements);
}

FailureRates NodeConfigurationWidget::failureRatesFromFields() const
{
    FailureRates values{};
    for (int i = 0; i < static_cast<int>(values.size()); ++i)
        values[i] = failureRateSpinBoxes[i]->value();
    return values;
}

void NodeConfigurationWidget::setConfiguration(const QString& mode, const QString& idText, const QString& name, const QString& groupName, const FailureRates& failureRates, Node::StructureType structureType, int requiredElements)
{
    modeLabel->setText(mode);
    idEdit->setText(idText);
    nameEdit->setText(name);
    groupEdit->setText(groupName);

    for (int i = 0; i < static_cast<int>(failureRates.size()); ++i)
        failureRateSpinBoxes[i]->setValue(failureRates[i]);

    const int typeIndex = structureTypeComboBox->findData(static_cast<int>(structureType));
    structureTypeComboBox->setCurrentIndex(typeIndex >= 0 ? typeIndex : 0);
    requiredElementsSpinBox->setValue(requiredElements);
    updateFieldsAvailability();
}

void NodeConfigurationWidget::updateFieldsAvailability()
{
    const Node::StructureType structureType = static_cast<Node::StructureType>(structureTypeComboBox->currentData().toInt());
    const bool isElement = structureType == Node::StructureType::Element;
    for (QDoubleSpinBox* spinBox : failureRateSpinBoxes)
        spinBox->setEnabled(isElement);
    requiredElementsSpinBox->setEnabled(structureType == Node::StructureType::KOutOfN);
}
