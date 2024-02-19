#ifndef MAPFREEPATHOBJECT_H
#define MAPFREEPATHOBJECT_H

#include <QGraphicsPathItem>
#include <QGeoCoordinate>
#include <QMenu>

class MapObjectItem;
/*!
 * \brief 自由路径图元
 * \details 模拟画笔绘画
 */
class MapFreePathItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    MapFreePathItem();
    ~MapFreePathItem();
    /// 控制可编辑性
    void setEditable(bool editable);
    bool isEditable() const;
    void toggleEditable();
    /// 添加经纬点
    void append(const QGeoCoordinate &coord);
    /// 删除经纬点
    void remove(const QGeoCoordinate &coord);
    /// 获取自由绘画的经纬点
    const QVector<QGeoCoordinate> &points() const;
    /// 获取所有的实例
    static const QSet<MapFreePathItem *> &items();
signals:
    void added(const int index, const QGeoCoordinate &coord);
    void doubleClicked();
    void editableChanged(bool editable);

    void propertyRequset(MapFreePathItem *item);
protected:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
private:
    void updateFreePath(const QGeoCoordinate &coord);
    void updateEditable();
private:
    static QSet<MapFreePathItem*> m_items;         ///< 所有实例
private:
    bool                            m_editable;     ///< 鼠标是否可交互编辑
    QVector<QGeoCoordinate>         m_coords;       ///< 场景的点集合

    QMenu   *m_menu;
    QAction *m_action;
};

#endif // MAPFREEPATHOBJECT_H
