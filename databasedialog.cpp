#include "databasedialog.h"
#include "ui_databasedialog.h"
#include "writedialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

DatabaseDialog::DatabaseDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseDialog)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Batch" << "No of Wafers" << "Process" << "Location");

    connect(ui->refreshButton, &QPushButton::clicked, this, &DatabaseDialog::displayDatabaseContents);
    connect(ui->addButton, &QPushButton::clicked, this, &DatabaseDialog::openWriteDialog);
    connect(ui->exitButton, &QPushButton::clicked, this, &DatabaseDialog::close);
    connect(ui->deleteButton, &QPushButton::clicked, this, &DatabaseDialog::deleteSelectedEntry);
}

DatabaseDialog::~DatabaseDialog()
{
    delete ui;
}

void DatabaseDialog::displayDatabaseContents() {
    QSqlQuery query("SELECT batch, wafers, process, location FROM RFIDData");

    ui->tableWidget->setRowCount(0);

    int row = 0;
    while (query.next()) {
        QString batch = query.value(0).toString();
        int wafers = query.value(1).toInt();
        QString process = query.value(2).toString();
        QString location = query.value(3).toString();

        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(batch));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(wafers)));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(process));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(location));

        row++;
    }
}

void DatabaseDialog::openWriteDialog() {
    WriteDialog *writeDialog = new WriteDialog(this);
    connect(parentWidget(), SIGNAL(uidScanned(QString)), writeDialog, SLOT(setUID(QString)));  // Connect UID updates
    connect(writeDialog, &WriteDialog::dataSubmitted, this, &DatabaseDialog::addDataToDatabase);
    writeDialog->exec();  // Show as a modal dialog
}

void DatabaseDialog::addDataToDatabase(const QString &batch, int wafers, const QString &process, const QString &location) {
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO RFIDData (batch, wafers, process, location) "
                  "VALUES (:batch, :wafers, :process, :location)");
    query.bindValue(":batch", batch);
    query.bindValue(":wafers", wafers);
    query.bindValue(":process", process);
    query.bindValue(":location", location);

    if (!query.exec()) {
        QMessageBox::warning(this, "Insert/Replace Error", "Failed to insert or replace data: " + query.lastError().text());
    } else {
        QMessageBox::information(this, "Success", "Data inserted/replaced successfully!");
        displayDatabaseContents();
    }
}

void DatabaseDialog::deleteSelectedEntry() {
    int selectedRow = ui->tableWidget->currentRow();  // Get the currently selected row
    if (selectedRow < 0) {
        QMessageBox::warning(this, "Delete Error", "No entry selected!");
        return;
    }

    QString batch = ui->tableWidget->item(selectedRow, 0)->text();  // Get the UID (batch) from the first column

    // Prepare and execute the DELETE SQL query
    QSqlQuery query;
    query.prepare("DELETE FROM RFIDData WHERE batch = :batch");
    query.bindValue(":batch", batch);

    if (!query.exec()) {
        QMessageBox::warning(this, "Delete Error", "Failed to delete entry: " + query.lastError().text());
        qDebug() << "Error deleting entry:" << query.lastError().text();
    } else {
        QMessageBox::information(this, "Success", "Entry deleted successfully!");
        displayDatabaseContents();  // Refresh the table after deletion
    }
}
