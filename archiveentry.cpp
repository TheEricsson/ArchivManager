#include "archiveentry.h"
#include "ui_archiveentry.h"
#include <QList>
#include <QSqlQueryModel>
#include <QSettings>
#include "database.h"
#include <QMessageBox>

ArchiveEntry::ArchiveEntry(QWidget *parent, int aMode) :
  QDialog(parent),
  ui(new Ui::ArchiveEntry)
{
  ui->setupUi(this);

  if (0 == aMode)
    connect(ui->regalboden, SIGNAL(currentIndexChanged(int)), this, SLOT(handleRegalbodenIndexChange(int)), Qt::QueuedConnection);

  connect(ui->archivBoxNr, SIGNAL(currentIndexChanged(int)), this, SLOT(handleArchivBoxIndexChange(int)), Qt::QueuedConnection);

  mQueryModelRegal = 0;
  mQueryModelArchivbox = 0;
  mQueryModelArchivKapazitaet = 0;
  mQueryModelLiegenschaften = 0;
  mQueryModelDokumenttypen = 0;

  mQueryModelRegal = new QSqlQueryModel;
  mQueryModelArchivbox = new QSqlQueryModel;
  mQueryModelArchivKapazitaet = new QSqlQueryModel;
  mQueryModelLiegenschaften = new QSqlQueryModel;
  mQueryModelDokumenttypen = new QSqlQueryModel;

  mSelectedRegalboden = -1;
  mSelectedArchivbox = -1;
  mInitialRegalboden = -1;
  mInitialArchivbox = -1;
  mHouseId = -1;
  mDocTypeId = -1;
  mMisc = "";
  mSelectedYear = -1;
  mInitArchivboxLocked = false;

  mIdArchiveintrag = -1;

  mMode = aMode;

  updateData ();
}

ArchiveEntry::~ArchiveEntry()
{
  delete mQueryModelRegal;
  delete mQueryModelArchivbox;
  delete mQueryModelArchivKapazitaet;
  delete mQueryModelLiegenschaften;
  delete mQueryModelDokumenttypen;
  delete ui;
}

void ArchiveEntry::updateData()
{
  QSqlDatabase* db = Database::getInstance()->getDatabase();

  if (0 != db)
  {
    db->open();

    QSqlQuery query (*db);

    query.prepare("SELECT DISTINCT Regalboden FROM Ablage ORDER BY Regalboden ASC");
    query.exec();

    mQueryModelRegal->setQuery(query);

    ui->regalboden->setModel(mQueryModelRegal);
    ui->regalboden->setModelColumn(0);

    int selectedRow = ui->regalboden->currentIndex();
    setRegalboden (ui->regalboden->model()->index(selectedRow,0).data().toInt());

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
      QStringList list;
      list << "unbekannt" << "0" << "25" << "50" << "75" << "100";
      ui->archivBoxKap->insertItems(0, list);

      //Liegenschaften
      query.clear();
      query.prepare("SELECT * FROM Liegenschaft");
      query.exec();

      mQueryModelLiegenschaften->setQuery(query);

      ui->liegenschaft->setModel(mQueryModelLiegenschaften);
      ui->liegenschaft->setModelColumn(1);

      //Dokumenttypen
      query.clear();
      query.prepare("SELECT * FROM Dokumenttyp");
      query.exec();

      mQueryModelDokumenttypen->setQuery(query);

      ui->dokmentTyp->setModel(mQueryModelDokumenttypen);
      ui->dokmentTyp->setModelColumn(1);

      //Wirtschaftsjahre
      list.clear();
      int i = 1995;

      for (i;i<2050;i++)
      {
        list << QString::number(i);
      }

      ui->wirtschaftsjahr->insertItems(0, list);
      db->close();
    }
  }
}

void ArchiveEntry::setMode(int aType) //0-insert (default), 1-update
{
  mMode = aType;
}

