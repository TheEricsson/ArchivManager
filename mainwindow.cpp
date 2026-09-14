#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database.h"
#include "newRackEntry.h"
#include <QMessageBox>
#include <QSqlQueryModel>
#include "archiveentry.h"
#include "freeboxesdialog.h"
#include <QSettings>
#include "newfuellstanddialog.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  connect (this->ui->regalBodenArchivBoxNeu, SIGNAL (triggered(bool)), this, SLOT (neuerArchivEintrag (bool)));
  connect (this->ui->freieBoxenZeigen, SIGNAL (triggered(bool)), this, SLOT (zeigeFreieBoxen (bool)));

  mQueryModel = 0;
  mQueryModelHouses = 0;

  mFilterHouseId = -1;
  mFilterShowDeleted = false;

  mQueryModel = new QSqlQueryModel ();
  mQueryModelHouses = new QSqlQueryModel ();

  ui->tableView->setColumnHidden(0, true);

  QStringList list;
  list << "Aktive Einträge" << "Gelöschte Einträge";
  ui->CbAnzeigeArt->insertItems(0, list);
}

MainWindow::~MainWindow()
{
  QString path = QApplication::applicationDirPath() + "/cfg/config.ini";
  QSettings settings (path, QSettings::IniFormat);

  QString databaseFile = settings.value("database/name", "").toString();
  QString databaseFileBackup = databaseFile;
  databaseFileBackup.replace(".sqlite", QDate::currentDate().toString("'backup_'yyyy_MM_dd'.sqlite'"));

  QFile::copy(databaseFile, databaseFileBackup);

  delete mQueryModel;
  delete mQueryModelHouses;
  delete ui;
}

void MainWindow::zeigeFreieBoxen (bool aEnabled)
{
  FreeBoxesDialog boxes(this);
  boxes.setModal(true);

  //ok
  if (boxes.exec() == QDialog::Accepted)
  {
    return;
  }
  return;
}

void MainWindow::neuerArchivEintrag (bool aEnabled)
{
  newRackEntry newEntry (this);
  newEntry.setModal(true);

  //ok
  if (newEntry.exec() == QDialog::Accepted)
  {
    QSqlDatabase* db = Database::getInstance()->getDatabase();

    if (0 != db)
    {
      if (db->open())
      {
        QSqlQuery query (*db);

        //check if value already exists
        query.prepare("SELECT * FROM Ablage WHERE Regalboden = :Regalboden AND Archivbox = :Archivbox");
        query.bindValue(":Regalboden", newEntry.getRegalbodenNr());
        query.bindValue(":Archivbox", newEntry.getArchivboxNr());
        query.exec();

        int numRows = 0;
        if (db->driver()->hasFeature(QSqlDriver::QuerySize))
        {
          numRows = query.size();
        }
        else
        {
          // this can be very slow
          query.last();
          numRows = query.at() + 1;
        }

        if (numRows > 0)
        {
          QMessageBox msgBox;
          msgBox.setText("Diese Regalnummer-/Archivboxnummer Kombination existiert bereits und kann deshalb nicht angelegt werden.");
          msgBox.exec();
        }
        else
        {
          query.clear();
          query.prepare("INSERT INTO Ablage (Regalboden, Archivbox, Fuellstand) VALUES (:Regalboden, :Archivbox, 0);");
          query.bindValue(":Regalboden", newEntry.getRegalbodenNr());
          query.bindValue(":Archivbox", newEntry.getArchivboxNr());

          if (true == query.exec())
          {
            int idAblage = query.lastInsertId().toInt();

            query.clear();
            query.prepare("INSERT INTO Archiveintrag (id_ablage) VALUES (:id);");
            query.bindValue(":id", idAblage);
            query.exec();
            update();
          }
          else
          {
            QMessageBox msgBox;
            msgBox.setText("Regalnummer-/Archivboxnummer Kombination konnte nicht erstellt werden. Datenbankfehler.");
            msgBox.exec();
          }
        }
        qDebug() << "db Close";
        db->close();
      }
    }
  }
  return;
}

