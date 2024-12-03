#include "databasedialog.h"
#include "mainwindow.h"
#include "ui_databasedialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

DatabaseDialog::DatabaseDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseDialog)
{
    ui->setupUi(this);
    // Set up the table widget headers
    ui->tableWidget->setColumnCount(2);  // 2 columns: Name and Age
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Name" << "Age");

    connect(ui->addButton, &QPushButton::clicked, this, &DatabaseDialog::insertIntoDatabase);
    // Connect exit button
    connect(ui->exitButton, &QPushButton::clicked, this, &DatabaseDialog::close);
    // Connect refreshButton to the displayDatabaseContents function
    connect(ui->refreshButton, &QPushButton::clicked, this, &DatabaseDialog::displayDatabaseContents);
    // Connect deleteButton to the delete specific entries
    connect(ui->deleteButton, &QPushButton::clicked, this, &DatabaseDialog::deleteSelectedEntry);
}

DatabaseDialog::~DatabaseDialog()
{
    delete ui;
}

void DatabaseDialog::setDatabase(QSqlDatabase database) {
    db = database; // Set the passed database connection
}

void DatabaseDialog::insertIntoDatabase() {
    QString name = ui->nameInput->text();  // Get name as a string
    QString ageString = ui->ageInput->text();  // Get age as a string

    QSqlQuery query;
    query.prepare("INSERT INTO people (name, age) VALUES (:name, :age)");
    query.bindValue(":name", name);
    query.bindValue(":age", ageString);

    if (name.isEmpty()) {
        qDebug() << "Name field is empty.";
        QMessageBox::warning(this, "Input Error", "Please enter a name and/or age wanker");
        return;
    }
    if (ageString.isEmpty()) {
            qDebug() << "Age field is empty.";
            QMessageBox::warning(this, "Input Error", "Please enter a name and/or age wanker");
            return;
        }

    if (!query.exec()) {
        qDebug() << "Error inserting into the database: " << query.lastError().text();
        QMessageBox::warning(this, "Insert Error", "Failed to insert data.");
    }
    else {
        QMessageBox::information(this, "Success", "Data inserted successfully!");
    }
    displayDatabaseContents(); // Refresh the table
    ui->nameInput->clear(); // Clear the input fields after successful insertion
    ui->ageInput->clear();
}

void DatabaseDialog::displayDatabaseContents() {
    // SQL query to retrieve all entries from the 'people' table
    QSqlQuery query("SELECT name, age FROM people");

    // Clear the table before adding new data
    ui->tableWidget->setRowCount(0);

    int row = 0;
    while (query.next()) {
        QString name = query.value(0).toString();  // Get the name
        int age = query.value(1).toInt();          // Get the age

        // Insert a new row in the table
        ui->tableWidget->insertRow(row);

        // Populate the table with the retrieved data
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(name));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(age)));

        row++;  // Move to the next row
    }
}
void DatabaseDialog::connectToDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("test.db");

    if (!db.open()) {
        QMessageBox::warning(this, "Database Error", "Unable to open database: " + db.lastError().text());
    }
}
void DatabaseDialog::deleteSelectedEntry() {
    // Get the selected row
    int selectedRow = ui->tableWidget->currentRow();
    if (selectedRow < 0) {
        QMessageBox::warning(this, "Delete Error", "No entry selected!");
        return;
    }

    // Get the name from the selected row (or another identifier like ID)
    QString name = ui->tableWidget->item(selectedRow, 0)->text();

    // Prepare the SQL query to delete the entry
    QSqlQuery query;
    query.prepare("DELETE FROM people WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qDebug() << "Error deleting from the database: " << query.lastError().text();
        QMessageBox::warning(this, "Delete Error", "Failed to delete entry.");
    } else {
        QMessageBox::information(this, "Success", "Entry deleted successfully!");
        displayDatabaseContents(); // Refresh the table
    }
}
