#ifndef RESULTSMODEL_H
#define RESULTSMODEL_H
#include <QAbstractTableModel>

class ResultsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ResultsModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    void setResults(std::vector<int> results);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
private:
    std::vector<int> _results;
    float _sum;
};

#endif // RESULTSMODEL_H
