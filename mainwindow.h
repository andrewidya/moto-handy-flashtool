#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QProcess>

class QPushButton;
class QLineEdit;
class QComboBox;
class QTextBrowser;
class QDir;
class QStringList;
class QTime;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QProcess* fastbootErase;
    QProcess* killADB;
    QProcess* startADB;
    QProcess* fastbootFlash;
    QProcess* fastbootReboot;
    QProcess* recoveryProcess;
    QProcess* bootloaderProcess;

private:
    QPushButton* bootloaderButton;
    QPushButton* rebootButton;
    QPushButton* recoveryButton;

    QLineEdit* fileImage;
    QPushButton* browseButton;

    QComboBox* partitionButton;

    QTextBrowser* consoleOutput;

    QPushButton* flashButton;
    QPushButton* exitButton;

    QDir* directory;
    QStringList arg;

    QTime* time;

    // for phone info

private slots:
    void selectImageDialog();
    void bootloaderStageMode();
    void readProcessOutput();
    void readErrorOutput();

    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void fastbootRebootFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void bootloaderProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void flashProcess();
    void flashErase();

    void killADBProcess();
    void startADBProcess();

    void rebootPhone();
    void recoveryMode();

    void setupButton(int buttonCode);
    void initiateButton();
    void flashStart();
};

#endif // MAINWINDOW_H
