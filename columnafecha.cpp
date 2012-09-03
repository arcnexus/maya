/*
 * ColumnaFecha.cpp
 *
 *  Created on: September 2012
 *      Author: ArcNexus
 */

#include "columnafecha.h"
#include <QStyleOptionViewItem>
#include <QDate>

ColumnaFecha::ColumnaFecha()
    {
    // TODO Auto-generated constructor stub

    }

ColumnaFecha::~ColumnaFecha()
    {
    // TODO Auto-generated destructor stub
    }

void ColumnaFecha::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
 {
    QString cFecha = index.model()->data(index, Qt::DisplayRole).toString();
    QDate dFecha;
    dFecha = QDate::fromString(cFecha, "yyyy-MM-dd");
    cFecha= dFecha.toString("dd/MM/yyyy");

    QStyleOptionViewItem myOption = option;
    myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    drawDisplay(painter, myOption, myOption.rect,cFecha);
    drawFocus(painter, myOption, myOption.rect);

 }