QSqlError MainWindow::addConnection(const QString &driver, const QString &dbName, const QString &host,
                                  const QString &user, const QString &passwd, int port)
{
    static int cCount = 0;

    QSqlError err;
    db = QSqlDatabase::addDatabase(driver, QString("Browser%1").arg(++cCount));
    db.setDatabaseName(dbName);
    db.setHostName(host);
    db.setPort(port);
    if (!db.open(user, passwd))
    {
        err = db.lastError();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(QString("Browser%1").arg(cCount));
    }

    Database::getInstance()->setDatabase(&db);
    db.close();

    update ();
    updateHouseSelector();

    return err;
}

void MainWindow::updateHouseSelector ()
{
  QSqlDatabase* db = Database::getInstance()->getDatabase();

  if (0 != db)
  {
    if (db->open())
    {
      QSqlQuery query (*db);

      query.prepare("SELECT * FROM Liegenschaft");
      query.exec();

      mQueryModelHouses->setQuery(query);

      ui->selectHouse->setModel(mQueryModelHouses);
      ui->selectHouse->setModelColumn(1);

      db->close();
    }
  }
}

void MainWindow::update()
{
  if (mFilterHouseId != -1)
    updateSetFilter(mFilterHouseId);
  else
    updateNoFilter();
}

void MainWindow::updateNoFilter ()
{
  QSqlDatabase* db = Database::getInstance()->getDatabase();

  if (0 != db)
  {
    if (db->open())
    {
      QSqlQuery query (*db);

      if (false == mFilterShowDeleted)
      {
        query.prepare("SELECT Archiveintrag.id, Ablage.Regalboden, Ablage.Archivbox, Ablage.Fuellstand, "
                      "Liegenschaft.Liegenschaft, "
                      "Dokumenttyp.bezeichnung, "
                      "Archiveintrag.dokumenttyp_sonstiges, Archiveintrag.wirtschaftsjahr, Archiveintrag.bearbeiter "
                      "FROM Archiveintrag "
                      "INNER JOIN Ablage ON Archiveintrag.id_ablage = Ablage.id "
                      "INNER JOIN Dokumenttyp ON Archiveintrag.id_dokumenttyp = Dokumenttyp.id "
                      "INNER JOIN Liegenschaft ON Archiveintrag.id_liegenschaft = Liegenschaft.id "
                      "ORDER BY Ablage.Regalboden, Ablage.Archivbox;");
      }
      else
      {
        query.prepare("SELECT ArchiveintragGeloescht.id, Ablage.Regalboden, Ablage.Archivbox, Ablage.Fuellstand, "
                      "Liegenschaft.Liegenschaft, "
                      "Dokumenttyp.bezeichnung, "
                      "ArchiveintragGeloescht.dokumenttyp_sonstiges, ArchiveintragGeloescht.wirtschaftsjahr, ArchiveintragGeloescht.bearbeiter "
                      "FROM ArchiveintragGeloescht "
                      "INNER JOIN Ablage ON ArchiveintragGeloescht.id_ablage = Ablage.id "
                      "INNER JOIN Dokumenttyp ON ArchiveintragGeloescht.id_dokumenttyp = Dokumenttyp.id "
                      "INNER JOIN Liegenschaft ON ArchiveintragGeloescht.id_liegenschaft = Liegenschaft.id "
                      "ORDER BY Ablage.Regalboden, Ablage.Archivbox;");
      }

      query.exec();

      mQueryModel->setQuery(query);
      mQueryModel->setHeaderData(0, Qt::Horizontal, tr("Regalboden"));
      mQueryModel->setHeaderData(1, Qt::Horizontal, tr("Box"));
      mQueryModel->setHeaderData(5, Qt::Horizontal, tr("Anmerkungen"));

      ui->tableView->setModel(mQueryModel);
      ui->tableView->setColumnHidden(0, true);
      ui->tableView->verticalHeader()->setVisible(false);
      ui->tableView->setSortingEnabled(true);
      ui->tableView->show();
      db->close();
    }
  }
}

