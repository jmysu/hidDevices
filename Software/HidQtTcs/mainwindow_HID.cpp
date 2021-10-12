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


QMap<QString, int> hidPages = {
	{ "Desktop", 0x01 },
	{ "Keyboard", 0x07 },
	{ "Consumer", 0x0C },
	{ "Vendor", 0xFF00 },
};

void MainWindow::scanHID()
{
	QStringList slDevice;
	struct hid_device_info *devs, *cur_dev;
	if (hid_init() == 0) {
		devs = hid_enumerate(0x0, 0x0);
		cur_dev = devs;
		while (cur_dev) {
			char buf[256];
			sprintf(buf, "0x%04X:0x%04X - %ls %ls  %04X:%d %s ",
				cur_dev->vendor_id, cur_dev->product_id,
				cur_dev->manufacturer_string, cur_dev->product_string,
				cur_dev->usage_page, cur_dev->usage,
				hidPages.key(cur_dev->usage_page).toLatin1().constData());
			//qDebug() << hidPages.key(cur_dev->usage_page);
			slDevice << QString(buf);
			cur_dev = cur_dev->next;
		}
		hid_free_enumeration(devs);
	}

	ui->cbDevice->clear();
	ui->cbDevice->addItems(slDevice);
	ui->cbDevice->setFocus();

	ui->buttonConnect->setEnabled(false);
	//Locate for HID-TCS FF00:1 product
	for (int i = 0; i < slDevice.size(); i++) {
		if (slDevice[i].contains("TCS HID") && slDevice[i].contains("FF00:1")) {
			ui->cbDevice->setCurrentIndex(i);
			ui->buttonConnect->setFocus();
			ui->buttonConnect->setEnabled(true);
			break;
		}
	}

	//ui->boxSendData->setEnabled(false);
	//ui->boxReceiveData->setEnabled(false);
	ui->tabWidget->setEnabled(false);

	if (ui->buttonConnect->isEnabled())
		ui->statusbar->showMessage("HIDAPI-" + QString(hid_version_str()) + " found devices:" + QString::number(slDevice.size()));
	else
		ui->statusbar->showMessage("HIDAPI-" + QString(hid_version_str()) + " No Sensor Found!");
}


void MainWindow::hidTx()
{
	//Parse cbSendData string into outBuffer[64], size is spinSizeOut
	parseSendData();

	int iSizeOut = ui->spinSizeOut->value();
	io_result = hid_write(handleHID, outBuffer, (size_t)iSizeOut);  //outBuffer[0] is reportID
	ui->statusbar->showMessage("Sending bytes:" + QString::number(iSizeOut));
}

/*
   Feature reports are sent over the Control endpoint as a Set_Report transfer.
   The first byte of data[] must contain the Report ID.
   For devices which only support a single report, this must be set to 0x0. The remaining bytes
   contain the report data. Since the Report ID is mandatory, calls to hid_send_feature_report()
   will always contain one more byte than the report contains.
 */
void MainWindow::hidTxFeature(hid_device *dev, uint8_t *data, size_t length)
{
	Q_UNUSED(data);
	Q_UNUSED(length);
	//Parse cbSendData string into outBuffer[64], size is spinSizeOut
	parseSendData();

	int iSizeOut = ui->spinSizeOut->value() + 1;
	io_result = hid_send_feature_report(dev, outBuffer, (size_t)iSizeOut);       //outBuffer[0] is reportID
	ui->statusbar->showMessage("[Feature]Sending bytes:" + QString::number(iSizeOut));
}

void MainWindow::hidCmd(const char *cmd)
{
	//Clear outBuffer
	//for (auto& i : outBuffer) i = 0;          //Clear outBuffer

	//outBuffer[0] = 0x01;
	//outBuffer[1] = 0xC1;
	parseCmd(cmd);

	if (isHidConnected) { //Write HID only when connected!
		int iSizeOut = 8;
		io_result = hid_write(handleHID, outBuffer, (size_t)iSizeOut); //outBuffer[0] is reportID

		//ui->statusbar->showMessage("Sending bytes:" + QString::number(iSizeOut));
		QByteArray temp = QByteArray::fromRawData((const char*)outBuffer, io_result);
		QString sO = QString(temp.toHex());
		qDebug() << "CMD:" << sO;
	}else
		qDebug() << "HID not connected yet!";
}

