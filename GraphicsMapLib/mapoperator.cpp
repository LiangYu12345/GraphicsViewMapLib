#include "mapoperator.h"
#include "mapellipseitem.h"
#include "mappolygonitem.h"
#include "maprouteitem.h"
#include "mapobjectitem.h"
#include "maprangeringitem.h"
#include "maptrailitem.h"
#include "maplineitem.h"
#include "maprectitem.h"
#include "maptextitem.h"
#include "mapBezierCurveitem.h"
#include "mapfreepathobject.h"
#include "mapbrokenline.h"
#include "MapPointItem.h"
#include <QDebug>

MapEllipseOperator::MapEllipseOperator(QObject *parent) : MapOperator(parent)
{
}

void MapEllipseOperator::takeOver(MapEllipseItem *item)
{
    m_ellipse = item;
}

void MapEllipseOperator::ready()
{
    MapOperator::ready();
    m_ellipse = nullptr;
}

void MapEllipseOperator::end()
{
    MapOperator::end();
    if(m_ellipse)
        m_ellipse->setEditable(false);
    m_ellipse = nullptr;
}

bool MapEllipseOperator::keyPressEvent(QKeyEvent *event)
{
   if(!m_ellipse)
        return false;
    if(event->key() == Qt::Key_Backspace) {
        emit deleted(m_ellipse);
        m_ellipse = nullptr;
    }
    return false;
}

bool MapEllipseOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    skipOnceMouseEvent();
    // finish
    if(event->buttons() & Qt::LeftButton) {
        detach();
        emit completed();
    }
    return false;
}

bool MapEllipseOperator::mousePressEvent(QMouseEvent *event)
{
    m_first  = m_map->toCoordinate(event->pos());
    // we should ignore event whatever we press at EditOnly mode
    if(mode() == EditOnly) {
        skipOnceMouseEvent();
        return false;
    }
    // just create one
    else if(mode() == CreateOnly) {
        detach();
        m_first  = m_map->toCoordinate(event->pos());
        return true;
    }
    // else, do creating operation
    else {  //CreateEdit
        if(auto ctrlPoint = dynamic_cast<QGraphicsEllipseItem*>(m_map->itemAt(event->pos()))) {
            auto cast = dynamic_cast<MapEllipseItem*>(ctrlPoint->parentItem());
            // we should ignore event if we pressed the control point
            if(cast && cast == m_ellipse) {
                skipOnceMouseEvent();
                return false;
            }
        }
        detach();
        return true;
    }
    return false;
}

bool MapEllipseOperator::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_ellipse)
        return false;
    // Check that if the two point is too close
    auto point0 = m_map->toPoint(m_first);
    auto point1 = event->pos();
    if((point0 - point1).manhattanLength() < 50) {
        point1 = point0 + QPoint(25, 25);
    }
    auto second =  m_map->toCoordinate(point1);
    m_ellipse->setRect(m_first, second);
    return true;
}

bool MapEllipseOperator::mouseMoveEvent(QMouseEvent *event)
{
    // lambda function to create ellipse
    auto createEllipse = [=]() {
        // unset editable for previous created item
        auto ellipse = m_map->addMapItem<MapEllipseItem>();
        ellipse->setRect(m_first, m_first);
        ellipse->setEditable(true);
        //
        return ellipse;
    };

    if(!m_ellipse) {
        m_ellipse = createEllipse();
        emit created(m_ellipse);
    }

    auto second =  m_map->toCoordinate(event->pos());
    m_ellipse->setRect(m_first, second);
    // Press Event didn't propagte to QGraphicsView ,
    // so we should to return false that helps up to zooming on cursor,
    // and map will not be moved by cursor move
    return false;
}

void MapEllipseOperator::detach()
{
    if(m_ellipse) {
        m_ellipse->setEditable(false);
        m_ellipse = nullptr;
    }
}

MapPolygonOperator::MapPolygonOperator(QObject *parent) : MapOperator(parent)
{

}

void MapPolygonOperator::takeOver(MapPolygonItem *item)
{
    if(m_polygon) {
        m_polygon->setEditable(false);
    }
    m_polygon = item;
}

void MapPolygonOperator::ready()
{
    MapOperator::ready();
    m_polygon = nullptr;
}

void MapPolygonOperator::end()
{
    MapOperator::end();
    detach();
}

