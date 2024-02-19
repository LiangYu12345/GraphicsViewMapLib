﻿#ifndef MAPPOLYGONITEM_H
#define MAPPOLYGONITEM_H

#include <QGraphicsItemGroup>
#include <QGraphicsPolygonItem>
#include <QGraphicsEllipseItem>
#include <QGeoCoordinate>
#include <QMenu>

/*!
 * \brief 多边形
 * \note 暂缺少图形拖动的实现
 */
class MapPolygonItem : public QObject, public QGraphicsPolygonItem
{
    Q_OBJECT
public:
    MapPolygonItem();
    ~MapPolygonItem();
    /// 控制可编辑性
    void setEditable(bool editable);
    bool isEditable() const;
    void toggleEditable();
    /// 添加经纬点
    void append(const QGeoCoordinate &coord);
    /// 修改经纬点
    void replace(const int &index, const QGeoCoordinate &coord);
    /// 删除经纬点
    void remove(int index);
    void removeEnd();
    /// 设置多边形顶点
    void setPoints(const QVector<QGeoCoordinate> &coords);
    /// 获取多边形顶点
    const QVector<QGeoCoordinate> &points() const;
    int count();
    /// 获取某个点的位置
    const QGeoCoordinate &at(int i) const;

public:
    /// 获取所有的实例
    static const QSet<MapPolygonItem*> &items();

signals:
    void added(const int index, const QGeoCoordinate &coord);
    void removed(const int index, const QGeoCoordinate &coord);
    void updated(const int &index, const QGeoCoordinate &coord);
    void changed();
    void doubleClicked();
    void editableChanged(bool editable);

    void propertyRequset(MapPolygonItem *item);
protected:
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
private:
    void updatePolygon();   ///< 通过场景坐标更新图形
    void updateEditable();

private:
    static QSet<MapPolygonItem*> m_items;         ///< 所有实例

private:
    bool    m_editable;   ///< 鼠标是否可交互编辑
    bool    m_sceneAdded; ///< 是否已被添加到场景
    //
    QVector<QGeoCoordinate>      m_coords;     ///< 经纬点列表
    QVector<QPointF>             m_points;     ///< 场景坐标点列表

    QMenu   *m_menu;
    QAction *m_action;
};

#endif // MAPPOLYGONITEM_H
