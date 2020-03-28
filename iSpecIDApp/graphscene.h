#ifndef GRAPHVIEWER_H
#define GRAPHVIEWER_H
#include <QGraphicsView>
#include <QGraphicsItem>
#include <annotator.h>

class Node;
class Edge;
class Record;

class GraphScene : public QGraphicsScene
{
    Q_OBJECT
public:
    GraphScene(QWidget *parent, Annotator*_an);
    ~GraphScene();


    QPointF randomPos();

public slots:
    void componentChanged(Edge *);
    void setComponentVisible(QString key = QString::fromStdString(""));
    void onGraphChange();
    void onGraphColorChange();

signals:
    void updateRecords();
    void updateResults();

private:
    void clean();
    void clearScene();
    void setComponentVisibleDFS( Node *root, bool visible = true);
    void generateItems();
    int timerId = 0;
    Annotator *an;
    QMap<QString, QGraphicsItem*> nodes;
    QVector<QGraphicsItem*> edges;
    QGraphicsItem* cur_root;
};

#endif // GRAPHVIEWER_H
