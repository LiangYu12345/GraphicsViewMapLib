#ifndef MAPTEXTITEM_H
#define MAPTEXTITEM_H

#include <QGraphicsTextItem>
#include <QGeoCoordinate>
#include <QFont>

/*!
 * \brief 文字
 * \warning
 */
class MapTextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    MapTextItem();
    ~MapTextItem() override;
    /// 设置可编辑
    void setEditabel(bool edit);
    /// 设置可移动,默认可移动
    void setMovable(bool moveable);
    /// 设置是否忽略继承的转换,默认忽略
    void setTransformation(bool transable);
    /// 设置文字
    void setText(const QString &text);
    /// 设置文字颜色
    void setTextColor(const QColor &color);
    /// 设置文字大小
    void setFontSizeF(const qreal size);
    /// 设置文字样式
    void setFont(const QFont &font);
    /// 设置文本框大小

    void setRect(const QRectF &rectangle);

    /// 依附到地图对象，清除已存在的航迹，将会自动更新位置
    void attach(QGraphicsItem *obj);
    /// 取消依附地图对象，后续手动更新位置，如需清除航迹，请手动清除
    void detach();

    /// 获取文本Item
    QGraphicsTextItem * getTextItem();
    /// 获取所有的实例
    static const QSet<MapTextItem *> &items();
public:
    /// 所有的实例
    static QSet<MapTextItem *> m_items;
protected:
    virtual QRectF boundingRect() const override;

    virtual bool sceneEventFilter(QGraphicsItem * watched,QEvent * event) override;
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change,const QVariant &value) override;


private:
    void updateTextRect();
private:
    QGeoCoordinate m_coord;
    QString m_text;
    QColor m_color;
    QFont m_font;
    QRectF m_rect;
    qreal m_size;
    bool m_edit;

    QGraphicsTextItem * m_textItem;
    QGraphicsRectItem * m_rectItem;
};

#endif // MAPTEXTITEM_H
