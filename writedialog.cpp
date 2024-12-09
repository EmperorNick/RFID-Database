#include "writedialog.h"
#include "qprocess.h"
#include "qpushbutton.h"
#include "ui_writedialog.h"
#include <QMessageBox>
#include <QDebug>

WriteDialog::WriteDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WriteDialog)
    , rfidProcess(new QProcess(this))
{
    ui->setupUi(this);
    ui->processDropdown->addItems({"VB", "VL", "VC", "MR", "LB"}); //Adds Drop down options for recipes

    connect(rfidProcess, &QProcess::readyReadStandardOutput, this, &WriteDialog::handleScanOutput);
    connect(ui->submitButton, &QPushButton::clicked, this, &WriteDialog::submitData);
    connect(ui->cancelButton, &QPushButton::clicked, this, &WriteDialog::close);
}

WriteDialog::~WriteDialog()
{
    delete ui;
}

void WriteDialog::handleScanOutput() {
    QString uid = rfidProcess->readAllStandardOutput().trimmed();
    if (!uid.isEmpty()) {
        ui->uidLabel->setText(uid);
    }
}

void WriteDialog::submitData() {
    QString batch = ui->uidLabel->text();  // UID
    int wafers = ui->waferInput->text().toInt();
    QString process = ui->processDropdown->currentText();

    if (batch.isEmpty() || wafers < 1 || wafers > 25 || process.isEmpty()) { //Error check to make sure its within 1 and 25 wafers and the other two are filled out
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    emit dataSubmitted(batch, wafers, process, QString());  // Location is null for now until updated by Pi zero
    close();
}

void WriteDialog::setUID(const QString &uid) {
    if (!uid.isEmpty()) {
        ui->uidLabel->setText(uid);  // Display UID in the label
    }
}
