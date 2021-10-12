#include "mainwindow.h"
#include <QApplication>
#include <QSharedMemory>
#include <QDebug>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
/*

   //Check if an instance already made!
   QSharedMemory sharedMemory;
   sharedMemory.setKey("HidQtTcsV1");
   if (!sharedMemory.create(1)) {
      //QMessageBox::warning(this, tr("Warning!"), tr("An instance of this application is running!") );
      qDebug() << "!!!One Instance Only, Bye-Bye!!!";
      sharedMemory.deleteLater();
      exit(0); // Exit already a process running
   }
 */
	MainWindow w;
	w.show();
	return a.exec();
}
