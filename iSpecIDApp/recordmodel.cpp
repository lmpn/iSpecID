#include "iSpecIDApp/recordmodel.h"
#include <QItemSelectionRange>

RecordModel::RecordModel(QObject *parent, std::vector<Record> data)
    :QAbstractTableModel(parent)
{
    records = data;
}


int RecordModel::rowCount(const QModelIndex & /*parent*/) const
{
    return records.size();
}

int RecordModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 4;
}

QVariant RecordModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section) {
        case 0:
            return QString("Species");
        case 1:
            return QString("Bin");
        case 2:
            return QString("Institution");
        case 3:
            return QString("Grade");
        default:
            return QVariant();

        }
    }else if(role == Qt::DisplayRole && orientation == Qt::Vertical){
        return section;
    }
    return QVariant();
}

QVariant RecordModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (col == 0 && role == Qt::DisplayRole) {
        return QString::fromStdString( records[row]["species_name"] );
    }else if(col == 1 && role == Qt::DisplayRole){
        return QString::fromStdString( records.at(row)["bin_uri"] );
    }else if(col == 2 && role == Qt::DisplayRole){
        return QString::fromStdString( records[row]["institution_storing"]);
    }else if(col == 3 && role == Qt::DisplayRole){
        char grade = 'A' + ((int)records[row].getGrade());
        return QChar(grade);
    }
    return QVariant();
}


bool RecordModel::removeRow(int row, const QModelIndex &parent)
{
    return removeRows(row,1,parent);
}

bool RecordModel::removeRows(int position, int rows, const QModelIndex &parent){
    beginRemoveRows(parent,position, position+rows);
    for (int row=0; row < rows; ++row) {
        records.erase(records.begin()+position);
    }
    endRemoveRows();
    return true;
}

void RecordModel::setRecords(std::vector<Record>& records){
    size_t cur_count = this->records.size();
    removeRows(0, cur_count, QModelIndex());
    this->records = records;
    auto top_left = this->index(0,1,QModelIndex());
    auto bottom_right = this->index(rowCount(),columnCount(),QModelIndex());
    emit dataChanged(top_left, bottom_right, {Qt::EditRole});
}
bool RecordModel::setData(const QModelIndex &index, const QVariant &value, int role){
    int row = index.row();
    int col = index.column();
    if(role == Qt::EditRole && col != 3){
        auto field_name = this->headerData(col,Qt::Horizontal, Qt::DisplayRole).toString().toStdString();
        auto field_value = value.toString().toStdString();
        records[row].update(field_value,field_name);
        emit dataChanged(index, index, {Qt::DisplayRole});
        return true;
    }
    return false;
}
