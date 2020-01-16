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

enum IOAttri{
	Input,
	Output
};
class GPIO{
public:
	GPIO(IOAttri io, int vol, int freq, int s = 0){
		Status = 0;
		Attri = io;
		Voltage = vol;
		Freq = freq;
	}
	GPIO(){
	
	}
	int ReadIO(){
		return 0;
	}

	int WriteIO(int s){
		return -1;
	}
	int IsInput(){
		return (Attri == Input);
	}
	int IsOutput(){
		return 0;	
	}
protected:
	int Status;
	int Voltage;
	int Freq;
	IOAttri Attri;
};

enum FeedbackType{
	NoFeedback,
	Raster,
	Encoder
};

class RASTER{
public:
	RASTER(){
	}
	RASTER(FeedbackType type, int res){
		Type = type;
		Resolution = res;	
	}
private:
	int Channel;
	FeedbackType Type;
	int Resolution;
};

class LIMIT : public GPIO {
public:
	LIMIT(GPIO& gpio, int chanle, int s) : GPIO(gpio)
	{
	
	}

private:
	int NormalOpen;
	int FindWhenPoweron;
};

QStringList InputGpio = {
	"input0_24V_1M",
	"input1_24V_1M",
	"input2_24V_1M",
	"input3_24V_1M",
	"input4_5V_10M",
	"input5_5V_10M",
};
QStringList OnputGpio = {
	"onput0_24V_1M",
	"onput1_24V_1M",
	"onput2_24V_1M",
	"onput3_24V_1M",
	"onput4_5V_10M",
	"onput5_5V_10M",
};
class GpioWidget : public QWidget {
public:
	GpioWidget(GPIO& gpio, QWidget *parent = NULL) : 
		QWidget(parent),
		Gpio(gpio)
	{
		inputLabel = NULL;
		outputButton = NULL;

		Layout = new QHBoxLayout;

		gpioComboBox = new QComboBox;

		Layout->addWidget(gpioComboBox);
		if(gpio.IsInput()){
			gpioComboBox->addItems(InputGpio);
			inputLabel = new QLabel;
			inputLabel->setStyleSheet(m_green_SheetStyle);//改成 绿色圆形
			Layout->addWidget(inputLabel);
		}else{
			gpioComboBox->addItems(OnputGpio);
			outputButton = new QPushButton("关闭");
			Layout->addWidget(outputButton);
		}

		//->setStyleSheet(m_red_SheetStyle);//改成 红色圆形
		//->setStyleSheet(m_grey_SheetStyle);//改成 黄色圆形
		//->setStyleSheet(m_yellow_SheetStyle);//改成 灰色圆形

		setLayout(Layout);
	}
	int SelectChannel(int ch){	
		gpioComboBox->setCurrentIndex(ch);
	}
private:
	QHBoxLayout *Layout;
	QComboBox * gpioComboBox;
	QPushButton * outputButton;
	QLabel * inputLabel;
	GPIO Gpio;
	const QString m_red_SheetStyle = "min-width: 16px; min-height: 16px;max-width:16px; max-height: 16px;border-radius: 8px;  border:1px solid black;background:red";
	const QString m_green_SheetStyle = "min-width: 16px; min-height: 16px;max-width:16px; max-height: 16px;border-radius: 8px;  border:1px solid black;background:green";
	const QString m_grey_SheetStyle = "min-width: 16px; min-height: 16px;max-width:16px; max-height: 16px;border-radius: 8px;  border:1px solid black;background:grey";
	const QString m_yellow_SheetStyle = "min-width: 16px; min-height: 16px;max-width:16px; max-height: 16px;border-radius: 8px;  border:1px solid black;background:yellow";
};
class LimitGroupBox : public QGroupBox{
public:
	LimitGroupBox(QString name, GPIO& gpio, int chanle, int s, QWidget *parent = NULL) :
		QGroupBox(name, parent),
		Limit(gpio, chanle, s)

