#ifndef GRAPHVIEWER_H
#define GRAPHVIEWER_H
#include <QGraphicsView>
#include <QGraphicsItem>
#include "iengine.h"
#include "edge.h"
#include "node.h"

class Node;
class Edge;
class Record;

class GraphScene : public QGraphicsScene
{
    Q_OBJECT
public:
    GraphScene(QWidget *parent, IEngine *engine);
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    ~GraphScene();

public slots:
    void onRemoveEdge(Edge *);
    void setComponentVisible(QString key = QString::fromStdString(""));
    void onGraphChanged();
    void onGraphColorChanged();

signals:
    void updateRecords();
    void updateResults();
    void updateCombobox();
    void actionPerformed();

private:
    void clean();
    void clearScene();
    void setComponentVisibleDFS( Node *root, bool visible = true);
    void generateItems();
    IEngine * engine;
    QMap<QString, QGraphicsItem*> nodes;
    QVector<QGraphicsItem*> edges;
    QGraphicsItem* cur_root;
};

#endif // GRAPHVIEWER_H
