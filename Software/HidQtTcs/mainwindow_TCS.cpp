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

float ref_X = 95.047;
float ref_Y = 100.0;
float ref_Z = 108.883;
void convertRGBtoXYZ(int inR, int inG, int inB, float * outX, float * outY, float * outZ)
{
	float var_R = (inR / 255.0f);  //R from 0 to 255
	float var_G = (inG / 255.0f);  //G from 0 to 255
	float var_B = (inB / 255.0f);  //B from 0 to 255

	if (var_R > 0.04045f)
		var_R = powf(((var_R + 0.055f) / 1.055f), 2.4f);
	else
		var_R = var_R / 12.92f;

	if (var_G > 0.04045)
		var_G = powf(((var_G + 0.055f) / 1.055f), 2.4f);
	else
		var_G = var_G / 12.92f;

	if (var_B > 0.04045f)
		var_B = powf(((var_B + 0.055f) / 1.055f), 2.4f);
	else
		var_B = var_B / 12.92f;

	var_R = var_R * 100;
	var_G = var_G * 100;
	var_B = var_B * 100;

	//Observer. = 2°, Illuminant = D65
	*outX = var_R * 0.4124f + var_G * 0.3576f + var_B * 0.1805f;
	*outY = var_R * 0.2126f + var_G * 0.7152f + var_B * 0.0722f;
	*outZ = var_R * 0.0193f + var_G * 0.1192f + var_B * 0.9505f;
}

void MainWindow::slotGotRGBCraw(uint8_t *data)
{
	//QByteArray temp = QByteArray::fromRawData((const char*)data, 8);
	//QString sO = QString(temp.toHex());

	uint16_t iRraw = data[0] * 256 + data[1];
	uint16_t iGraw = data[2] * 256 + data[3];
	uint16_t iBraw = data[4] * 256 + data[5];
	uint16_t iCraw = data[6] * 256 + data[7];
	//qDebug() << "Got RGBCraw:" << sO << iRraw << iGraw << iBraw << iCraw;
	if (ui->tabTCS->isVisible()) {
		ui->labelRawR->setText(QString("Raw R:%1").arg(iRraw, 5));
		ui->labelRawG->setText(QString("Raw G:%1").arg(iGraw, 5));
		ui->labelRawB->setText(QString("Raw B:%1").arg(iBraw, 5));
		ui->labelRawC->setText(QString("Raw C:%1").arg(iCraw, 5));
	}
}


//
// (|01|F1|)| r | g | b | lx | ct | INT | gain |
//
void MainWindow::slotGotRGBlx(uint8_t *data)
{
	//QByteArray temp = QByteArray::fromRawData((const char*)data, 16);
	//QString sO = QString(temp.toHex());
	int r, g, b, lx, ct;
	r = *data; g = *(data + 1); b = *(data + 2); lx = *(data + 3); ct = *(data + 4);
	int iTimeINT = *(data + 5) * 10;
	int iGain = *(data + 6);
	//qDebug() << r << g << b << lx;

	if (ui->tabTCS->isVisible()) {
		double timeInt = ui->spinBoxIntergration->value();

		//qDebug() << "Got RGBlxCt:" << sO << iCT << iTimeINT << iGain;

		// add data to lines:
		static QTime time(QTime::currentTime());
		// calculate two new data points:

		double key = 0;
		if (timeInt > 0)
			key = time.elapsed() / timeInt;
		//qDebug() << key;

		static double lastPointKey = 0;
		if ((key - lastPointKey) > 0.5f) { // at most add point every 1/2sec
			for (int i = 0; i < 4; i++) {//rgb lx
				if (key > 10) //to keep only 10 samples
					ui->plot->graph(i)->data()->removeBefore(key - 10);
			}
			ui->plot->graph(ePlotR)->addData(key, r);
			ui->plot->graph(ePlotG)->addData(key, g);
			ui->plot->graph(ePlotB)->addData(key, b);
			ui->plot->graph(ePlotC)->addData(key, lx);

			// make key axis range scroll with the data (at a constant range size of 10):
			ui->plot->xAxis->setRange(key, 10, Qt::AlignRight);
			ui->plot->replot();
			lastPointKey = key;
			qDebug() << "qcp key added:" << key;
		}
		//}
		//qDebug() << "update LCD";
		//if (ui->tabTCS->isVisible()) {
		if (r >= 0) ui->lcdNumberR->display(r);
		if (g >= 0) ui->lcdNumberG->display(g);
		if (b >= 0) ui->lcdNumberB->display(b);
		if (lx >= 0) ui->lcdNumberLX->display(lx * 4);
		if (ct > 0) ui->lcdNumberCT->display(ct * 100); //°K

		char buf[24];
		sprintf(buf, "Integration Time:% 3dms", iTimeINT);
		ui->labelTimeINT->setText(QString(buf));
		sprintf(buf, "Gain:x%02d", iGain);
		ui->labelGain->setText(QString(buf));

		bool isPoint = ui->lcdNumberLX->smallDecimalPoint();
		if (isPoint) ui->lcdNumberLX->setSmallDecimalPoint(false);
		else ui->lcdNumberLX->setSmallDecimalPoint(true);
	}
	if (ui->tabCIE->isVisible()) {
		float X, Y, Z;
		convertRGBtoXYZ(r, g, b, &X, &Y, &Z);
		qDebug() << "x/y" << X / (X + Y + Z) << Y / (X + Y + Z);
		char buf[24];
		sprintf(buf, "% 5.2f", X);
		ui->labelX->setText(QString(buf));
		sprintf(buf, "% 5.2f", Y);
		ui->labelY->setText(QString(buf));
		sprintf(buf, "% 5.2f", Z);
		ui->labelZ->setText(QString(buf));

		float x, y;
		x = X / (X + Y + Z);
		y = Y / (X + Y + Z);
		sprintf(buf, "(% 5.3f, % 5.3f)", x, y);
		ui->labelxy->setText(QString(buf));

		plotCircle(x, y);
	}
	if (ui->tabLED->isVisible()) {
		QColor color(r, g, b);
		emit sigColorChange(color, lx);
	}
	if (ui->tabChecker->isVisible()) {
		char buf[32];
		sprintf(buf, "(% 3d,% 3d, % 3d)", r, g, b);
		ui->labelCheckerRGB->setText(QString(buf));
		ui->labelCheckerLux->setText(QString::number(lx * 4));

		float X, Y, Z;
		convertRGBtoXYZ(r, g, b, &X, &Y, &Z);
		sprintf(buf, "(% 5.2f,% 5.2f,% 5.2f)", X, Y, Z);
		ui->labelCheckerXYZ->setText(QString(buf));

		float x, y;
		x = X / (X + Y + Z);
		y = Y / (X + Y + Z);
		sprintf(buf, "(% 5.3f, % 5.3f)", x, y);
		ui->labelCheckerxy->setText(QString(buf));


		QColor c(r, g, b);
		QColor cx = c.lighter(75 + lx * 3);
		QString sColor = "QGroupBox { background-color:" + cx.name() + "; border:  none}"
				 "QGroupBox::title{background-color:#E0E0E0;subcontrol-origin:margin;padding:0 200 10 0;}";
		ui->groupBoxCheckerSensor->setStyleSheet(sColor);
	}
}


