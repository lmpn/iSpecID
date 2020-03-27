#ifndef GRAPHVIEWER_H
#define GRAPHVIEWER_H
#include <QGraphicsView>
#include <QGraphicsItem>
#include <annotator.h>

class Node;
class Edge;
class Record;

class GraphViewer : public QGraphicsView
{
    Q_OBJECT
public:
    GraphViewer(QWidget *parent, Annotator*_an);
    ~GraphViewer();


    void itemMoved();
    QPointF randomPos();

public slots:
    void shuffle();
    void zoomIn();
    void zoomOut();
    void componentChanged(Edge *);
    void setComponentVisible(QString key = QString::fromStdString(""));
    void onGraphChange();
    void onGraphColorChange();

signals:
    void updateRecords();
    void updateResults();

protected:
    void timerEvent(QTimerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    #if QT_CONFIG(wheelevent)
        void wheelEvent(QWheelEvent *event) override;
    #endif
    void scaleView(qreal scaleFactor);
    void drawBackground(QPainter *painter, const QRectF &rect) override;

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
