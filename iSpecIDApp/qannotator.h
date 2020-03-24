#ifndef QANNOTATOR_H
#define QANNOTATOR_H

#include <QObject>
#include <annotator.h>

class QAnnotator : public QObject
{
    Q_OBJECT
public:
    explicit QAnnotator(QObject *parent = nullptr);

signals:

private:
    Annotator *an;
};

#endif // QANNOTATOR_H
