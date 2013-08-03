/**********************************************************************
Copyright (C) 2013 Andry Widya Putra

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>

***********************************************************************/

#include "mainwindow.h"
#include "consolewindow.h"
#include <QtCore>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTextBrowser>
#include <QFileDialog>
#include <QString>
#include <QProcess>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QErrorMessage>
#include <QIcon>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // for 3 button on up of program
    QHBoxLayout* upButtonLayout = new QHBoxLayout;

    bootloaderButton = new QPushButton("Bootloader Mode");
    rebootButton = new QPushButton("Boot Phone Up");
    recoveryButton = new QPushButton("Recovery Mode");

    bootloaderButton->setFixedSize(150, 50);
    rebootButton->setFixedSize(150, 50);
    recoveryButton->setFixedSize(150, 50);

    QLabel* logo = new QLabel;
    QPixmap img(":/binary/binary/Logo.png");
    logo->setPixmap(img);

    upButtonLayout->addWidget(bootloaderButton);
    upButtonLayout->addWidget(rebootButton);
    upButtonLayout->addWidget(recoveryButton);

    // end of 3 button on up of program

    // for file browse and partition chooser

    QHBoxLayout* midleLayout = new QHBoxLayout;

    QHBoxLayout* fileLayout = new QHBoxLayout;
    QGroupBox* fileGroupBox = new QGroupBox("File Image");

    fileImage = new QLineEdit;
    browseButton = new QPushButton;
    browseButton->setIcon(QIcon(QPixmap(":/binary/images/folder-documents.png")));

    fileLayout->addWidget(fileImage);
    fileLayout->addWidget(browseButton);

    fileGroupBox->setLayout(fileLayout);

    QHBoxLayout* partitionLayout = new QHBoxLayout;
    QGroupBox* partitionGroupBox = new QGroupBox("Partition");

    partitionButton = new QComboBox;
    partitionButton->addItem("system");
    partitionButton->addItem("userdata");
    partitionButton->addItem("hidden");
    partitionButton->addItem("splash");
    partitionButton->addItem("recovery");

    partitionLayout->addWidget(partitionButton);
    partitionGroupBox->setLayout(partitionLayout);

    midleLayout->addWidget(fileGroupBox);
    midleLayout->addWidget(partitionGroupBox);

    // end of file browse and partition chooser

    // for Console ouptut
    consoleOutput = new QTextBrowser;
    consoleOutput->setFontFamily("monospace");

    QHBoxLayout* flashLayout = new QHBoxLayout;
    flashButton = new QPushButton("Start Flash");
    exitButton = new QPushButton("Exit");

    QLabel* credit = new QLabel("Created by Bocor");

    flashLayout->addWidget(credit);
    flashLayout->addStretch();
    flashLayout->addWidget(flashButton);
    flashLayout->addWidget(exitButton);

    mainLayout->addWidget(logo);
    mainLayout->addLayout(upButtonLayout);
    mainLayout->addLayout(midleLayout);
    mainLayout->addWidget(consoleOutput);
    mainLayout->addLayout(flashLayout);

    QIcon icon(":/binary/binary/icon.ico");
    setWindowIcon(icon);

    setFixedSize(482, 500);
    setWindowTitle("Motorola Fire XT530 Flashtool - 0.1");
    time = new QTime;

    QResource::registerResource("/home/andre/Documents/Andre/Experiment Room/Flashtool2/resource.rcc");

    // design end here


    bootloaderProcess = new QProcess(this);
    killADB = new QProcess;
    startADB = new QProcess;
    fastbootErase = new QProcess;
    fastbootReboot = new QProcess;
    fastbootFlash = new QProcess;
    recoveryProcess = new QProcess;

    directory = new QDir(QDir::currentPath());

    killADBProcess();
    initiateButton();

    connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(browseButton, SIGNAL(clicked()), this, SLOT(selectImageDialog()));
    connect(bootloaderButton, SIGNAL(clicked()), this, SLOT(bootloaderStageMode()));
    connect(flashButton, SIGNAL(clicked()), this, SLOT(flashStart()));
    connect(rebootButton, SIGNAL(clicked()), this, SLOT(rebootPhone()));
    connect(recoveryButton, SIGNAL(clicked()), this, SLOT(recoveryMode()));    

    connect(killADB, SIGNAL(finished(int)), this, SLOT(startADBProcess()));

    connect(startADB, SIGNAL(readyReadStandardError()), this, SLOT(readErrorOutput()));
    connect(startADB, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessOutput()));
    connect(startADB, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));

    connect(bootloaderProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessOutput()));
    connect(bootloaderProcess, SIGNAL(readyReadStandardError()), this, SLOT(readErrorOutput()));
    connect(bootloaderProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(bootloaderProcessFinished(int,QProcess::ExitStatus)));

    connect(fastbootReboot, SIGNAL(readyReadStandardError()), this, SLOT(readErrorOutput()));
    connect(fastbootReboot, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessOutput()));
    connect(fastbootReboot, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(fastbootRebootFinished(int,QProcess::ExitStatus)));

    connect(recoveryProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessOutput()));
    connect(recoveryProcess, SIGNAL(readyReadStandardError()), this, SLOT(readErrorOutput()));
    connect(recoveryProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));

    connect(fastbootErase, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessOutput()));
    connect(fastbootErase, SIGNAL(readyReadStandardError()), this, SLOT(readErrorOutput()));
    connect(fastbootErase, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
    connect(fastbootErase, SIGNAL(finished(int)), this, SLOT(flashProcess()));

    connect(fastbootFlash, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessOutput()));
    connect(fastbootFlash, SIGNAL(readyReadStandardError()), this, SLOT(readErrorOutput()));
    connect(fastbootFlash, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
}

