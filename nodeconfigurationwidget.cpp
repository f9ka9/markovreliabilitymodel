#include "nodeconfigurationwidget.h"

NodeConfigurationWidget::NodeConfigurationWidget(QWidget* parent): QWidget(parent)
{
    modeLabel = new QLabel(this);

    idEdit = new QLineEdit(this);
    idEdit->setReadOnly(true);

    nameEdit = new QLineEdit(this);
    groupEdit = new QLineEdit(this);

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
    formLayout->addRow("λ транспортирование", createLambdaEditor(OperationMode::Transportation));
    formLayout->addRow("λ хранение", createLambdaEditor(OperationMode::Storage));
    formLayout->addRow("λ функционирование", createLambdaEditor(OperationMode::Functioning));
    formLayout->addRow("λ выключенное состояние", createLambdaEditor(OperationMode::Off));
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
    configuration.lambdaDefinitions = lambdaDefinitionsFromFields();
    configuration.structureType = static_cast<StructureType>(structureTypeComboBox->currentData().toInt());
    configuration.requiredElements = requiredElementsSpinBox->value();
    return configuration;
}

LambdaDefinitions NodeConfigurationWidget::lambdaDefinitionsFromFields() const
{
    LambdaDefinitions definitions = defaultLambdaDefinitions();

    for (int i = 0; i < static_cast<int>(definitions.size()); ++i)
    {
        LambdaDefinition definition;

        definition.inputType = static_cast<LambdaInputType>(lambdaInputTypeComboBoxes[i]->currentData().toInt());
        definition.value = lambdaValueSpinBoxes[i]->value();
        definition.multiplier = lambdaMultiplierSpinBoxes[i]->value();
        definition.referenceMode = static_cast<OperationMode>(lambdaReferenceModeComboBoxes[i]->currentData().toInt());

        definitions[i] = definition;
    }

    return definitions;
}

void NodeConfigurationWidget::setConfiguration(const QString& mode, const QString& idText, const NodeConfiguration& configuration)
{
    modeLabel->setText(mode);
    idEdit->setText(idText);
    nameEdit->setText(configuration.name);
    groupEdit->setText(configuration.groupName);

    for (int i = 0; i < static_cast<int>(configuration.lambdaDefinitions.size()); ++i)
    {
        const LambdaDefinition& definition = configuration.lambdaDefinitions[i];
        const int inputTypeIndex = lambdaInputTypeComboBoxes[i]->findData(static_cast<int>(definition.inputType));
        lambdaInputTypeComboBoxes[i]->setCurrentIndex(inputTypeIndex >= 0 ? inputTypeIndex : 0);
        lambdaValueSpinBoxes[i]->setValue(definition.value);
        lambdaMultiplierSpinBoxes[i]->setValue(definition.multiplier);

        const int referenceIndex = lambdaReferenceModeComboBoxes[i]->findData(static_cast<int>(definition.referenceMode));
        lambdaReferenceModeComboBoxes[i]->setCurrentIndex(referenceIndex >= 0 ? referenceIndex : 0);
    }

    const int typeIndex = structureTypeComboBox->findData(static_cast<int>(configuration.structureType));
    structureTypeComboBox->setCurrentIndex(typeIndex >= 0 ? typeIndex : 0);
    requiredElementsSpinBox->setValue(configuration.requiredElements);
    updateFieldsAvailability();
}

