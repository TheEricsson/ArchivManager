#ifndef NEWFUELLSTANDDIALOG_H
#define NEWFUELLSTANDDIALOG_H

#include <QDialog>

namespace Ui {
  class newFuellstandDialog;
}

class newFuellstandDialog : public QDialog
{
  Q_OBJECT

public:
  explicit newFuellstandDialog(QWidget *parent = 0);
  ~newFuellstandDialog();
  QString getCurrentSelection ();

private:
  Ui::newFuellstandDialog *ui;
};

#endif // NEWFUELLSTANDDIALOG_H