MainWindow::~MainWindow()
{
    bootloaderProcess->close();
    killADB->close();
    startADB->close();
    fastbootErase->close();
    fastbootFlash->close();
    fastbootReboot->close();
    recoveryProcess->close();
    destroy(this);
}

/************************************************************
 always restart adb server everytime the program is execute
 it can be used to initiate first execution of the program

 ************************************************************/

void MainWindow::killADBProcess()
{
    QString adb(directory->filePath("binary/adb"));
    QStringList arg;
    arg << "kill-server";
    killADB->start(adb, arg);
    qDebug() << "kill-process()";
    qDebug() << directory->filePath("binary/adb");
}

void MainWindow::startADBProcess()
{
    QString sudo("sudo");
    QStringList arg;
    arg << directory->filePath("binary/adb");
    arg << "start-server";
    startADB->start(sudo, arg);
    consoleOutput->append(time->currentTime().toString() + " - " + "Starting adb server");
    consoleOutput->append(time->currentTime().toString() + " - " + "Please wait..");
}

void MainWindow::initiateButton()
{
    rebootButton->setEnabled(false);
    flashButton->setEnabled(false);
}

void MainWindow::setupButton(int buttonCode)
{
    bool switchButton;
    if (buttonCode == 1)
    {
        switchButton = true;
    }
    if (buttonCode == 2)
    {
        switchButton = false;
    }
    rebootButton->setEnabled(switchButton);
    flashButton->setEnabled(switchButton);
    bootloaderButton->setDisabled(switchButton);
}

/***************************************************************
   choosing image file
 ***************************************************************/
void MainWindow::selectImageDialog()
{
    QString imageFile = QFileDialog::getOpenFileName(this, "Open Image File", QDir::currentPath(), "Android Image File (*.img)");
    fileImage->setText(imageFile);
}

/**************************************************************
  to swithc mode in android phone
  1. bootloader mode
  2. recovery mode
  3. reboot mode, use to reboot phone while in bootloader mode

 **************************************************************/
void MainWindow::bootloaderStageMode()
{
    QString adb = directory->filePath("binary/adb");
    arg << "reboot-bootloader";
    bootloaderProcess->start(adb, arg);
    consoleOutput->append(time->currentTime().toString() + " - " + "Bring your phone into bootloader mode");
    consoleOutput->append(time->currentTime().toString() + " - " + "Please wait...");
}

void MainWindow::rebootPhone()
{
    QString sudo("sudo");
    QStringList arg;
    arg << directory->filePath("binary/fastboot");
    arg << "reboot";
    fastbootReboot->start(sudo, arg);
    consoleOutput->append(time->currentTime().toString() + " - " + "Rebooting your phone");
}

void MainWindow::recoveryMode()
{
    QString adb(directory->filePath("binary/adb"));
    QStringList arg;
    arg << "reboot";
    arg << "recovery";
    recoveryProcess->start(adb, arg);
    consoleOutput->append(time->currentTime().toString() + " - " + "Go into stock recovery mode");
}

/************************************************************
 reading input output of any process on program

 ************************************************************/

