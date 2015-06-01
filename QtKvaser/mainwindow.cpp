#include "mainwindow.h"
#include "ui_mainwindow.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connected = false;
    canHandle = -1;
    busOn = false;

    uitimer.setInterval(100);
    uitimer.start();

    connect(&uitimer, SIGNAL(timeout()), this, SLOT(uiUpdate()));

    connect(ui->lineEdit_CanByte0, SIGNAL(textChanged(QString)), this, SLOT(on_bytes_changed()));
    connect(ui->lineEdit_CanByte1, SIGNAL(textChanged(QString)), this, SLOT(on_bytes_changed()));
    connect(ui->lineEdit_CanByte2, SIGNAL(textChanged(QString)), this, SLOT(on_bytes_changed()));
    connect(ui->lineEdit_CanByte3, SIGNAL(textChanged(QString)), this, SLOT(on_bytes_changed()));
    connect(ui->lineEdit_CanByte4, SIGNAL(textChanged(QString)), this, SLOT(on_bytes_changed()));
    connect(ui->lineEdit_CanByte5, SIGNAL(textChanged(QString)), this, SLOT(on_bytes_changed()));
    connect(ui->lineEdit_CanByte6, SIGNAL(textChanged(QString)), this, SLOT(on_bytes_changed()));
    connect(ui->lineEdit_CanByte7, SIGNAL(textChanged(QString)), this, SLOT(on_bytes_changed()));

    canInitializeLibrary();
    on_pushButtonRefresh_clicked();

    ui->progressBarBusLoad->setMinimum(0);
    ui->progressBarBusLoad->setMaximum(10000);
    ui->comboBoxBaud->setCurrentIndex(2);
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief MainWindow::uiUpdate
 */
void MainWindow::uiUpdate(){
    ui->pushButtonBusOn->setEnabled(connected);
    ui->comboBoxBaud->setEnabled(!busOn);
    ui->checkBox_Silent->setEnabled(!busOn);

    if(connected){
        canBusStatistics stats;
        canStatus stat;
        stat = canRequestBusStatistics(canHandle);
        if( stat < 0){
            qDebug() debugprefix << "canRequestBusStatistics failed";
        }else{
            stat = canGetBusStatistics(canHandle, &stats, sizeof(canBusStatistics));
            if(stat < 0){
                qDebug() debugprefix << "canGetBusStatistics failed";
            }else{
                ui->label_corruptCounter->setText(QString().setNum(stats.errFrame));
                unsigned long c = stats.stdData + stats.stdRemote + stats.extData + stats.extRemote;
                ui->label_packetCounter->setText(QString().setNum(c));
                ui->progressBarBusLoad->setValue(stats.busLoad);
            }
        }
        if(busOn){
            // Dirty but effective
            while(1){
                long id;
                unsigned char data[8];
                unsigned int dlc, flags;
                unsigned long timestamp;

                canStatus stat = canRead(canHandle, &id, data, &dlc, &flags, &timestamp);
                if (stat < 0) {
                    //qDebug() debugprefix << "canRead() end of queue";
                    break;
                }else{
                    QString l;
                    if(flags & canMSG_ERROR_FRAME){
                        l.sprintf("%ld",timestamp);
                        qDebug() debugprefix << "canRead() got error";
                    }else{
                        int rtr = 0;
                        if( flags & canMSG_RTR ) rtr = 1;
                        l.sprintf("%ld\tID:0x%08X\tRTR:%d\t",timestamp,id,rtr);
                        if( dlc > 0 ) l.append(QString().sprintf("DATA: %02X", (quint8)data[0]));
                        if( dlc > 1 ) l.append(QString().sprintf("%02X", (quint8)data[1]));
                        if( dlc > 2 ) l.append(QString().sprintf("%02X", (quint8)data[2]));
                        if( dlc > 3 ) l.append(QString().sprintf("%02X", (quint8)data[3]));
                        if( dlc > 4 ) l.append(QString().sprintf("%02X", (quint8)data[4]));
                        if( dlc > 5 ) l.append(QString().sprintf("%02X", (quint8)data[5]));
                        if( dlc > 6 ) l.append(QString().sprintf("%02X", (quint8)data[6]));
                        if( dlc > 7 ) l.append(QString().sprintf("%02X", (quint8)data[7]));
                    }
                    qDebug() debugprefix << "canRead() got valid message";
                    ui->textEditCanIn->append(l);
                }
            }
        }
    }
}