bool MapPolygonOperator::keyPressEvent(QKeyEvent *event)
{
    if(!m_polygon)
        return false;
    if(event->key() == Qt::Key_Backspace) {
        m_polygon->remove(m_polygon->points().size()-1);
    }
    return false;
}

bool MapPolygonOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // the last point have been created since previous mouse release event
    if(m_polygon) {
        this->skipOnceMouseEvent();
        detach();
        emit completed();
        return true;    // prevent double Click propagate to item
    }
    return false;
}

bool MapPolygonOperator::mousePressEvent(QMouseEvent *event)
{
    if(m_mode == EditOnly) {
        skipOnceMouseEvent();
        return false;
    }
    m_pressPos = event->pos();
    return false;
}

bool MapPolygonOperator::mouseReleaseEvent(QMouseEvent *event)
{
    // do nothing
    if(m_pressPos != event->pos())
        return false;
    // create begin or append
    if(!m_polygon) {
        m_polygon = new MapPolygonItem;
        m_polygon->setEditable(true);
        m_scene->addItem(m_polygon);
        //
        emit created(m_polygon);
    }
    m_polygon->append(m_map->toCoordinate(event->pos()));
    return false;
}

void MapPolygonOperator::detach()
{
    if(m_polygon)
        m_polygon->setEditable(false);
    m_polygon = nullptr;
}

MapObjectOperator::MapObjectOperator(QObject *parent) : MapOperator(parent)
{
}

void MapObjectOperator::takeOver(MapObjectItem *item)
{
    detach();
    m_obj = item;
    item->setCheckable(true);
    item->setChecked(true);
    item->setMoveable(true);
}

void MapObjectOperator::ready()
{
    MapOperator::ready();
}

void MapObjectOperator::end()
{
    MapOperator::end();
    detach();
}

bool MapObjectOperator::keyPressEvent(QKeyEvent *event)
{
   if(!m_obj)
        return false;
    if(event->key() == Qt::Key_Backspace) {
        emit deleted(m_obj);
        m_obj = nullptr;
    }
    return false;
}

bool MapObjectOperator::mousePressEvent(QMouseEvent *event)
{
    if(this->mode() == EditOnly) {
        return false;
    }
    m_pressPos = event->pos();
    return false;
}

bool MapObjectOperator::mouseReleaseEvent(QMouseEvent *event)
{
    if(this->mode() == EditOnly) {
        return false;
    }
    // do nothing
    if(m_pressPos != event->pos())
        return false;

    // create
    auto coord = m_map->toCoordinate(event->pos());
    auto object = new MapObjectItem;
    m_scene->addItem(object);
    object->setZValue(1);
    object->setCoordinate(coord);
    //
    emit created(object);
    return false;
}

bool MapObjectOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    // finish
    if(event->buttons() & Qt::LeftButton) {
        detach();
        emit completed();
    }
    return false;
}

void MapObjectOperator::detach()
{
    if(m_obj) {
        m_obj->setCheckable(false);
        m_obj->setChecked(false);
        m_obj->setMoveable(false);
    }
    m_obj = nullptr;
}

MapRouteOperator::MapRouteOperator(QObject *parent) : MapOperator(parent)
{
}

void MapRouteOperator::takeOver(MapRouteItem *item)
{
    if(m_route) {
        m_route->setMoveable(false);
        m_route->setCheckable(false);
    }
    m_route = item;
    m_route->setMoveable(true);
    m_route->setCheckable(true);
}

void MapRouteOperator::setWaypointIcon(const QPixmap &pixmap)
{
    m_waypointIcon = pixmap;
}

void MapRouteOperator::ready()
{
    m_route = nullptr;
}

void MapRouteOperator::end()
{
    if(m_route) {
        m_route->setMoveable(false);
        m_route->setCheckable(false);
    }
    m_route = nullptr;
}

bool MapRouteOperator::keyPressEvent(QKeyEvent *event)
{
    if(!m_route)
        return false;
    if(event->key() == Qt::Key_Backspace) {
        auto points = m_route->checked();
        for(auto point : qAsConst(points)) {
            m_route->remove(point);
        }
    }
    return false;
}

bool MapRouteOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // the last point have been craeted since previous mouse release event
    // complete
    if(m_route) {
        m_route->setMoveable(false);
        m_route->setCheckable(false);
    }
    m_route = nullptr;
    emit completed();
    return false;
}

