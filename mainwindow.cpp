// SPDX-License-Identifier: LGPL-3.0-only
// 许可声明：本文件依据 GNU LGPL v3 许可条款分发与使用。
// 完整许可文本见项目根目录：LPGL3.md

#include "mainwindow.h"
#include "Modbus/gt_modbus.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
    setWindowTitle("OMFT-Tool");
  // 注册 DeviceInfo 类型
  qRegisterMetaType<DeviceInfo>("DeviceInfo");
  // 注册 QList<DeviceInfo> 类型（这是解决你错误的核心）
  qRegisterMetaType<QList<DeviceInfo>>("QList<DeviceInfo>");
  qRegisterMetaType<QList<CLTDeviceInfo>>("QList<CLTDeviceInfo>");
  // 设置测试任务的控制阀内容
  pxTestWorkerHandler->startWorker();
  // 连接槽函数
  connect(pxTestWorkerHandler->pxTestLoop, &TestLoop::sendMethodToSerial,
          pxSerialWorkerUART_Handler, &SerialWorker::writeData);

  SerialWorkerInit();
  // GUI Table
  GUI_TableInit();

  // HTTP 客户端测试
//  pxHttpClient->postMesCheck( "<root><info SN=\"CE02_251120_10006\" STA=\"OMFT\"/></root>" );
  connect( pxTestWorkerHandler->pxTestLoop , &TestLoop::sendHttpParam , pxHttpClient , &HttpClient::postMesCheck );
  connect( pxHttpClient , &HttpClient::requestFinished , pxBrowserLogs , &Logs::LogBrowserWrite );

  ui->tableWidget->setItem(0 , 0 , new QTableWidgetItem("CE02_251205_10151")) ;
  ui->tableWidget->setItem(1 , 0 , new QTableWidgetItem("CE02_251205_10015")) ;
  for ( int i = 0 ; i < 8 ; i++) {
        QTableWidgetItem *item1 = ui->tableWidget->item(i,0) ;
        QString text = item1->text() ;
        qDebug()<<"第一列文本内容 : "<<text ;
  }
  // Table函数改良后测试
  qDebug()<<"返回查询到的行 : "<<findFirstColumnMatchRow( ui->tableWidget , "15" );

  ui->tableWidget->clearContents() ; // 清除所有Item，保留行数
  ui->tableWidget->setItem(0 , 0 , new QTableWidgetItem("HAHAH 00000")) ;
  ui->tableWidget->setItem(1 , 0 , new QTableWidgetItem("JJJJJ 11111")) ;

  pxTable->SetCellItem( 1 , 1 , "彻底失败" , TableControl::RED );
}

MainWindow::~MainWindow() {
  // 回收SerialWorker类
  if (pxSerialWorkerUART_Handler) {
    pxSerialWorkerUART_Handler->stopWorker();
  }

  if (pxTestWorkerHandler) {
    pxTestWorkerHandler->stopWorker();
  }

  // UI类回收
  ReleaseUiClasses();

  // 清空整个TableWidget
  ui->tableWidget->setRowCount(0) ;

  delete ui;
}

void MainWindow::SerialWorkerInit() {
  if (pxSerialWorkerUART_Handler == nullptr) {
    return;
  }
  // 启动 worker 线程
  pxSerialWorkerUART_Handler->startWorker();
  // 连接信号
  connect(pxSerialWorkerUART_Handler, &SerialWorker::sig_connectStatus, this,
          &MainWindow::onConnectStatusChanged);
  connect(pxSerialWorkerUART_Handler, &SerialWorker::sig_readData, this,
          &MainWindow::onSerialWorkerReadData);

  RefreshSerialPorts(); // 刷新串口

  connect(pxTimerReadSN, &QTimer::timeout, this,
          &MainWindow::onTimerTimeoutReadSN);
}

void MainWindow::onConnectStatusChanged(int status) {
  switch (status) {
  case BaseSerialPort::Connected: {
    ui->statusbar->showMessage("串口状态 : 设备端口连接成功");
    ui->pushButton_PortOpen->setText("串口已连接");
  } break;
  case BaseSerialPort::UnConnected: {
    ui->statusbar->showMessage("串口状态 : 设备端口断开连接");
    ui->pushButton_PortOpen->setText("串口已断开");
  } break;
  case BaseSerialPort::Error: {
    ui->statusbar->showMessage("串口状态 : 设备端口连接错误，请查看连接方式");
    ui->pushButton_PortOpen->setText("串口发生错误");
  } break;
  default: {
    ui->statusbar->showMessage("串口状态 : unkown");
  } break;
  }
}

void MainWindow::ResetButtonEnable()
{
    ui->pushButton->setEnabled(true) ;
    ui->pushButton_2->setEnabled(true) ;
    ui->pushButton_3->setEnabled(true) ;
    ui->pushButton_4->setEnabled(true) ;
}

