#ifndef CONFIG_WIDGET_H
#define CONFIG_WIDGET_H

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

class GpioWidget : public QWidget {
public:
	GpioWidget(GPIO& gpio, QWidget *parent = NULL) : 
		QWidget(parent),
		Gpio(gpio)
	{
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
	Q_OBJECT 
public:
	MoveDbgGroupBox(QString name, QWidget *parent = NULL) : 
		QGroupBox(name, parent)
	{
		QLabel *speedLabel = new QLabel(tr("移动速度"));
		speedBox = new QComboBox();
		for(int i = 0; i < 4; i++){
			QString s = "Speed";
			speedBox->addItem(s);
		}
		dir_positive = new QRadioButton(tr("正向"));
		dir_reverse  = new QRadioButton(tr("反向"));

		QGridLayout * dbgLayout = new QGridLayout;
		pulsePushButton = new QPushButton("移动固定(脉冲)");
	        pulseLineEdit = new QLineEdit;

	        movePushButton = new QPushButton("移动到(mm)");
	        moveLineEdit = new QLineEdit;

	        posPushButton = new QPushButton("位置");
		posLineEdit = new QLineEdit;
		
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
	QPushButton* GetPosPushButton(){
		return posPushButton;
	}
	
        QLineEdit* GetPosLineEdit(){
		return posLineEdit;
	}
	QPushButton* GetPulsePushButton(){
		return pulsePushButton;
	}
	
 	QPushButton * GetMovePushButton(){
		return movePushButton;
	}
	QLineEdit* GetPulseLineEdit(){
		return pulseLineEdit;
	}
	QLineEdit* GetMoveLineEdit(){
		return moveLineEdit;
	}
        QComboBox * GetSpeedBox(){
		return speedBox;
	}
	QRadioButton * GetRadioButton(){
		return dir_positive;
	}
private:
	QPushButton * posPushButton; 
	QLineEdit * posLineEdit;

	QPushButton * movePushButton; 
	QLineEdit * moveLineEdit;

	QLineEdit * pulseLineEdit;
        QPushButton * pulsePushButton;
	
	QRadioButton* dir_positive;
	QRadioButton* dir_reverse;

	QComboBox *speedBox;
};


#endif
