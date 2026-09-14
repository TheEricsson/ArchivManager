#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QtSql>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  QString path = QApplication::applicationDirPath() + "/cfg/config.ini";
  //QString path = "cfg/config.ini";

  QSettings settings (path, QSettings::IniFormat);

  QSqlError err = w.addConnection(settings.value("database/type", "").toString(),
                     settings.value("database/name", "").toString(),
                     settings.value("database/host", "").toString(),
                     settings.value("database/user", "").toString(),
                     settings.value("database/pw", "").toString(),
                     settings.value("database/port", "").toInt());

/*  if (err.type() != QSqlError::NoError)
  {
      qDebug() << "Unable to open predefined connection:" << err;
      qDebug()    << "### VALUES ###" << std::endl
                  << "database/type: " << settings.value("database/type", "").toString() << endl
                  << "database/name: " << settings.value("database/name", "").toString() << endl
                  << "database/host: " << settings.value("database/host", "").toString() << endl
                  << "database/user: " << settings.value("database/user", "").toString() << endl
                  << "database/pw: " << settings.value("database/pw", "").toString() << endl
                  << "database/port: " << settings.value("database/port", "").toInt() << endl
                  << "##############";

  }
  else
  {
      qDebug() << "Opened predefined database connection";
      qDebug()    << "### VALUES ###" << endl
          << "database/type: " << settings.value("database/type", "").toString() << endl
          << "database/name: " << settings.value("database/name", "").toString() << endl
          << "database/host: " << settings.value("database/host", "").toString() << endl
          << "database/user: " << settings.value("database/user", "").toString() << endl
          << "database/pw: " << settings.value("database/pw", "").toString() << endl
          << "database/port: " << settings.value("database/port", "").toInt() << endl
          << "##############";
  }
*/

  if (QSqlDatabase::connectionNames().isEmpty())
      QMetaObject::invokeMethod(&w, "addConnection", Qt::QueuedConnection);

  return a.exec();
}
