#ifndef RECORDMODEL_H
#define RECORDMODEL_H
#include <QAbstractTableModel>
#include <annotator.h>


class RecordModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    RecordModel(QObject *parent, Annotator *an);
    std::vector<Record> getRecords() {
        return std::vector<Record>(records.begin(), records.end());
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex()) override;
    bool removeRows(int position, int row, const QModelIndex &parent = QModelIndex()) override;
public slots:
    void onRecordsChange();
signals:
    void updateGraph();
    void updateResults();

private:
    QVector<Record> records;
    Annotator *an;
};

#endif // RECORDMODEL_H