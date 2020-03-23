#include "iSpecIDApp/resultsmodel.h"
#include <iostream>

ResultsModel::ResultsModel(QObject *parent)
    :QAbstractTableModel(parent)
{
    this->_results = std::vector<int>(this->rowCount(),0);
    this->_sum = 0;

}

int ResultsModel::rowCount(const QModelIndex & /*parent*/) const
{
   return 5;
}

int ResultsModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

void ResultsModel::setResults(std::vector<int> results){
    _sum = 0;
    size_t count = _results.size();
    for(size_t i = 0; i < count; i++){
        auto count_index = this->index(i,1,QModelIndex());
        auto val = results[i];
        _sum += val;
        setData(count_index, results[i]);
    }
    std::cout << _sum << std::endl;
    _sum = 1/_sum;
    for(size_t i = 0; i < count; i++){
        auto perc_index = this->index(i,2,QModelIndex());
        setData(perc_index, QVariant());
    }
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
                return QString("Perc");
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
        char grade = 'A' + row;
       return QString("%1")
                   .arg(grade);
    }else if(col == 1 && role == Qt::DisplayRole){
        return _results.at(row);
    }else if(col == 2 && role == Qt::DisplayRole){
        return _sum == 0?0:_results.at(row)*_sum;
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
                _results.at(row) = count;
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
