/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "aes.h"
#include "sha256.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"

#include <QLabel>
#include <QMessageBox>
#include <QComboBox>
#include <QGridLayout>
#include <QRadioButton>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include  <QDebug>
#include <QFileDialog>
#include <QTimer>

using namespace std;

void hexdump(const uint8_t *p, unsigned int len);

QByteArray HexStringToByteArray(QString HexString)
  {
      bool ok;
      QByteArray ret;
      HexString = HexString.trimmed();
      HexString = HexString.simplified();
      QStringList sl = HexString.split(" ");
 
      foreach (QString s, sl) {
         if(!s.isEmpty()) {
             char c = s.toInt(&ok,16)&0xFF;
             if(ok){
                 ret.append(c);
             }else{
                 qDebug()<<"非法的16进制字符："<<s;
             }
         }
     }
     return ret;
 }




//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    //m_console(new Console),
    m_settings(new SettingsDialog)
{
	m_serial = new MotionUart(0x80, this);

	mainwidget = new QWidget;
	QGridLayout * mainlayout = new QGridLayout;
	QLabel *axisLabel = new QLabel(tr("移动轴"));
	axisBox = new QComboBox();
	axisBox->addItem("x轴");
	axisBox->addItem("y0轴");
	axisBox->addItem("y1轴");
	axisBox->addItem("z轴");
	for(int i = 5; i < 8; i++){
		QString s = "轴";
		axisBox->addItem(s);
	}

	QGroupBox * cfgGroupBox = new QGroupBox("参数配置");
	QGridLayout * cfgLayout = new QGridLayout;

	GPIO dir(Output, 5, 1);
	GPIO pulse(Output, 5, 10);
	MoterGroupBox *moterGroupBox = new MoterGroupBox("Moter", dir, pulse);

	GPIO gpio;
	LimitGroupBox * orgLimit = new LimitGroupBox("原点", gpio, 0, 0);
	LimitGroupBox * endLimit = new LimitGroupBox("终点", gpio, 0, 0);

	RASTER raster;
	RasterGroupBox *rasterGroupBox = new RasterGroupBox("光栅/编码器", raster);


	cfgLayout->addWidget(moterGroupBox,	0, 0);
	cfgLayout->addWidget(rasterGroupBox,	0, 1);
	cfgLayout->addWidget(orgLimit,		0, 2);
	cfgLayout->addWidget(endLimit,		0, 3);

	cfgGroupBox->setLayout(cfgLayout);

	dbgGroupBox = new MoveDbgGroupBox("运动调试");
	MoterParamGroupBox * paramGroupBox = new MoterParamGroupBox("电机参数");

	mainlayout->addWidget(axisLabel, 		0, 0);
	mainlayout->addWidget(axisBox, 			0, 1);

	mainlayout->addWidget(cfgGroupBox, 		1, 0, 1, 2);
	mainlayout->addWidget(dbgGroupBox, 		2, 0);
	mainlayout->addWidget(paramGroupBox, 		2, 1);

	mainwidget->setLayout(mainlayout);

    
	m_ui->setupUi(this);
    	//_console->setEnabled(false);
    	//setCentralWidget(m_console);
    	setCentralWidget(mainwidget);

	m_ui->actionConnect->setEnabled(true);
	m_ui->actionDisconnect->setEnabled(false);
	m_ui->actionQuit->setEnabled(true);
	m_ui->actionConfigure->setEnabled(true);
	m_ui->statusBar->addWidget(m_status);

	initActionsConnections();
	
	connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
	connect(dbgGroupBox->GetPosPushButton(), SIGNAL(clicked()), this, SLOT(GetPostion()));
	connect(dbgGroupBox->GetMovePushButton(), SIGNAL(clicked()), this, SLOT(MoveTo()));
	connect(dbgGroupBox->GetPulsePushButton(), SIGNAL(clicked()), this, SLOT(MovePulse()));

	//connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
	// qDebug()<<ByteArrayToHexString(cmdBA);
}

MainWindow::~MainWindow()
{
    delete m_settings;
    delete m_ui;
}

//! [4]
void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        //m_console->setEnabled(true);
        mainwidget->setEnabled(true);
        //m_console->setLocalEchoEnabled(p.localEchoEnabled);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

	if(m_serial->ReadCarStatus()){
		printf("read status error");
	}
	//
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    //m_console->setEnabled(false);
    mainwidget->setEnabled(false);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}
//! [5]

#define		UI_VERSION		"1.0.0"
void MainWindow::about()
{
	char info[512];
	sprintf(info, "UI Version: %s\nBuild Date: %s\nBuild Time: %s\nBoard Id:\nProduct Id:\nVender Id:", UI_VERSION,  __DATE__, __TIME__);

	QMessageBox::about(this, tr("版本信息"), info);
}