void MainWindow::on_applyFilter_clicked()
{
  int selectedRow = ui->selectHouse->currentIndex();
  int id = ui->selectHouse->model()->index(selectedRow,0).data().toInt();

  mFilterHouseId = id;

  update ();
}

void MainWindow::updateSetFilter(int id, int type)
{
  QSqlDatabase* db = Database::getInstance()->getDatabase();

  if (0 != db)
  {
    if (db->open())
    {
      QSqlQuery query (*db);

      if (false == mFilterShowDeleted)
      {
        query.prepare("SELECT Archiveintrag.id, Ablage.Regalboden, Ablage.Archivbox, Ablage.Fuellstand, "
                      "Liegenschaft.Liegenschaft, "
                      "Dokumenttyp.bezeichnung, "
                      "Archiveintrag.dokumenttyp_sonstiges, Archiveintrag.wirtschaftsjahr, Archiveintrag.bearbeiter "
                      "FROM Archiveintrag "
                      "INNER JOIN Ablage ON Archiveintrag.id_ablage = Ablage.id "
                      "INNER JOIN Dokumenttyp ON Archiveintrag.id_dokumenttyp = Dokumenttyp.id "
                      "INNER JOIN Liegenschaft ON Archiveintrag.id_liegenschaft = Liegenschaft.id "
                      "WHERE Archiveintrag.id_liegenschaft = :id;");
      }
      else
      {
        query.prepare("SELECT ArchiveintragGeloescht.id, Ablage.Regalboden, Ablage.Archivbox, Ablage.Fuellstand, "
                      "Liegenschaft.Liegenschaft, "
                      "Dokumenttyp.bezeichnung, "
                      "ArchiveintragGeloescht.dokumenttyp_sonstiges, ArchiveintragGeloescht.wirtschaftsjahr, ArchiveintragGeloescht.bearbeiter "
                      "FROM ArchiveintragGeloescht "
                      "INNER JOIN Ablage ON ArchiveintragGeloescht.id_ablage = Ablage.id "
                      "INNER JOIN Dokumenttyp ON ArchiveintragGeloescht.id_dokumenttyp = Dokumenttyp.id "
                      "INNER JOIN Liegenschaft ON ArchiveintragGeloescht.id_liegenschaft = Liegenschaft.id "
                      "WHERE ArchiveintragGeloescht.id_liegenschaft = :id;");
      }

      query.bindValue(":id", id);
      query.exec();

      mQueryModel->setQuery(query);
      mQueryModel->setHeaderData(0, Qt::Horizontal, tr("Regalboden"));
      mQueryModel->setHeaderData(1, Qt::Horizontal, tr("Box"));
      mQueryModel->setHeaderData(5, Qt::Horizontal, tr("Anmerkungen"));

      ui->tableView->setModel(mQueryModel);
      ui->tableView->verticalHeader()->setVisible(false);
      ui->tableView->setSortingEnabled(true);
      ui->tableView->show();

      db->close();
    }
  }
}

void MainWindow::on_deleteFilter_clicked()
{
  mFilterHouseId = -1;
  update ();
}

void MainWindow::on_buttonChangeEntry_clicked()
{
  //get selected entry

  //check if selection in tableview is valid
  if (ui->tableView->selectionModel()->selectedRows().count() == 1)
  {
    ui->tableView->setColumnHidden(0, false);
    int selectedRow = ui->tableView->selectionModel()->selection().indexes().value(0).row();
    int archiveintrag_id = ui->tableView->model()->index(selectedRow,0).data().toInt();
    ui->tableView->setColumnHidden(0, true);

    ArchiveEntry entry (this, 1);
    entry.setValues(archiveintrag_id);

    //ok
    if (entry.exec() == QDialog::Accepted)
    {
      entry.updateDatabase();
      update ();
    }
  }
  else if (ui->tableView->selectionModel()->selection().indexes().count() == 0)
    QMessageBox::information(this, "Fehler", "Bitte eine Zeile auswählen.");

  return;
}

