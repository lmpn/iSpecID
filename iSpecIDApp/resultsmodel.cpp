#include "iSpecIDApp/resultsmodel.h"
#include <iostream>
#include <qdebug.h>

ResultsModel::ResultsModel(QObject *parent, Annotator *an)
    :QAbstractTableModel(parent)
{
    this->an = an;
    this->an->calculateGradeResults();
    this->results = this->an->getGradeResults();
    this->perc = this->an->size() == 0 ? 0 : 1/this->an->size();

}

int ResultsModel::rowCount(const QModelIndex & /*parent*/) const
{
   return results.size();
}

int ResultsModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}


void ResultsModel::onResultsChange(){
    this->an->calculateGradeResults();
    this->results = an->getGradeResults();
    this->perc = this->an->size() == 0 ? 0 : 1.f/this->an->size();
    auto top_left = this->index(0,1,QModelIndex());
    auto bottom_right = this->index(rowCount(),columnCount(),QModelIndex());
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
                return QString("Weight");
            default:
                return QVariant();

        }
    }else if(role == Qt::DisplayRole && orientation == Qt::Vertical){
        return section;
    }
    return QVariant();
}

//NIKE M2K tEKNO DESERT SAND/PHANTOM

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
        return results.at(row);
    }else if(col == 2 && role == Qt::DisplayRole){
        return QString::number(results.at(row)*perc*100, 'G', 4);
    }
    return QVariant();
}


bool ResultsModel::setData(const QModelIndex &index, const QVariant &value, int role){
    int row = index.row();
    int col = index.column();
    if(role == Qt::EditRole){
        if(col == 1){
            bool valid;
            auto count = value.toInt(&valid);
            if(valid){
                results.at(row) = count;
                emit dataChanged(index, index, {role});
            }
            return valid;
        }
        else if (col == 2){
            emit dataChanged(index, index, {role});
            return true;
        }
    }
    return false;
}
