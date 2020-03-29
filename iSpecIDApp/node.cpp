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

#include "iSpecIDApp/edge.h"
#include "iSpecIDApp/node.h"
#include "iSpecIDApp/graphscene.h"
#include <utils.h>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QTextItem>
#include <QFontDatabase>
#include <qdebug.h>


Node::Node(QString name, QString grade, GraphScene *_graph)
    : graph(_graph)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    this->name = name;
    setColor(grade);
    f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    f.setStyleHint(QFont::Monospace);
}

void Node::addEdge(Edge *edge)
{
    edge_set << edge;
    edge->adjust();
}

void Node::removeEdge(Edge *edge)
{
    edge_set.remove(edge);
}


QSet<Edge *> Node::edges() const
{
    return edge_set;
}


QString Node::getName() const{
    return name;
}

void Node::setColor(QString grade){
    if(grade== "A"){
        mc = QColor(Qt::green);
        dmc = QColor(Qt::darkGreen);
    }
    else if( grade == "B"){
        mc = QColor(Qt::cyan);
        dmc = QColor(Qt::darkCyan);
    }
    else if( grade == "C"){
        mc = QColor(Qt::yellow);
        dmc = QColor(Qt::darkYellow);
    }
    else if( grade == "D"){
        mc = QColor(Qt::gray);
        dmc = QColor(Qt::darkGray);
    }
    else if( grade == "E1" || grade == "E2"){
        mc = QColor(Qt::red);
        dmc = QColor(Qt::darkRed);
    }else{
        mc = QColor(Qt::blue);
        dmc = QColor(Qt::darkBlue);
    }
}


QRectF Node::boundingRect() const
{
    qreal adjust = 2;
    QRectF rect1( -10 - adjust, -10 - adjust, 23 + adjust, 23 + adjust);
    QString text(this->name);
    QFontMetricsF fontMetrics(f);
    QRectF rect2 = fontMetrics.boundingRect(text).translated(20,20);
    rect2.setWidth(rect2.width()*2);
    rect2.setHeight(rect2.height()*2);
    return rect1.united(rect2);
}




QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(-10, -10, 20, 20);

    return path;
}


void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setFont(f);
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(-7, -7, 20, 20);


    // d 237, 151, 12
    // c 251, 255, 0
    // b 175, 255, 3
    // a 175, 255, 3
    QRadialGradient gradient(-3, -3, 10);
    if (option->state & QStyle::State_Sunken) {
        gradient.setCenter(3, 3);
        gradient.setFocalPoint(3, 3);
        gradient.setColorAt(1, mc.lighter(120));
        gradient.setColorAt(0, dmc.lighter(120));
    } else {
        gradient.setColorAt(0, mc);
        gradient.setColorAt(1, dmc);
    }
    painter->setBrush(gradient);

    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(-10, -10, 20, 20);
    // Text

    QFontMetricsF fontMetrics(f);
    QRectF rect2 = fontMetrics.boundingRect(this->name).translated(12,12);
    painter->setPen(Qt::black);
    rect2.setWidth(rect2.width()*2);
    rect2.setHeight(rect2.height()*2);
    painter->drawText(rect2, this->name);
}


QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{

    switch (change) {
    case ItemPositionHasChanged:
        for (Edge *edge : qAsConst(edge_set))
            edge->adjust();
        //graph->itemMoved();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mousePressEvent(event);
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}


void Node::calculateForces()
{
    if (!scene() || scene()->mouseGrabberItem() == this || !this->isVisible()) {
        new_pos = pos();
        return;
    }

    qreal xvel = 0;
    qreal yvel = 0;
    int count = 0;
    for (auto& edge : edge_set) {
        Node *node = edge->destNode() != this ? edge->destNode() : edge->sourceNode();
        count++;
        qreal dx = this->pos().x() - node->pos().x();
        qreal dy = this->pos().y() - node->pos().y();
        xvel += dx;
        yvel += dy;
    }

    if(count > 0){
        xvel = -1*xvel/(count);
        yvel = -1*yvel/(count);
    }
    if (qAbs(xvel) <= 200 && qAbs(yvel) <= 200)
        xvel = yvel = 0;

    QRectF sceneRect = scene()->sceneRect();
    new_pos = pos() + QPointF(xvel, yvel);
    new_pos.setX(qMin(qMax(new_pos.x(), sceneRect.left() + 10), sceneRect.right() - 10));
    new_pos.setY(qMin(qMax(new_pos.y(), sceneRect.top() + 10), sceneRect.bottom() - 10));
}

bool Node::advancePosition()
{
    if (new_pos == pos())
        return false;

    setPos(new_pos);
    return true;
}
