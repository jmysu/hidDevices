#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <HIDAPI/hidapi.h>
#include <qmath.h>

#include <QColorDialog>
#include <QApplication>
//#include <QSvgRenderer>
#include <QPainter>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "kled.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	const int PLOT_SAMPLES = 64;
	enum { ePlotR, ePlotG, ePlotB, ePlotC };
	void initTCSplot();

	void scanHID();

	bool isHidConnected = false;
	bool isTCSenabled = false;
	hid_device *handleHID;
	unsigned char outBuffer[65];    //outBuffer[0] is for endpoint number
	unsigned char inBuffer[65];

	void updateCIEchart();
	void plotCircle(float x, float y);

	void addNeoLEDs();
	enum ShapeNEO { Bar, Round, Matrix };

	void createChecker4x6();
//void createTCSrgbw();
//void createTCSmatrix();
	void initTcsConfig();
	void updateRGBHSVvalue(const QColor &);

signals:
	//void sigRGBlx(uint8_t *data);
	//void sigRGBCraw(uint8_t *data);
	void sigHIDIO(uint8_t *data, int io_result);

	void sigColorChange(QColor c, int lx);

private slots:
	void on_buttonReScan_clicked();
	void on_buttonConnect_clicked();

	void hidTx();
	void hidTxFeature(hid_device *dev, uint8_t *data, size_t length);
	void hidCmd(const char *cmd);
	void hidRx();
	void parseSendData();
	void parseCmd(const char *cmd);

	void on_buttonSendOutReport_clicked();
	void on_buttonSendFeatureReport_clicked();

	void on_pushButton_clicked();
	void on_checkBoxTCS_Enable_clicked(bool checked);
	void on_checkBoxTCS_Lamp_clicked(bool checked);

	void slotGotRGBlx(uint8_t *data);
	void slotGotRGBCraw(uint8_t *data);
	void slotGotHIDIO(uint8_t *data, int io_result);

	void slotLedColorChange(QColor c, int lx);

	void on_spinBoxIntergration_valueChanged(int arg1);
	void on_comboBoxGain_currentIndexChanged(int index);

	void on_SwatchBox_swatchChanged(const QColor &);
	void on_ColorWheelBox_colorChanged(const QColor &);
	void on_pushButtonSetColorLED_clicked();

private:
	Ui::MainWindow *ui;

	QTimer *timerHidRx;
	QTimer *dataTimer;
	int io_result;

	QVector<double>  vtime[4], vdata[4]; // two dimentional array for plot rgbc

	//NeoPixels
	QGraphicsScene *NeoScene;
	KLed *kLed[16];
	int iKLedNumbers = 8;
	int iKLsize = 24;
	//QColor cKLcolor = Qt::blue;
	QColor cKLcolor = QColor(64, 64, 80);

	ShapeNEO shapeNeo = ShapeNEO::Round;
};
#endif // MAINWINDOW_H