void MainWindow::slotGotHIDIO(uint8_t *data, int io_result)
{
	QByteArray temp = QByteArray::fromRawData((const char*)data, io_result);
	QString sO = QString(temp.toHex());
	qDebug() << "[HID IO]:" << sO << io_result;


	//The TCS HIDIO composed by rgb lx INT gain, raw data
	// | ReportID | 0xF1 | RGB lx Temp INT gain | raw |
	//dispatch RGBlxTemp 7 bytes
	slotGotRGBlx(data + 2);
	//dispathch RGBCraw  8 bytes
	slotGotRGBCraw(data + 9);
}

void MainWindow::initTcsConfig()
{
	QStringList listIntergrationTime, listGain;
	listGain << "x01" << "x04" << "x16" << "x60";

	ui->comboBoxGain->addItems(listGain);
	ui->comboBoxGain->setCurrentIndex(1); //x4
}


/*
   QList<QColor>  listRGBWcolor = {
   QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), QColor(255, 255, 255) };

   void MainWindow::createTCSrgbw()
   {
   int row = 2;
   int col = 2;
   QPushButton *btn[row][col];

   ui->widgetRGBW->setStyleSheet("QWidget { background: darkGray; }");

   for (int r = 0; r < row; r++)
      for (int c = 0; c < col; c++) {
         int idx = (r * col + c);
         btn[r][c] = new QPushButton();
         btn[r][c]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
         QString sColor = "QPushButton { background-color:" + listRGBWcolor[idx].name() + ";}";
         btn[r][c]->setStyleSheet(sColor);

         ui->gridLayoutRGBW->addWidget(btn[r][c], r, c);
         ui->gridLayoutRGBW->setContentsMargins(12, 0, 12, 12);

         connect(btn[r][c], &QPushButton::clicked, [ = ]()
         {
            qDebug() << "clicked" << r << c << listRGBWcolor[idx];
         });
      }

   ui->gridLayoutRGBW->setSpacing(16);
   }

   void MainWindow::createTCSmatrix()
   {
   int row = 3;
   int col = 3;
   QLineEdit *le[row][col];

   ui->widgetMatrix->setStyleSheet("QWidget { background: lightGray; }");

   for (int r = 0; r < row; r++)
      for (int c = 0; c < col; c++) {
         int idx = (r * col + c);
         le[r][c] = new QLineEdit();
         le[r][c]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
         le[r][c]->setAlignment(Qt::AlignHCenter);
         if (r == c) le[r][c]->setText("1.00");
         else le[r][c]->setText("0.00");

         //QString sColor = "QPushButton { background-color:" + listRGBWcolor[idx].name() + ";}";
         //le[r][c]->setStyleSheet(sColor);

         ui->gridLayoutMatrix->addWidget(le[r][c], r, c);
         ui->gridLayoutMatrix->setContentsMargins(8, 0, 8, 8);

         connect(le[r][c], &QLineEdit::selectionChanged, [ = ]()
         {
            qDebug() << "Changed" << r << c;
         });
      }

   ui->gridLayoutMatrix->setSpacing(8);
   }

 */
