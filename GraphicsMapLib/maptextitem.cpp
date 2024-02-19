#include "maptextitem.h"
#include "graphicsmap.h"
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QDebug>

QSet<MapTextItem*> MapTextItem::m_items;

MapTextItem::MapTextItem() :
    m_text(u8"测试文本"),
    m_color(Qt::white),
    m_font("Microsoft YaHei",0,1,0),
    m_rect(0,0,72,30),
    m_size(15.00),
    m_edit(true)
{
    QPen pen;
    pen.setColor(Qt::lightGray);
    pen.setWidth(1);

    m_textItem = new QGraphicsTextItem(this);
    m_textItem->setPlainText(m_text);
    m_textItem->setFont(m_font);
    m_textItem->setDefaultTextColor(QColor(Qt::white));
    m_textItem->setFlag(QGraphicsItem::ItemIsMovable);
    m_textItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    ///QGraphicsTextItem 将文字居中显示
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignCenter);
    QTextCursor cursor = m_textItem->textCursor();
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(format);
    cursor.clearSelection();
    m_textItem->setTextCursor(cursor);
    m_textItem->setPos(pos());

    auto it = m_textItem->document();
    connect(it,&QTextDocument::contentsChanged,this,&MapTextItem::updateTextRect);

    m_rectItem = new QGraphicsRectItem();
    m_rectItem->setRect(0,0,m_rect.width(),m_rect.height());
    pen.setStyle(Qt::DotLine);
    m_rectItem->setPen(pen);

    m_items.insert(this);
}

MapTextItem::~MapTextItem()
{

}

void MapTextItem::setText(const QString &text)
{
    m_text = text;
    m_textItem->setPlainText(text);
}

void MapTextItem::setTextColor(const QColor &color)
{
    m_color = color;
    m_textItem->setDefaultTextColor(color);
}

void MapTextItem::setFontSizeF(const qreal size)
{
    m_size = size;
    m_textItem->setTextWidth(size);
}

void MapTextItem::setFont(const QFont &font)
{
    m_font = font;
    m_textItem->setFont(font);
}

void MapTextItem::setRect(const QRectF &rectangle)
{
    m_rectItem->setRect(rectangle);
}

void MapTextItem::attach(QGraphicsItem *obj)
{

}

void MapTextItem::detach()
{

}

QGraphicsTextItem *MapTextItem::getTextItem()
{
    return m_textItem;
}

const QSet<MapTextItem *> &MapTextItem::items()
{
    return m_items;
}

void MapTextItem::setEditabel(bool edit)
{
    m_edit = edit;
    m_rectItem->setVisible(m_edit);
}

void MapTextItem::setMovable(bool moveable)
{
    m_textItem->setFlag(QGraphicsItem::ItemIsMovable, moveable);
    m_textItem->setAcceptHoverEvents(moveable);
}

void MapTextItem::setTransformation(bool transable)
{
    m_textItem->setFlag(QGraphicsItem::ItemIgnoresTransformations, transable);
}

QRectF MapTextItem::boundingRect() const
{
    return {-m_rect.width()/2,-m_rect.height()/2, m_rect.width(), m_rect.height()};
}

bool MapTextItem::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    Q_UNUSED(watched);
    if(!m_edit)
        return false;

    switch (event->type()) {
    case QEvent::GraphicsSceneHoverEnter:
        m_textItem->setScale(1.2);
        break;
    case QEvent::GraphicsSceneHoverLeave:
        m_textItem->setScale(1);
        break;

    default:
        break;
    }
    return false;
}

QVariant MapTextItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if(change != ItemSceneHasChanged)
           return QGraphicsTextItem::itemChange(change, value);

    m_rectItem->setParentItem(m_textItem);

    m_textItem->installSceneEventFilter(this);

    return QGraphicsTextItem::itemChange(change, value);
}

void MapTextItem::updateTextRect()
{
    m_rectItem->setRect(0,0,m_textItem->document()->size().width(),m_textItem->document()->size().height());
}