bool MapRouteOperator::mousePressEvent(QMouseEvent *event)
{
    // complete
    if(event->buttons() & Qt::RightButton) {
        if(m_route) {
            m_route->setMoveable(false);
            m_route->setCheckable(false);
        }
        m_route = nullptr;
        skipOnceMouseEvent();
        emit completed();
        return false;
    }
    m_pressPos = event->pos();
    if(!m_route)
        return false;

    // check that if we clicked waypoint
    auto mouseItems = m_map->items(event->pos());
    for(auto item : qAsConst(mouseItems)) {
        // find MapObjectItem child
        if(m_route->childItems().contains(item)) {
            skipOnceMouseEvent();
            return false;   // the waypoint itself will process setChecked
        }
    }
    return false;
}

bool MapRouteOperator::mouseReleaseEvent(QMouseEvent *event)
{
    // do nothing
    if((m_pressPos-event->pos()).manhattanLength() > 3)
        return false;

    // create begin or append
    auto coord = m_map->toCoordinate(event->pos());
    coord.setAltitude(0);
    if(!m_route) { // create route
        m_route = new MapRouteItem;
        m_scene->addItem(m_route);
        m_route->setMoveable(true);
        m_route->setExclusive(true);
        m_route->setCheckable(true);
        //
        emit created(m_route);
    }
    auto checked = m_route->checkedIndex();
    auto index = checked.isEmpty() ? -1 : checked.last();
    // append coordinate for route
    m_route->insert(index + 1, coord)->setIcon(m_waypointIcon);
    auto pointItem = m_route->points().at(index + 1);
    pointItem->setText(QString::number(index + 1));
    m_route->setChecked(index + 1);
    return false;
}
MapRangeLineOperator::MapRangeLineOperator(QObject *parent) : MapOperator(parent)
{
}

void MapRangeLineOperator::takeOver(MapLineItem *item)
{
    if(m_line) {
        m_line->setCheckable(false);
    }
    m_line = item;
    m_line->setCheckable(true);
}

void MapRangeLineOperator::ready()
{
    MapOperator::ready();
    m_line = nullptr;
}

void MapRangeLineOperator::end()
{
    if(m_line) {
        m_line->setCheckable(false);
    }
    MapOperator::end();
    m_line = nullptr;
}

bool MapRangeLineOperator::keyPressEvent(QKeyEvent *event)
{
    if(!m_line)
        return false;
    if(event->key() == Qt::Key_Backspace) {
        emit deleted(m_line);
        m_line = nullptr;
    }
    return false;
}

bool MapRangeLineOperator::mousePressEvent(QMouseEvent *event)
{
    if(mode() == EditOnly) {
        skipOnceMouseEvent();
        return false;
    }
    else if(mode() == CreateOnly) {
        detach();
        m_pressFirstPos = event->pos();
        return true;
    }
    // else, do creating operation
    else {  //CreateEdit
        if(auto ctrlPoint = dynamic_cast<QGraphicsEllipseItem*>(m_map->itemAt(event->pos()))) {
            auto cast = dynamic_cast<MapLineItem*>(ctrlPoint->parentItem());
            // we should ignore event if we pressed the control point
            if(cast && cast == m_line) {
                skipOnceMouseEvent();
                return false;
            }
        }
        detach();
        return true;
    }
    return false;
}

bool MapRangeLineOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    skipOnceMouseEvent();
    if(event->buttons() & Qt::LeftButton){
        detach();
        emit completed();
    }
    return false;
}

bool MapRangeLineOperator::mouseReleaseEvent(QMouseEvent *event)
{
//    Q_UNUSED(event)
    if(!m_line)
        return false;
    auto point0 = m_pressFirstPos;
    auto point1 = event->pos();
    if((point0 - point1).manhattanLength() < 50) {
        point1 = point0 + QPoint(25, 25);
    }
    //m_line->setLine({m_pressFirstPos, point1});
	return false;
}