void MainWindow::readProcessOutput()
{
    if (bootloaderProcess->isReadable()) {
        consoleOutput->append(time->currentTime().toString() + " - " + bootloaderProcess->readAllStandardOutput());
    } else if (startADB->isReadable()) {
        consoleOutput->append(time->currentTime().toString() + " - " + startADB->readAllStandardOutput());
        startADB->terminate();
        killADB->terminate();
        delete killADB;
        delete startADB;
    } else if (fastbootErase->isReadable()) {
        consoleOutput->append(time->currentTime().toString() + " - " + fastbootErase->readAllStandardOutput());
    } else if (fastbootFlash->isReadable()) {
        consoleOutput->append(time->currentTime().toString() + " - " + fastbootFlash->readAllStandardOutput());
    } else if (fastbootReboot->isReadable()) {
        consoleOutput->append(time->currentTime().toString() + " - " + fastbootReboot->readAllStandardOutput());
    }
}

void MainWindow::readErrorOutput()
{
    if (bootloaderProcess->isReadable()) {
        consoleOutput->append(time->currentTime().toString() + " - " + bootloaderProcess->readAllStandardError());
    } else if (startADB->isReadable()) {
        consoleOutput->append(time->currentTime().toString() + " - " + startADB->readAllStandardError());
    } else if (fastbootErase->isReadable()) {
        consoleOutput->append(time->currentTime().toString() + " - " + fastbootErase->readAllStandardError());
    } else if (fastbootFlash->isReadable()) {
        consoleOutput->append(time->currentTime().toString() + " - " + fastbootFlash->readAllStandardError());
    } else if (fastbootReboot->isReadable()) {
        consoleOutput->append(time->currentTime().toString() + " - " + fastbootReboot->readAllStandardError());
    }
}

void MainWindow::bootloaderProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        consoleOutput->append(time->currentTime().toString() + " - " + "Program Crashed!");
    } else if (exitCode != 0) {
        consoleOutput->append(time->currentTime().toString() + " - " + "Program Interupted");
    } else {
        consoleOutput->append(time->currentTime().toString() + " - " + "Phone is on bootloader mode");
        setupButton(1);
    }
}

void MainWindow::fastbootRebootFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        consoleOutput->append(time->currentTime().toString() + " - " + "Program Crashed!");
    } else if (exitCode !=0 ) {
        consoleOutput->append(time->currentTime().toString() + " - " + "Program Interupted");
    } else {
        consoleOutput->append(time->currentTime().toString() + " - " + "Done");
        setupButton(2);
    }
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        consoleOutput->append(time->currentTime().toString() + " - " + "Program Crashed!!!");
    } else if (exitCode != 0) {
        consoleOutput->append(time->currentTime().toString() + " - " + "Process Interupted");
    } else {
        consoleOutput->append(time->currentTime().toString() + " - " + "Done...");
    }
}

/************************************************************

 declaration of any process
 ************************************************************/

void MainWindow::flashErase()
{
    QString fastboot("sudo");
    QStringList arg;
    arg << directory->filePath("binary/fastboot");
    arg << "erase";
    arg << partitionButton->currentText();
    consoleOutput->append(time->currentTime().toString() + " - " + "Erasing partition");
    fastbootErase->start(fastboot, arg);
    qDebug() << "Erasing Partition  - " << fastbootErase->readChannel();
}

void MainWindow::flashProcess()
{
    QString fastboot("sudo");
    QStringList arg;
    arg << directory->filePath("binary/fastboot");
    arg << "flash";
    arg << partitionButton->currentText();
    arg << fileImage->text();
    consoleOutput->append(time->currentTime().toString() + " - " + "Extracting image file");
    fastbootFlash->start(fastboot, arg);
    qDebug() << "Flashing " << partitionButton->currentText() << fastbootFlash->readAll();
}

void MainWindow::flashStart()
{
    if (fileImage->text().isEmpty())
    {
        QErrorMessage msgBox;
        msgBox.showMessage("There is no file image selected to be flashed");
        msgBox.exec();
        msgBox.setFixedSize(700, 10);
    } else {
        QMessageBox questionBox;
        questionBox.setText("Do you want to continue");
        questionBox.setInformativeText("All data will be deleted, and this process cannot be undone");
        questionBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        questionBox.setDefaultButton(QMessageBox::Ok);
        questionBox.setFixedSize(500, 100);

        int ret = questionBox.exec();
        switch (ret) {
        case QMessageBox::Ok:
            flashErase();
            break;
        case QMessageBox::Cancel:
            break;
        }
    }
}
