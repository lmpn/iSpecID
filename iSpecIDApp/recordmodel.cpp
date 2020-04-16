#include <QItemSelectionRange>
#include <QDataStream>
#include "csv.hpp"
#include "recordmodel.h"
#include <qdebug.h>

RecordModel::RecordModel(QObject *parent,IEngine *engine)
    : QAbstractTableModel(parent), cur_count(0), engine(engine)
{
    remove = false;
}


int RecordModel::rowCount(const QModelIndex & /*parent*/) const
{
    return cur_count;//engine->getEntries().size();
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
    auto& records = engine->getEntries();
    int row = index.row();
    int col = index.column();
    int size = records.size();


    if (!index.isValid())
        return QVariant();
    if (index.row() >= size)
        return QVariant();
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
    return success;
}

bool RecordModel::removeRows(int position, int rows, const QModelIndex &parent){
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), position, position+rows-1);
    auto& records = engine->getEntries();
    for (int row=0; row < rows; ++row) {
        if(remove){
            records.erase(records.begin()+position);
            cur_count--;
        }
    }
    endRemoveRows();
    return true;
}

void RecordModel::onRecordsChange(){
    beginResetModel();
    if(cur_count!=0){
        removeRows(0, cur_count, QModelIndex());
    }
    auto& records = engine->getEntries();
    std::sort(records.begin(), records.end(), [](Record a, Record b){ return a["species_name"].compare(b["species_name"]) < 0;});
    cur_count = records.size();
    insertRows(0, cur_count, QModelIndex());
    endResetModel();
}

bool RecordModel::setData(const QModelIndex &index, const QVariant &value, int role){
    int row = index.row();
    int col = index.column();
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
        auto& records = engine->getEntries();
        auto& entry = records[row];
        auto mod_value = entry["modification"];
        if(field_value != entry[field_name]){
            emit actionPerformed();
            entry.update(field_value,field_name);
            if(mod_value.empty()){
                mod_value += field_name;
            }else{
                mod_value = mod_value + ";" + field_name;
            }
            entry.update(mod_value, "modification");
            engine->group();
            emit dataChanged(index, index, {Qt::DisplayRole});
            emit updateGraph();
            emit updateComboBox();
        }
        return true;
    }
    return false;
}



Qt::ItemFlags RecordModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}


