#include "iSpecIDApp/recordmodel.h"
#include <QItemSelectionRange>

RecordModel::RecordModel(QObject *parent, Annotator *an)
    :QAbstractTableModel(parent)
{
    this->an = an;
    auto tmp = an->getRecords();
    records = QVector<Record>(tmp.begin(), tmp.end());

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
    if (col == 0 && (role == Qt::DisplayRole ||
            role == Qt::EditRole)) {
        return QString::fromStdString( records[row]["species_name"] );
    }else if(col == 1 && (role == Qt::DisplayRole ||
            role == Qt::EditRole)) {
        return QString::fromStdString( records.at(row)["bin_uri"] );
    }else if(col == 2 && (role == Qt::DisplayRole ||
            role == Qt::EditRole)) {
        return QString::fromStdString( records[row]["institution_storing"]);
    }else if(col == 3 && role == Qt::DisplayRole){
        return QString::fromStdString( records[row]["grade"]);
    }
    return QVariant();
}


bool RecordModel::removeRow(int row, const QModelIndex &parent)
{
    bool success = removeRows(row,1,parent);
    PRINT(records.size());
    return success;
}

bool RecordModel::removeRows(int position, int rows, const QModelIndex &parent){
    Q_UNUSED(parent);
    beginRemoveRows(parent,position, position+rows);
    for (int row=0; row < rows; ++row) {
        records.erase(records.begin()+position);
    }
    endRemoveRows();
    return true;
}

bool RecordModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);
    endInsertRows();
    return true;
}

void RecordModel::onRecordsChange(){
    beginResetModel();
    size_t cur_count = this->records.size();
    if(cur_count!=0){
        removeRows(0, cur_count, QModelIndex());
    }
    auto tmp = an->getRecords();
    records = QVector<Record>(tmp.begin(), tmp.end());
    cur_count = this->records.size();
    insertRows(0, cur_count, QModelIndex());
    endResetModel();
}

bool RecordModel::setData(const QModelIndex &index, const QVariant &value, int role){
    int row = index.row();
    int col = index.column();
    PRINT("row: " << row);
    PRINT("col: " << col);
    if(role == Qt::EditRole && col != 3){
        std::string field_name;
        switch(col){
            case 0:
                field_name = "species_name";
                break;
            case 1:
                field_name = "bin_uri";
                break;
            case 2:
                field_name = "institution_storing";
                break;
            default:
                return false;
        }
        auto field_value = value.toString().toStdString();
        records[row].update(field_value,field_name);
        emit dataChanged(index, index, {Qt::DisplayRole});
        return true;
    }
    return false;
}



Qt::ItemFlags RecordModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) ;
}
