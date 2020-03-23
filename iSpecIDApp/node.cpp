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
#include "iSpecIDApp/graphviewer.h"
#include <utils.h>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QTextItem>


Node::Node(QString name, NODE_TYPE ntype, GraphViewer *_graph)
    : graph(_graph)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    this->name = name;
    this->node_type = ntype;
    highlight = false;
}

void Node::addEdge(Edge *edge)
{
    edge_list << edge;
    edge->adjust();
}

void Node::removeEdge(Edge *edge, QString other)
{
    edge_list.removeOne(edge);
    /*
    if(ntype == NODE_TYPE::SpeciesNode){
        QString species = this->name;
        graph->filter([species, other](Record item) {
            return item["species_name"] == species.toStdString() && item["bin_uri"]==other.toStdString();
        });
    }
    */
    if(edge_list.size() == 0){
        delete this;
    }
}


QVector<Edge *> Node::edges() const
{
    return edge_list;
}


QString Node::getName() const{
    return name;
}


QRectF Node::boundingRect() const
{
    qreal adjust = 2;
    QRectF rect1( -10 - adjust, -10 - adjust, 23 + adjust, 23 + adjust);
    QString text(this->name);
    QFont currentFont = scene()->font();
    QFontMetricsF fontMetrics(currentFont);
    QRectF rect2 = fontMetrics.boundingRect(text).translated(20,20);
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
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(-7, -7, 20, 20);

    QRadialGradient gradient(-3, -3, 10);
    if (option->state & QStyle::State_Sunken) {
        gradient.setCenter(3, 3);
        gradient.setFocalPoint(3, 3);
        gradient.setColorAt(1, QColor(Qt::yellow).lighter(120));
        gradient.setColorAt(0, QColor(Qt::darkYellow).lighter(120));
    }
    else if (highlight && option->state & QStyle::State_Sunken) {
        gradient.setCenter(3, 3);
        gradient.setFocalPoint(3, 3);
        gradient.setColorAt(1, QColor(Qt::red).lighter(120));
        gradient.setColorAt(0, QColor(Qt::darkRed).lighter(120));
    }
    else if(highlight){
        gradient.setColorAt(0, Qt::red);
        gradient.setColorAt(1, Qt::darkRed);

    } else {
        gradient.setColorAt(0, Qt::yellow);
        gradient.setColorAt(1, Qt::darkYellow);
    }
    painter->setBrush(gradient);

    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(-10, -10, 20, 20);
    // Text


    QFont currentFont = scene()->font();
    QFontMetricsF fontMetrics(currentFont);
    QRectF rect2 = fontMetrics.boundingRect(this->name).translated(12,12);
    painter->setPen(Qt::black);
    painter->drawText(rect2, this->name);
}


QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{

    switch (change) {
    case ItemPositionHasChanged:
        for (Edge *edge : qAsConst(edge_list))
            edge->adjust();
        graph->itemMoved();
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
    for (auto& edge : edge_list) {
        Node *node = edge->destNode() != this ? edge->destNode() : edge->sourceNode();
        count++;
        qreal dx = this->pos().x() - node->pos().x();
        qreal dy = this->pos().y() - node->pos().y();
        xvel += dx;
        yvel += dy;
    }

    if(count > 0){
        xvel = -1*xvel/(count*10);
        yvel = -1*yvel/(count*10);
    }
    if (qAbs(xvel) <= 10 && qAbs(yvel) <= 10)
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

