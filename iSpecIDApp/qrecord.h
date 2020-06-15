#ifndef QRECORD_H
#define QRECORD_H
#include "datatypes.h"
#include <QStringList>
#include <qdebug.h>
typedef struct qrecord{
    QStringList ids;
    QString modification;
    ispecid::datatypes::Record record;
    bool joinRecord(qrecord& rec){
        if(rec.record.getCluster() == record.getCluster()
                && rec.record.getSource() == record.getSource()
                && rec.record.getSpeciesName() == record.getSpeciesName()){
            record.addCount(rec.record.count());
            ids.append(rec.ids);
            return true;
        }
        return false;
    }
    void precord(){
        qDebug() << QString::fromStdString(record.getSpeciesName()) << QString::fromStdString(record.getSource()) << QString::fromStdString(record.getCluster());
    }

}QRecord;


#endif // QRECORD_H
