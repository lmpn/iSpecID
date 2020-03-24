#include "iSpecIDApp/graphviewer.h"
//#include <math.h>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QContextMenuEvent>
#include <QMenu>
#include "iSpecIDApp/edge.h"
#include "iSpecIDApp/node.h"



GraphViewer::GraphViewer(Annotator *_an, QWidget *parent)
    :  QGraphicsView(parent), an(_an), cur_root(nullptr)
{
    this->setGeometry(parent->rect());
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(parent->rect());
    setScene(scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    auto data = an->getGroupData();
    for(auto& pair: data){
        auto bins = pair.second.bins;
        Node *node = new Node(QString::fromStdString(pair.first), Node::NODE_TYPE::SpeciesNode,this);
        node->setPos(randomPos());
        scene->addItem(node);
        node->hide();
        nodes.insert(node->getName(), node);
        for (auto bin_name : bins) {
            auto key = QString::fromStdString(bin_name);
            Node *bin = qgraphicsitem_cast<Node *>(nodes[key]);
            if(bin == nullptr){
                bin = new Node(key, Node::NODE_TYPE::BinNode, this);
                bin->setPos(randomPos());
                nodes.insert(key,bin);
                bin->hide();
                scene->addItem(bin);
            }
            Edge * edge = new Edge(node, bin);
            connect(edge, SIGNAL(edgeRemoval(Edge *)),
                    this, SLOT(componentChanged(Edge *)));
            edges << edge;
            edge->hide();
            scene->addItem(edge);
        }
    }

}

void GraphViewer::componentChanged(Edge *edge){
    auto src = edge->sourceNode();
    auto dest = edge->destNode();
    auto species = src->getName().toStdString();
    auto bin = dest->getName().toStdString();
    an->filter([species, bin](Record item) {
        return item["species_name"] == species && item["bin_uri"]==bin;
    });
    setComponentVisibleDFS(edge->destNode(), false);
    setComponentVisibleDFS(edge->sourceNode(), false);
    delete edge;
    setComponentVisible();
}

void GraphViewer::setComponentVisibleDFS( Node *root, bool visible){
    if(root->isVisible() == visible) return;
    root->setVisible(visible);
    std::cout << root->getName().toStdString() <<";" << visible << std::endl;
    for(auto& edge : root->edges()){
        edge->setVisible(visible);
        auto dest = edge->destNode();
        auto src = edge->sourceNode();
        setComponentVisibleDFS(dest, visible);
        setComponentVisibleDFS(src, visible);
    }
    if(root->edges().size() == 0){
        scene()->removeItem(root);
        nodes.remove(root->getName());
    }
}

void GraphViewer::setComponentVisible(QString key){
    Node *root;
    if(cur_root != nullptr){
        root = qgraphicsitem_cast<Node *>(cur_root);
        setComponentVisibleDFS(root, false);
    }
    if(!key.isEmpty()){
        cur_root = nodes[key];
        root = qgraphicsitem_cast<Node *>(cur_root);
    }

    setComponentVisibleDFS(root, true);
}



GraphViewer::~GraphViewer()
{

}



void GraphViewer::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Space:
    case Qt::Key_Enter:
        shuffle();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}


#if QT_CONFIG(wheelevent)
void GraphViewer::wheelEvent(QWheelEvent *event)
{
    scaleView(pow(2., -event->angleDelta().y() / 240.0));
}
#endif

void GraphViewer::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
}

QPointF GraphViewer::randomPos(){
    int mid_w =sceneRect().width()/2;
    int mid_h =sceneRect().height()/2;
    int w = -mid_w + QRandomGenerator::global()->bounded(mid_w, sceneRect().width());
    int h = -mid_h + QRandomGenerator::global()->bounded(mid_h, sceneRect().height());
    return QPointF(w,h);
}

void GraphViewer::shuffle()
{
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        if (qgraphicsitem_cast<Node *>(item) && item->isVisible())
            item->setPos(randomPos());
    }
}

void GraphViewer::zoomIn()
{
    scaleView(qreal(1.2));
}

void GraphViewer::zoomOut()
{
    scaleView(1 / qreal(1.2));
}

void GraphViewer::itemMoved()
{
    if (!timerId)
        timerId = startTimer(1000 / 25);
}

void GraphViewer::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    QVector<Node *> nodes;
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        if (Node *node = qgraphicsitem_cast<Node *>(item))
            nodes << node;
    }

    for (Node *node : qAsConst(nodes))
    {
        node->calculateForces();
    }

    bool itemsMoved = false;
    for (Node *node : qAsConst(nodes)) {
        if (node->advancePosition())
            itemsMoved = true;
    }

    if (!itemsMoved) {
        killTimer(timerId);
        timerId = 0;
    }
}
