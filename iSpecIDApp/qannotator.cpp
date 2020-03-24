#include "qannotator.h"

QAnnotator::QAnnotator(QObject *parent) : QObject(parent)
{
    an = new Annotator();
}