bool MapRangeLineOperator::mouseMoveEvent(QMouseEvent *event)
{
	auto second = m_map->toCoordinate(event->pos());

    if (!m_line) { // create rangLine
        m_line = new MapLineItem;
        m_scene->addItem(m_line);
        m_line->setStartPoint(m_map->toCoordinate(event->pos()));
        m_line->setStartIcon(QPixmap(":/Resources/location.png"), Qt::AlignHCenter | Qt::AlignTop);
        m_line->setEndIcon(QPixmap(":/Resources/location.png"), Qt::AlignHCenter | Qt::AlignTop);
        //
        emit created(m_line);
        return true;
    }

    m_line->setEndPoint(second);
    double dis = m_line->endings().first.distanceTo(m_line->endings().second);
    if (dis > 1000.0) {
		dis = dis * 1E-3;
		QString str = QString::number(dis, 'f', 2) + QString("km");
		m_line->setText(str);
	}
    else if (dis < 1000.0) {
		QString str = QString::number(dis,'f',2) + QString("m");
		m_line->setText(str);
	}
	// Press Event didn't propagte to QGraphicsView ,
	// so we should to return false that helps up to zooming on cursor,
	// and map will not be moved by cursor move
    return false;
}

void MapRangeLineOperator::detach()
{
    if(m_line){
        m_line->setCheckable(false);
    }
    m_line = nullptr;
}

MapRectOperator::MapRectOperator(QObject *parent) : MapOperator(parent)
{
}

void MapRectOperator::takeOver(MapRectItem *item)
{
    m_rect = item;
}

void MapRectOperator::ready()
{
    MapOperator::ready();
    m_rect = nullptr;
}

void MapRectOperator::end()
{
    MapOperator::end();
    if(m_rect)
        m_rect->setEditable(false);
    m_rect = nullptr;
}

bool MapRectOperator::keyPressEvent(QKeyEvent *event)
{
    if(!m_rect)
        return false;
    if(event->key() == Qt::Key_Backspace) {
        emit deleted(m_rect);
        m_rect = nullptr;
    }
    return false;
}

bool MapRectOperator::mousePressEvent(QMouseEvent *event)
{
    if(mode() == EditOnly) {
        skipOnceMouseEvent();
        return false;
    }
    // CreateOnly & CreateEdit
    // Ignore the event when click on the control point
    if(auto ctrlPoint = dynamic_cast<QGraphicsEllipseItem*>(m_map->itemAt(event->pos()))) {
        if(dynamic_cast<MapRectItem*>(ctrlPoint->parentItem()) == m_rect) {
            skipOnceMouseEvent();
            return false;
        }
    }

    // unset editable for previous created item
    detach();

    m_first  = m_map->toCoordinate(event->pos());
    return true;
}

bool MapRectOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    // todo: ignore release event and move event because doubleClick will prepagate that event
    skipOnceMouseEvent();

    // finish
    if(event->buttons() & Qt::LeftButton) {
        detach();
        emit completed();
        return false;
    }
    return false;
}

bool MapRectOperator::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_rect)
        return false;
    // Check that if the two point is too close, we should delete such an rect
    auto point0 = m_map->toPoint(m_first);
    auto point1 = event->pos();
    if((point0 - point1).manhattanLength() < 50) {
        point1 = point0 + QPoint(25, 25);
    }
    auto second =  m_map->toCoordinate(point1);
    m_rect->setRect(m_first, second);

    return false;
}

bool MapRectOperator::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_rect) {
        m_rect = m_map->addMapItem<MapRectItem>();
        m_rect->setEditable(true);
        emit created(m_rect);
    }
    auto second =  m_map->toCoordinate(event->pos());
    m_rect->setRect(m_first, second);
    // Press Event didn't propagte to QGraphicsView ,
    // so we should to return false that helps up to zooming on cursor,
    // and map will not be moved by cursor move
    return false;
}

void MapRectOperator::detach()
{
    if(m_rect) {
        m_rect->setEditable(false);
        m_rect = nullptr;
    }
}

MapScutcheonOperator::MapScutcheonOperator(QObject *parent) : MapOperator(parent)
{
    m_Offset = QPoint(0, 0);
    m_bMousePress = false;
}

void MapScutcheonOperator::takeOver(MapTableItem *item)
{
    m_toolTip = item;
}

void MapScutcheonOperator::setIgnoreMouseEvent(bool bIgnore)
{
    m_bIgnoreEvent = bIgnore;
}

void MapScutcheonOperator::setOffset(const QPoint &offset)
{
    m_Offset = offset;
    emit changeOffset();
}

void MapScutcheonOperator::ready()
{
    MapOperator::ready();
    m_toolTip = nullptr;
}

void MapScutcheonOperator::end()
{
    MapOperator::end();
//    if(m_toolTip)
//        m_toolTip->setEditable(false);
    m_toolTip = nullptr;
}

