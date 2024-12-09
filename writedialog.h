#ifndef WRITEDIALOG_H
#define WRITEDIALOG_H

#include <QDialog>
#include <QProcess>

namespace Ui {
class WriteDialog;
}

class WriteDialog : public QDialog
{
    Q_OBJECT

public slots:
    void setUID(const QString &uid);

public:
    explicit WriteDialog(QWidget *parent = nullptr);
    ~WriteDialog();

signals:
    void dataSubmitted(const QString &batch, int wafers, const QString &process, const QString &location);

private slots:
    void handleScanOutput();
    void submitData();

private:
    Ui::WriteDialog *ui;
    QProcess *rfidProcess;  // For RFID scanning
};

#endif // WRITEDIALOG_H
