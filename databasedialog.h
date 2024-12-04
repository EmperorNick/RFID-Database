#ifndef DATABASEDIALOG_H
#define DATABASEDIALOG_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class DatabaseDialog;
}

class DatabaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseDialog(QWidget *parent = nullptr);
    ~DatabaseDialog();
    void setDatabase(QSqlDatabase db);
    void displayDatabaseContents();
    void connectToDatabase();

private:
    Ui::DatabaseDialog *ui;
    QSqlDatabase db;
    void insertIntoDatabase();
    void deleteSelectedEntry();
    void openWriteDialog();
    void addDataToDatabase(const QString &batch, int wafers, const QString &process, const QString &location);
};

#endif // DATABASEDIALOG_H
