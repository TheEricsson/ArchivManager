#ifndef ARCHIVEENTRY_H
#define ARCHIVEENTRY_H

#include <QDialog>

namespace Ui {
  class ArchiveEntry;
}

class QSqlQueryModel;

class ArchiveEntry : public QDialog
{
  Q_OBJECT

public:
  explicit ArchiveEntry(QWidget *parent = 0, int aMode = 0);
  ~ArchiveEntry();
  void updateData ();
  void setMode(int aType); //0-insert (default), 1-update
  void setValues (int aIdArchiveintrag);
  void updateDatabase ();
  void setRegalboden      (int aId);
  void setArchivboxNummer (int aId);
  void setArchivboxKapazitaet (QString aValue);
  void setLiegenschaft    (int aId);
  void setDokumenttyp     (int aId);
  void setDokumentSonstiges (QString aValue);
  void setWirtschaftsjahr (int aValue);
  void setHeader (QString aValue);

  int getRegalboden ();
  int getArchivboxNummer ();
  QString getArchivboxKapazitaet ();
  int getLiegenschaft ();
  int getDokumenttyp ();
  QString getDokumentSonstiges ();
  int getWirtschaftsjahr ();
  bool insertDbEntry ();
  bool updateDbEntry ();

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();
  void handleRegalbodenIndexChange(int index);
  void handleArchivBoxIndexChange(int index);
  void updateFuellstand();

private:

  void initArchivboxNumber ();

  QSqlQueryModel* mQueryModelRegal;
  QSqlQueryModel* mQueryModelArchivbox;
  QSqlQueryModel* mQueryModelArchivKapazitaet;
  QSqlQueryModel* mQueryModelLiegenschaften;
  QSqlQueryModel* mQueryModelDokumenttypen;
  Ui::ArchiveEntry *ui;

  int mSelectedRegalboden;
  int mSelectedArchivbox;
  int mInitialRegalboden;
  int mInitialArchivbox;
  int mIdArchiveintrag;
  bool mInitArchivboxLocked;

  QString mSelectedArchivboxKap;
  int mHouseId;
  int mDocTypeId;
  QString mMisc;
  int mSelectedYear;
  int mMode;
};

#endif // ARCHIVEENTRY_H
