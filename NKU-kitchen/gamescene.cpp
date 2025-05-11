#include "gamescene.h"
#include "player.h"
#include "item_definitions.h"
#include <QPainter>
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QGraphicsPixmapItem>

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent),
    m_scoreTextItem(nullptr),
    m_currentOrderTextItem(nullptr),
    m_currentOrderProductImage(nullptr)
{
    qDebug() << "Custom GameScene created with visual order display.";

   //加载背景图片
    QString backgroundImagePath = ":/images/kitchen_background.png";
    if (!m_backgroundImage.load(backgroundImagePath)) {
        qWarning() << "错误：无法加载背景图片：" << backgroundImagePath;
        setBackgroundBrush(QColor(200, 200, 200));
    } else {
        qDebug() << "背景图片加载成功。尺寸：" << m_backgroundImage.size();
    }

    // 初始化分数显示
    m_scoreTextItem = new QGraphicsTextItem();
    m_scoreTextItem->setPlainText(QString("得分: 0"));
    m_scoreTextItem->setDefaultTextColor(Qt::white);
    QFont scoreFont("Arial", 16, QFont::Bold);
    m_scoreTextItem->setFont(scoreFont);
    m_scoreTextItem->setPos(10, 10);
    m_scoreTextItem->setZValue(100);
    addItem(m_scoreTextItem);

    // 初始化当前订单文本显示
    m_currentOrderTextItem = new QGraphicsTextItem();
    m_currentOrderTextItem->setPlainText(" "); // 初始为空，等待 MainWindow 更新
    m_currentOrderTextItem->setDefaultTextColor(Qt::yellow);
    QFont orderFont("Arial", 14, QFont::Bold);
    m_currentOrderTextItem->setFont(orderFont);
    m_currentOrderTextItem->setPos(10, 40); // 分数下方
    m_currentOrderTextItem->setZValue(100);
    addItem(m_currentOrderTextItem);

    // 初始化当前订单产品图片显示
    m_currentOrderProductImage = new QGraphicsPixmapItem();
    // 初始位置可以设置在文本旁边或下方，具体位置会在 updateCurrentOrderDisplay 中根据文本调整
    //暂时放在文本下方
    m_currentOrderProductImage->setPos(10, m_currentOrderTextItem->pos().y() + m_currentOrderTextItem->boundingRect().height() + 5);
    m_currentOrderProductImage->setZValue(100);
    m_currentOrderProductImage->setVisible(false);
    addItem(m_currentOrderProductImage);
}

void GameScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);
    if (!m_backgroundImage.isNull()) {
        painter->drawPixmap(this->sceneRect(), m_backgroundImage, m_backgroundImage.rect());
    }
}

void GameScene::updateScoreDisplay(int score)
{
    if (m_scoreTextItem) {
        m_scoreTextItem->setPlainText(QString("得分: %1").arg(score));
    }
}

void GameScene::updateCurrentOrderDisplay(const QString& currentOrderFullText, const QString& orderProductImageKey)
{
    if (m_currentOrderTextItem) {
        m_currentOrderTextItem->setPlainText(currentOrderFullText);
    }

    if (m_currentOrderProductImage) {
        if (orderProductImageKey.isEmpty() || currentOrderFullText == "所有订单完成!") {
            m_currentOrderProductImage->setPixmap(QPixmap()); // 清除图片
            m_currentOrderProductImage->setVisible(false);
        } else {
            QString imagePath = Player::getImagePathForItem(orderProductImageKey);
            if (!imagePath.isEmpty()) {
                QPixmap orderPixmap(imagePath);
                if (!orderPixmap.isNull()) {
                    QSize imageDisplaySize(50, 50); // 订单图片显示大小
                    m_currentOrderProductImage->setPixmap(orderPixmap.scaled(imageDisplaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

                    // 调整图片位置，例如放在订单文本的右边
                    if (m_currentOrderTextItem) {
                        m_currentOrderProductImage->setPos(
                            m_currentOrderTextItem->pos().x() + m_currentOrderTextItem->boundingRect().width() + 10, // X: 文本右侧加间距
                            m_currentOrderTextItem->pos().y() // Y: 与文本对齐
                            );
                    } else {
                        // 备用位置
                        m_currentOrderProductImage->setPos(150, 40);
                    }
                    m_currentOrderProductImage->setVisible(true);
                } else {
                    qWarning() << "GameScene: 无法加载订单图片，路径:" << imagePath << "，键:" << orderProductImageKey;
                    m_currentOrderProductImage->setPixmap(QPixmap());
                    m_currentOrderProductImage->setVisible(false);
                }
            } else {
                qWarning() << "GameScene: 未找到订单图片的路径，键:" << orderProductImageKey;
                m_currentOrderProductImage->setPixmap(QPixmap());
                m_currentOrderProductImage->setVisible(false);
            }
        }
    }
}