void MainWindow::SetButtonDisable()
{
    pxTimerReadSN->stop();
    ui->pushButton->setDisabled(true) ;
    ui->pushButton_2->setDisabled(true) ;
    ui->pushButton_3->setDisabled(true) ;
    ui->pushButton_4->setDisabled(true) ;
}

void MainWindow::ReleaseUiClasses() {}

/**
 * @brief MainWindow::RefreshSerialPorts
 * 刷新搜索串口列表
 */
void MainWindow::RefreshSerialPorts() {
  QString currentSelection = ui->comboBox->currentText();
  ui->comboBox->clear();
  // comboBox
  QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo &portInfo : ports) {
    QString portDisplay = portInfo.portName();
    if (!portInfo.description().isEmpty()) {
      portDisplay += " - " + portInfo.description();
    }
    ui->comboBox->addItem(portDisplay, portInfo.portName());
  }
  // 尝试恢复之前的选择
  if (!currentSelection.isEmpty()) {
    int index = ui->comboBox->findText(currentSelection);
    if (index >= 0) {
      ui->comboBox->setCurrentIndex(index);
    }
  }
}

void MainWindow::GUI_TableInit() {
  // pxTable
  pxTable = new TableControl(ui->tableWidget, this);
  // 宽度适配
  ui->tableWidget->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  // 高度适配
  ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  // 日志系统
  pxBrowserLogs = new Logs(ui->textBrowser, this);

  /**
   * @brief 连接 Modbus 信号槽，更新大气压值
   * 此处除了更新大气压之外还应该判定大气压范围，同时根据调用的测试模式来更新窗口和判断标准
   */
  connect(&(pxTestWorkerHandler->pxTestLoop->GT_ModbusHandler),
          &GT_Modbus::sig_updateAirPressure, this,
          [=](int slaveID, int location, int value) {
            QByteArray recvSN = QByteArray::number(slaveID);
//            pxBrowserLogs->LogBrowserWrite("SN : " + recvSN + "-value : " + QByteArray::number( value ) );
            for (DeviceInfo &device : GT_DeviceList) {
              if (device.slaveID.contains(recvSN)) {
                if (location == GT_Modbus::AIR) {
                    pxBrowserLogs->LogBrowserWrite("SN : " + recvSN +" 传感器位置 : 大气压 " + " value : " + QByteArray::number( value ) );
                  device.airPress = value;
                  device.airPressUpdateFlag = true;
                } else if (location == GT_Modbus::INF) {
                    pxBrowserLogs->LogBrowserWrite("SN : " + recvSN +" 传感器位置 : 阀前压 " + " value : " + QByteArray::number( value ) );
                  device.infPress = value;
                  device.infPressUpdateFlag = true;
                } else if (location == GT_Modbus::END) {
                    pxBrowserLogs->LogBrowserWrite("SN : " + recvSN +" 传感器位置 : 阀后压 " + " value : " + QByteArray::number( value ) );
                  device.endPress = value;
                  device.endPressUpdateFlag = true;
                }
              }
            }
            onTableMapping();
          });

  connect(&(pxTestWorkerHandler->pxTestLoop->GT_ModbusHandler),
          &GT_Modbus::sig_updateSN, this, [=](QByteArray data) {
            // 在这里直接处理序列号更新逻辑
            DeviceInfo device;
            device.SN = data;
            if (data.size() > 2)
              device.slaveID = data.mid(data.size() - 2, data.size());
            GT_DeviceList.append(device);
            int size = GT_DeviceList.size();
            pxTable->SetCellItem(size, 1, GT_DeviceList.at(size - 1).SN , TableControl::GREEN);
            ui->lineEdit->setText("");

            for( int i=0;i<GT_DeviceList.size();i++ )
            {
                qDebug()<<"SN - "<<GT_DeviceList.at(i).SN;
                qDebug()<<"ID - "<<GT_DeviceList.at(i).slaveID ;
            }

            return ;
          });
  // 槽函数绑定，将结果显示在窗口内
  connect(&(pxTestWorkerHandler->pxTestLoop->GT_ModbusHandler),
          &GT_Modbus::sig_updateValveStatus, this,
          [=](uint8_t slaveID, uint8_t value) {
            // 在这里直接处理阀门状态更新逻辑
//            qDebug() << "从机地址:" << slaveID << "阀门状态:" << value;
//            qDebug() << "转换后查询地址 - " << QString::number(slaveID);
            for( DeviceInfo &device : GT_DeviceList )
            {
                if( device.slaveID.toUInt() == slaveID )
                {
                    device.sw_status = true ;
                    break ;
                }
            }
            // 遍历整个Table，然后查询到对应的item，value=0表示打开成功，1表示关闭成功
            int row = (findFirstColumnMatchRow(ui->tableWidget,QString::number(slaveID)));
            if( row == -1 ){return ;}
            if (value == 0) {
                qDebug()<<"绘制行数"<<row ;
              pxTable->SetCellItem(row+1, 4, "PASS" , TableControl::GREEN);
            } else {
              pxTable->SetCellItem(row+1, 4, "FAIL" , TableControl::RED);
            }
          });
    // 发送完毕执行复位操作
    connect( pxTestWorkerHandler->pxTestLoop , &TestLoop::readPressureComplete , this , &MainWindow::onResetTestFlag );
    // 日志打印
    connect( pxTestWorkerHandler->pxTestLoop , &TestLoop::logCurrentStep , pxBrowserLogs , &Logs::LogBrowserWrite );
    connect( pxSerialWorkerUART_Handler , &SerialWorker::logSendMessage , pxBrowserLogs , &Logs::LogBrowserWrite );
    // 点火开阀执行完毕 处理回调判定
    connect( pxTestWorkerHandler->pxTestLoop , &TestLoop::simulateIgnitionComplete , this , &MainWindow::checkAllSwitchStatus );
}