QWidget* NodeConfigurationWidget::createLambdaEditor(OperationMode mode)
{
    const int index = static_cast<int>(mode);
    QWidget* editor = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(editor);
    layout->setContentsMargins(0, 0, 0, 0);

    lambdaInputTypeComboBoxes[index] = new QComboBox(editor);
    lambdaInputTypeComboBoxes[index]->addItem("значение", static_cast<int>(LambdaInputType::Fixed));
    lambdaInputTypeComboBoxes[index]->addItem("выражение", static_cast<int>(LambdaInputType::Expression));

    lambdaValueSpinBoxes[index] = new QDoubleSpinBox(editor);
    lambdaValueSpinBoxes[index]->setDecimals(10);
    lambdaValueSpinBoxes[index]->setRange(0.0, 1.0e9);
    lambdaValueSpinBoxes[index]->setSingleStep(0.000001);
    lambdaValueSpinBoxes[index]->setSuffix(" 1/h");
    lambdaValueSpinBoxes[index]->setButtonSymbols(QAbstractSpinBox::NoButtons);

    lambdaMultiplierSpinBoxes[index] = new QDoubleSpinBox(editor);
    lambdaMultiplierSpinBoxes[index]->setDecimals(6);
    lambdaMultiplierSpinBoxes[index]->setRange(0.0, 1.0e9);
    lambdaMultiplierSpinBoxes[index]->setSingleStep(0.1);
    lambdaMultiplierSpinBoxes[index]->setButtonSymbols(QAbstractSpinBox::NoButtons);

    lambdaReferenceModeComboBoxes[index] = new QComboBox(editor);
    fillReferenceModeComboBox(lambdaReferenceModeComboBoxes[index], mode);

    layout->addWidget(lambdaInputTypeComboBoxes[index]);
    layout->addWidget(lambdaValueSpinBoxes[index]);
    layout->addWidget(lambdaMultiplierSpinBoxes[index]);
    layout->addWidget(new QLabel("×", editor));
    layout->addWidget(lambdaReferenceModeComboBoxes[index]);

    connect(lambdaInputTypeComboBoxes[index], &QComboBox::currentIndexChanged, this, &NodeConfigurationWidget::updateLambdaFieldsAvailability);

    return editor;
}

void NodeConfigurationWidget::fillReferenceModeComboBox(QComboBox* comboBox, OperationMode editedMode)
{
    auto addMode = [comboBox, editedMode](const QString& label, OperationMode mode)
    {
        if (mode != editedMode)
            comboBox->addItem(label, static_cast<int>(mode));
    };

    addMode("λ транспортирование", OperationMode::Transportation);
    addMode("λ хранение", OperationMode::Storage);
    addMode("λ функционирование", OperationMode::Functioning);
    addMode("λ выключенное состояние", OperationMode::Off);
}

void NodeConfigurationWidget::updateFieldsAvailability()
{
    const StructureType structureType = static_cast<StructureType>(structureTypeComboBox->currentData().toInt());
    const bool isElement = structureType == StructureType::Element;

    for (int i = 0; i < static_cast<int>(lambdaInputTypeComboBoxes.size()); ++i)
    {
        if (!lambdaInputTypeComboBoxes[i]
            || !lambdaValueSpinBoxes[i]
            || !lambdaMultiplierSpinBoxes[i]
            || !lambdaReferenceModeComboBoxes[i])
            continue;

        lambdaInputTypeComboBoxes[i]->setEnabled(isElement);
        lambdaValueSpinBoxes[i]->setEnabled(isElement);
        lambdaMultiplierSpinBoxes[i]->setEnabled(isElement);
        lambdaReferenceModeComboBoxes[i]->setEnabled(isElement);
    }

    requiredElementsSpinBox->setEnabled(structureType == StructureType::KOutOfN);
    updateLambdaFieldsAvailability();
}

void NodeConfigurationWidget::updateLambdaFieldsAvailability()
{
    const StructureType structureType = static_cast<StructureType>(structureTypeComboBox->currentData().toInt());
    const bool isElement = structureType == StructureType::Element;

    for (int i = 0; i < static_cast<int>(lambdaInputTypeComboBoxes.size()); ++i)
    {
        if (!lambdaInputTypeComboBoxes[i]
            || !lambdaValueSpinBoxes[i]
            || !lambdaMultiplierSpinBoxes[i]
            || !lambdaReferenceModeComboBoxes[i])
            continue;

        const LambdaInputType inputType = static_cast<LambdaInputType>(lambdaInputTypeComboBoxes[i]->currentData().toInt());
        const bool expression = inputType == LambdaInputType::Expression;

        lambdaValueSpinBoxes[i]->setEnabled(isElement && !expression);
        lambdaMultiplierSpinBoxes[i]->setEnabled(isElement && expression);
        lambdaReferenceModeComboBoxes[i]->setEnabled(isElement && expression);
    }
}