/**
 * @brief MainWindow::on_pushButton_Connect_clicked
 */
void MainWindow::on_pushButton_Connect_clicked()
{
    if(canHandle < 0){
        int ch = ui->lineEditInterface->text().toInt();
        canHandle = canOpenChannel(ch, canOPEN_ACCEPT_VIRTUAL);
        if( canHandle < 0 ){
            qDebug() debugprefix << "canOpenChannel("<< ch << ") failed";
            return;
        }
        kvFlashLeds(canHandle, kvLED_ACTION_ALL_LEDS_ON, 1000);
        connected = true;
        ui->pushButton_Connect->setText("Disconnect");
        qDebug() debugprefix << "canOpenChannel("<< ch << ") connected";
    }else{
        if(connected){
            if(busOn) on_pushButtonBusOn_clicked();
            canClose(canHandle);
            ui->pushButton_Connect->setText("Connect");
            qDebug() debugprefix << "canClose("<< canHandle << ") disconnected";
            canHandle = -1;
            connected = false;
            return;
        }
    }
}

/**
 * @brief MainWindow::on_checkBox_PassiveMode_clicked
 * @param checked
 */
void MainWindow::on_checkBox_PassiveMode_clicked(bool checked)
{
    (void)checked;
}

/**
 * @brief MainWindow::on_checkBox_ASCIImode_clicked
 * @param checked
 */
void MainWindow::on_checkBox_ASCIImode_clicked(bool checked)
{
    (void)checked;
}

/**
 * @brief MainWindow::on_spinBoxCanIDHex_valueChanged
 * @param arg1
 */
void MainWindow::on_spinBoxCanIDHex_valueChanged(const QString &arg1)
{
    (void)arg1;
    if( ui->spinBoxCanIDHex->value() > 0x7FF && !ui->checkBox_CanExtended->isChecked() ){
        ui->checkBox_CanExtended->setChecked(true);
    }
}

/**
 * @brief MainWindow::on_checkBox_RTR_clicked
 * @param checked
 */
void MainWindow::on_checkBox_RTR_clicked(bool checked)
{
    if(checked) ui->spinBoxDLC->setValue(0);
}

/**
 * @brief MainWindow::on_spinBoxDLC_valueChanged
 * @param x
 */
void MainWindow::on_spinBoxDLC_valueChanged(int x)
{
    ui->lineEdit_CanByte0->setEnabled(false);
    ui->lineEdit_CanByte1->setEnabled(false);
    ui->lineEdit_CanByte2->setEnabled(false);
    ui->lineEdit_CanByte3->setEnabled(false);
    ui->lineEdit_CanByte4->setEnabled(false);
    ui->lineEdit_CanByte5->setEnabled(false);
    ui->lineEdit_CanByte6->setEnabled(false);
    ui->lineEdit_CanByte7->setEnabled(false);
    if(x > 0){ ui->lineEdit_CanByte0->setEnabled(true); ui->checkBox_RTR->setChecked(false); }
    if(x > 1) ui->lineEdit_CanByte1->setEnabled(true);
    if(x > 2) ui->lineEdit_CanByte2->setEnabled(true);
    if(x > 3) ui->lineEdit_CanByte3->setEnabled(true);
    if(x > 4) ui->lineEdit_CanByte4->setEnabled(true);
    if(x > 5) ui->lineEdit_CanByte5->setEnabled(true);
    if(x > 6) ui->lineEdit_CanByte6->setEnabled(true);
    if(x > 7) ui->lineEdit_CanByte7->setEnabled(true);
    on_lineEdit_CanOut_textChanged(NULL);
    QString n;
    for(int i=0; i<x; i++) n.append("hh");
    ui->lineEdit_CanOut->setInputMask(n);
}

