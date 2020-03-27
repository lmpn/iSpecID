#include "iSpecIDApp/graphviewer.h"
//#include <math.h>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QContextMenuEvent>
#include <QMenu>
#include <qdebug.h>
#include "iSpecIDApp/edge.h"
#include "iSpecIDApp/node.h"



GraphViewer::GraphViewer(QWidget *parent, Annotator *_an)
    :  QGraphicsView(parent), an(_an), cur_root(nullptr)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    auto srect = parent->rect();
    this->setSceneRect(srect);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    //fitInView(this->scene()->sceneRect(), Qt::IgnoreAspectRatio);
    cur_root = nullptr;
}

void GraphViewer::generateItems(){

    clearScene();
    clean();

    auto data = an->getGroupRecords();
    for(auto& pair: data){
        auto bins = pair.second.bins;
        Node *node = new Node(QString::fromStdString(pair.first), QString::fromStdString(pair.second.grade), this);
        scene()->addItem(node);
        node->hide();
        nodes.insert(node->getName(), node);
        for (auto bin_name : bins) {
            if(bin_name == "") continue;
            auto key = QString::fromStdString(bin_name);
            Node *bin = qgraphicsitem_cast<Node *>(nodes[key]);
            if(bin == nullptr){
                bin = new Node(key, QString::fromStdString("U"), this);
                nodes.insert(key,bin);
                bin->hide();
                scene()->addItem(bin);
            }
            Edge * edge = new Edge(node, bin);
            connect(edge, SIGNAL(edgeRemoval(Edge *)),
                    this, SLOT(componentChanged(Edge *)));
            edges << edge;
            edge->hide();
            scene()->addItem(edge);
        }
    }
}


void GraphViewer::onGraphChange(){
    generateItems();

}
void GraphViewer::onGraphColorChange(){
    auto group_records = an->getGroupRecords();
    for(auto item: nodes){
        auto node = qgraphicsitem_cast<Node *>(item);
        std::string key =node->getName().toStdString();
        Species sp = group_records[key];
        node->setColor(QString::fromStdString(sp.grade));
    }
}

void GraphViewer::drawBackground(QPainter *painter, const QRectF &rect)
{
    /*
    Q_UNUSED(rect);

    // Shadow
    QRectF sceneRect = this->sceneRect();
    QRectF rightShadow(sceneRect.right(), sceneRect.top() + 5, 5, sceneRect.height());
    QRectF bottomShadow(sceneRect.left() + 5, sceneRect.bottom(), sceneRect.width(), 5);
    if (rightShadow.intersects(rect) || rightShadow.contains(rect))
        painter->fillRect(rightShadow, Qt::darkGray);
    if (bottomShadow.intersects(rect) || bottomShadow.contains(rect))
        painter->fillRect(bottomShadow, Qt::darkGray);

    // Fill
    QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::lightGray);
    painter->fillRect(rect.intersected(sceneRect), gradient);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sceneRect);

    // Text
    QRectF textRect(0,sceneRect.height() - 50,
                    50,50);
    painter->setBrush(Qt::black);
    painter->setPen(Qt::black);
    painter->drawRect(textRect);
    /*
    QString message(tr("Click and drag the nodes around, and zoom with the mouse "
                       "wheel or the '+' and '-' keys"));

    QFont font = painter->font();
    font.setBold(true);
    font.setPointSize(14);
    painter->setFont(font);
    painter->setPen(Qt::lightGray);
    painter->drawText(textRect.translated(2, 2), message);
    painter->drawText(textRect, message);*/

}


void GraphViewer::componentChanged(Edge *edge){
    auto src = edge->sourceNode();
    auto dest = edge->destNode();
    auto species = src->getName().toStdString();
    auto bin = dest->getName().toStdString();
    setComponentVisibleDFS(edge->destNode(), false);
    setComponentVisibleDFS(edge->sourceNode(), false);
    setComponentVisible();

    delete edge;
    an->filter([species, bin](Record item) {
        return item["species_name"] == species && item["bin_uri"]==bin;
    });
    an->clearGroup();
    an->group();
    an->calculateGradeResults();
    emit updateRecords();
    emit updateResults();
}

void GraphViewer::setComponentVisibleDFS( Node *root, bool visible){
    if(root->isVisible() == visible) return;
    root->setVisible(visible);
    QSet<Edge*> current = root->edges();
    QSet<Edge*> next;
    float h = this->rect().height();
    float w = this->rect().width();
    int size = current.size();
    float offset = w/size;
    float s_w = (w - offset*size)/2;
    float s_h = (h/2) - 150;
    auto it = current.begin();
    root->setPos(h/2,w/2);
    while(current.size() > 0){
        for(int i = 0; i < size; i++){
            auto edge = *(it + i);
            auto dest = edge->destNode();
            auto src = edge->sourceNode();
            edge->setVisible(visible);
            if(dest->isVisible() != visible){
                dest->setVisible(visible);
                next += dest->edges();
                dest->setPos(s_w+i*offset,s_h);
            }
            if(src->isVisible() != visible){
                src->setVisible(visible);
                next += src->edges();
                src->setPos(s_w+i*offset,s_h);
            }
        }
        current = next;
        next = QSet<Edge*>();
        s_h -= 150;
        size = current.size();
        offset = w/size;
        s_w = (w - offset*size)/2;
        it = current.begin();
    }
    /*
    for(auto& edge : root->edges()){
        setComponentVisibleDFS(dest, visible);
        setComponentVisibleDFS(src, visible);
    }
    if(root->edges().size() == 0){
        scene()->removeItem(root);
        nodes.remove(root->getName());
    }*/
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
    if(root != nullptr)
        setComponentVisibleDFS(root, true);
}

void GraphViewer::clearScene(){
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        scene()->removeItem(item);
        delete item;
    }
}

void GraphViewer::clean(){
    this->nodes.clear();
    this->edges.clear();
    cur_root = nullptr;
}


GraphViewer::~GraphViewer()
{
    clearScene();
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
    /*if (!timerId)
        timerId = startTimer(1000 / 50);*/
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
