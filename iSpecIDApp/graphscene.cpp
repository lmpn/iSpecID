//#include <math.h>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QContextMenuEvent>
#include <QMenu>
#include <qdebug.h>
#include "graphscene.h"



GraphScene::GraphScene(QWidget *parent, IEngine * engine)
    :  QGraphicsScene(parent), engine(engine), cur_root(nullptr)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(QRectF(0, 0, 0, 0));
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
            connect(edge, SIGNAL(remove_edge(Edge *)),
                    this, SLOT(on_remove_edge(Edge *)));
            edges << edge;
            edge->hide();
            this->addItem(edge);
        }
    }
    update();
}


void GraphScene::on_graph_changed(){
    clearScene();
    clean();
    generateItems();
    this->setSceneRect(this->itemsBoundingRect());                          // Re-shrink the scene to it's bounding contents
}
void GraphScene::on_graph_color_changed(){
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



void GraphScene::on_remove_edge(Edge *edge){
    emit action_performed();
    auto src = edge->sourceNode();
    auto dest = edge->destNode();
    auto species = src->getName().toStdString();
    auto bin = dest->getName().toStdString();
    setComponentVisibleDFS(edge->destNode(), false);
    setComponentVisibleDFS(edge->sourceNode(), false);
    set_component_visible();
    update();

    delete edge;
    engine->filter([species, bin](Record item) {
        return item["species_name"] == species && item["bin_uri"]==bin;
    });
    engine->group();
    emit update_combobox();
    emit update_records();
    emit update_results();
}


void GraphScene::on_save_graph(QString path){
    this->clearSelection();                                                  // Selections would also render to the file
    this->setSceneRect(this->itemsBoundingRect());                          // Re-shrink the scene to it's bounding contents
    QImage *image = new QImage(this->sceneRect().size().toSize(), QImage::Format_ARGB32);  // Create the image with the exact size of the shrunk scene
    image->fill(Qt::white);                                              // Start all pixels transparent

    QPainter painter(image);
    this->render(&painter);
    Node * n = qgraphicsitem_cast<Node*>(cur_root);
    if(n != nullptr) {
        image->save(path);
    }
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

void GraphScene::set_component_visible(QString key){
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

void GraphScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    Q_UNUSED(painter);
}