	{
		gpioLabel = new QLabel;
		gpioWidget = new GpioWidget(gpio);
		orgCheck = new QCheckBox(tr("上电初始化"));
		orgPushButton = new QPushButton(tr("找限位"));
		//endCheck = new QCheckBox(tr("常开"));
	
		QRadioButton * normal_open   = new QRadioButton(tr("常开"));
		QRadioButton * normal_close  = new QRadioButton(tr("常闭"));

		Layout = new QGridLayout;
		Layout->addWidget(gpioWidget,		0, 0, 1, 2);
		Layout->addWidget(gpioLabel,		0, 1);

		Layout->addWidget(normal_open,		1, 0);
		Layout->addWidget(normal_close,		1, 1);

		Layout->addWidget(orgCheck,		2, 0);
		Layout->addWidget(orgPushButton,	3, 0);

		setLayout(Layout);
	}
private:
	QLabel * gpioLabel;
	GpioWidget *gpioWidget;

	QCheckBox *orgCheck;
	QPushButton *orgPushButton;

	QCheckBox *endCheck;
	QGridLayout * Layout;

	LIMIT Limit;
};

class RasterGroupBox : public QGroupBox{
public:
	RasterGroupBox(QString name, RASTER& raster, QWidget *parent = NULL) :
		QGroupBox(name, parent),
		Raster(raster)
	{
		QGridLayout * Layout = new QGridLayout;
		QComboBox *rasterComboBox = new QComboBox();
		rasterComboBox->addItem("NULL");
		rasterComboBox->addItem("光栅");
		rasterComboBox->addItem("编码器");
	
		QLabel * channelLabel = new QLabel("Channel");
		QComboBox *channelComboBox = new QComboBox();
		channelComboBox->addItem("Channel0");
		channelComboBox->addItem("Channel1");
		channelComboBox->addItem("Channel2");

		QLabel * typeLabel = new QLabel("Type");
		QLabel * rasterLabel = new QLabel("分辨率");
		QLineEdit * rasterLineEdit = new QLineEdit;

		Layout->addWidget(typeLabel,		0, 0);
		Layout->addWidget(rasterComboBox,	0, 1);
		Layout->addWidget(channelLabel,		1, 0);
		Layout->addWidget(channelComboBox,	1, 1);
		Layout->addWidget(rasterLabel,		2, 0);
		Layout->addWidget(rasterLineEdit,	2, 1);

		setLayout(Layout);
	}
private:
	RASTER Raster; 
};

class MoterGroupBox : public QGroupBox{
public:
	MoterGroupBox(QString name, GPIO& dir, GPIO& pulse, QWidget *parent = NULL) :
		QGroupBox(name, parent),
		Dir(dir),
		Pulse(pulse)
	{
		QGridLayout * Layout = new QGridLayout;
	
		dirLabel = new QLabel("Dir");
		dirWidget = new GpioWidget(dir);

		pulseLabel = new QLabel("Pulse");
		pulseWidget = new GpioWidget(pulse);

		Layout->addWidget(dirLabel,	0, 0);
		Layout->addWidget(dirWidget,	0, 1);
		Layout->addWidget(pulseLabel,	1, 0);
		Layout->addWidget(pulseWidget,	1, 1);

		setLayout(Layout);
	}

private:
	GPIO Dir;
	GPIO Pulse;

	QLabel * dirLabel;
	QLabel * pulseLabel;
	GpioWidget * dirWidget;
	GpioWidget * pulseWidget;
};

class MoterParamGroupBox : public QGroupBox{
public:
	MoterParamGroupBox(QString name, QWidget *parent = NULL) : 
		QGroupBox(name, parent)
	{
		QLabel * maxfreqLabel = new QLabel("最高频率");
		QLabel * minfreqLabel = new QLabel("启动频率");
		QLabel * torqueLabel = new QLabel("力矩");
		QLabel * accLabel = new QLabel("加速度");
		QLabel * subdivLabel = new QLabel("细分数");

		QLineEdit * maxfreqLineEdit = new QLineEdit;
		QLineEdit * minfreqLineEdit = new QLineEdit;
		QLineEdit * torqueLineEdit = new QLineEdit;
		QLineEdit * accLineEdit = new QLineEdit;
		QLineEdit * subdivLineEdit = new QLineEdit;


		QGridLayout * Layout = new QGridLayout;
		Layout->addWidget(maxfreqLabel, 	0, 0);
		Layout->addWidget(maxfreqLineEdit, 	0, 1);
		Layout->addWidget(minfreqLabel, 	1, 0);
		Layout->addWidget(minfreqLineEdit, 	1, 1);
		Layout->addWidget(torqueLabel, 		2, 0);
		Layout->addWidget(torqueLineEdit, 	2, 1);
		Layout->addWidget(accLabel, 		3, 0);
		Layout->addWidget(accLineEdit, 		3, 1);
		Layout->addWidget(subdivLabel, 		4, 0);
		Layout->addWidget(subdivLineEdit, 	4, 1);

		setLayout(Layout);
	}
private:

};

