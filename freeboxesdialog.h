#ifndef FREEBOXESDIALOG_H
#define FREEBOXESDIALOG_H

#include <QDialog>

class QSqlQueryModel;

namespace Ui {
  class FreeBoxesDialog;
}

class FreeBoxesDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FreeBoxesDialog(QWidget *parent = 0);
  ~FreeBoxesDialog();

private:
  Ui::FreeBoxesDialog *ui;
  QSqlQueryModel* mQueryModel;
};

#endif // FREEBOXESDIALOG_H
