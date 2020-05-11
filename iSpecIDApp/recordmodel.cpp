#include <QItemSelectionRange>
#include <QDataStream>
#include "csv.hpp"
#include "recordmodel.h"
#include <qdebug.h>

RecordModel::RecordModel(IEngine *engine, QObject *parent)
    : QAbstractTableModel(parent), cur_count(0), engine(engine)
{
    remove = false;
    sort_order = QVector<bool>(this->columnCount(), true);
    last_col = -1;
}

void sort_column(std::vector<Record>& entries, std::string key, bool order){
    if(order)
        std::sort(entries.begin(), entries.end(), [&key](Record a, Record b){ return a[key].compare(b[key]) < 0;});
    else
        std::sort(entries.begin(), entries.end(), [&key](Record a, Record b){ return a[key].compare(b[key]) > 0;});
}




void RecordModel::sortBySection(int col){

    auto& entries = engine->getEntries();
    if(col== 0){
        if(last_col == col)
            sort_order[col]=!sort_order[col];
        sort_column(entries, "species_name" , sort_order[col]);
    }
    else if(col== 1){
        if(last_col == col)
            sort_order[col]=!sort_order[col];
        sort_column(entries, "bin_uri" , sort_order[col]);
    }
    else if(col== 2){
        if(last_col == col)
            sort_order[col]=!sort_order[col];
        sort_column(entries, "institution_storing" , sort_order[col]);
    }
    else if(col== 3){
        if(last_col == col)
            sort_order[col]=!sort_order[col];
        sort_column(entries, "grade" , sort_order[col]);
    }
    last_col = col;
    auto topLeft = this->index(0,0);
    auto rightBottom = this->index(entries.size(),columnCount());
    emit dataChanged(topLeft, rightBottom, {Qt::DisplayRole});
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
        QString sort = "";
        if(last_col == section){
            sort = sort_order[last_col] ? "(ASC)" : "(DESC)";
        }
        switch (section) {
        case 0:
            return QString("Species") + sort;
        case 1:
            return QString("Bin")+sort;
        case 2:
            return QString("Source")+sort;
        case 3:
            return QString("Grade")+sort;
        default:
            return QVariant();

        }
    }else if(role == Qt::DecorationRole && orientation == Qt::Horizontal)
    {
        switch (section) {
        case 0:
            return QString("Species");
        case 1:
            return QString("Bin");
        case 2:
            return QString("Source");
        case 3:
            return QString("Grade");
        default:
            return QVariant();

        }
    }
    else if(role == Qt::DisplayRole && orientation == Qt::Vertical){
        return section;
    }
    return QVariant();
}

QVariant RecordModel::data(const QModelIndex &index, int role) const
{
    auto& entries = engine->getEntries();
    int row = index.row();
    int col = index.column();
    int size = entries.size();


    if (!index.isValid())
        return QVariant();
    if (index.row() >= size)
        return QVariant();
    if (col == 0 && (role == Qt::DisplayRole ||
            role == Qt::EditRole)) {
        return QString::fromStdString( entries[row]["species_name"] );
    }else if(col == 1 && (role == Qt::DisplayRole ||
            role == Qt::EditRole)) {
        return QString::fromStdString( entries.at(row)["bin_uri"] );
    }else if(col == 2 && (role == Qt::DisplayRole ||
            role == Qt::EditRole)) {
        return QString::fromStdString( entries[row]["institution_storing"]);
    }else if(col == 3 && role == Qt::DisplayRole){
        return QString::fromStdString( entries[row]["grade"]);
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
    auto& entries = engine->getEntries();
    auto& filtered = engine->getFilteredEntries();
    for (int row=0; row < rows; ++row) {
        if(remove){
            auto item = entries.at(position);
            filtered.push_back(item);
            entries.erase(entries.begin()+position);
            cur_count--;
        }
    }
    endRemoveRows();
    return true;
}

void RecordModel::onRecordsChanged(){
    beginResetModel();
    if(cur_count!=0){
        removeRows(0, cur_count, QModelIndex());
    }
    auto& entries = engine->getEntries();
    sort_column(entries,"species_name", true);
    cur_count = entries.size();
    insertRows(0, cur_count, QModelIndex());
    endResetModel();


    /*
    auto topLeft = this->index(0,0);
    auto rightBottom = this->index(this->engine->size(),columnCount());
    emit dataChanged(topLeft, rightBottom, {Qt::DisplayRole});
*/
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
        auto& entries = engine->getEntries();
        auto& entry = entries[row];
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
            emit updateCombobox();
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