void ArchiveEntry::setValues (int aIdArchiveintrag)
{
  mIdArchiveintrag = aIdArchiveintrag;
  QSqlDatabase* db = Database::getInstance()->getDatabase();

  if (0 != db)
  {
    if (db->open())
    {
      QSqlQuery query (*db);

      //Archiveintrag
      query.clear();
      query.prepare("SELECT id_dokumenttyp, dokumenttyp_sonstiges, wirtschaftsjahr, id_ablage, id_liegenschaft FROM Archiveintrag WHERE id = :id");
      query.bindValue(":id", aIdArchiveintrag);
      query.exec();

      QSqlDatabase defaultDB = QSqlDatabase::database();
      int numRows = 0;

      if (defaultDB.driver()->hasFeature(QSqlDriver::QuerySize))
      {
        numRows = query.size();
      }
      else
      {
        // this can be very slow
        query.last();
        numRows = query.at() + 1;
      }

      int id_ablage = -1;

      if (numRows > 0)
      {
        this->setDokumenttyp(query.value(0).toInt()-1);
        ui->dokmentTyp->setCurrentIndex(getDokumenttyp());

        this->setDokumentSonstiges(query.value(1).toString());
        ui->dokumentTypSonstigesDescr->setText(getDokumentSonstiges());

        this->setWirtschaftsjahr(query.value(2).toInt());
        ui->wirtschaftsjahr->setCurrentText(query.value(2).toString());

        id_ablage = query.value(3).toInt();
        this->setLiegenschaft(query.value(4).toInt());

        //Liegenschaft Name setzen
        query.clear();
        query.prepare("SELECT Liegenschaft FROM Liegenschaft WHERE id = :id");
        query.bindValue(":id", getLiegenschaft());
        query.exec();
        query.next();
        ui->liegenschaft->setCurrentText(query.value(0).toString());

        query.clear();
        query.prepare("SELECT Regalboden,Archivbox,Fuellstand FROM Ablage WHERE id = :id");
        query.bindValue(":id", id_ablage);
        query.exec();
        query.next();

        int lRegalboden = query.value(0).toInt();
        int lArchivboxNummer = query.value(1).toInt();
        QString lArchivboxKapazitaet = query.value(2).toString();

        //update Archivboxnummern
        query.clear();
        query.prepare("SELECT DISTINCT Archivbox FROM Ablage WHERE Regalboden = :id ORDER BY Archivbox ASC;");
        query.bindValue(":id", lRegalboden);
        query.exec();

        mQueryModelArchivbox->setQuery(query);

        ui->archivBoxNr->setModel(mQueryModelArchivbox);
        ui->archivBoxNr->setModelColumn(0);

        if (defaultDB.driver()->hasFeature(QSqlDriver::QuerySize))
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
          this->setRegalboden(lRegalboden);
          this->setArchivboxNummer(lArchivboxNummer);
          this->setArchivboxKapazitaet(lArchivboxKapazitaet);

          ui->regalboden->setCurrentText(QString::number(lRegalboden));
          ui->archivBoxNr->setCurrentText(QString::number(lArchivboxNummer));
        }

        //save current Regalboden/Archivboxnummer
        mInitialRegalboden = getRegalboden();
        mInitialArchivbox = getArchivboxNummer();
      }
    }
    db->close();
    connect(ui->regalboden, SIGNAL(currentIndexChanged(int)), this, SLOT(handleRegalbodenIndexChange(int)), Qt::QueuedConnection);
  }
}

void ArchiveEntry::updateDatabase()
{

  if (mInitialRegalboden != getRegalboden()
      || mInitialArchivbox != getArchivboxNummer())
  {
    //delete old entry and insert new entry
  }
  else
  {
    //update entry
  }
}

void ArchiveEntry::setRegalboden (int aId)
{
  mSelectedRegalboden = aId;
}

void ArchiveEntry::setArchivboxNummer (int aId)
{
  mSelectedArchivbox = aId;
}

void ArchiveEntry::setArchivboxKapazitaet (QString aValue)
{
  mSelectedArchivboxKap = aValue;
}

void ArchiveEntry::setLiegenschaft (int aId)
{
  mHouseId = aId;
}

void ArchiveEntry::setDokumenttyp (int aId)
{
  mDocTypeId = aId;
}

