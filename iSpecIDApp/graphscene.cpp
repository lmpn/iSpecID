//#include <math.h>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QContextMenuEvent>
#include <QMenu>
#include <qdebug.h>
#include "graphscene.h"



GraphScene::GraphScene(QWidget *parent, IEngine * engine)
    : engine(engine)
{
    setParent(parent);
    setItemIndexMethod(QGraphicsScene::NoIndex);
    setSceneRect(QRectF(0, 0, 0, 0));
    cur_node_key = "";
}


void GraphScene::generateItems(){
    auto data = engine->getGroupedEntries();
    for(auto& pair: data){
        auto bins = pair.second.bins;
        Node *node = new Node(QString::fromStdString(pair.first), QString::fromStdString(pair.second.grade));
        this->addItem(node);
        node->hide();
        nodes.insert(node->getName(), node);
        for (auto bin_pair : bins) {
            if(bin_pair.first == "") continue;
            auto key = QString::fromStdString(bin_pair.first);
            Node *bin = qgraphicsitem_cast<Node *>(nodes[key]);
            if(bin == nullptr){
                bin = new Node(key, QString());
                nodes.insert(key,bin/*, bin_pair.second*/);
                bin->hide();
                this->addItem(bin);
            }
            Edge * edge = new Edge(node, bin, bin_pair.second);
            connect(edge, SIGNAL(removeEdge(Edge *)),
                    this, SLOT(onRemoveEdge(Edge *)));
            edges << edge;
            edge->hide();
            this->addItem(edge);
        }
    }
    update();
}


void GraphScene::onGraphChanged(){
    clearScene();
    clean();
    generateItems();
    setSceneRect(itemsBoundingRect());                          // Re-shrink the scene to it's bounding contents
}
void GraphScene::onGraphColorChanged(){
    auto group_records = engine->getGroupedEntries();
    for(auto item: nodes){
        auto node = qgraphicsitem_cast<Node *>(item);
        std::string key = node->getName().toStdString();
        Species sp = group_records[key];
        if(sp.grade != "U")
            node->setColor(QString::fromStdString(sp.grade));
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
    engine->filter([species_str, bin_str](Record item) {
        return item["species_name"] == species_str && item["bin_uri"]==bin_str;
    });
    engine->group();
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

