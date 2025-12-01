// SPDX-License-Identifier: LGPL-3.0-only
// 许可声明：本文件依据 GNU LGPL v3 许可条款分发与使用。
// 完整许可文本见项目根目录：LPGL3.md

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // 注册 DeviceInfo 类型
  qRegisterMetaType<DeviceInfo>("DeviceInfo");
  // 注册 QList<DeviceInfo> 类型（这是解决你错误的核心）
  qRegisterMetaType<QList<DeviceInfo>>("QList<DeviceInfo>");

  SerialWorkerInit();
  // GUI Table
  GUI_TableInit();

  // 设置测试任务的控制阀内容
  pxTestWorkerHandler->startWorker() ;

  // 连接槽函数
  connect( pxTestWorkerHandler->pxTestLoop , &TestLoop::sendMethodToSerial , pxSerialWorkerUART_Handler , &SerialWorker::writeData );
}

MainWindow::~MainWindow() {
  // 回收SerialWorker类
  if (pxSerialWorkerUART_Handler) {
    pxSerialWorkerUART_Handler->stopWorker();
  }

  if( pxTestWorkerHandler )
  {
      pxTestWorkerHandler->stopWorker() ;
  }

  // UI类回收
  ReleaseUiClasses();

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

  //    ui->pushButton_6->setVisible(false) ;
  //    pxBrowserLogs->LogBrowserWrite("test code") ;
  connect(&GT_ModbusHandler, &GT_Modbus::sig_updateAirPressure, this,
          [=](int location, int value) {
            // 在这里直接处理大气压值更新逻辑
            qDebug() << "位置:" << location << "大气压值:" << value;
          });
  connect(&GT_ModbusHandler, &GT_Modbus::sig_updateSN, this,
          [=](QByteArray data) {
            // 在这里直接处理序列号更新逻辑
            DeviceInfo device;
            device.SN = data;
            if( data.size() > 2 )
            device.slaveID = data.mid( data.size()-2 , data.size() );
            GT_DeviceList.append(device);
            int size = GT_DeviceList.size();
            pxTable->SetCellItem(size, 1, GT_DeviceList.at(size - 1).SN);
            ui->lineEdit->setText("");
          });
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
  qDebug() << "tempSlaveID - " << tempSlaveID << "status - " << ok;
  if (!ok) {
    return;
  }
  QByteArray qbyData = GT_ModbusHandler.GT_ModbusWrite(tempSlaveID, 0x03, 0x4045, NULL, 0, NULL);
  HexPrintf(qbyData);
  pxSerialWorkerUART_Handler->writeData(qbyData);
}

/**
 * @brief 向所有控制阀设备从机发送指令
 * @param status
 */
void MainWindow::sendDeviceControlSignal( int slaveID , int status )
{
    Q_UNUSED(status) ;
    Q_UNUSED(slaveID) ;
}

void MainWindow::onSerialWorkerReadData(const QByteArray &data) {
  QByteArray hexData = data;
  HexPrintf(hexData);
  // 在这里做接收的解析
  //    int ret = xModbsuFunc( hexData );
  //    qDebug()<<"ret - "<<ret ;
  GT_ModbusHandler.GT_RetMsgVerify(hexData);
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
  //    QByteArray qbyData = GT_ModbusHandler.GT_ModbusWrite( 0xAA , 0x03 ,
  //    0xA0A0 , NULL , 0 , NULL ) ;
//  QByteArray qbyData =
//      GT_ModbusHandler.GT_ModbusWrite(0x0A, 0x03, 0x4012, NULL, 0, NULL);
//  HexPrintf(qbyData);
//  pxSerialWorkerUART_Handler->writeData(qbyData);

    pxTestWorkerHandler->onTakeStep1Test( GT_DeviceList ) ;
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
