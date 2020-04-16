#ifndef RECORDMODEL_H
#define RECORDMODEL_H
#include <QAbstractTableModel>
#include "iengine.h"


class RecordModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    RecordModel(QObject *parent, IEngine *engine);
    bool remove;
    std::vector<Record> getRecords() {
        return engine->getEntries();
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int row, const QModelIndex &parent = QModelIndex()) override;
public slots:
    void onRecordsChange();
signals:
    void action_performed();
    void update_graph();
    void update_results();
    void update_combobox();

private:
    //QVector<Record> records;
    size_t cur_count;
    IEngine *engine;
};

#endif // RECORDMODEL_H