bool MapScutcheonOperator::keyPressEvent(QKeyEvent *event)
{
    if(!m_toolTip)
        return false;
    if(event->key() == Qt::Key_Backspace) {
        //emit deleted(m_toolTip);
        m_toolTip = nullptr;
    }
    return false;
}

bool MapScutcheonOperator::mousePressEvent(QMouseEvent *event)
{
    if(!m_bIgnoreEvent){
        m_bMousePress = true;
        m_PressPosition = event->screenPos().toPoint();
        return true;
    }
    return false;
}

bool MapScutcheonOperator::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_bIgnoreEvent && m_bMousePress)
    {
        auto mousePos = event->screenPos().toPoint();
        QPoint moveSpan = mousePos - m_PressPosition;
        m_PressPosition = mousePos;
        m_Offset += moveSpan;
        emit changeOffset();
    }
    return false;
}

bool MapScutcheonOperator::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_bIgnoreEvent){
        m_bMousePress = false;
    }
    return false;
}

bool MapScutcheonOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    //skipOnceMouseEvent();

    // finish
    if(event->buttons() & Qt::LeftButton) {
        //detach();
        emit completed();
        return false;
    }
    return false;
}

MapTextOperator::MapTextOperator(QObject *parent) : MapOperator(parent)
{

}

void MapTextOperator::takeOver(MapTextItem *item)
{
    if(m_text == item)
        return;
    m_text = item;
}

void MapTextOperator::ready()
{
    MapOperator::ready();
    m_text = nullptr;
    m_finishRequested = false;
}

void MapTextOperator::end()
{
    MapOperator::end();
    if(m_text)
    {
        m_finishRequested = true;
        m_text->setEditabel(false);
    }
    m_text = nullptr;
}

bool MapTextOperator::keyPressEvent(QKeyEvent *event)
{
    // 分模式判断Key_Backspace
    if(!m_isEditTextFinish)
        return false;
    if(m_finishRequested == false && m_isEditTextFinish) {
        if(event->key() == Qt::Key_Backspace)
        {
            m_text->setEditabel(false);
            m_currentItem->setTextInteractionFlags(Qt::NoTextInteraction);
            m_map->scene()->removeItem(m_currentItem);
            m_map->scene()->removeItem(m_text);
            m_currentItem = nullptr;
            m_text = nullptr;
            m_isEditTextFinish = true;
            m_isDelete = true;

            emit deleted(m_text);
        }
    }
    return false;
}

bool MapTextOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        if(auto ctrlPoint = dynamic_cast<QGraphicsRectItem*>(m_map->itemAt(event->pos()))) {
            auto Text = dynamic_cast<QGraphicsTextItem*>(ctrlPoint->parentItem());
            auto cast = dynamic_cast<MapTextItem*>(Text->parentItem());
            if(MapTextItem::items().contains(cast)) {
                m_currentItem = cast->getTextItem();
                m_currentItem->setTextInteractionFlags(Qt::TextEditorInteraction);
                m_isEditTextFinish = false;
                m_isDelete = false;
                return false;
            }
        }
    }
    return false;
}

bool MapTextOperator::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        if(auto ctrlPoint = dynamic_cast<QGraphicsTextItem*>(m_map->itemAt(event->pos()))) {
            m_currentItem = ctrlPoint;
            auto cast = dynamic_cast<MapTextItem*>(ctrlPoint->parentItem());
            if(MapTextItem::items().contains(cast)) {
                m_text = cast;
                m_text->setEditabel(true);
                m_currentItem->setFlag(QGraphicsItem::ItemIsMovable, true);
                m_finishRequested = false;
                m_isDelete = false;
                return false;
            }
        }
    }
    if(!m_isDelete){      // 当前一个文本被激活 并未被删除
        if(event->button() == Qt::RightButton)
        {
            m_finishRequested = true;
            m_text->setEditabel(false);
            m_currentItem->setFlag(QGraphicsItem::ItemIsMovable, false);
            m_currentItem->setTextInteractionFlags(Qt::NoTextInteraction);
            m_isEditTextFinish = true;
            return false;
        }
    }
    m_pressPos = event->pos();
    return false;
}

