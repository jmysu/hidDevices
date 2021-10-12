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



/*
   QMap<QString, QColor> mapChecker = {
   { "Dark skin", QColor(94, 28, 13) },
   { "light skin", QColor(241, 149, 108) },
   { "blue sky", QColor(97, 119, 171) },
   { "foliage", QColor(90, 103, 39) },
   { "blue flower", QColor(164, 131, 196) },
   { "bluish green", QColor(140, 253, 153) },
   { "orange", QColor(255, 116, 21) },
   { "purplish blue", QColor(7, 47, 122) },
   { "moderate red", QColor(222, 29, 42) },
   { "purple", QColor(69, 0, 68) },
   { "yellow green", QColor(187, 255, 19) },
   { "orange yellow", QColor(255, 142, 0) },
   { "blue", QColor(0, 0, 142) },
   { "green", QColor(64, 173, 38) },
   { "red", QColor(203, 0, 0) },
   { "yellow", QColor(255, 217, 0) },
   { "magenta", QColor(207, 3, 124) },
   { "cyan", QColor(0, 148, 189) },
   { "white", QColor(255, 255, 255) },
   { "neutral 8", QColor(249, 249, 249) },
   { "neutral 6.5", QColor(180, 180, 180) },
   { "neutral 5", QColor(117, 117, 117) },
   { "neutral 3.5", QColor(53, 53, 53) },
   { "black", QColor(0, 0, 0) },
   };*/
QList<QString> listCheckerName = {
	"dark skin", "light skin", "blue sky", "foliage", "blue flower", "bluish green",
	"orange", "purplish blue", "moderate red", "purple", "yellow green", "orange yellow",
	"blue", "green", "red", "yellow", "magenta", "cyan",
	"white", "neutral 8", "neutral 6.5", "neutral 5", "neutral 3.5", "black"
};
QList<QColor>  listCheckerColor = {
	QColor(94, 28, 13), QColor(241, 149, 108), QColor(97, 119, 171), QColor(90, 103, 39), QColor(164, 131, 196), QColor(140, 253, 153),
	QColor(255, 116, 21), QColor(7, 47, 122), QColor(222, 29, 42), QColor(69, 0, 68), QColor(187, 255, 19), QColor(255, 142, 0),
	QColor(0, 0, 142), QColor(64, 173, 38), QColor(203, 0, 0), QColor(255, 217, 0), QColor(207, 3, 124), QColor(0, 148, 189),
	QColor(255, 255, 255), QColor(249, 249, 249), QColor(180, 180, 180), QColor(117, 117, 117), QColor(53, 53, 53), QColor(0, 0, 0)
};

void MainWindow::createChecker4x6()
{
	int row = 4;
	int col = 6;
	QPushButton *b[row][col];

	ui->widgetChecker->setStyleSheet("QWidget { background: black; }");

	for (int r = 0; r < row; r++)
		for (int c = 0; c < col; c++) {
			int idx = (r * col) + c;
			b[r][c] = new QPushButton();
			b[r][c]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			QString sColor = "QPushButton { background-color:" + listCheckerColor[idx].name() + ";}";
			b[r][c]->setStyleSheet(sColor);

			ui->gridLayoutChecker->addWidget(b[r][c], r, c);
			ui->gridLayoutChecker->setContentsMargins(32, 16, 32, 16);

			connect(b[r][c], &QPushButton::clicked, [ = ]()
			{
				//qDebug() << "clicked" << r << c << listCheckerName[idx] << listCheckerColor[idx];
				ui->statusbar->showMessage("RGB:" + listCheckerColor[idx].name());
				ui->frameChecker->setStyleSheet("background-color:" + listCheckerColor[idx].name());
			});
		}
	ui->gridLayoutChecker->setSpacing(32);
}

