/*
 *  Qt5 HIDAPI read/write simple GUI
 *
 *  by jimmy.su, 2021
 *
 *  reference:
 *    https://github.com/todbot/hidpytoy
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <HIDAPI/hidapi.h>
#include <QDebug>
#include <QTimer>
#include <QLayout>
#include <QDateTime>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

#include "tempcolor/source/tempcolor.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	qDebug("HidQtToy Compiled with hidapi version %s, runtime version %s.\n", HID_API_VERSION_STR, hid_version_str());
	QString sV = QString(HID_API_VERSION_STR) + " [" + QString(hid_version_str()) + "]";
	this->setWindowTitle("HidQtToy w/ hidapi:" + sV);
	//ui->statusbar->showMessage("libHidApi runtime:" + QString(hid_version_str()));

	scanHID();
	timerHidRx = new QTimer(this);
	connect(timerHidRx, &QTimer::timeout, this, &MainWindow::hidRx);

	initTCSplot();
	ui->checkBoxTCS_Enable->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton)));
	//connect(this, &MainWindow::sigRGBlx, this, &MainWindow::slotGotRGBlx);
	//connect(this, &MainWindow::sigRGBCraw, this, &MainWindow::slotGotRGBCraw);
	connect(this, &MainWindow::sigHIDIO, this, &MainWindow::slotGotHIDIO);

	connect(this, &MainWindow::sigColorChange, this, &MainWindow::slotLedColorChange);

	//LCD digits
	ui->lcdNumberR->setStyleSheet("QLCDNumber{background-color:rgb(160, 0,  0);color:rgb(220, 220,  0);border:2px solid rgb(113, 113, 113);border-width: 2px;border-radius:4px;}");
	ui->lcdNumberG->setStyleSheet("QLCDNumber{background-color:rgb(0, 160,  0);color:rgb(220, 220,  0);border:2px solid rgb(113, 113, 113);border-width: 2px;border-radius:4px;}");
	ui->lcdNumberB->setStyleSheet("QLCDNumber{background-color:rgb(0,   0,160);color:rgb(220, 220,  0);border:2px solid rgb(113, 113, 113);border-width: 2px;border-radius:4px;}");
	ui->lcdNumberLX->setStyleSheet("QLCDNumber{background-color:rgb(40, 60, 40);border:2px solid rgb(113, 113, 113);border-width: 2px;border-radius:4px;}");
	ui->lcdNumberCT->setStyleSheet("QLCDNumber{background-color:rgb(10, 20, 10);border:2px solid rgb(113, 113, 113);border-width: 2px;border-radius:4px;}");

	updateCIEchart();


	//Adding NeoPixels
	NeoScene = new QGraphicsScene(this);
	NeoScene->setSceneRect(0, 0, iKLedNumbers * iKLsize, iKLedNumbers * iKLsize);
	ui->graphicsView->setScene(NeoScene);
	//GraphicView (0,0) at left-bottom
	//        |-y
	//        |
	//-x------+--------x
	//        |(0,0)
	//        |
	//        |y
	addNeoLEDs();

	createChecker4x6();
	on_checkBoxTCS_Lamp_clicked(false);

	//createTCSrgbw();
	//createTCSmatrix();
	//http://www.vendian.org/mncharity/dir3/blackbody/UnstableURLs/bbr_color.html 3000K~7000K
	//for (int k = 1000; k < 7200; k += 1000) {
	//	QColor c = temp.color(k, 1.0f);
	//	qDebug() << c.name();
	//	}

	QString sC = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,"
		     "stop:0    rgba(255, 185, 105,  33), "
		     "stop:0.25 rgba(255, 209, 163,  55),"
		     "stop:0.50 rgba(255, 231, 204,  77), "
		     "stop:0.75 rgba(255, 244, 237,  99),"
		     "stop:1    rgba(243, 243, 255, 128))";
	ui->labelTitle->setStyleSheet("background:" + sC + ";");

	//TCS config
	initTcsConfig();

	ui->frameLED->setStyleSheet("QFrame {background-color:#404050};QLabel color:Gray");
	ui->groupBoxLED->setStyleSheet("QGroupBox {background-color:#505060}");
	//TCS is OFF, set Manual LED Control ON; Allow Manual Control
	//ui->checkBoxLED_Control->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton)));
	ui->checkBoxLED_Control->setIcon(QIcon(QPixmap(":/connect1.png")));
	ui->frameLED->setEnabled(true);
	ui->frameLEDSet->setEnabled(true);
	ui->checkBoxLED_Control->setCheckState(Qt::Checked);

	ui->BrightnessSlider->setStyleSheet("QSlider {background-color:"
					    "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, "
					    "stop:0 rgba(64, 64, 80, 255), stop:1 rgba(255, 255, 255, 255));}");
}

MainWindow::~MainWindow()

{
	delete ui;
}

void MainWindow::initTCSplot()
{
	// QCustomPlot /////////////////////////////////////////////////////////////////////
	// generate some data:
	for (int i = 0; i < 4; i++) {
		vtime[i].resize(PLOT_SAMPLES);
		vdata[i].resize(PLOT_SAMPLES);

		for (int j = 0; j < PLOT_SAMPLES; j++) {
			vtime[i][j] = j - PLOT_SAMPLES;
			vdata[i][j] = (int)((i + j) * 3);
		}
	}
	// create graph and assign data to it:
	ui->plot->addGraph();
	ui->plot->graph(ePlotR)->setData(vtime[ePlotR], vdata[ePlotR]);
	ui->plot->graph(ePlotR)->setPen(QPen(Qt::red, 2.0, Qt::DashLine));
	//ui->plot->graph(ePlotR)->setBrush(QBrush(QColor(255, 0, 0, 100)));
	ui->plot->addGraph();
	ui->plot->graph(ePlotG)->setData(vtime[ePlotG], vdata[ePlotG]);
	ui->plot->graph(ePlotG)->setPen(QPen(Qt::darkGreen, 2.0, Qt::DashDotLine));
	//ui->plot->graph(ePlotG)->setBrush(QBrush(QColor(0, 255, 0, 100)));
	ui->plot->addGraph();
	ui->plot->graph(ePlotB)->setData(vtime[ePlotB], vdata[ePlotB]);
	ui->plot->graph(ePlotB)->setPen(QPen(Qt::blue, 2.0, Qt::DashLine));
	//ui->plot->graph(ePlotB)->setBrush(QBrush(QColor(0, 0, 255, 100)));

	ui->plot->addGraph();
	ui->plot->graph(ePlotC)->setData(vtime[ePlotC], vdata[ePlotC]);
	ui->plot->graph(ePlotC)->setPen(QPen(Qt::darkGray, 2.0, Qt::SolidLine));

	ui->plot->graph(ePlotR)->setData(vtime[ePlotR], vdata[ePlotR]);
	ui->plot->graph(ePlotG)->setData(vtime[ePlotG], vdata[ePlotG]);
	ui->plot->graph(ePlotB)->setData(vtime[ePlotB], vdata[ePlotB]);
	ui->plot->graph(ePlotC)->setData(vtime[ePlotC], vdata[ePlotC]);

	// give the axes some labels:
	ui->plot->xAxis->setLabel("time");
	ui->plot->yAxis->setLabel("RGB");
	// set axes ranges, so we see all data:
	ui->plot->xAxis->setRange(-(PLOT_SAMPLES + 5), +5);
	ui->plot->yAxis->setRange(-5, 260);

	ui->plot->setInteraction(QCP::iRangeDrag, true);
	ui->plot->setInteraction(QCP::iRangeZoom, true);
	ui->plot->axisRect()->setRangeZoom(Qt::Vertical);
	ui->plot->setEnabled(true);

	QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
	//timeTicker->setTimeFormat("%h:%m:%s");
	timeTicker->setTimeFormat("%m:%s");

	ui->plot->xAxis->setTicker(timeTicker);
	ui->plot->axisRect()->setupFullAxesBox();
	//ui->plot->yAxis->setRange(-1.2, 1.2);
	ui->plot->setBackground(QColor("#C0C0C0"));

	//Set second yAxis for Lux
	ui->plot->axisRect()->axis(QCPAxis::atRight, 0)->setTickLabels(true);
	ui->plot->axisRect()->axis(QCPAxis::atRight, 0)->setRange(0, 1024);
	ui->plot->axisRect()->axis(QCPAxis::atRight, 0)->setTickLabelColor(QColor(Qt::darkGray)); // add an extra axis on the left and color its numbers
	ui->plot->axisRect()->axis(QCPAxis::atRight, 0)->setLabel("Lux");

	// make left and bottom axes transfer their ranges to right and top axes:
	connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->xAxis2, SLOT(setRange(QCPRange)));
	connect(ui->plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->yAxis2, SLOT(setRange(QCPRange)));

	//test for adding data automatically
	//dataTimer = new QTimer();
	// setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
	//connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
	//dataTimer->start(1000); // Interval 0 means to refresh as fast as possible
	// /////////////////////////////////////////////////////////////////////////

	ui->groupBoxTCS->setEnabled(false);
	ui->groupBoxTCS->setStyleSheet("QGroupBox {background-color:#C0C0C0};");
}


void MainWindow::on_buttonReScan_clicked()
{
	scanHID();
}

void MainWindow::on_buttonConnect_clicked()
{
	//unsigned short vid, pid;

	if (!isHidConnected) {
		QString sText = ui->cbDevice->currentText();
		qDebug() << sText;

		QStringList slVidPid;
		const QRegularExpression hexRegExp(QStringLiteral("0[xX][0-9a-fA-F]{4}")); // regular expression that matches 4 hex digits
		for (QRegularExpressionMatchIterator i = hexRegExp.globalMatch(sText); i.hasNext();) { // iterate over all the matches
			const QRegularExpressionMatch hexMatch = i.next();
			qDebug() << "Hex: " << hexMatch.capturedRef(0) << " Dec: " << hexMatch.capturedRef(0).toUInt(Q_NULLPTR, 16); //capturedRef contains the part matched by the regular expression
			slVidPid << hexMatch.capturedRef(0).toLocal8Bit();
		}
		qDebug() << "VIDPID:" << slVidPid << slVidPid[0].toUInt(Q_NULLPTR, 16) << slVidPid[1].toUInt(Q_NULLPTR, 16);

		handleHID = hid_open(slVidPid[0].toUInt(Q_NULLPTR, 16), slVidPid[1].toUInt(Q_NULLPTR, 16), NULL);
		if (handleHID)
			hid_set_nonblocking(handleHID, 1); //to prevent blocking
	}else {       //disconnect device
		if (handleHID) {
			if (timerHidRx->isActive()) timerHidRx->stop();
			hid_close(handleHID);
			handleHID = NULL;
		}
	}

	if (handleHID) {
		isHidConnected = true;
		//ui->boxSendData->setEnabled(true);
		//ui->boxReceiveData->setEnabled(true);
		ui->tabWidget->setEnabled(true);

		ui->buttonConnect->setText("disconnect");
		timerHidRx->start(500);//500ms rx timer
		qDebug() << "HID Connected!";
		ui->statusbar->showMessage("HID Connected!");
		ui->cbSendData->setFocus();
		ui->groupBoxTCS->setEnabled(true);
	}else {
		isHidConnected = false;
		//ui->boxSendData->setEnabled(false);
		//ui->boxReceiveData->setEnabled(false);
		ui->tabWidget->setEnabled(false);

		ui->buttonConnect->setText("Connect");
		if (timerHidRx->isActive()) timerHidRx->stop();
		qDebug() << "HID disconnected!" << handleHID;
		ui->statusbar->showMessage("HID Disonnected!");
		ui->cbDevice->setFocus();
		ui->groupBoxTCS->setEnabled(false);
	}
}

void MainWindow::on_buttonSendOutReport_clicked()
{
	hidTx();
}

void MainWindow::on_buttonSendFeatureReport_clicked()
{
	int len = ui->spinSizeOut->value();
	if (handleHID) hidTxFeature(handleHID, outBuffer, len);
}

void MainWindow::on_pushButton_clicked()
{
	ui->textGetData->clear();
}


void MainWindow::on_checkBoxTCS_Enable_clicked(bool checked)
{
	if (!isHidConnected) return;

	if (checked) {
		hidCmd("1,0xF1");
		ui->statusbar->showMessage("[TCS]Starts!");
		isTCSenabled = true;
		ui->plot->setBackground(QColor("#E0E0E0"));
		ui->plot->replot();
		ui->checkBoxTCS_Enable->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton)));
		ui->groupBoxTCS->setStyleSheet("QGroupBox {background-color:#E8E8E8};");

		//TCS is ON, set Manual LED Control OFF; NOT Allow Manual Control
		ui->frameLED->setEnabled(false); //Neopixel update enabled
		ui->frameLEDSet->setStyleSheet("QLabel { background-color:#404050; color:#808080;}");
		ui->frameLEDSet->setEnabled(false);
		ui->checkBoxLED_Control->setEnabled(false);
		//ui->checkBoxLED_Control->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton)));
		ui->checkBoxLED_Control->setIcon(QIcon(QPixmap(":/connect0.png")));
		ui->checkBoxLED_Control->setCheckState(Qt::Unchecked);
	}else {
		hidCmd("1,0xF0");
		ui->statusbar->showMessage("[TCS]Stopped!");
		isTCSenabled = false;
		ui->plot->setBackground(QColor("#C0C0C0"));
		ui->plot->replot();
		ui->checkBoxTCS_Enable->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton)));
		ui->groupBoxTCS->setStyleSheet("QGroupBox {background-color:#C0C0C0};");

		//TCS is OFF, set Manual LED Control ON; Allow Manual Control
		ui->frameLED->setEnabled(true); //Neopixel update disabled
		ui->frameLEDSet->setEnabled(true);
		ui->frameLEDSet->setStyleSheet("QLabel { background-color:#606080;color:#C0C0C0;}");
		ui->checkBoxLED_Control->setEnabled(true);
		ui->checkBoxLED_Control->setCheckState(Qt::Checked);
		//ui->checkBoxLED_Control->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton)));
		ui->checkBoxLED_Control->setIcon(QIcon(QPixmap(":/connect1.png")));
	}
}

//
// 0xE0:TCS LED off, 0xE1:TCS LED on
//
void MainWindow::on_checkBoxTCS_Lamp_clicked(bool checked)
{
	if (!isHidConnected) return;

	if (checked) {
		hidCmd("1,0xE1");
		ui->statusbar->showMessage("[TCS]Lamp On!");
		ui->checkBoxTCS_Lamp->setIcon(QIcon(QPixmap(":/light1_256.png")));
	}else {
		hidCmd("1,0xE0");
		ui->statusbar->showMessage("[TCS]Lamp Off!");
		ui->checkBoxTCS_Lamp->setIcon(QIcon(QPixmap(":/light0_256.png")));
	}
}

/*
   void MainWindow::on_tabWidget_currentChanged(int index)
   {
   switch (index) {
   default:
   case 1:    //HID
      break;
   case 2:    //TCS
      break;
   case 3:    //CIE


         QSvgRenderer renderer(QString(":/CIE-1931.svg"));
         //QPixmap pm(ui->plotCIE->width(), ui->plotCIE->height());
         QPixmap pm(":/KL58Vaccin.jpg");
         pm.fill(0xaaA08080);
         QPainter painter(&pm);
         renderer.render(&painter, pm.rect());


         // give the axes some labels:
         //ui->plotCIE->addGraph();
         ui->plotCIE->xAxis->setLabel("x");
         ui->plotCIE->yAxis->setLabel("y");

         //ui->plotCIE->axisRect()->setBackgroundScaledMode(Qt::AspectRatioMode::KeepAspectRatio);
         //ui->plotCIE->axisRect()->setBackgroundScaled(true);
         //ui->plotCIE->axisRect()->setBackground(QBrush(QPixmap(":/CIE-1931.svg")));
         ui->plotCIE->xAxis->setRange(-1, 1);
         ui->plotCIE->yAxis->setRange(-1, 1);

         ui->plotCIE->axisRect()->setupFullAxesBox();
         ui->plotCIE->setBackground(QColor("#C0C0C0"));
         ui->plotCIE->replot();




      //axisRect->setBackground(QBrush(QPixmap(":/sun.png"))); // (":", not ".")
      break;
   }
   }
 */



void MainWindow::on_spinBoxIntergration_valueChanged(int arg1)
{
	char buf[16];
	if (isTCSenabled) {
		sprintf(buf, "2,0xF6,0x%02X", arg1 / 10); //Intergration Time 100~600 => 10~60
		qDebug() << "INT:" << QString(buf);
		hidCmd(buf);
	}
}

void MainWindow::on_comboBoxGain_currentIndexChanged(int index)
{
	char buf[16];
	if (isTCSenabled) {
		sprintf(buf, "2,0xF7,0x%02X", (index + 1));
		qDebug() << "Gain x:" << QString(buf);
		hidCmd(buf);
	}
}