void MainWindow::onTimerTimeoutReadSN() {
  qDebug() << "触发定时器";
  if (pxTimerReadSN->isActive() && GT_DeviceList.size() >= 8) {
    pxTimerReadSN->stop();
    ui->pushButton_6->setText("开始监听");
  }
  QByteArray info = ui->lineEdit->text().toUtf8();
  QByteArray id = info.mid(info.size() - 2, info.size());
  // 根据扫码枪提取的数据，发送参数
  if (id.isEmpty()) {
    return;
  }
  bool ok = false;
  uint8_t tempSlaveID = id.toInt(&ok, 10);
//  qDebug() << "tempSlaveID - " << tempSlaveID << "status - " << ok;
  if (!ok) {
    return;
  }
  QByteArray qbyData =
      //      GT_ModbusHandler.GT_ModbusWrite(tempSlaveID, 0x03, 0x4045, NULL,
      //      0, NULL);
      pxTestWorkerHandler->pxTestLoop->GT_ModbusHandler.GT_ModbusWrite(
          tempSlaveID, 0x03, 0x4045, NULL, 0, NULL);
  HexPrintf(qbyData);
  pxSerialWorkerUART_Handler->writeData(qbyData);
}

/**
 * @brief 向所有控制阀设备从机发送指令
 * @param status
 */
void MainWindow::sendDeviceControlSignal(int slaveID, int status) {
  Q_UNUSED(status);
  Q_UNUSED(slaveID);
}

void MainWindow::onSerialWorkerReadData(const QByteArray &data) {
  QByteArray hexData = data;
  //  HexPrintf(hexData);
  // 在这里做接收的解析
  //    int ret = xModbsuFunc( hexData );
  //    qDebug()<<"ret - "<<ret ;
  pxBrowserLogs->LogBrowserWrite("RD : "+ data.toHex() );
  pxTestWorkerHandler->pxTestLoop->GT_ModbusHandler.GT_RetMsgVerify(hexData);
}

void MainWindow::on_pushButton_PortOpen_clicked() {
  if (ui->pushButton_PortOpen->text() != "串口已连接") {
    QString port = ui->comboBox->currentData().toString();
    pxSerialWorkerUART_Handler->openPort(port, 115200);
  } else {
    pxSerialWorkerUART_Handler->closePort();
  }
}

void MainWindow::on_pushButton_PortRefresh_clicked() { RefreshSerialPorts(); }

void MainWindow::on_pushButton_5_clicked() {
  pxTimerReadSN->stop();
  pxBrowserLogs->LogBrowserClear() ;
    pxTable->ClearAllItems();
    GT_DeviceList.clear();
}

void MainWindow::on_pushButton_6_clicked() {
  if (pxTimerReadSN) {
    if (!pxTimerReadSN->isActive()) {
      pxTimerReadSN->start(1000);
      ui->pushButton_6->setText("停止监听");
    } else {
      pxTimerReadSN->stop();
      ui->pushButton_6->setText("开始监听");
    }
  }
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1) {
  qDebug() << "触发扫码枪 - " << arg1;
  QByteArray data = arg1.toUtf8();
  qDebug() << "SLAVE ADDR - " << data.mid(data.size() - 2, data.size());
}

/**
 * @brief 欠压功能测试
 */
void MainWindow::on_pushButton_clicked() {
    DoTestFlag.QianYaTest = true;
//    pxTestWorkerHandler->onReadAllBoardPressure(GT_DeviceList);
    pxTestWorkerHandler->OMFT_LowPressureFunc( GT_DeviceList );
    SetButtonDisable();
}