void ArchiveEntry::setDokumentSonstiges (QString aValue)
{
  mMisc = aValue;
}

void ArchiveEntry::setWirtschaftsjahr (int aValue)
{
  mSelectedYear = aValue;
}

void ArchiveEntry::setHeader (QString aValue)
{
  this->ui->header->setText(aValue);
}

int ArchiveEntry::getRegalboden ()
{
  return mSelectedRegalboden;
}

int ArchiveEntry::getArchivboxNummer ()
{
  return mSelectedArchivbox;
}

QString ArchiveEntry::getArchivboxKapazitaet ()
{
  return mSelectedArchivboxKap;
}

int ArchiveEntry::getLiegenschaft ()
{
  return mHouseId;
}

int ArchiveEntry::getDokumenttyp ()
{
  return mDocTypeId;
}

QString ArchiveEntry::getDokumentSonstiges ()
{
  return mMisc;
}

int ArchiveEntry::getWirtschaftsjahr ()
{
  return mSelectedYear;
}

void ArchiveEntry::handleRegalbodenIndexChange(int index)
{
  qDebug () << "handleRbIndex";

  int selectedRow = ui->regalboden->currentIndex();
  setRegalboden (ui->regalboden->model()->index(selectedRow,0).data().toInt());

  initArchivboxNumber ();
}

void ArchiveEntry::initArchivboxNumber ()
{
  QSqlDatabase* db = Database::getInstance()->getDatabase();

  if (0 != db)
  {
    db->open();

    QSqlQuery query (*db);
    //update Ablage
    query.clear();
    query.prepare("SELECT DISTINCT Archivbox FROM Ablage WHERE Regalboden = :id ORDER BY Archivbox ASC;");
    query.bindValue(":id", this->getRegalboden());
    query.exec();

    mQueryModelArchivbox->setQuery(query);

    ui->archivBoxNr->setModel(mQueryModelArchivbox);
    ui->archivBoxNr->setModelColumn(0);

    int selectedRow = -1;
    selectedRow = ui->archivBoxNr->currentIndex();
    setArchivboxNummer (ui->archivBoxNr->model()->index(selectedRow,0).data().toInt());

    db->close();
  }
}

void ArchiveEntry::on_buttonBox_accepted()
{
  this->setArchivboxKapazitaet(ui->archivBoxKap->currentText());

  int selectedRow = ui->liegenschaft->currentIndex();
  int id = ui->liegenschaft->model()->index(selectedRow,0).data().toInt();
  this->setLiegenschaft(id);

  selectedRow = ui->dokmentTyp->currentIndex();
  id = ui->dokmentTyp->model()->index(selectedRow,0).data().toInt();
  this->setDokumenttyp(id);

  selectedRow = ui->archivBoxNr->currentIndex();
  id = ui->archivBoxNr->model()->index(selectedRow,0).data().toInt();
  this->setArchivboxNummer(id);
  this->setDokumentSonstiges (ui->dokumentTypSonstigesDescr->text());
  this->setWirtschaftsjahr(ui->wirtschaftsjahr->currentText().toInt());

  //Membervariablen plausibel?
  if (  mSelectedRegalboden != -1 &&
        mSelectedArchivbox !=  -1 &&
        mHouseId != -1 &&
        mDocTypeId != -1 &&
        mSelectedYear != -1)
  {
    //alles ok

    QSqlDatabase* db = Database::getInstance()->getDatabase();

    db->open();

    if (mMode == 0) // insert
    {
      insertDbEntry();
    }
    else
    {
      updateDbEntry(); //update
    }

    db->close();
  }
  else
  {
    QMessageBox msgBox;
    msgBox.setText("Verarbeitungsfehler: ArchiveEntry::on_buttonBox_accepted() - Abbruch");
    msgBox.setStandardButtons(QMessageBox::Ok);
  }

  this->accept();
}

