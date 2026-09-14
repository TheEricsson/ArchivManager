#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
//#include "global.h"

class Database
{
public:
    static Database *getInstance();
    void setDatabase (QSqlDatabase* pDb);
    QSqlDatabase* getDatabase ();


    //does not really belong here (to lazy to move this somewhere else and it does its job)
    int getCurrentPropertyId ();
    void setCurrentPropertyId (int aId);
    void setCurrentEtvNumber (int aNumber);
    int getCurrentEtvNumber ();
    int getCurrentYear ();
    void setCurrentYear (int aYear);
    //const QString getTextPattern (eTextPattern aId);
    bool dbIsOk ();

protected:
    QSqlDatabase* mDb;

private:
    Database(void);
    ~Database(void);

    int mPropertyId;
    int mYear;
    int mEtvNumber;
};

#endif // DATABASE_H