void MainWindow::onTableMapping() {
  if (DoTestFlag.QianYaTest) {
    // 当执行完循环之后，执行标志位判定，判定都是TRUE则执行数据的相减
    for (DeviceInfo &device : GT_DeviceList) {
      if (device.airPressUpdateFlag && device.infPressUpdateFlag) {
        int pressDiff = device.infPress - device.airPress;
        if (pressDiff < 800) {
          pxTable->SetCellItem(
              (findFirstColumnMatchRow(ui->tableWidget, device.slaveID) + 1), 2,
              QString::number(pressDiff).toUtf8(), TableControl::GREEN);
          DeviceInfoReset(device);

          device.low_press_status = true ;

        } else {
          pxTable->SetCellItem(
              (findFirstColumnMatchRow(ui->tableWidget, device.slaveID) + 1), 2,
              QString::number(pressDiff).toUtf8() , TableControl::RED);
          DeviceInfoReset(device);

          device.low_press_status = false ;
        }
      }
    }
  } else if (DoTestFlag.ChaoYaTest) {
    for (DeviceInfo &device : GT_DeviceList) {
      if (device.airPressUpdateFlag && device.infPressUpdateFlag) {
        int pressDiff = device.infPress - device.airPress;
        if (pressDiff > 6000) {
          pxTable->SetCellItem(
              (findFirstColumnMatchRow(ui->tableWidget, device.slaveID) + 1), 3,
              QString::number(pressDiff).toUtf8() , TableControl::GREEN);
          DeviceInfoReset(device);
          device.over_press_status = true ;
        } else {
          pxTable->SetCellItem(
              (findFirstColumnMatchRow(ui->tableWidget, device.slaveID) + 1), 3,
              QString::number(pressDiff).toUtf8() ,  TableControl::RED);
          DeviceInfoReset(device);
          device.over_press_status = false ;
        }
      }
    }
  }
}

void MainWindow::onResetTestFlag()
{
    // 超时之后，应该先判定该SN是否受到过数据，如果收到过则忽略，如果没有则ERROR
    // 可以通过数据是否为0判断是否收到了数据
    for (DeviceInfo &device : GT_DeviceList)
    {
        // 出现了未收到的情况，直接报ERROR
        if( (device.airPress == 0 || device.infPress == 0) && DoTestFlag.ChaoYaTest != false )
        {
            pxTable->SetCellItem(
                (findFirstColumnMatchRow(ui->tableWidget, device.slaveID) + 1), 3,
                "FAIL" , TableControl::RED);
        }
        // 欠压查询
        if( (device.airPress == 0 || device.infPress == 0) && DoTestFlag.QianYaTest != false )
        {
            pxTable->SetCellItem(
                (findFirstColumnMatchRow(ui->tableWidget, device.slaveID) + 1), 2,
                "FAIL" , TableControl::RED);
        }
        // 销毁数据
        device.airPress = 0;
        device.infPress = 0;
        device.endPress = 0;
    }
    ResetButtonEnable();
    DoTestFlag.ChaoYaTest = false;
    DoTestFlag.QianYaTest = false;
}

/**
 * @brief 对所有的限位开关最终状态检测，如若存在未回传的则表示错误
 */
void MainWindow::checkAllSwitchStatus()
{
    for( DeviceInfo &device : GT_DeviceList )
    {
        if( device.sw_status == false )
        {
            pxTable->SetCellItem(
                (findFirstColumnMatchRow(ui->tableWidget, device.slaveID) + 1), 4,
                "FAIL" ,  TableControl::RED);
        }
    }
    ResetButtonEnable();
}

void MainWindow::on_pushButton_3_clicked()
{
    DoTestFlag.ChaoYaTest = true;
    pxTestWorkerHandler->OMFT_OverPressureFunc(GT_DeviceList);
    // 按钮互斥
    SetButtonDisable();
}

void MainWindow::on_pushButton_2_clicked()
{
    pxTestWorkerHandler->OMFT_OpenFireFunc(GT_DeviceList);
    // 按钮互斥
    SetButtonDisable();
}

/**
 * @brief 数据提交到服务器
 */
void MainWindow::on_pushButton_4_clicked()
{
//    QString info = InfoParser::generateXmlString("CE02_251120_10006" , "OMFT") ;
//    pxHttpClient->postMesCheck( info ) ;
    pxTestWorkerHandler->OMFT_SubmitTestResultToMES(GT_DeviceList) ;
}

void MainWindow::on_pushButton_7_clicked()
{
//    pxTestWorkerHandler->OMFT_ControlTaskInit() ;
    for( DeviceInfo &device : GT_DeviceList )
    {
        device.sw_status = false ;
        device.low_press_status = false ;
        device.over_press_status = false ;
    }
    pxTestWorkerHandler->onBuildTaskInfo( GT_DeviceList );
}