/**
 * @brief MainWindow::on_bytes_changed
 */
void MainWindow::on_bytes_changed()
{
    if( ignoreBytesChanged ){
        ignoreBytesChanged = false;
    }else{
        // Compile word
        //quint64 word = 0;
        //bool c;
        int x = ui->spinBoxDLC->value();
        //    if(x > 0) word += (quint64)ui->lineEdit_CanByte0->text().toUInt(&c, 16);
        //    if(x > 1) word += (quint64)ui->lineEdit_CanByte1->text().toUInt(&c, 16)<<8;
        //    if(x > 2) word += (quint64)ui->lineEdit_CanByte2->text().toUInt(&c, 16)<<16;
        //    if(x > 3) word += (quint64)ui->lineEdit_CanByte3->text().toUInt(&c, 16)<<24;
        //    if(x > 4) word += (quint64)ui->lineEdit_CanByte4->text().toUInt(&c, 16)<<32;
        //    if(x > 5) word += (quint64)ui->lineEdit_CanByte5->text().toUInt(&c, 16)<<40;
        //    if(x > 6) word += (quint64)ui->lineEdit_CanByte6->text().toUInt(&c, 16)<<48;
        //    if(x > 7) word += (quint64)ui->lineEdit_CanByte7->text().toUInt(&c, 16)<<56;
        QString n;
        if(x > 0) n.append(ui->lineEdit_CanByte0->text());
        if(x > 1) n.append(ui->lineEdit_CanByte1->text());
        if(x > 2) n.append(ui->lineEdit_CanByte2->text());
        if(x > 3) n.append(ui->lineEdit_CanByte3->text());
        if(x > 4) n.append(ui->lineEdit_CanByte4->text());
        if(x > 5) n.append(ui->lineEdit_CanByte5->text());
        if(x > 6) n.append(ui->lineEdit_CanByte6->text());
        if(x > 7) n.append(ui->lineEdit_CanByte7->text());
        ui->lineEdit_CanOut->setText(n);
    }
}

/**
 * @brief MainWindow::on_lineEdit_CanOut_textChanged
 * @param arg1
 */
void MainWindow::on_lineEdit_CanOut_textChanged(const QString &arg1)
{
    (void)arg1;
    QString n = ui->lineEdit_CanOut->text();
    QString r;
    int x = ui->spinBoxDLC->value();

    if(x > 0){ignoreBytesChanged = true; ui->lineEdit_CanByte0->setText( n.mid(0,2) ); n.remove(0,2); }
    if(x > 1){ignoreBytesChanged = true; ui->lineEdit_CanByte1->setText( n.mid(0,2) ); n.remove(0,2); }
    if(x > 2){ignoreBytesChanged = true; ui->lineEdit_CanByte2->setText( n.mid(0,2) ); n.remove(0,2); }
    if(x > 3){ignoreBytesChanged = true; ui->lineEdit_CanByte3->setText( n.mid(0,2) ); n.remove(0,2); }
    if(x > 4){ignoreBytesChanged = true; ui->lineEdit_CanByte4->setText( n.mid(0,2) ); n.remove(0,2); }
    if(x > 5){ignoreBytesChanged = true; ui->lineEdit_CanByte5->setText( n.mid(0,2) ); n.remove(0,2); }
    if(x > 6){ignoreBytesChanged = true; ui->lineEdit_CanByte6->setText( n.mid(0,2) ); n.remove(0,2); }
    if(x > 7){ignoreBytesChanged = true; ui->lineEdit_CanByte7->setText( n.mid(0,2) ); n.remove(0,2); }
}

/**
 * @brief MainWindow::on_checkBox_CanExtended_clicked
 * @param checked
 */
void MainWindow::on_checkBox_CanExtended_clicked(bool checked)
{
    if(!checked && ui->spinBoxCanIDHex->value() > 0x7FF){
        ui->spinBoxCanIDHex->setValue(0x7FF);
    }
}

