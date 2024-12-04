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

    // Populate process dropdown
    ui->processDropdown->addItems({"Recipe A", "Recipe B", "Recipe C"});

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
        ui->uidLabel->setText(uid);  // Display the scanned UID
    }
}

void WriteDialog::submitData() {
    QString batch = ui->uidLabel->text();  // UID
    int wafers = ui->waferInput->text().toInt();
    QString process = ui->processDropdown->currentText();

    if (batch.isEmpty() || wafers <= 0 || process.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    emit dataSubmitted(batch, wafers, process, QString());  // Location is null for now
    close();
}

void WriteDialog::setUID(const QString &uid) {
    if (!uid.isEmpty()) {
        ui->uidLabel->setText(uid);  // Update the UID label in the dialog
    }
}