bool ArchiveEntry::insertDbEntry()
{
  QSqlDatabase* db = Database::getInstance()->getDatabase();

  if (0 != db)
  {
    if (db->isOpen())
    {
      QSqlQuery query (*db);
      //update Ablage
      query.clear();

      //Eintrag vorhanden
      query.prepare("SELECT id FROM Ablage WHERE Regalboden=:regalboden AND Archivbox=:archivbox;");
      query.bindValue(":regalboden", this->getRegalboden());
      query.bindValue(":archivbox", this->getArchivboxNummer());
      query.exec();

      QSqlDatabase defaultDB = QSqlDatabase::database();
      int numRows = 0;
      int idAblage = -1;
      bool newEntry = false;

      if (defaultDB.driver()->hasFeature(QSqlDriver::QuerySize))
      {
        numRows = query.size();
      }
      else
      {
        // this can be very slow
        query.last();
        numRows = query.at() + 1;
      }

      if (numRows > 0) //Fuellstand in allen "Ablage"-Tabellen auf neuen Wert bringen
      {
        idAblage = query.value(0).toInt();
        query.prepare("UPDATE Ablage SET Fuellstand=:fuellstand WHERE Regalboden=:regalboden AND Archivbox=:archivbox;");
        query.bindValue(":regalboden", this->getRegalboden());
        query.bindValue(":archivbox", this->getArchivboxNummer());
        query.bindValue(":fuellstand", this->getArchivboxKapazitaet());
        query.exec();
      }
      else  //neue Regalboden/Archivbox Kombination - kann nicht passieren
      {
        QMessageBox msgBox;
        msgBox.setText("Verarbeitungsfehler: ArchiveEntry::insertDbEntry() - Abbruch");
        msgBox.setStandardButtons(QMessageBox::Ok);
        return false;
      }

      //neuen Archiveintrag in Datenbank eintragen
      query.clear();
      query.prepare("INSERT INTO Archiveintrag (id_Ablage, id_Liegenschaft, id_dokumenttyp, dokumenttyp_sonstiges, wirtschaftsjahr, bearbeiter) VALUES (:idAblage, :idLiegenschaft, :idDokumentTyp, :dokumentSonstiges, :jahr, :bearb)");

      //Bearbeiter
      QString path = QApplication::applicationDirPath() + "/cfg/config.ini";
      QSettings settings (path, QSettings::IniFormat);

      QString bearbeiter = settings.value("settings/bearbeiter", "unbekannt").toString();

      query.bindValue(":idAblage", idAblage);
      query.bindValue(":idLiegenschaft", this->getLiegenschaft());
      query.bindValue(":idDokumentTyp", this->getDokumenttyp());
      query.bindValue(":dokumentSonstiges", this->getDokumentSonstiges());
      query.bindValue(":jahr", this->getWirtschaftsjahr());
      query.bindValue(":bearb", bearbeiter);
      query.exec();

      return true;
    }
  }
  return false;
}