bool MapTextOperator::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_finishRequested)
    {
        m_finishRequested = false;
        if(m_text)
        {
            m_text->setEditabel(false);
            m_currentItem->setFlag(QGraphicsItem::ItemIsMovable, false);
            m_text = nullptr;
        }
        return false;
    }

    if(event->button() ==  Qt::LeftButton)
    {
        if(!m_text)
        {
            if((m_pressPos-event->pos()).manhattanLength() > 3)
                return false;
            if(auto ctrlPoint = dynamic_cast<QGraphicsRectItem*>(m_map->itemAt(event->pos()))) {
                //auto Text = dynamic_cast<QGraphicsTextItem*>(ctrlPoint->parentItem());
                //auto cast = dynamic_cast<MapTextItem*>(Text->parentItem());
            }
            m_text = new MapTextItem;
            auto coord = m_map->toCoordinate(event->pos());
            m_text->setPos(m_map->toScene(coord));
            m_text->setEditabel(true);
            m_scene->addItem(m_text);
            m_currentItem = m_text->getTextItem();
            m_finishRequested = false;
            m_isDelete = false;
        }
    }
    return false;
}

bool MapTextOperator::mouseMoveEvent(QMouseEvent *event)
{
    MapOperator::mouseMoveEvent(event);

    if (m_finishRequested)
        return false;
    return false;
}

MapItemOperator::MapItemOperator(QObject *parent)
    : MapOperator(parent),
    m_ctrlPressed(false)
{

}

void MapItemOperator::takeOver(QList<QGraphicsItem *>items)
{
    //qDeleteAll(m_currentItems);
    //m_currentItems.clear();

    for(auto item : qAsConst(items))
        m_currentItems.insert(item);
}

void MapItemOperator::ready()
{
    MapOperator::ready();
}

void MapItemOperator::end()
{
    MapOperator::end();
}

bool MapItemOperator::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control){
        m_ctrlPressed = true;
    }
    return false;
}

bool MapItemOperator::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control){
        m_ctrlPressed = false;
    }
    return false;
}

bool MapItemOperator::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::RightButton){
        for(auto item :  qAsConst(m_currentItems)){
            if(auto object = dynamic_cast<MapObjectItem *>(item)){
                object->setChecked(false);
                object->setMoveable(false);
                object->setCheckable(false);
            }
            else if(auto ellipse = dynamic_cast<MapEllipseItem *>(item)){
                ellipse->setEditable(false);
            }
            else if(auto rect = dynamic_cast<MapRectItem *>(item)){
                rect->setEditable(false);
            }
            else if(auto polygon = dynamic_cast<MapPolygonItem *>(item)){
                polygon->setEditable(false);
            }
            else if(auto line = dynamic_cast<MapLineItem *>(item)){
                line->setCheckable(false);
            }
        }
        emit finished();
        return false;
    }
    if(m_ctrlPressed && event->buttons() & Qt::LeftButton){
        auto mouseItems = m_map->items(event->pos());
        for(auto item : qAsConst(mouseItems)){
            if(m_currentItems.contains(item)){
                if(auto object = dynamic_cast<MapObjectItem *>(item)){
                    object->setChecked(false);
                    object->setMoveable(false);
                    object->setCheckable(false);
                }
                else if(auto ellipse = dynamic_cast<MapEllipseItem *>(item)){
                    ellipse->setEditable(false);
                }
                else if(auto rect = dynamic_cast<MapRectItem *>(item)){
                    rect->setEditable(false);
                }
                else if(auto polygon = dynamic_cast<MapPolygonItem *>(item)){
                    polygon->setEditable(false);
                }
                else if(auto line = dynamic_cast<MapLineItem *>(item)){
                    line->setCheckable(false);
                }
                m_currentItems.remove(item);
                emit removeItem(item);
            }
            else{
                if(auto object = dynamic_cast<MapObjectItem *>(item)){
                    object->setMoveable(true);
                    object->setCheckable(true);
                    object->setChecked(true);
                }
                else if(auto ellipse = dynamic_cast<MapEllipseItem *>(item)){
                    ellipse->setEditable(true);
                }
                else if(auto rect = dynamic_cast<MapRectItem *>(item)){
                    rect->setEditable(true);
                }
                else if(auto polygon = dynamic_cast<MapPolygonItem *>(item)){
                    polygon->setEditable(true);
                }
                else if(auto line = dynamic_cast<MapLineItem *>(item)){
                    line->setCheckable(true);
                }
                m_currentItems.insert(item);
                emit addItem(item);
            }
        }
    }
    return false;
}

bool MapItemOperator::mouseReleaseEvent(QMouseEvent *event)
{
    return false;
}

bool MapItemOperator::mouseMoveEvent(QMouseEvent *event)
{
    return false;
}

bool MapItemOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    return false;
}

