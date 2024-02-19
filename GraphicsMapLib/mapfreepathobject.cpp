#include "mapfreepathobject.h"
#include "graphicsmap.h"
#include "mapobjectitem.h"

QSet<MapFreePathItem*> MapFreePathItem::m_items;
MapFreePathItem::MapFreePathItem() :
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

MapFreePathItem::~MapFreePathItem()
{
    m_items.remove(this);
}

void MapFreePathItem::setEditable(bool editable)
{
    if(m_editable == editable)
        return;

    m_editable = editable;
    emit editableChanged(editable);
    updateEditable();
}

bool MapFreePathItem::isEditable() const
{
    return m_editable;
}

void MapFreePathItem::toggleEditable()
{
    setEditable(!m_editable);
}

void MapFreePathItem::append(const QGeoCoordinate &coord)
{
    if(m_coords.contains(coord)){
        return;
    }

    m_coords.append(coord);
    updateFreePath(coord);
    emit added(m_coords.indexOf(coord), coord);
}

void MapFreePathItem::remove(const QGeoCoordinate &coord)
{

}

const QVector<QGeoCoordinate> &MapFreePathItem::points() const
{
    return m_coords;
}

const QSet<MapFreePathItem *> &MapFreePathItem::items()
{
    return m_items;
}

void MapFreePathItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPathItem::mouseDoubleClickEvent(event);
    emit doubleClicked();
}

void MapFreePathItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(m_editable)
        m_menu->exec(QCursor::pos());
}

void MapFreePathItem::updateFreePath(const QGeoCoordinate &coord)
{
    auto path = this->path();
    auto fPoint = GraphicsMap::toScene(m_coords.first());
    path.moveTo(fPoint);
    for(auto &item : qAsConst(m_coords)){
        path.lineTo(GraphicsMap::toScene(item));
    }

    this->setPath(path);
}

void MapFreePathItem::updateEditable()
{
    auto pen = this->pen();
    pen.setColor(m_editable ? Qt::white : Qt::lightGray);
    this->setPen(pen);
}