class MoveDbgGroupBox : public QGroupBox{
public:
	MoveDbgGroupBox(QString name, QWidget *parent = NULL) : 
		QGroupBox(name, parent)
	{
		QLabel *speedLabel = new QLabel(tr("移动速度"));
		QComboBox * speedBox = new QComboBox();
		for(int i = 0; i < 4; i++){
			QString s = "Speed";
			speedBox->addItem(s);
		}
		QRadioButton * dir_positive = new QRadioButton(tr("正向"));
		QRadioButton * dir_reverse  = new QRadioButton(tr("反向"));

		QGridLayout * dbgLayout = new QGridLayout;
		QPushButton * pulsePushButton = new QPushButton("移动固定(脉冲)");
		QLineEdit * pulseLineEdit = new QLineEdit;
		QPushButton * movePushButton = new QPushButton("移动到(mm)");
		QLineEdit * moveLineEdit = new QLineEdit;
		QPushButton * posPushButton = new QPushButton("位置");
		QLineEdit * posLineEdit = new QLineEdit;
		QPushButton * gearPushButton = new QPushButton("齿轮比");
		QLineEdit * gearLineEdit = new QLineEdit;

		dbgLayout->addWidget(speedLabel, 	0, 0);
		dbgLayout->addWidget(speedBox, 		0, 1);
		dbgLayout->addWidget(dir_positive, 	0, 3);
		dbgLayout->addWidget(dir_reverse, 	0, 4);
		dbgLayout->addWidget(pulsePushButton,	1, 0);
		dbgLayout->addWidget(pulseLineEdit,	1, 1, 1, 4);
		dbgLayout->addWidget(movePushButton,	2, 0);
		dbgLayout->addWidget(moveLineEdit,	2, 1, 1, 4);
		dbgLayout->addWidget(posPushButton,	3, 0);
		dbgLayout->addWidget(posLineEdit,	3, 1, 1, 4);
		dbgLayout->addWidget(gearPushButton,	4, 0);
		dbgLayout->addWidget(gearLineEdit,	4, 1, 1, 4);

		setLayout(dbgLayout);
	}

private:
};

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    //m_console(new Console),
    m_settings(new SettingsDialog),
    m_serial(new QSerialPort(this))
{
	mainwidget = new QWidget;
	QGridLayout * mainlayout = new QGridLayout;
	QLabel *axisLabel = new QLabel(tr("移动轴"));
	QComboBox * axisBox = new QComboBox();
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

	MoveDbgGroupBox * dbgGroupBox = new MoveDbgGroupBox("运动调试");
	MoterParamGroupBox * paramGroupBox = new MoterParamGroupBox("电机参数");

	mainlayout->addWidget(axisLabel, 		0, 0);
	mainlayout->addWidget(axisBox, 			0, 1);

	mainlayout->addWidget(cfgGroupBox, 		1, 0, 1, 2);
	mainlayout->addWidget(dbgGroupBox, 		2, 0);
	mainlayout->addWidget(paramGroupBox, 		2, 1);

	mainwidget->setLayout(mainlayout);

    m_ui->setupUi(this);
    //m_console->setEnabled(false);
    //setCentralWidget(m_console);
    setCentralWidget(mainwidget);

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);

    m_ui->statusBar->addWidget(m_status);

    initActionsConnections();

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    //connect(m_console, &Console::getData, this, &MainWindow::writeData);
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

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
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

//! [8]
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
