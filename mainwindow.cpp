#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFinalState>
#include <QSettings>
#include <QStateMachine>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , controller(nullptr)
{
    ui->setupUi(this);
    controller = new Controller(this);
    connect(ui->inputDirectoryLineEdit, &QLineEdit::textChanged, controller, &Controller::setInputDirectory);
    connect(ui->outputDirectoryLineEdit, &QLineEdit::textChanged, controller, &Controller::setOutputDirectory);
    connect(ui->fileFilterLineEdit, &QLineEdit::textChanged, controller, &Controller::setFileFilter);
    connect(ui->processingToolLineEdit, &QLineEdit::textChanged, controller, &Controller::setTool);
    connect(ui->processingToolArgumentsLineEdit, &QLineEdit::textChanged, controller, &Controller::setToolArguments);
    connect(controller, &Controller::filesNumberChanged, ui->progressBar, &QProgressBar::setMaximum);
    connect(controller, &Controller::filesProcessed, ui->progressBar, &QProgressBar::setValue);
    setupStateMachine();
    loadSavedState();
}

MainWindow::~MainWindow()
{
    saveCurrentState();
    delete ui;
}

void MainWindow::loadSavedState()
{
    QSettings settings;
    ui->inputDirectoryLineEdit->setText(settings.value(QStringLiteral("inputDirectory"), QDir::toNativeSeparators(QDir::homePath())).toString());
    ui->outputDirectoryLineEdit->setText(settings.value(QStringLiteral("outputDirectory"), ui->inputDirectoryLineEdit->text()+"-out").toString());
    ui->fileFilterLineEdit->setText(settings.value(QStringLiteral("fileFilter")).toString());
    ui->processingToolLineEdit->setText(settings.value(QStringLiteral("tool")).toString());
    ui->processingToolArgumentsLineEdit->setText(settings.value(QStringLiteral("toolArgs")).toString());
}

void MainWindow::saveCurrentState()
{
    QSettings settings;

    settings.setValue(QStringLiteral("inputDirectory"), ui->inputDirectoryLineEdit->text());
    settings.setValue(QStringLiteral("outputDirectory"), ui->outputDirectoryLineEdit->text());
    settings.setValue(QStringLiteral("fileFilter"), ui->fileFilterLineEdit->text());
    settings.setValue(QStringLiteral("tool"), ui->processingToolLineEdit->text());
    settings.setValue(QStringLiteral("toolArgs"), ui->processingToolArgumentsLineEdit->text());
}

void MainWindow::on_inputDirectoryBrowse_clicked()
{
    QString input = QFileDialog::getExistingDirectory(this, tr("Input directory"), QDir::fromNativeSeparators(ui->inputDirectoryLineEdit->text()));
    if(!input.isEmpty()) {
        ui->inputDirectoryLineEdit->setText(QDir::toNativeSeparators(input));
        ui->outputDirectoryLineEdit->setText(QDir::toNativeSeparators(input+"-out"));
    }
}

void MainWindow::on_outputDirectoryBrowse_clicked()
{
    QString output = QFileDialog::getExistingDirectory(this, tr("Output directory"), QDir::fromNativeSeparators(ui->outputDirectoryLineEdit->text()));
    if(!output.isEmpty()) {
        ui->outputDirectoryLineEdit->setText(QDir::toNativeSeparators(output));
    }
}

void MainWindow::on_processingToolBrowse_clicked()
{
    QString tool = QFileDialog::getOpenFileName(this, tr("Processing tool"), QDir::fromNativeSeparators(ui->processingToolLineEdit->text()), tr("Executable files (*.exe)"));
    if(!tool.isEmpty()) {
        ui->processingToolLineEdit->setText(QDir::toNativeSeparators(tool));
    }
}

void MainWindow::setupStateMachine()
{
    QStateMachine *machine = new QStateMachine(this);

    QState *idle = new QState();
    idle->setObjectName(QStringLiteral("idle"));

    idle->assignProperty(ui->progressBar, "value", 0);
    idle->assignProperty(ui->startStopButton, "text", tr("&Start"));
    idle->assignProperty(ui->startStopButton, "enabled", true);
    connect(idle, &QState::entered, this, &MainWindow::loadSavedState);
    connect(idle, &QState::exited, this, &MainWindow::saveCurrentState);

    QState *working = new QState();
    working->setObjectName(QStringLiteral("working"));
    working->assignProperty(ui->startStopButton, "text", tr("&Stop"));

    QState *running = new QState(working);
    running->setObjectName(QStringLiteral("running"));
    running->assignProperty(ui->startStopButton, "enabled", true);
    connect(running, &QState::entered, controller, &Controller::start);

    QState *stopping = new QState(working);
    stopping->setObjectName(QStringLiteral("stopping"));
    stopping->assignProperty(ui->startStopButton, "enabled", false);
    connect(stopping, &QState::entered, controller, &Controller::stop);

    QFinalState *stopWorking = new QFinalState(working);
    stopWorking->setObjectName(QStringLiteral("stopWorking"));

    QFinalState *final = new QFinalState();
    final->setObjectName(QStringLiteral("final"));

    QWidget* commonUiObject[] = {
        ui->inputDirectoryLabel, ui->inputDirectoryLineEdit, ui->inputDirectoryBrowse,
        ui->outputDirectoryLabel, ui->outputDirectoryLineEdit, ui->outputDirectoryBrowse,
        ui->fileFilterLabel, ui->fileFilterLineEdit,
        ui->processingToolLabel, ui->processingToolLineEdit, ui->processingToolBrowse,
        ui->processingToolArgumentsLabel, ui->processingToolArgumentsLineEdit
    };
    for(QWidget* widget : commonUiObject) {
        idle->assignProperty(widget, "enabled", true);
        working->assignProperty(widget, "enabled", false);
    }

    machine->addState(idle);
    machine->setInitialState(idle);
    machine->addState(working);
    working->setInitialState(running);
    machine->addState(final);

    idle->addTransition(ui->startStopButton, SIGNAL(clicked(bool)), working);
    working->addTransition(working, SIGNAL(finished()), idle);

    running->addTransition(ui->startStopButton, SIGNAL(clicked(bool)), stopping);
    running->addTransition(controller, SIGNAL(finished()), stopWorking);
    stopping->addTransition(controller, SIGNAL(finished()), stopWorking);

    machine->start();
}
