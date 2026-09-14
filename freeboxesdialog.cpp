#include "freeboxesdialog.h"
#include "ui_freeboxesdialog.h"
#include "database.h"
#include <QSqlDatabase>
#include <QSqlQueryModel>

FreeBoxesDialog::FreeBoxesDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FreeBoxesDialog)
{
  ui->setupUi(this);
  mQueryModel = new QSqlQueryModel;

  QSqlDatabase* db = Database::getInstance()->getDatabase();

  if (0 != db)
  {
    qDebug() << "db Open";
    if (db->open())
    {
      QSqlQuery query (*db);

      query.prepare("SELECT DISTINCT Regalboden,Archivbox,Fuellstand FROM Ablage ORDER BY Fuellstand ASC;");
      query.exec();

      mQueryModel->setQuery(query);
      mQueryModel->setHeaderData(0, Qt::Horizontal, tr("Regal"));
      mQueryModel->setHeaderData(1, Qt::Horizontal, tr("Box"));
      mQueryModel->setHeaderData(5, Qt::Horizontal, tr("Fuellstand"));

      ui->tableView->setModel(mQueryModel);
      ui->tableView->verticalHeader()->setVisible(false);
      ui->tableView->setSortingEnabled(true);
      ui->tableView->show();

      qDebug() << "db Close";
      db->close();
    }
  }
}

FreeBoxesDialog::~FreeBoxesDialog()
{
  delete ui;
  delete mQueryModel;
}
