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
    void setDatabase(QSqlDatabase db); // Set the database for this dialog
    void displayDatabaseContents();
    void connectToDatabase();

private:
    Ui::DatabaseDialog *ui;
    QSqlDatabase db; // Store the database connection
    void insertIntoDatabase();
    void deleteSelectedEntry();
};

#endif // DATABASEDIALOG_H
