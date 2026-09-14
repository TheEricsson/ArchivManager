#include "database.h"

static int invalid = -999;

Database::Database()
{
    mDb = 0;
    mPropertyId = invalid;
    mYear = invalid;
    mEtvNumber = invalid;
}

Database::~Database()
{
  /*//fucks up otherwise
  {
    if (0 != mDb)
      delete mDb;
  }
  mDb = 0;*/
}

Database* Database::getInstance()
{
    static Database theInstance;
    return &theInstance;
}

void Database::setDatabase (QSqlDatabase* pDb)
{
    mDb = pDb;
}

/*QSqlError Database::open ()
{
  if (!mDb->open())
  {
    return mDb->lastError();
  }
  return true;
}*/

QSqlDatabase* Database::getDatabase ()
{
    return mDb;
}

/*const QString Database::getTextPattern (eTextPattern aId)
{
  QString retVal ("");

  if (dbIsOk())
  {
    QSqlQuery query (*mDb);
    query.prepare("SELECT Text FROM TextPatterns WHERE id = :id");
    query.bindValue(":id", (int)aId);
    query.exec();

    if (query.next())
      retVal = query.value(0).toString();
  }
  return retVal;
}*/

bool Database::dbIsOk()
{
  if (mDb)
  {
    if (mDb->isValid())
    {
        if (mDb->isOpen())
        {
            return true;
        }
    }
  }
  return false;
}

int Database::getCurrentPropertyId ()
{
    if (mPropertyId == invalid)
        qDebug() << "mPropertyId is not valid";

    return mPropertyId;
}

void Database::setCurrentPropertyId (int aId)
{
    mPropertyId = aId;
    qDebug() << "current property id: " << mPropertyId;
}

void Database::setCurrentEtvNumber (int aNumber)
{
    mEtvNumber = aNumber;
    qDebug() << "current etv number: " << mEtvNumber;
}

int Database::getCurrentEtvNumber ()
{
    if (mEtvNumber == invalid)
        qDebug() << "mEtvNumber is not valid";

    return mEtvNumber;
}

int Database::getCurrentYear ()
{
    if (mYear == invalid)
        qDebug() << "mYear is not valid";

    return mYear;
}

void Database::setCurrentYear (int aYear)
{
    mYear = aYear;
    qDebug() << "current year: " << mYear;
}
