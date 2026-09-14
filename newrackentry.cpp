#include "newrackentry.h"
#include "ui_newrackentry.h"

newRackEntry::newRackEntry(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::newRackEntry)
{
  ui->setupUi(this);

  int i = 1;

  QStringList regalBoden;

  for (i;i<300;i++)
    regalBoden << QString::number(i);

  QStringList archivBox;

  for (i=1; i<20;i++)
    archivBox << QString::number(i);

  ui->regalboden->insertItems(0,regalBoden);
  ui->archivbox->insertItems(0,archivBox);
}

newRackEntry::~newRackEntry()
{
  delete ui;
}

int newRackEntry::getRegalbodenNr ()
{
  return ui->regalboden->currentText().toInt(0, 10);
}

int newRackEntry::getArchivboxNr ()
{
  return ui->archivbox->currentText().toInt(0, 10);
}

void newRackEntry::on_buttonBox_accepted()
{
  this->accept();
}

void newRackEntry::on_buttonBox_rejected()
{
  this->reject();
}
