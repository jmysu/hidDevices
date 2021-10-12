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


void MainWindow::updateCIEchart()
{
	// generate some data:
	QVector<double> x(101), y(101); // initialize with entries 0..100
	for (int i = 0; i < 101; ++i) {
		x[i] = i / 50.0 - 1;     // x goes from -1 to 1
		y[i] = x[i] * x[i];     // let's plot a quadratic function
	}
	// create graph and assign data to it:
	ui->plotCIE->addGraph();
	//ui->plotCIE->graph(0)->setData(x, y); //don't add data, to show the test data, uncomment this line

	// give the axes some labels:
	ui->plotCIE->xAxis->setLabel("x");
	ui->plotCIE->yAxis->setLabel("y");
	// set axes ranges, so we see all data:
	ui->plotCIE->xAxis->setRange(-0.06, 0.95);
	ui->plotCIE->yAxis->setRange(-0.04, 0.82);
	ui->plotCIE->setBackground(QColor("#e0e0e0"));

	//QPixmap p(":/CIExy1931.png");
	QPixmap p(":/CIExy1931_400x440.png");
	ui->plotCIE->axisRect()->setBackground(QBrush(QPixmap(p).scaled(380, 430)));

	plotCircle(.313, .330);
	ui->plotCIE->replot();
}

void MainWindow::plotCircle(float x, float y)
{
	static bool isBW = false;

	QVector<double> vx, vy;
	vx << x;
	vy << y;

	//Set the pen style for blinking the circle
	QPen drawPen;
	isBW = !isBW;
	if (isBW)
		drawPen.setColor(Qt::darkGray);
	else
		drawPen.setColor(Qt::black);

	drawPen.setWidth(2);

	ui->plotCIE->removeGraph(1); //the ssPlusCircle should be in #1
	//Draw Scatter
	QCPGraph * curGraph;
	curGraph = ui->plotCIE->addGraph();
	curGraph->setPen(drawPen);
	curGraph->setLineStyle(QCPGraph::lsNone);
	curGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssPlusCircle, 16));

	curGraph->setData(vx, vy);
	ui->plotCIE->replot();
}