// Use HIDAPI non-blocking read
void MainWindow::hidRx()
{
	if (!handleHID) return;

	hid_set_nonblocking(handleHID, 1);
	int last_bytes_read = 0;
	int bytes_read = 0;
	do{ //read all HID data until bytes==0
		last_bytes_read = bytes_read;
		bytes_read = hid_read(handleHID, inBuffer, 64);
	}while(bytes_read > 0);

	//io_result = hid_read(handleHID, inBuffer, 64); //Max 64 bytes for the HID IO
	io_result = last_bytes_read;
	if (io_result > 0) {
		//qDebug() << "io_result:" << io_result;
		QString s = "";
		char buf[64];

		QByteArray temp = QByteArray::fromRawData((const char*)inBuffer, io_result);
		QString sIn = QString(temp.toHex());
		qDebug() << "[HID Rx]:" << sIn << io_result;

		QString sMillis = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

		sprintf(buf, "%s %02X>", sMillis.toLocal8Bit().constData(), inBuffer[0]); //[0] reportID
		s += QString(buf);
		//qDebug() << s;

		int iCount = 0;
		for (int i = 1; i < io_result; i++) {
			//s += QString("%1").arg(inBuffer[i], 3, 16);
			sprintf(buf, "%02X", inBuffer[i]);
			s += QString(buf);
			if (iCount++ >= 3) { //display data every 4 bytes
				iCount = 0;
				s += " ";
			}
		}
		//qDebug() << s;

		/*
		   //01F1rrgg bbcc0000
		   if (*(inBuffer + 1) == 0xF1) { //Got code 0xF1 @ [1]?
		   //qDebug() << "Got RGBlx:" << *(inBuffer + 2) << *(inBuffer + 3) << *(inBuffer + 4) << *(inBuffer + 5);
		   if (isTCSenabled) emit sigRGBlx(inBuffer + 2);
		   }else if (*(inBuffer + 1) == 0xF2) { //Got code 0xF1 @ [1]?
		   if (isTCSenabled) emit sigRGBCraw(inBuffer + 2);
		   }
		 */

		//emit HIDio signal w/ bytes received
		emit sigHIDIO(inBuffer, io_result);


		ui->textGetData->append(s);
		ui->textGetData->ensureCursorVisible();   // (for auto-scolling)
	}else {
		qDebug() << "[HID]: nothing!";
	}
}

/*
 *  parseOutput()
 *    parse Output SendData string into outBuffer[64]
 *
 */
void MainWindow::parseSendData()
{
	QByteArray buffer;

	QString sOut = ui->cbSendData->currentText().toLatin1();
	sOut.replace(",", " ");
	QStringList slOut = sOut.split(" ");
	for (int i = 0; i < slOut.size(); i++) {
		if (slOut[i].startsWith("0x", Qt::CaseInsensitive)) {
			slOut[i].remove("0x", Qt::CaseInsensitive);
			buffer.append(QByteArray::fromHex(slOut[i].toUtf8()));
		}else if (slOut[i].contains((QChar)'"')) {     //Char ""
			slOut[i].remove((QChar)'"', Qt::CaseInsensitive);
			buffer.append(slOut[i].toLatin1());
		}else if (slOut[i].contains("'")) {            //Char ''
			slOut[i].remove("'", Qt::CaseInsensitive);
			buffer.append(slOut[i].toLatin1());
		}else {
			buffer.append(slOut[i].toInt());
		}
	}
	qDebug() << "TX:" << buffer << buffer.toHex();

	if (buffer.size() <= 64) {
		for (auto& i : outBuffer) i = 0;           //Clear outBuffer
		for (int i = 0; i < buffer.size(); i++) {
			outBuffer[i] = buffer[i];            //outBuffer[0]->EP
		}
	}
}

/*
 *  parseCmd()
 *    parse Output SendData string into outBuffer[64]
 *
 */
void MainWindow::parseCmd(const char *cmd)
{
	QByteArray buffer;

	QString sOut = QString::fromLocal8Bit(cmd);
	sOut.replace(",", " ");
	QStringList slOut = sOut.split(" ");
	for (int i = 0; i < slOut.size(); i++) {
		if (slOut[i].startsWith("0x", Qt::CaseInsensitive)) { //Hex starts w/ 0x
			slOut[i].remove("0x", Qt::CaseInsensitive);
			buffer.append(QByteArray::fromHex(slOut[i].toUtf8()));
		}else if (slOut[i].contains((QChar)'"')) {   //Char ""
			slOut[i].remove((QChar)'"', Qt::CaseInsensitive);
			buffer.append(slOut[i].toLatin1());
		}else if (slOut[i].contains("'")) {   //Char ''
			slOut[i].remove("'", Qt::CaseInsensitive);
			buffer.append(slOut[i].toLatin1());
		}else { //Decimal
			buffer.append(slOut[i].toInt());
		}
	}
	qDebug() << "TX Buffer:" << buffer.size() << buffer.toHex();

	if ((buffer.size() > 0) && (buffer.size() <= 64)) {
		for (auto& i : outBuffer) i = 0;               //Clear outBuffer
		for (int i = 0; i < buffer.size(); i++) {
			outBuffer[i] = buffer[i];                     //outBuffer[0]->EP
		}
	}
}

