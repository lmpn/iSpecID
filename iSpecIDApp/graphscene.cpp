#include "iSpecIDApp/graphscene.h"
//#include <math.h>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QContextMenuEvent>
#include <QMenu>
#include <qdebug.h>
#include "iSpecIDApp/edge.h"
#include "iSpecIDApp/node.h"



GraphScene::GraphScene(QWidget *parent, Annotator *_an)
    :  QGraphicsScene(parent), an(_an), cur_root(nullptr)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(QRectF(0, 0, 5000, 5000));

    cur_root = nullptr;
}






void GraphScene::generateItems(){

    clearScene();
    clean();

    auto data = an->getGroupRecords();
    for(auto& pair: data){
        auto bins = pair.second.bins;
        Node *node = new Node(QString::fromStdString(pair.first), QString::fromStdString(pair.second.grade), this);
        this->addItem(node);
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
                this->addItem(bin);
            }
            Edge * edge = new Edge(node, bin);
            connect(edge, SIGNAL(edgeRemoval(Edge *)),
                    this, SLOT(componentChanged(Edge *)));
            edges << edge;
            edge->hide();
            this->addItem(edge);
        }
    }
    update();
}


void GraphScene::onGraphChange(){
    generateItems();

}
void GraphScene::onGraphColorChange(){
    auto group_records = an->getGroupRecords();
    for(auto item: nodes){
        auto node = qgraphicsitem_cast<Node *>(item);
        std::string key =node->getName().toStdString();
        Species sp = group_records[key];
        node->setColor(QString::fromStdString(sp.grade));
    }
    update();
}



void GraphScene::componentChanged(Edge *edge){
    auto src = edge->sourceNode();
    auto dest = edge->destNode();
    auto species = src->getName().toStdString();
    auto bin = dest->getName().toStdString();
    setComponentVisibleDFS(edge->destNode(), false);
    setComponentVisibleDFS(edge->sourceNode(), false);
    setComponentVisible();
    update();

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

void GraphScene::setComponentVisibleDFS( Node *root, bool visible){
    if(root->isVisible() == visible) return;
    root->setVisible(visible);
    QSet<Edge*> current = root->edges();
    QSet<Edge*> next;
    float h = this->sceneRect().height();
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

void GraphScene::clearScene(){
    const QList<QGraphicsItem *> items = this->items();
    for (QGraphicsItem *item : items) {
        this->removeItem(item);
        delete item;
    }
}

void GraphScene::clean(){
    this->nodes.clear();
    this->edges.clear();
    cur_root = nullptr;
}


GraphScene::~GraphScene()
{
    clearScene();
}




QPointF GraphScene::randomPos(){
    int mid_w =sceneRect().width()/2;
    int mid_h =sceneRect().height()/2;
    int w = -mid_w + QRandomGenerator::global()->bounded(mid_w, sceneRect().width());
    int h = -mid_h + QRandomGenerator::global()->bounded(mid_h, sceneRect().height());
    return QPointF(w,h);
}


