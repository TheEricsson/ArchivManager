#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>

class QSqlQueryModel;

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  QSqlError addConnection(const QString &driver, const QString &dbName, const QString &host,
              const QString &user, const QString &passwd, int port = -1);
  ~MainWindow();

private slots:
  void zeigeFreieBoxen (bool aEnabled);
  void neuerArchivEintrag (bool aEnabled);
  void on_applyFilter_clicked();
  void on_deleteFilter_clicked();
  void on_buttonChangeEntry_clicked();
  void on_buttonNewEntry_clicked();
  void on_tableView_doubleClicked(const QModelIndex &index);
  void on_buttonDeleteEntry_clicked();
  void on_CbAnzeigeArt_currentIndexChanged(int index);

private:
  void updateHouseSelector ();
  void update ();
  void updateNoFilter ();
  void updateSetFilter (int id, int type = 0);

  Ui::MainWindow *ui;
  QSqlDatabase db;
  QSqlQueryModel* mQueryModel;
  QSqlQueryModel* mQueryModelHouses;

  int mFilterHouseId;
  bool mFilterShowDeleted;
};

#endif // MAINWINDOW_H
