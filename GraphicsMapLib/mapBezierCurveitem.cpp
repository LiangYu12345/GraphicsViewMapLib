#include "mapBezierCurveitem.h"
#include "graphicsmap.h"
#include "mapobjectitem.h"
#include <QDebug>

QSet<MapBezierCurveItem*> MapBezierCurveItem::m_items;
MapBezierCurveItem::MapBezierCurveItem() :
    m_editable(false)
{
    QPen pen;
    pen.setColor(Qt::lightGray);
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);

    m_menu = new QMenu;
    m_action = new QAction;
    m_menu->addAction(m_action);
    m_action->setText(u8"右键属性");
    connect(m_action, &QAction::triggered, this, [=](){
        emit propertyRequset(this);
    }, Qt::DirectConnection);
    m_items.insert(this);
}

MapBezierCurveItem::~MapBezierCurveItem()
{

}

void MapBezierCurveItem::setEditable(bool editable)
{
    if(m_editable == editable)
        return;

    m_editable = editable;
    emit editableChanged(editable);

    auto pen = this->pen();
    pen.setColor(m_editable ? Qt::white : Qt::lightGray);
    this->setPen(pen);
}

bool MapBezierCurveItem::isEditable() const
{
    return m_editable;
}

void MapBezierCurveItem::toggleEditable()
{
    setEditable(!m_editable);
}

void MapBezierCurveItem::append(const QGeoCoordinate &coord, bool isPressPos)
{
    BezierPoint point;
    point.pos = GraphicsMap::toScene(coord);
    point.isPressPos = isPressPos;

    if(isPressPos == false){
        replacedLastPoint(coord, isPressPos);
    }
    else{
        m_points.append(point);
    }

    updateBezierCurve();
}

void MapBezierCurveItem::replacedLastPoint(const QGeoCoordinate &coord, bool isPressPos)
{
    BezierPoint point;
    point.pos = GraphicsMap::toScene(coord);
    point.isPressPos = isPressPos;

    m_points.insert(m_points.size(), point);
}

void MapBezierCurveItem::removeTempPoint()
{
    for(auto &item : qAsConst(m_points)){
        if(item.isPressPos == false)
            m_points.removeOne(item);
    }
}

const QSet<MapBezierCurveItem *> &MapBezierCurveItem::items()
{
    return m_items;
}

QRectF MapBezierCurveItem::boundingRect() const
{
    return this->path().boundingRect();
}

void MapBezierCurveItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPathItem::mouseDoubleClickEvent(event);
    emit doubleClicked();
}

void MapBezierCurveItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(m_editable)
        m_menu->exec(QCursor::pos());
}

void MapBezierCurveItem::InsertBezierCurve(const QVector<QPointF> &vecSrc, QVector<QPointF> &vecBezier)
{
    QVector<QPointF> controlPoints;
    for (int i = 0; i < vecSrc.size() - 1; ++i)
    {
        int n1 = i - 1;
        int n2 = i;
        int n3 = i + 1;
        int n4 = i + 2;
        if (n1 < 0){
            n1 = 0;
        }
        if(n3 > vecSrc.size() - 1){
            n3 = vecSrc.size() - 1;
        }
        if (n4 > vecSrc.size() - 1){
            n4 = vecSrc.size() - 1;
        }

        QPointF a1 = vecSrc[n2] + (vecSrc[n3] - vecSrc[n1]) / 6.0;
        QPointF a2 = vecSrc[n3] - (vecSrc[n4] - vecSrc[n2]) / 6.0;
        controlPoints.push_back(a1);
        controlPoints.push_back(a2);
    }

   vecBezier.clear();
   for (int i = 0; i < vecSrc.size() - 1; ++i)
   {
       auto x1 = vecSrc[i].x();
       auto y1 = vecSrc[i].y();
       auto x2 = controlPoints[2 * i + 0].x();
       auto y2 = controlPoints[2 * i + 0].y();
       auto x3 = controlPoints[2 * i + 1].x();
       auto y3 = controlPoints[2 * i + 1].y();
       auto x4 = vecSrc[i + 1].x();
       auto y4 = vecSrc[i + 1].y();
       int threshold = 15;
       float one_over_threshold_minus_1 = float(1) / (threshold - 1);
       for (int t = 0; t < threshold - 1; ++t)
       {
           auto temp = t * one_over_threshold_minus_1;
           float x, y;
           //不包含结束点
           float m_t = 1. - temp;
           {
               float a = x1*m_t + x2*temp;
               float b = x2*m_t + x3*temp;
               float c = x3*m_t + x4*temp;
               a = a*m_t + b*temp;
               b = b*m_t + c*temp;
               x = a*m_t + b*temp;
           }
           {
               float a = y1*m_t + y2*temp;
               float b = y2*m_t + y3*temp;
               float c = y3*m_t + y4*temp;
               a = a*m_t + b*temp;
               b = b*m_t + c*temp;
               y = a*m_t + b*temp;
           }
           vecBezier.push_back({x , y});
       }
   }
   //加入整个曲线结束点
   if (vecSrc.size() >= 2)
   {
       vecBezier.push_back(vecSrc.last());
   }
}

void MapBezierCurveItem::updateBezierCurve()
{
    if(m_points.isEmpty()){
        setPath(QPainterPath());
        return;
    }
    setPath(QPainterPath());

    auto path = this->path();
    QVector<QPointF> vecSrc;
    QVector<QPointF> vecBezier;

    for(auto &item : qAsConst(m_points)){
        vecSrc.append(item.pos);
    }
    InsertBezierCurve(vecSrc, vecBezier);
    for(int Index = 0; Index < vecBezier.size(); ++Index){
        if(Index == 0)
            path.moveTo(vecBezier.at(Index));
        else
            path.lineTo(vecBezier.at(Index));
    }
    setPath(path);
}