MapBexierCurveOperator::MapBexierCurveOperator(QObject *parent)
{
    setMouseTracking(true);
}

void MapBexierCurveOperator::takeOver(MapBezierCurveItem *item)
{
    if(m_bezier == item)
        return;
    m_bezier = item;
}

void MapBexierCurveOperator::ready()
{
    m_bezier = nullptr;
}

void MapBexierCurveOperator::end()
{
    if(m_bezier) {
        m_bezier->setEditable(false);
    }
    m_bezier = nullptr;
}

bool MapBexierCurveOperator::keyPressEvent(QKeyEvent *event)
{
    if(!m_bezier)
        return false;
    if(event->key() == Qt::Key_Backspace) {
        emit deleted(m_bezier);
        m_bezier = nullptr;
    }
    return false;
}

bool MapBexierCurveOperator::keyReleaseEvent(QKeyEvent *event)
{
    return false;
}

bool MapBexierCurveOperator::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::RightButton) {
        if(m_bezier) {
            m_bezier->setEditable(false);
        }
        m_bezier = nullptr;
        skipOnceMouseEvent();
        emit completed();
        return false;
    }

    auto coord = m_map->toCoordinate(event->pos());
    coord.setAltitude(0);

    if(!m_bezier){
        m_bezier = new MapBezierCurveItem;
        m_scene->addItem(m_bezier);
        m_bezier->setEditable(true);
        emit created(m_bezier);
    }
    m_bezier->append(coord, true);

    return true;
}

bool MapBexierCurveOperator::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return false;
}

bool MapBexierCurveOperator::mouseMoveEvent(QMouseEvent *event)
{
    if(m_bezier){
        auto coord = m_map->toCoordinate(event->pos());
        coord.setAltitude(0);

        m_bezier->removeTempPoint();
        m_bezier->append(coord, false);
    }
    return false;
}

bool MapBexierCurveOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    skipOnceMouseEvent();
    // finish
    if(event->buttons() & Qt::LeftButton) {
        detach();
        emit completed();
    }
    return false;
}

void MapBexierCurveOperator::detach()
{
    if(m_bezier) {
        m_bezier->setEditable(false);
        m_bezier = nullptr;
    }
}

MapFreeDrawOperator::MapFreeDrawOperator(QObject *parent)
{

}

void MapFreeDrawOperator::takeOver(MapFreePathItem *item)
{
    if(m_freePath) {
        m_freePath->setEditable(false);
    }
    m_freePath = item;
    m_freePath->setEditable(true);
}

void MapFreeDrawOperator::ready()
{
    m_freePath = nullptr;
}

void MapFreeDrawOperator::end()
{
    if(m_freePath) {
        m_freePath->setEditable(false);
    }
    m_freePath = nullptr;
}

bool MapFreeDrawOperator::keyPressEvent(QKeyEvent *event)
{
    if(!m_freePath)
        return false;
    if(event->key() == Qt::Key_Backspace) {
        auto points = m_freePath->points();
        for(auto &point : qAsConst(points)) {
            m_freePath->remove(point);
        }
    }
    return false;
}

bool MapFreeDrawOperator::mousePressEvent(QMouseEvent *event)
{
    if(mode() == EditOnly) {
        skipOnceMouseEvent();
        return false;
    }
    // complete
    if(event->buttons() & Qt::RightButton) {
        if(m_freePath) {
            m_freePath->setEditable(false);
        }
        m_freePath = nullptr;
        skipOnceMouseEvent();
        emit completed();
        return false;
    }

    // unset editable for previous created item
    detach();

    m_pressPos = event->pos();
    qDebug()<< "pressPos" << m_pressPos;

    return true;
}

bool MapFreeDrawOperator::mouseReleaseEvent(QMouseEvent *event)
{
    return false;
}

bool MapFreeDrawOperator::mouseMoveEvent(QMouseEvent *event)
{
    auto coord = m_map->toCoordinate(event->pos());
    coord.setAltitude(0);
    if(!m_freePath) { // create freePath
        m_freePath = new MapFreePathItem;
        m_scene->addItem(m_freePath);
        m_freePath->setEditable(true);
        //
        emit created(m_freePath);
    }
    m_freePath->append(coord);
    return false;
}

bool MapFreeDrawOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // the last point have been craeted since previous mouse release event
    // complete
    if(m_freePath) {
        m_freePath->setEditable(false);
    }
    m_freePath = nullptr;
    emit completed();
    return false;
}

