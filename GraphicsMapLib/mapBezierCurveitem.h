#ifndef MAPBEZIERCURVEITEM_H
#define MAPBEZIERCURVEITEM_H

#include <QGraphicsPathItem>
#include <QGeoCoordinate>
#include <QMenu>

class MapObjectItem;

/*!
 * \brief 贝塞尔曲线
 * \warning
 */
class MapBezierCurveItem : public QObject,public QGraphicsPathItem
{
    Q_OBJECT
public:
    MapBezierCurveItem();
    ~MapBezierCurveItem();

    void setEditable(bool editable);
    bool isEditable() const;
    void toggleEditable();
    /// 添加曲线点
    void append(const QGeoCoordinate &coord, bool isPressPos);
    /// 替换曲线点
    void replacedLastPoint(const QGeoCoordinate &coord, bool isPressPos);

    /// 去掉临时点
    void removeTempPoint();
    /// 获取所有的实例
    static const QSet<MapBezierCurveItem *> &items();
public:
    /// 所有的实例
    static QSet<MapBezierCurveItem *> m_items;
signals:
    void added(const int &index, const QGeoCoordinate &coord);
    void doubleClicked();
    void editableChanged(bool editable);

    void propertyRequset(MapBezierCurveItem *item);
protected:
    virtual QRectF boundingRect() const override;

    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
private:
    void InsertBezierCurve(const QVector<QPointF> &vecSrc, QVector<QPointF> &vecBezier);
    void updateBezierCurve();
private:
    struct BezierPoint{
        QPointF                 pos;
        bool                    isPressPos;
        BezierPoint(){
            pos = QPointF();
            isPressPos = false;
        }
        bool operator==(const BezierPoint b) const
        {
            return this->pos == b.pos && this->isPressPos == b.isPressPos;
        }
    };
    bool                        m_editable;
    QVector<BezierPoint>        m_points;

    QMenu   *m_menu;
    QAction *m_action;
};

#endif // MAPBEZIERCURVEITEM_H