bool ArchiveEntry::updateDbEntry()
{
  if (-1 != mIdArchiveintrag)
  {
    QSqlDatabase* db = Database::getInstance()->getDatabase();

    if (0 != db)
    {
      if (db->isOpen())
      {
        QSqlQuery query (*db);
        query.clear();

        //Fuellstand updaten

        //Test ob Eintrag vorhanden
        query.prepare("SELECT id FROM Ablage WHERE Regalboden=:regalboden AND Archivbox=:archivbox;");
        query.bindValue(":regalboden", this->getRegalboden());
        query.bindValue(":archivbox", this->getArchivboxNummer());
        query.exec();

        QSqlDatabase defaultDB = QSqlDatabase::database();
        int numRows = 0;

        if (defaultDB.driver()->hasFeature(QSqlDriver::QuerySize))
        {
          numRows = query.size();
        }
        else
        {
          // this can be very slow
          query.last();
          numRows = query.at() + 1;
        }

        if (numRows > 0) //Fuellstand in allen "Ablage"-Tabellen auf neuen Wert bringen
        {
          query.clear();
          query.prepare("UPDATE Ablage SET Fuellstand=:fuellstand WHERE Regalboden=:regalboden AND Archivbox=:archivbox;");
          query.bindValue(":regalboden", this->getRegalboden());
          query.bindValue(":archivbox", this->getArchivboxNummer());
          query.bindValue(":fuellstand", this->getArchivboxKapazitaet());
          query.exec();
        }
        else  //neue Regalboden/Archivbox Kombination - kann nicht passieren
        {
          QMessageBox msgBox;
          msgBox.setText("Verarbeitungsfehler: ArchiveEntry::insertDbEntry() - Abbruch");
          msgBox.setStandardButtons(QMessageBox::Ok);
          return false;
        }

        //Regalboden oder Archivboxnummer wurde geändert -> Neueintrag nötig
        if (mInitialArchivbox != getArchivboxNummer() ||
            mInitialRegalboden != getRegalboden())
        {
          if (true == insertDbEntry ()) //alles ok?
          {
            //Archiveintrag in Datenbank löschen
            query.clear();
            query.prepare("DELETE FROM Archiveintrag WHERE id=:idArchiveintrag");
            query.bindValue(":idArchiveintrag", mIdArchiveintrag);
            query.exec();
          }
          else
          {
            return false;
          }
        }
        else
        {
          QSqlQuery query (*db);
          query.clear();

          //Archiveintrag in Datenbank updaten
          query.prepare("UPDATE Archiveintrag SET id_liegenschaft=:idLiegenschaft, id_dokumenttyp=:idDok, dokumenttyp_sonstiges=:idDokSonstiges, wirtschaftsjahr=:wiJahr, bearbeiter=:bearb WHERE id=:idAblage;");

          //Bearbeiter
          QString path = QApplication::applicationDirPath() + "/cfg/config.ini";
          QSettings settings (path, QSettings::IniFormat);

          QString bearbeiter = settings.value("settings/bearbeiter", "unbekannt").toString();

          query.bindValue(":idLiegenschaft", this->getLiegenschaft());
          query.bindValue(":idDok", this->getDokumenttyp());
          query.bindValue(":idDokSonstiges", this->getDokumentSonstiges());
          query.bindValue(":wiJahr", this->getWirtschaftsjahr());
          query.bindValue(":idAblage", mIdArchiveintrag);
          query.bindValue(":bearb", bearbeiter);
          query.exec();
        }
        return true;
      }
    }
  }

  QMessageBox msgBox;
  msgBox.setText("Verarbeitungsfehler: ArchiveEntry::updateDbEntry() - Abbruch");
  msgBox.setStandardButtons(QMessageBox::Ok);

  return false;
}

void ArchiveEntry::on_buttonBox_rejected()
{
  this->reject();
}

void ArchiveEntry::handleArchivBoxIndexChange(int index)
{
  int selectedRow = ui->archivBoxNr->currentIndex();
  setArchivboxNummer(ui->archivBoxNr->model()->index(selectedRow,0).data().toInt());

  updateFuellstand ();
}

void ArchiveEntry::updateFuellstand ()
{
  QSqlDatabase* db = Database::getInstance()->getDatabase();

  if (0 != db)
  {
    db->open();

    QSqlQuery query (*db);
    query.clear();
    query.prepare("SELECT Fuellstand FROM Ablage WHERE Archivbox = :id AND Regalboden = :idRegalboden");
    query.bindValue(":id", this->getArchivboxNummer());
    query.bindValue(":idRegalboden", this->getRegalboden());

    query.exec();
    query.next();

    if ("unbekannt" == query.value(0).toString())
      ui->archivBoxKap->setCurrentIndex(0);
    if ("0" == query.value(0).toString())
      ui->archivBoxKap->setCurrentIndex(1);
    if ("25" == query.value(0).toString())
      ui->archivBoxKap->setCurrentIndex(2);
    if ("50" == query.value(0).toString())
      ui->archivBoxKap->setCurrentIndex(3);
    if ("75" == query.value(0).toString())
      ui->archivBoxKap->setCurrentIndex(4);
    if ("100" == query.value(0).toString())
      ui->archivBoxKap->setCurrentIndex(5);

    mQueryModelArchivKapazitaet->setQuery(query);

    int selectedRow = ui->archivBoxKap->currentIndex();
    setArchivboxKapazitaet(ui->archivBoxKap->model()->index(selectedRow,0).data().toString());

    db->close();
  }
}
