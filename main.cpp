#include "mainwindow.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <wiringPi.h>
#include <gpiomanager.h>

int main(int argc, char *argv[])
{
 QApplication a(argc, argv);
    MainWindow w;
    w.connectToDatabase(); // Call the connection to the database after the window is shown
    w.show(); // Displays Widgets
    return a.exec();
}
