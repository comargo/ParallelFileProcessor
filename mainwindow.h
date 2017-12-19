#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "controller.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void loadSavedState();
    void saveCurrentState();

protected:
    void setupStateMachine();
protected slots:
    void on_inputDirectoryBrowse_clicked();
    void on_outputDirectoryBrowse_clicked();
    void on_processingToolBrowse_clicked();
private:
    Ui::MainWindow *ui;
    Controller *controller;
};

#endif // MAINWINDOW_H
