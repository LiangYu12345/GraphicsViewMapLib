#include "mapbrokenline.h"
#include "graphicsmap.h"
#include <QDebug>

QSet<MapBrokenLine*> MapBrokenLine::m_items;

MapBrokenLine::MapBrokenLine() :
    m_editable(false),
    m_sceneAdded(false)
{
    // keep the outline width of 1-pixel when item scales
    auto pen = this->pen();
    pen.setWidth(1);
//    pen.setCosmetic(true);
    this->setPen(pen);

    m_menu = new QMenu;
    m_action = new QAction;
    m_menu->addAction(m_action);
    m_action->setText(u8"右键属性");
    connect(m_action, &QAction::triggered, this, [=](){
        emit propertyRequset(this);
    }, Qt::DirectConnection);
    //
    m_items.insert(this);
    updateEditable();
}

MapBrokenLine::~MapBrokenLine()
{
    m_items.remove(this);
}

void MapBrokenLine::setEditable(bool editable)
{
    if(m_editable == editable)
        return;

    m_editable = editable;
    emit editableChanged(editable);
    updateEditable();
}

bool MapBrokenLine::isEditable() const
{
    return m_editable;
}

void MapBrokenLine::toggleEditable()
{
    setEditable(!m_editable);
}

void MapBrokenLine::append(const QGeoCoordinate &coord)
{
    m_coords.append(coord);
    // Adding scene point
    auto point = GraphicsMap::toScene(coord);
    m_points.append(point);
    updatePolygon();
    //
    emit added(m_points.size()-1, coord);
}

void MapBrokenLine::replace(const int &index, const QGeoCoordinate &coord)
{
    if(m_coords.at(index) == coord)
        return;
    m_coords.replace(index, coord);
    // Modify scene point
    auto point = GraphicsMap::toScene(coord);
    m_points.replace(index, point);
    updatePolygon();
    //
    emit updated(index, coord);
}

void MapBrokenLine::remove(int index)
{
    if(index < 0 || index >= m_coords.size())
        return;
    auto coord = m_coords.at(index);
    m_coords.removeAt(index);
    m_points.removeAt(index);
    //
    updatePolygon();
    //
    emit removed(index, coord);
}

void MapBrokenLine::removeEnd()
{
    remove(m_coords.size() - 1);
}

void MapBrokenLine::setPoints(const QVector<QGeoCoordinate> &coords)
{
    if(m_coords == coords)
        return;

    // Change previous coords and points
    m_coords = coords;
    m_points.clear();
    for(auto &coord : coords) {
        auto point = GraphicsMap::toScene(coord);
        m_points.append(point);
    }
    updatePolygon();
    //
    emit changed();
}

const QVector<QGeoCoordinate> &MapBrokenLine::points() const
{
    return m_coords;
}

int MapBrokenLine::count()
{
    return m_coords.size();
}

const QGeoCoordinate &MapBrokenLine::at(int i) const
{
    return m_coords.at(i);
}

const QSet<MapBrokenLine *> &MapBrokenLine::items()
{
    return m_items;
}

QVariant MapBrokenLine::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
   if(change != ItemSceneHasChanged)
       return QGraphicsPathItem::itemChange(change, value);

   m_sceneAdded = true;
   return QGraphicsPathItem::itemChange(change, value);
}

void MapBrokenLine::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPathItem::mouseDoubleClickEvent(event);
    emit doubleClicked();
}

void MapBrokenLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(m_editable)
        m_menu->exec(QCursor::pos());
}

void MapBrokenLine::updatePolygon()
{
    // Reset polygon data to QGraphicsPolygonItem
    QPainterPath p;
    for(auto item : qAsConst(m_points)){
        if(item == m_points.first())
            p.moveTo(item);
        p.lineTo(item);
    }
    this->setPath(p);

    updateEditable();
}

void MapBrokenLine::updateEditable()
{
    auto pen = this->pen();
    pen.setColor(m_editable ? Qt::white : Qt::lightGray);
    this->setPen(pen);
}
