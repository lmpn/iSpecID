#ifndef GRAPHVIEWER_H
#define GRAPHVIEWER_H
#include <QGraphicsView>
#include <QGraphicsItem>
#include <annotator.h>

class Node;

class GraphViewer : public QGraphicsView
{
    Q_OBJECT
public:
    GraphViewer(Annotator *_an, QWidget *parent);
    ~GraphViewer();


    void itemMoved();
    QPointF randomPos();

public slots:
    void shuffle();
    void zoomIn();
    void zoomOut();
    void setComponentVisible(QString key = QString::fromStdString(""));

protected:
    void timerEvent(QTimerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    #if QT_CONFIG(wheelevent)
        void wheelEvent(QWheelEvent *event) override;
    #endif
    void scaleView(qreal scaleFactor);

private:
    inline void setComponentVisibleDFS( Node *root, bool visible = true);
    int timerId = 0;
    Annotator *an;
    QMap<QString, QGraphicsItem*> nodes;
    QVector<QGraphicsItem*> edges;
    QGraphicsItem* cur_root;
};

#endif // GRAPHVIEWER_H
