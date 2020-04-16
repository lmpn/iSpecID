#include "resultsmodel.h"
#include <iostream>
#include <qdebug.h>

ResultsModel::ResultsModel(QObject *parent, IEngine *engine)
    :QAbstractTableModel(parent), engine(engine)
{
}

int ResultsModel::rowCount(const QModelIndex & /*parent*/) const
{
   return 6;
}

int ResultsModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}


void ResultsModel::on_results_changed(){
    results = engine->calculateGradeResults();
    perc = 1.f/engine->getEntries().size();
    auto top_left = index(0,1,QModelIndex());
    auto bottom_right = index(rowCount(),columnCount(),QModelIndex());
    emit dataChanged(top_left, bottom_right, {Qt::EditRole});
}




QVariant ResultsModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section) {
            case 0:
                return QString("Grade");
            case 1:
                return QString("Count");
            case 2:
                return QString("Frequency (%)");
            default:
                return QVariant();

        }
    }else if(role == Qt::DisplayRole && orientation == Qt::Vertical){
        return section;
    }
    return QVariant();
}


QVariant ResultsModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (col == 0 && role == Qt::DisplayRole)
    {
        if(row == rowCount()-1)
            return QString("Total");
        char grade = 'A' + row;
        return QString("%1")
                   .arg(grade);
    }else if(col == 1 && role == Qt::DisplayRole){
        if(results.size() == 0) return 0;
        return results.at(row);
    }else if(col == 2 && role == Qt::DisplayRole){
        if(results.size() == 0) return 0;
        return QString::number(results.at(row)*perc*100, 'G', 2);
    }
    return QVariant();
}


bool ResultsModel::setData(const QModelIndex &index, const QVariant &value, int role){
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
}
