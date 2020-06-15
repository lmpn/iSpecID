//#include <math.h>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QContextMenuEvent>
#include <QMenu>
#include <qdebug.h>
#include "graphscene.h"
#include "utils.h"



GraphScene::GraphScene(QWidget *parent, std::vector<QRecord> * records)
    : records(records)
{
    setParent(parent);
    setItemIndexMethod(QGraphicsScene::NoIndex);
    setSceneRect(QRectF(0, 0, 0, 0));
    cur_node_key = "";
}


void GraphScene::generateItems(){
    for(auto& qrec: *records){
        auto species_name = qrec.record.getSpeciesName();
        auto cluster_name = qrec.record.getCluster();
        auto grade = qrec.record.getGrade();
        auto node_it = nodes.find(QString::fromStdString(species_name));
        auto cluster_it = nodes.find(QString::fromStdString(cluster_name));
        Node *node;
        Node *cluster;
        if(node_it == nodes.end()){
            node = new Node(QString::fromStdString(species_name), QString::fromStdString(grade));
            node->hide();
            this->addItem(node);
            nodes.insert(node->getName(), node);
        }else{
            node = qgraphicsitem_cast<Node *>(*node_it);
        }
        if(cluster_it == nodes.end()){
            cluster = new Node(QString::fromStdString(cluster_name), "");
            cluster->hide();
            this->addItem(cluster);
            nodes.insert(cluster->getName(), cluster);
        }else{
            cluster = qgraphicsitem_cast<Node *>(*cluster_it);
        }
        auto edges = node->edges();
        Edge* edge = nullptr;
        for(auto& e : edges){
            if(e != nullptr && node == e->sourceNode() && cluster == e->destNode()){
                edge = e;
            }
        }
        if(edge == nullptr){
            edge = new Edge(node,cluster, qrec.record.count());
            connect(edge, SIGNAL(removeEdge(Edge *)),
                    this, SLOT(onRemoveEdge(Edge *)));
            edges << edge;
            edge->hide();
            this->addItem(edge);
        }	else{
            edge->addCount(qrec.record.count());
        }
    }
    update();
}

void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent* pMouseEvent) {
    QGraphicsItem* pItemUnderMouse = itemAt( pMouseEvent->scenePos().x(), pMouseEvent->scenePos().y(), QTransform());
    if (pItemUnderMouse != nullptr && pItemUnderMouse->isEnabled() &&
        pMouseEvent->modifiers() &  Qt::ControlModifier &&
        pItemUnderMouse->flags() & QGraphicsItem::ItemIsSelectable){
        auto m = qgraphicsitem_cast<Node *>(pItemUnderMouse);
        m->setSelectNode(!pItemUnderMouse->isSelected());
        return;
    }
    QGraphicsScene::mousePressEvent(pMouseEvent);
}

void GraphScene::onGraphChanged(){
    clearScene();
    clean();
    generateItems();
    setSceneRect(itemsBoundingRect());                          // Re-shrink the scene to it's bounding contents
}
void GraphScene::onGraphColorChanged(){
    for(auto& qrec: *records){
        auto key = QString::fromStdString(qrec.record.getSpeciesName());
        auto node = qgraphicsitem_cast<Node *>(nodes[key]);
        node->setColor(QString::fromStdString(qrec.record.getGrade()));

    }
    update();
}



void GraphScene::onRemoveEdge(Edge *edge){
    emit actionPerformed();
    auto src = edge->sourceNode();
    auto dest = edge->destNode();
    auto species = src->getName();
    auto species_str = src->getName().toStdString();
    auto bin = dest->getName();
    auto bin_str = dest->getName().toStdString();
    setComponentVisibleDFS(edge->destNode(), false);
    setComponentVisibleDFS(edge->sourceNode(), false);
    if(src->edges().count() == 0){
        delete nodes[species];
    }
    if(dest->edges().count() == 0){
        delete nodes[bin];
    }
    delete edge;
    records->erase(std::remove_if(records->begin(), records->end(),
                                  [species_str, bin_str](QRecord qrec){
        return qrec.record.getSpeciesName() == species_str && qrec.record.getCluster() == bin_str;
    }),records->end());
    cur_node_key = "";
    emit updateCombobox();
    emit updateRecords();
    emit updateResults();
}



void GraphScene::setComponentVisibleDFS( Node *root, bool visible){
    if(root->isVisible() == visible) return;
    root->setVisible(visible);
    QSet<Edge*> current = root->edges();
    QSet<Edge*> next;
    float w = this->sceneRect().width();
    int size = current.size();
    float offset = w/size;
    float s_w = 50+(w - offset*size)/2;
    float s_h = 75;
    auto it = current.begin();
    root->setPos(w/2,0);
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
        s_h += 75;
        size = current.size();
        offset = w/size;
        s_w = 50 + (w - offset*size)/2;
        it = current.begin();
    }
}

void GraphScene::setComponentVisible(QString key){
    if(!key.isEmpty()) {
        if(!cur_node_key.isEmpty()){
            auto hide_root = qgraphicsitem_cast<Node *>(nodes[cur_node_key]);
            if(hide_root->getName() != key){
                setComponentVisibleDFS(hide_root, false);
            }
        }
        cur_node_key = key;
        auto show_root = qgraphicsitem_cast<Node *>(nodes[cur_node_key]);
        setComponentVisibleDFS(show_root, true);
    }
}

void GraphScene::clearScene(){
    const QList<QGraphicsItem *> items = this->items();
    for (QGraphicsItem *item : items) {
        if(item != nullptr){
            this->removeItem(item);
            delete item;
        }
    }
}

void GraphScene::clean(){
    this->nodes.clear();
    this->edges.clear();
    cur_node_key = "";
}


GraphScene::~GraphScene()
{
    clearScene();
}

void GraphScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    Q_UNUSED(painter);
}