void MapFreeDrawOperator::detach()
{
    if(m_freePath){
        m_freePath->setEditable(false);
    }
    m_freePath = nullptr;
}

MapBrokenLineOperator::MapBrokenLineOperator(QObject *parent)
{

}

void MapBrokenLineOperator::takeOver(MapBrokenLine *item)
{
    if(m_brokenLine) {
        m_brokenLine->setEditable(false);
    }
    m_brokenLine = item;
}

void MapBrokenLineOperator::ready()
{
    MapOperator::ready();
    m_brokenLine = nullptr;
}

void MapBrokenLineOperator::end()
{
    MapOperator::end();
    detach();
}

bool MapBrokenLineOperator::keyPressEvent(QKeyEvent *event)
{
    if(!m_brokenLine)
        return false;
    if(event->key() == Qt::Key_Backspace) {
        m_brokenLine->remove(m_brokenLine->points().size()-1);
    }
    return false;
}

bool MapBrokenLineOperator::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // the last point have been created since previous mouse release event
    if(m_brokenLine) {
        this->skipOnceMouseEvent();
        detach();
        emit completed();
        return true;    // prevent double Click propagate to item
    }
    return false;
}

bool MapBrokenLineOperator::mousePressEvent(QMouseEvent *event)
{
    if(m_mode == EditOnly) {
        skipOnceMouseEvent();
        return false;
    }
    m_pressPos = event->pos();
    return false;
}

bool MapBrokenLineOperator::mouseReleaseEvent(QMouseEvent *event)
{
    // do nothing
    if(m_pressPos != event->pos())
        return false;
    // create begin or append
    if(!m_brokenLine) {
        m_brokenLine = new MapBrokenLine;
        m_brokenLine->setEditable(true);
        m_scene->addItem(m_brokenLine);
        //
        emit created(m_brokenLine);
    }
    m_brokenLine->append(m_map->toCoordinate(event->pos()));
    return false;
}

void MapBrokenLineOperator::detach()
{
    if(m_brokenLine){
        m_brokenLine->setEditable(false);
    }
    m_brokenLine = nullptr;
}


MapLocationPointOperator::MapLocationPointOperator(QObject *parent) : MapOperator(parent),
    m_item(nullptr),
    m_bDoubleClicked(false),
    m_bShow(false)
{

}

bool MapLocationPointOperator::mouseClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return false;

    auto scenePos = m_map->mapToScene(event->pos());
    if (!m_item)    // first create
    {
        m_item = new MapPointItem();
        QPixmap pix(":/res/star.png");
        m_item->setIcon(pix);
        m_map->scene()->addItem(m_item);
        connect(m_item, &MapPointItem::coordinateChanged, this, &MapLocationPointOperator::coordinateChanged);


    }

    m_item->SetPoint(scenePos, m_PointType);
    show(false);

    auto textItem = m_item->GetTextItem();

    return false;
}

void MapLocationPointOperator::setAmuizth(float amuizth)
{
    if (m_item)
    {
        m_item->setAmuizth(amuizth);
    }
}

void MapLocationPointOperator::setPos(QGeoCoordinate coord)
{
    if(m_item)
        m_item->setPos(m_map->toScene(coord));
}

void MapLocationPointOperator::setPos(double x, double y)
{
    if (m_item)
        m_item->setPos(QPointF(x, y));
}

void MapLocationPointOperator::setIconShow(bool isShow)
{
    if (m_item)
        m_item->setVisible(isShow);
}

bool MapLocationPointOperator::mousePressEvent(QMouseEvent *event)
{
    return mouseClickEvent(event);
}

bool MapLocationPointOperator::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_item)
        return false;

    if (m_PointType >= 1)
    {
        emit SigEnd();//当为标记点时，鼠标释放直接结束
    }
    if (event->button() == Qt::RightButton)
    {
        m_bShow = false;
        emit SigEnd();
        if (m_PointType < 1) //定位点右键隐藏，标记点右键不隐藏 m_PointType>=1 为标记点
        {
            m_item->SetCtrlVisiable(false);
        }
    }
    emit sigPosition(m_map->toCoordinate(event->pos()));

    return  false;
}

void MapLocationPointOperator::show(bool bIsShow)
{
    m_bShow = bIsShow;
    if (!m_item)
    {
        return ;
    }
    m_item->SetCtrlVisiable(bIsShow);
}
