/* This file is part of Qadastre
 * Copyright (C) 2010 Pierre Ducroquet <pinaraf@pinaraf.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include "graphicproducer.h"
#include <QRegExp>
#include <QDebug>
#include <QList>
#include <QDateTime>
#include <podofo/PdfDictionary.h>
#include <podofo/PdfObject.h>
#include <podofo/PdfParser.h>
#include <podofo/PdfStream.h>
#include <podofo/PdfVecObjects.h>

GraphicProducer::GraphicProducer(QObject *parent) :
    QObject(parent)
{
}

bool GraphicProducer::parsePDF(const QString &fileName) {
    PoDoFo::PdfVecObjects objects;
    PoDoFo::PdfParser parser(&objects, fileName.toLocal8Bit());
    PoDoFo::TIVecObjects it = objects.begin();

    bool result = false;
    do {
        PoDoFo::PdfObject *obj = (*it);
        if (obj->HasStream() && (obj->GetObjectLength(PoDoFo::ePdfWriteMode_Compact) > 10000)) {
            PoDoFo::PdfStream *stream = obj->GetStream();
            char *buffer;
            PoDoFo::pdf_long bufferLen;
            stream->GetFilteredCopy(&buffer, &bufferLen);
            qDebug() << "Buffer length : " << bufferLen;
            if (bufferLen > 1000)
                result = parseStream(buffer);
            free(buffer);
        }
        it++;
    } while (it != objects.end());
    emit parsingDone(result);
    return result;
}

bool GraphicProducer::parseStream(const char *stream) {
    qDebug() << "GraphicProducer::parse";
    QString tokenStream(stream);

    // We don't need anything else...
    QRegExp tokenizer("((\\-|\\w|\\.)+|\\[|\\])");

    QList<double> stack;
    QList<GraphicContext> contexts;
    GraphicContext currentContext;
    currentContext.brush.setStyle(Qt::SolidPattern);
    currentContext.brush.setColor(Qt::black);
    currentContext.pen.setStyle(Qt::SolidLine);
    currentContext.pen.setColor(Qt::black);

    QPainterPath currentPath;

    QVector<double> lastArray;

    int tokenPosition = 0;
    bool inArray = false;
    bool isDouble = false;
    double currentDoubleValue;
    int numPath = 0;
    int numPoints = 0;

    QTime start = QTime::currentTime();
    while (tokenPosition >= 0) {
//        QTime start;
        tokenPosition = tokenizer.indexIn(tokenStream, tokenPosition);
        QString token = tokenizer.cap(0);
//        qDebug() << start.msecsTo(QTime::currentTime()) << " msec";

        tokenPosition += token.length();

        currentDoubleValue = token.toDouble(&isDouble);
        if (inArray) {
            if (isDouble) {
                lastArray.append(currentDoubleValue);
                continue;
            } else if (token == "]") {
                inArray = false;
                continue;
            } else {
                qDebug() << "Illegal content for an array";
                return false;
            }
        }

        if (isDouble) {
            stack.push_back(currentDoubleValue);
        } else {
            // Handle the 21 known operators
            if (token == "l") {
                double y = stack.takeLast();
                double x = stack.takeLast();
                currentPath.lineTo(x, y);
                ++numPoints;
            } else if (token == "v") {
                double y3 = stack.takeLast();
                double x3 = stack.takeLast();
                double y2 = stack.takeLast();
                double x2 = stack.takeLast();
                currentPath.quadTo(x2, y2, x3, y3);
                ++numPoints;
            } else if (token == "m") {
                double y = stack.takeLast();
                double x = stack.takeLast();
                currentPath.moveTo(x, y);
                ++numPoints;
            } else if (token == "h") {
                currentPath.closeSubpath();
            } else if (token == "W") {
                if (currentContext.clipPath.length() == 0)
                    currentContext.clipPath = currentPath;
                currentContext.clipPath.setFillRule(Qt::WindingFill);
                currentPath.setFillRule(Qt::WindingFill);
                currentContext.clipPath = currentContext.clipPath.intersected(currentPath);
                //currentPath = QPainterPath();
            } else if (token == "W*") {
                if (currentContext.clipPath.length() == 0)
                    currentContext.clipPath = currentPath;
                currentContext.clipPath.setFillRule(Qt::OddEvenFill);
                currentPath.setFillRule(Qt::OddEvenFill);
                currentContext.clipPath = currentContext.clipPath.intersected(currentPath);
                //currentPath = QPainterPath();
            } else if (token == "n") {
                // ? clipPath = QPainterPath();
                currentPath = QPainterPath();
            } else if (token == "q") {
                contexts.append(currentContext);
            } else if (token == "Q") {
                currentContext = contexts.takeLast();
            } else if (token == "S") {
                emit strikePath(currentPath, currentContext);
                currentPath = QPainterPath();
                ++numPath;
            } else if (token == "w") {
                currentContext.pen.setWidth(stack.takeLast());
            } else if (token == "RG") {
                double b = stack.takeLast();
                double g = stack.takeLast();
                double r = stack.takeLast();
                currentContext.pen.setColor(QColor(r*255, g*255, b*255));
            } else if (token == "J") {
                double capStyle = stack.takeLast();
                if (capStyle == 0)
                    currentContext.pen.setCapStyle(Qt::FlatCap);
                else if (capStyle == 1)
                    currentContext.pen.setCapStyle(Qt::RoundCap);
                else
                    currentContext.pen.setCapStyle(Qt::SquareCap);
            } else if (token == "M") {
                currentContext.pen.setMiterLimit(stack.takeLast());
            } else if (token == "f") {
                emit fillPath(currentPath, currentContext, Qt::WindingFill);
                currentPath = QPainterPath();
                ++numPath;
            } else if (token == "f*") {
                emit fillPath(currentPath, currentContext, Qt::OddEvenFill);
                currentPath = QPainterPath();
                ++numPath;
            } else if (token == "d") {
                double offset = stack.takeLast();
                if (lastArray.count() == 0) {
                    currentContext.pen.setStyle(Qt::SolidLine);
                } else {
                    currentContext.pen.setDashOffset(offset);
                    currentContext.pen.setDashPattern(lastArray);
                    lastArray.clear();
                }
            } else if (token == "[") {
                inArray = true;
                if (lastArray.size() > 0) {
                    qDebug() << "Array after an unused array is forbidden";
                    return false;
                }
            } else if (token == "rg") {
                double b = stack.takeLast();
                double g = stack.takeLast();
                double r = stack.takeLast();
                currentContext.brush.setColor(QColor(r*255, g*255, b*255));
            } else if (token == "c") {
                double y3 = stack.takeLast();
                double x3 = stack.takeLast();
                double y2 = stack.takeLast();
                double x2 = stack.takeLast();
                double y1 = stack.takeLast();
                double x1 = stack.takeLast();
                currentPath.cubicTo(x1, y1, x2, y2, x3, y3);
            } else if (token == "j") {
                double joinStyle = stack.takeLast();
                if (joinStyle  == 0)
                    currentContext.pen.setJoinStyle(Qt::MiterJoin);
                else if (joinStyle  == 1)
                    currentContext.pen.setJoinStyle(Qt::RoundJoin);
                else
                    currentContext.pen.setJoinStyle(Qt::BevelJoin);
            } else if (token == "") {
                // Is that an error ?
            } else {
                qDebug() << "Unknown token :" << token;
                qDebug() << "Stack : " << stack;
                return false;
            }
        }
    }

    int sec = start.secsTo(QTime::currentTime());
    qDebug() << "GraphicProducer::parse done: " << numPath << numPoints << sec;
    qDebug() << stack; // check empty ?
    return true;
}
