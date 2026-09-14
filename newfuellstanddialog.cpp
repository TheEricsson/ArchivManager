#include "newfuellstanddialog.h"
#include "ui_newfuellstanddialog.h"

newFuellstandDialog::newFuellstandDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::newFuellstandDialog)
{
  ui->setupUi(this);

  QStringList list;
  list << "unbekannt" << "0" << "25" << "50" << "75" << "100";
  ui->fuellstand->insertItems (0, list);
}

newFuellstandDialog::~newFuellstandDialog()
{
  delete ui;
}

QString newFuellstandDialog::getCurrentSelection ()
{
  return ui->fuellstand->currentText();
}
