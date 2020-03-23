#ifndef RECORDMODEL_H
#define RECORDMODEL_H
#include <QAbstractTableModel>
#include <record.h>


class RecordModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    RecordModel(QObject *parent, std::vector<Record> data);

    void setRecords( std::vector<Record>& records);

protected:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int row, const QModelIndex &parent = QModelIndex()) override;
private:
    std::vector<Record> records;
};

#endif // RECORDMODEL_H
