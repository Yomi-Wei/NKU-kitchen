#include "gamescene.h"
#include <QPainter>
#include <QColor>  // 如果你还想用纯色背景作为备用
#include <QDebug>
#include <qgraphicsitem.h>

GameScene::GameScene(QObject *parent) : QGraphicsScene(parent)
{
    qDebug() << "Custom GameScene created.";

    // 加载背景图片
    QString backgroundImagePath = ":/images/kitchen_background.png"; //确保这是正确的资源路径
    if (!m_backgroundImage.load(backgroundImagePath)) {
        qWarning() << "错误：无法加载背景图片：" << backgroundImagePath;
        // 如果图片加载失败，可以设置一个默认的纯色背景作为备用
        setBackgroundBrush(QColor(200, 200, 200)); // 例如浅灰色
    } else {
        qDebug() << "背景图片加载成功。尺寸：" << m_backgroundImage.size();
    }
}

void GameScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect); // 可以先调用基类的方法（可选）

    if (!m_backgroundImage.isNull()) {

        //将图片拉伸以填充整个场景 (保持图片的宽高比可能会导致部分裁剪或留白，
        // 或者不保持宽高比导致图片变形。这里我们简单拉伸填满)
        painter->drawPixmap(this->sceneRect(), m_backgroundImage, m_backgroundImage.rect());

        // 保持宽高比缩放并居中 (更复杂一些，需要计算)
        // QPixmap scaledPixmap = m_backgroundImage.scaled(this->sceneRect().size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        // QRectF targetRect = scaledPixmap.rect();
        // targetRect.moveCenter(this->sceneRect().center());
        // painter->drawPixmap(targetRect.topLeft(), scaledPixmap);

    } else {
        // 如果图片加载失败，这里的代码可以绘制一个备用背景
        // (在构造函数中我们已经设置了 setBackgroundBrush 作为备用，所以这里可能不需要额外操作)
        // painter->fillRect(rect, QColor(230, 230, 250)); // 例如之前的淡紫色
    }
}