/**
 * @brief MainWindow::on_pushButton_CanSend_clicked
 */
void MainWindow::on_pushButton_CanSend_clicked()
{
    //    CanMsg m;
    //    m.dlc = ui->spinBoxDLC->value();
    //    m.extmsg = ui->checkBox_CanExtended->isChecked();
    //    if(m.extmsg)
    //        m.extid = ui->spinBoxCanIDHex->value();
    //    else
    //        m.stdid = ui->spinBoxCanIDHex->value();
    //    m.rtr = ui->checkBox_RTR->isChecked();

    //    if(m.dlc > 0 && ui->lineEdit_CanByte0->text() == QString("")) ui->lineEdit_CanByte0->setText("00");
    //    if(m.dlc > 1 && ui->lineEdit_CanByte1->text() == QString("")) ui->lineEdit_CanByte1->setText("00");
    //    if(m.dlc > 2 && ui->lineEdit_CanByte2->text() == QString("")) ui->lineEdit_CanByte2->setText("00");
    //    if(m.dlc > 3 && ui->lineEdit_CanByte3->text() == QString("")) ui->lineEdit_CanByte3->setText("00");
    //    if(m.dlc > 4 && ui->lineEdit_CanByte4->text() == QString("")) ui->lineEdit_CanByte4->setText("00");
    //    if(m.dlc > 5 && ui->lineEdit_CanByte5->text() == QString("")) ui->lineEdit_CanByte5->setText("00");
    //    if(m.dlc > 6 && ui->lineEdit_CanByte6->text() == QString("")) ui->lineEdit_CanByte6->setText("00");
    //    if(m.dlc > 7 && ui->lineEdit_CanByte7->text() == QString("")) ui->lineEdit_CanByte7->setText("00");

    //    bool c;
    //    if(m.dlc > 0) m.data.append(ui->lineEdit_CanByte0->text().toUInt(&c,16));
    //    if(m.dlc > 1) m.data.append(ui->lineEdit_CanByte1->text().toUInt(&c,16));
    //    if(m.dlc > 2) m.data.append(ui->lineEdit_CanByte2->text().toUInt(&c,16));
    //    if(m.dlc > 3) m.data.append(ui->lineEdit_CanByte3->text().toUInt(&c,16));
    //    if(m.dlc > 4) m.data.append(ui->lineEdit_CanByte4->text().toUInt(&c,16));
    //    if(m.dlc > 5) m.data.append(ui->lineEdit_CanByte5->text().toUInt(&c,16));
    //    if(m.dlc > 6) m.data.append(ui->lineEdit_CanByte6->text().toUInt(&c,16));
    //    if(m.dlc > 7) m.data.append(ui->lineEdit_CanByte7->text().toUInt(&c,16));

    if(connected && busOn && !ui->checkBox_Silent->isChecked()){
        char data[8];
        int flags = 0;
        data[0] = ui->lineEdit_CanByte0->text().toUInt(NULL,16);
        data[1] = ui->lineEdit_CanByte1->text().toUInt(NULL,16);
        data[2] = ui->lineEdit_CanByte2->text().toUInt(NULL,16);
        data[3] = ui->lineEdit_CanByte3->text().toUInt(NULL,16);
        data[4] = ui->lineEdit_CanByte4->text().toUInt(NULL,16);
        data[5] = ui->lineEdit_CanByte5->text().toUInt(NULL,16);
        data[6] = ui->lineEdit_CanByte6->text().toUInt(NULL,16);
        data[7] = ui->lineEdit_CanByte7->text().toUInt(NULL,16);
        if( ui->checkBox_CanExtended->isChecked() ){
            flags += canMSG_EXT;
        }
        if(ui->checkBox_RTR->isChecked()){
            flags += canMSG_RTR;
        }
        canWrite(canHandle, ui->spinBoxCanIDHex->value(), data, ui->spinBoxDLC->value(), flags );
    }else{
        QMessageBox msgBox;
        msgBox.setText("Current interface status does not allow transmitting can frames.\n\n- Connected?\n- Bus On?\n- Silent?");
        msgBox.exec();
    }
}

