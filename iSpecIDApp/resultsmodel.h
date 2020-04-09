#ifndef RESULTSMODEL_H
#define RESULTSMODEL_H
#include <QAbstractTableModel>
#include "iengine.h"

class ResultsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ResultsModel(QObject *parent, IEngine *engine);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
public slots:
    void onResultsChange();
private:
    IEngine *engine;
    std::vector<int> results;
    float perc;
};

#endif // RESULTSMODEL_H
