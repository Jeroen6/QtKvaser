#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include <QProcess>
#include "canlib.h"

// My debug with prefix
#define debugprefix  << __LINE__ << " "

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_Connect_clicked();
    void uiUpdate();
    void on_checkBox_PassiveMode_clicked(bool checked);
    void on_checkBox_ASCIImode_clicked(bool checked);
    void on_spinBoxCanIDHex_valueChanged(const QString &arg1);
    void on_checkBox_RTR_clicked(bool checked);
    void on_spinBoxDLC_valueChanged(int arg1);
    void on_bytes_changed();
    void on_lineEdit_CanOut_textChanged(const QString &arg1);
    void on_checkBox_CanExtended_clicked(bool checked);
    void on_pushButton_CanSend_clicked();
    void on_pushButtonRefresh_clicked();
    void on_treeWidgetInterfaces_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_pushButtonBusOn_clicked();

    void on_actionInstall_Drivers_triggered();

private:
    Ui::MainWindow *ui;
    QList<QTreeWidgetItem *> treeWidgetItems;

    QTimer uitimer;
    bool connected;
    bool busOn;
    bool ignoreBytesChanged;
    int canHandle;

};

#endif // MAINWINDOW_H