void MainWindow::on_buttonNewEntry_clicked()
{
  ArchiveEntry entry (this);
  entry.setHeader("Neuer Archiveintrag");

  //ok
  if (entry.exec() == QDialog::Accepted)
  {
    update ();
  }
  return;
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
  on_buttonChangeEntry_clicked ();
}

void MainWindow::on_buttonDeleteEntry_clicked()
{
  //check if selection in tableview is valid
  if (ui->tableView->selectionModel()->selectedRows().count() == 1)
  {
    ui->tableView->setColumnHidden(0, false);
    int selectedRow = ui->tableView->selectionModel()->selection().indexes().value(0).row();
    int archiveintrag_id = ui->tableView->model()->index(selectedRow,0).data().toInt();
    ui->tableView->setColumnHidden(0, true);

    //wirklich loeschen msgBox
    QMessageBox msgBox (this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Abort);
    msgBox.setText("Soll dieser Eintrag wirklich gelöscht werden? Dieser Vorgang kann nicht rückgängig gemacht werden!");
    msgBox.show();

    if (msgBox.exec() == QMessageBox::Yes)
    {
      QSqlDatabase* db = Database::getInstance()->getDatabase();

      if (0 != db)
      {
        if (db->open())
        {
          QSqlQuery query (*db);

          //Neuen Füllstand der Archivbox eingeben
          newFuellstandDialog msgBox3 (this);
          if (msgBox3.exec() == QDialog::Accepted)
          {
            query.clear();
            query.prepare("SELECT id_ablage FROM Archiveintrag WHERE id=:idArchiveintrag");
            query.bindValue(":idArchiveintrag", archiveintrag_id);
            query.exec();

            if (query.next())
            {
              int id_ablage = -1;
              id_ablage = query.value(0).toInt();

              if (-1 != id_ablage)
              {
                query.clear();
                query.prepare("UPDATE Ablage SET Fuellstand=:fuellstand WHERE id=:id_ablage;");
                query.bindValue(":id_ablage", id_ablage);
                query.bindValue(":fuellstand", msgBox3.getCurrentSelection());
                query.exec();
              }
            }
            else
            {
              //Fehler
              QMessageBox msgBox (this);
              msgBox.setStandardButtons(QMessageBox::Ok);
              msgBox.setText("Datenbankfehler - MainWindow::on_buttonDeleteEntry_clicked()");
              msgBox.exec();
            }
          }

          //Fehleintrag oder gewollte Löschung?
          QMessageBox msgBox2 (this);
          msgBox2.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
          msgBox2.setText("Löschung aufgrund eines Fehleintrages?");
          msgBox2.show();

          if (msgBox2.exec() == QMessageBox::No)
          {
            //Archiveintrag in gelöschten Einträgen speichern
            query.clear();
            query.prepare("INSERT INTO ArchiveintragGeloescht SELECT * FROM Archiveintrag WHERE id=:idArchiveintrag");
            query.bindValue(":idArchiveintrag", archiveintrag_id);
            query.exec();
          }

          //Archiveintrag in Datenbank löschen
          query.clear();
          query.prepare("DELETE FROM Archiveintrag WHERE id=:idArchiveintrag");
          query.bindValue(":idArchiveintrag", archiveintrag_id);
          query.exec();

          db->close();

          update ();
        }
      }
    }
  }
  else if (ui->tableView->selectionModel()->selection().indexes().count() == 0)
    QMessageBox::information(this, "Fehler", "Bitte eine Zeile auswählen.");
}

void MainWindow::on_CbAnzeigeArt_currentIndexChanged(int index)
{
  if (index == 1)
  {
    mFilterShowDeleted = true;

    ui->buttonChangeEntry->setEnabled(false);
    ui->buttonDeleteEntry->setEnabled(false);
    ui->buttonNewEntry->setEnabled(false);
  }
  else if (index == 0)
  {
    mFilterShowDeleted = false;

    ui->buttonChangeEntry->setEnabled(true);
    ui->buttonDeleteEntry->setEnabled(true);
    ui->buttonNewEntry->setEnabled(true);
  }
  update ();
}
