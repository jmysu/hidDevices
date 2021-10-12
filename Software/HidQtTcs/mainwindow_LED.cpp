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


void MainWindow::addNeoLEDs()
{
	NeoScene->setSceneRect(ui->graphicsView->sceneRect());
	qDebug() << "scene rect" << NeoScene->sceneRect();
	for (int k = 0; k < iKLedNumbers; k++) {
		kLed[k] = new KLed();
		kLed[k]->setColor(cKLcolor);
		kLed[k]->resize(iKLsize, iKLsize);
		qDebug() << "new KLed" << iKLedNumbers << cKLcolor.name() << iKLsize;

		switch (shapeNeo) {
			qreal X, Y, row;
		case ShapeNEO::Bar:
			kLed[k]->setShape(KLed::Rectangular); //enum Shape { Rectangular, Circular, Matrix };
			X = (iKLsize + 2) * k;
			Y = 0;
			kLed[k]->move(X - (NeoScene->width() - iKLsize) / 4, Y + (NeoScene->height() - iKLsize) / 2);
			break;
		case ShapeNEO::Matrix:
			kLed[k]->setShape(KLed::Rectangular); //enum Shape { Rectangular, Circular, Matrix };
			row = qSqrt(iKLedNumbers) + 0.5f;
			X = (iKLsize) * (k / int(row));
			Y = (iKLsize) * (k % int(row));
			kLed[k]->move(X - (NeoScene->width() - iKLsize) / 4, Y + (NeoScene->height() - iKLsize) / 2);
			break;
		case ShapeNEO::Round:
		default:
			qreal angle = 360.0f * k / iKLedNumbers;
			qreal w = iKLsize * 1.75; //1.75x just fine for 16 leds
			qreal h = iKLsize * 1.75; //1.75x
			qreal c = cos(angle * M_PI / 180);
			qreal s = sin(angle * M_PI / 180);
			X = w * c - h * s;
			Y = w * s + h * c;
			kLed[k]->move(X + (NeoScene->width() - iKLsize) / 2, Y + (NeoScene->height() - iKLsize) / 2);
			break;
		}
		NeoScene->addWidget(kLed[k]);
	}
}


void MainWindow::slotLedColorChange(QColor c, int lx)
{
	qDebug() << "Color change:" << c.name(QColor::HexArgb) << "Lx:" << lx;
	QColor cx = c.lighter(75 + lx * 3);  //3x lx lighter

	//QColor color = QColorDialog::getColor(Qt::yellow, this);
	//if (color.isValid()) {
	//	qDebug() << "Color Choosen : " << color.name();
	//}

	for (int k = 0; k < iKLedNumbers; k++)
		//kLed[k]->setColor(c3x); //3x lighter for showing neopixels
		kLed[k]->setColor(cx);
	//QString sColor = "QPushButton { background-color:" + c.name() + "; border:  none}";
	//ui->pushButtonColor->setStyleSheet(sColor);

	//Update Neo color mainwindow
	cKLcolor = cx;
	qDebug() << "Color changed:" << cx.name(QColor::HexArgb);

	//if (ui->tabLED->isVisible()) {
	//QString sColor = cx.name();
	//ui->labelCurrentColor->setStyleSheet("QLabel {background-color:" + sColor + ";}");
	//}
}

void MainWindow::updateRGBHSVvalue(const QColor &color)
{
	if (ui->tabLED->isVisible()) {
		QString sColor = color.name();
		ui->labelCurrentColor->setStyleSheet("QLabel {background-color:" + sColor + ";}");

		ui->RedSpinBox->setValue(color.red());
		ui->GreenSpinBox->setValue(color.green());
		ui->BlueSpinBox->setValue(color.blue());

		ui->HueSpinBox->setValue(color.hue());
		ui->SatSpinBox->setValue(color.saturation());
		ui->ValSpinBox->setValue(color.value());

		ui->BrightnessSlider->setValue(color.value());
	}
}

void MainWindow::on_ColorWheelBox_colorChanged(const QColor &color)
{
	qDebug() << "[ColorWheel]:" << color.name();
	updateRGBHSVvalue(color);
}
void MainWindow::on_SwatchBox_swatchChanged(const QColor &color)
{
	qDebug() << "[Swatch]:" << color.name();
	updateRGBHSVvalue(color);
}

void MainWindow::on_pushButtonSetColorLED_clicked()
{
	char buf[64];
	unsigned char r = ui->RedSpinBox->value();
	unsigned char g = ui->GreenSpinBox->value();
	unsigned char b = ui->BlueSpinBox->value();
	unsigned char brt = ui->BrightnessSlider->value();
	sprintf(buf, "0x02,0xC1,0x%02X,0x%02X,0x%02X,0x%02X", r, g, b, brt);
	qDebug() << "[SetColor]:" << QString(buf);
	hidCmd(buf); //To update device neopixels

	QColor color(r, g, b);
	emit sigColorChange(color, brt);//To update Neopixel LED color
}