//! [6]
void MainWindow::writeData( const QByteArray &data)
{
    m_serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    const QByteArray data = m_serial->readAll();
    //m_console->putData(data);
}
//! [7]
void MainWindow::GetPostion()
{
	//m_serial->MoveTo(0, 0, 0, 12800);
	int pos = 0;
	if(m_serial->ReadCarPos(0, &pos) == 0){
		dbgGroupBox->GetPosLineEdit()->setText(QString::number(pos));
	}

}

void MainWindow::MoveTo()
{
	int axis  = axisBox->currentIndex();
	int speed = dbgGroupBox->GetSpeedBox()->currentIndex();
	int dir = dbgGroupBox->GetRadioButton()->isChecked(); 
	int dist = dbgGroupBox->GetMoveLineEdit()->text().toInt();
	qDebug() << "dist" << dist;
       //	if(m_serial->MoveTo())		
       if( m_serial->MoveTo(axis,speed,dir,dist))
       {

       }
}
void MainWindow::MovePulse()
{
	int axis  = axisBox->currentIndex();
	int speed = dbgGroupBox->GetSpeedBox()->currentIndex();
	int dir = dbgGroupBox->GetRadioButton()->isChecked(); 
	int dist = dbgGroupBox->GetMoveLineEdit()->text().toInt();
       //	if(m_serial->MoveTo())		
	qDebug() << "dist" << dist;
	if( m_serial->MoveTo(axis,speed,dir,dist)){

	}
}//! [8]

struct UpdataPackage{
	unsigned int Header;
	unsigned int Length;
	unsigned int Target;
	unsigned int Offset;
	unsigned int Ency;
	unsigned char Hash[32];
};

int MainWindow::UpdataProcess(QString& file)
{
	int ret = -1;
	std::string str = file.toStdString();
	
	FILE * fp = fopen(str.c_str(), "rb");
	if(fp != NULL){
		UpdataPackage header;
		int size = fread(&header, 1, sizeof(header), fp);
		if(size != sizeof(header)){

		}

		char * key = "this is a 16 key";
		unsigned char iv[16];

		int offset = header.Offset - 16 - sizeof(header);
		unsigned char * ency_buf = new unsigned char [1024 * 1024];
		unsigned char * data = new unsigned char [1024 * 1024];

		size = fread(ency_buf, 1, offset, fp);
		if(size != offset){
			qDebug() << "error offset:" << size;
		}
		size = fread(iv, 1, 16, fp);
		if(size != 16){
			qDebug() << "error iv:" << size;
		}

		WORD key_schedule[60];
		size = fread(ency_buf, 1, 1024 * 1024, fp);
		fclose(fp);

		aes_key_setup((uint8_t*)key, key_schedule, 128);
		aes_decrypt_cbc(ency_buf, size, data, key_schedule, 128, iv);
		
		BYTE buf[SHA256_BLOCK_SIZE];

		SHA256_CTX ctx;
		sha256_init(&ctx);
		sha256_update(&ctx, data, header.Length);
		sha256_final(&ctx, buf);

		if(memcmp(buf, header.Hash, 32) == 0){
#if 1
			//QTimer * timer = new QTimer(this);
			//connect(timer, SIGNAL(timeout()), this,  SLOT(UpdateTimer()));
			//timer->start(1000);

			ret = 0;
			if(m_serial->UpdateStart(header.Length)){
				ret = 1;
			}
			if(ret || m_serial->UpdateFirmwear(data, header.Length)){
				ret = 2;
			}
			if(ret || m_serial->UpdateFinish(header.Hash, 32)){
				ret = 3;
			}
			//timer->stop();
			//disconnect(timer, SIGNAL(timeout()), this,  SLOT(UpdateTimer()));
#else
			fp = fopen("./check.bin", "wb+");
			if(fp != NULL){
				fwrite(data, 1, header.Length, fp);
				fclose(fp);
			}
#endif
		}else{
			qDebug() << "hash check error";
		}

		delete ency_buf;
		delete data;
	}

	return ret;
}

void MainWindow::Update()
{
	QFileDialog *fileDialog = new QFileDialog();
	fileDialog->setWindowTitle("选择文件");
	fileDialog->setDirectory(".");
	fileDialog->setFileMode(QFileDialog::ExistingFiles);
	
	QDir dir("./");
	dir.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	QFileInfoList list = dir.entryInfoList();

	QString filename;
	if(fileDialog->exec()){
		filename = fileDialog->selectedFiles()[0];
		qDebug() << "selected file is" << filename;
		
		if(UpdataProcess(filename)){
			QMessageBox::warning(NULL, 
				tr("ERROR"), "升级失败", QMessageBox::Ok);
		}else{
			QMessageBox::warning(NULL, 
				tr("WARNING"), "升级成功", QMessageBox::Ok);
		}
	}

}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionUpdate, &QAction::triggered, this, &MainWindow::Update);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    //connect(m_ui->actionClear, &QAction::triggered, m_console, &Console::clear);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::UpdateTimer()
{
	qDebug() << "update percent " << m_serial->GetUpdateProcess();
	m_status->setText(QString::number(m_serial->GetUpdateProcess()));
}
