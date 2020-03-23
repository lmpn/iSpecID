/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "iSpecIDApp/graphwidget.h"
#include "iSpecIDApp/edge.h"
#include "iSpecIDApp/node.h"
#include "iSpecIDCore/species.h"
#include "iSpecIDCore/annotator.h"

#include <iostream>
#include <math.h>
#include <QKeyEvent>
#include <QRandomGenerator>

//! [0]
GraphWidget::GraphWidget(Annotator* _an, QWidget *parent)
    : QGraphicsView(parent)
{
    this->an = _an;
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    scene->setSceneRect(parent->rect().x()-100, parent->rect().y()-100, parent->rect().width()+100, parent->rect().height()+100);
    auto data = an->getGroupData();
    for(auto& pair: data){
        auto bins = pair.second.bins;
        Node *node = new Node(this, QString::fromStdString(pair.first), NODE_TYPE::SpeciesNode);
        scene->addItem(node);
        node->setPos(0, 0);
        for (auto bin_name : bins) {
            Node *bin = new Node(this, QString::fromStdString(bin_name), NODE_TYPE::BinNode);
            bin->setPos(0, 0);
            scene->addItem(bin);
            scene->addItem(new Edge(node, bin));
        }
    }
    shuffle();
}


void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
     setCacheMode(CacheNone);  /* cache or not Background  QGraphicsView::CacheBackground*/

     painter->save();
     painter->setPen( QPen(Qt::darkGray,1) );
     painter->drawRect(sceneRect());
     painter->restore();

}

GraphWidget::GraphWidget(QWidget *parent)
    : QGraphicsView(parent)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    //scene->setSceneRect(-100, -100, 300, 300);
    setScene(scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);


    Node *node = new Node(this, QString::fromStdString("ola mundo"), NODE_TYPE::SpeciesNode);
    Node *node1 = new Node(this, QString::fromStdString("ola mundo"), NODE_TYPE::SpeciesNode);
    Node *node2 = new Node(this, QString::fromStdString("ola mundo"), NODE_TYPE::SpeciesNode);
    scene->addItem(node);
    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(new Edge(node,node1));
    scene->addItem(new Edge(node,node2));
    node->setPos(0,0);
    node1->setPos(-50,0);
    node2->setPos(50,0);

}

void GraphWidget::keyPressEvent(QKeyEvent *event)
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
//! [5]
void GraphWidget::wheelEvent(QWheelEvent *event)
{
    scaleView(pow(2., -event->angleDelta().y() / 240.0));
}
//! [5]
#endif

//! [7]
void GraphWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
}
//! [7]

void GraphWidget::shuffle()
{
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        if (qgraphicsitem_cast<Node *>(item))
            item->setPos(
                -sceneRect().width()/2 + QRandomGenerator::global()->bounded(sceneRect().width()),
                -sceneRect().height()/2 + QRandomGenerator::global()->bounded(sceneRect().height()));
    }
}

void GraphWidget::zoomIn()
{
    scaleView(qreal(1.2));
}

void GraphWidget::zoomOut()
{
    scaleView(1 / qreal(1.2));
}

void GraphWidget::itemMoved()
{
    if (!timerId)
        timerId = startTimer(1000 / 25);
}

void GraphWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    QVector<Node *> nodes;
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items) {
        if (Node *node = qgraphicsitem_cast<Node *>(item))
            nodes << node;
    }

    for (Node *node : qAsConst(nodes))
        node->calculateForces();

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

void GraphWidget::highlight(QString name){
    const QList<QGraphicsItem *> items = scene()->items();
    Node *root;
    for (QGraphicsItem *item : items) {
        if (Node *node = qgraphicsitem_cast<Node *>(item))
        {
            if(node->getName() == name){
                root = node;
            }
            node->switch_high();
        }
    }
    root->highlight_path();
}

