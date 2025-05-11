#include "gamescene.h"
#include <QPainter>
#include <QColor>  // 如果你还想用纯色背景作为备用
#include <QDebug>
#include <QFont>   // << 新增：用于设置字体
#include <qgraphicsitem.h> // 确保 QGraphicsTextItem 被正确包含

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent), m_scoreTextItem(nullptr) // 初始化分数文本指针
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

    // 初始化分数显示文本对象
    m_scoreTextItem = new QGraphicsTextItem();
    m_scoreTextItem->setPlainText(QString("得分: 0")); // 设置初始文本
    m_scoreTextItem->setDefaultTextColor(Qt::white);   // 设置文本颜色为白色，以便在背景上更清晰
    QFont scoreFont("Arial", 16, QFont::Bold);       // 创建字体对象
    m_scoreTextItem->setFont(scoreFont);               // 应用字体
    m_scoreTextItem->setPos(10, 10);                   // 设置文本位置 (例如，场景左上角，留出一些边距)
    m_scoreTextItem->setZValue(100);                   //确保分数显示在最上层
    addItem(m_scoreTextItem);                          // 将文本对象添加到场景中
}

void GameScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect); // 可以先调用基类的方法（可选）

    if (!m_backgroundImage.isNull()) {
        //将图片拉伸以填充整个场景
        painter->drawPixmap(this->sceneRect(), m_backgroundImage, m_backgroundImage.rect());
    }
}

// 更新分数显示的方法实现
void GameScene::updateScoreDisplay(int score)
{
    if (m_scoreTextItem) { // 确保对象有效
        m_scoreTextItem->setPlainText(QString("得分: %1").arg(score));
    }
}
