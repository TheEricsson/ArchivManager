#ifndef NEWRACKENTRY_H
#define NEWRACKENTRY_H

#include <QDialog>

namespace Ui {
  class newRackEntry;
}

class newRackEntry : public QDialog
{
  Q_OBJECT

public:
  explicit newRackEntry(QWidget *parent = 0);
  int getRegalbodenNr ();
  int getArchivboxNr ();
  ~newRackEntry();

private slots:
  void on_buttonBox_accepted();

  void on_buttonBox_rejected();

private:
  Ui::newRackEntry *ui;
};

#endif // NEWRACKENTRY_H