/**
 * @brief MainWindow::on_pushButtonRefresh_clicked
 */
void MainWindow::on_pushButtonRefresh_clicked()
{
    qDebug() debugprefix << "on_pushButtonRefresh_clicked() scanning channels" ;
    int c,i;
    treeWidgetItems.clear();
    ui->treeWidgetInterfaces->clear();
    canStatus stat = canGetNumberOfChannels(&c);
    if( canOK == stat ){
        for (i=0; i < c; i++) {
            char tmp[255];
            stat = canGetChannelData(i, canCHANNELDATA_DEVDESCR_ASCII, &tmp, sizeof(tmp));
            if (stat < 0){
                char buf[128];
                canGetErrorText(stat,buf,128);
                qDebug() debugprefix << "canGetChannelData() " << buf ;
            }else{
                QTreeWidgetItem *port = new QTreeWidgetItem(ui->treeWidgetInterfaces);
                QTreeWidgetItem *name = new QTreeWidgetItem(port);
                port->setText(0, QString().setNum(i,10));
                name->setText(0, QString(tmp));
                qDebug() debugprefix << "canGetChannelData("<< i << ") " << tmp;
                port->setExpanded(true);
            }
        }
    }else{
        char buf[128];
        canGetErrorText(stat,buf,128);
        qDebug() debugprefix << "canGetNumberOfChannels() " << buf ;
    }
}


void MainWindow::on_treeWidgetInterfaces_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    qDebug() debugprefix << "on_treeWidgetInterfaces_itemDoubleClicked(" << item->text(0) << "," << column << ")";
    if(item->childCount())
        ui->lineEditInterface->setText( item->text(0) );
    else
        ui->lineEditInterface->setText( item->parent()->text(0) );

}

void MainWindow::on_pushButtonBusOn_clicked()
{
    bool fail = false;
    canStatus stat;
    qDebug() debugprefix << "on_pushButtonBusOn_clicked()";

    if(busOn){
        canBusOff(canHandle);
        ui->pushButtonBusOn->setText("Go On Bus");
        busOn = false;
    }else{
        // Timings
        int br = (ui->comboBoxBaud->currentIndex()+1)*-1;
        stat = canSetBusParams(canHandle, br  , 0, 0, 0, 0, 0);
        if (stat < 0){
            char buf[128];
            canGetErrorText(stat,buf,128);
            qDebug() debugprefix << "canSetBusParams("<< br << ") " << buf;
            fail = true;
        }

        // Silent
        bool silent = ui->checkBox_Silent->isChecked();
        if(silent){
            stat = canSetBusOutputControl(canHandle, canDRIVER_SILENT);
        }else{
            stat = canSetBusOutputControl(canHandle, canDRIVER_NORMAL);
        }
        if (stat < 0){
            char buf[128];
            canGetErrorText(stat,buf,128);
            qDebug() debugprefix << "canSetBusOutputControl("<< silent << ") " << buf;
            fail = true;
        }

        if(fail == false){
            stat = canBusOn(canHandle);
            if (stat < 0){
                char buf[128];
                canGetErrorText(stat,buf,128);
                qDebug() debugprefix << "canBusOn("<< br << ") " << buf;
                fail = true;
                QMessageBox msgBox;
                msgBox.setText("Couldn't enter Bus-On state");
                msgBox.exec();
            }else{
                ui->pushButtonBusOn->setText("Go Off Bus");
                busOn = true;
            }
        }else{
            QMessageBox msgBox;
            msgBox.setText("Couldn't enter Bus-On state due to initialization errors");
            msgBox.exec();
        }
    }
}

void MainWindow::on_actionInstall_Drivers_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Install", "Install kvaser drivers?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // install drivers
        system("kvaser_drivers_setup.exe");
        canInitializeLibrary();
        on_pushButtonRefresh_clicked();
    }else{
        qDebug() << "Yes was *not* clicked";
    }

}
