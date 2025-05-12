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
    m_currentOrderProductImage(nullptr),
    m_orderTimerTextItem(nullptr)
{
    qDebug() << "Custom GameScene created with visual order and timer display.";

    QString backgroundImagePath = ":/images/kitchen_background.png";
    if (!m_backgroundImage.load(backgroundImagePath)) {
        qWarning() << "错误：无法加载背景图片：" << backgroundImagePath;
        setBackgroundBrush(QColor(200, 200, 200));
    } else {
        qDebug() << "背景图片加载成功。尺寸：" << m_backgroundImage.size();
    }

    // 分数显示
    m_scoreTextItem = new QGraphicsTextItem();
    m_scoreTextItem->setPlainText(QString("得分: 0"));
    m_scoreTextItem->setDefaultTextColor(Qt::white);
    QFont scoreFont("Arial", 16, QFont::Bold);
    m_scoreTextItem->setFont(scoreFont);
    m_scoreTextItem->setPos(10, 10);
    m_scoreTextItem->setZValue(100);
    addItem(m_scoreTextItem);

    // 当前订单文本显示
    m_currentOrderTextItem = new QGraphicsTextItem();
    m_currentOrderTextItem->setPlainText(" ");
    m_currentOrderTextItem->setDefaultTextColor(Qt::yellow);
    QFont orderFont("Arial", 14, QFont::Bold);
    m_currentOrderTextItem->setFont(orderFont);
    m_currentOrderTextItem->setPos(10, 40);
    m_currentOrderTextItem->setZValue(100);
    addItem(m_currentOrderTextItem);

    // 当前订单产品图片显示
    m_currentOrderProductImage = new QGraphicsPixmapItem();
    m_currentOrderProductImage->setPos(10, m_currentOrderTextItem->pos().y() + m_currentOrderTextItem->boundingRect().height() + 5); // 初始预估位置
    m_currentOrderProductImage->setZValue(100);
    m_currentOrderProductImage->setVisible(false);
    addItem(m_currentOrderProductImage);

    // << 新增：订单倒计时文本显示 >>
    m_orderTimerTextItem = new QGraphicsTextItem();
    m_orderTimerTextItem->setPlainText(QString("时间: --"));
    m_orderTimerTextItem->setDefaultTextColor(Qt::cyan); // 使用不同颜色
    QFont timerFont("Arial", 14, QFont::Bold);
    m_orderTimerTextItem->setFont(timerFont);
    // 将其放置在订单图片下方或旁边
    m_orderTimerTextItem->setPos(10, m_currentOrderTextItem->pos().y() + m_currentOrderTextItem->boundingRect().height() + 5 + 50 + 5); // 预估位置，在图片下方
    m_orderTimerTextItem->setZValue(100);
    addItem(m_orderTimerTextItem);
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
        if (orderProductImageKey.isEmpty() || currentOrderFullText.contains("所有订单完成!") || currentOrderFullText.contains("订单超时:")) {
            m_currentOrderProductImage->setPixmap(QPixmap());
            m_currentOrderProductImage->setVisible(false);
        } else {
            QString imagePath = Player::getImagePathForItem(orderProductImageKey);
            if (!imagePath.isEmpty()) {
                QPixmap orderPixmap(imagePath);
                if (!orderPixmap.isNull()) {
                    QSize imageDisplaySize(50, 50);
                    m_currentOrderProductImage->setPixmap(orderPixmap.scaled(imageDisplaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    if (m_currentOrderTextItem) {
                        m_currentOrderProductImage->setPos(
                            m_currentOrderTextItem->pos().x() + m_currentOrderTextItem->boundingRect().width() + 10,
                            m_currentOrderTextItem->pos().y()
                            );
                    }
                    m_currentOrderProductImage->setVisible(true);
                } else {
                    m_currentOrderProductImage->setPixmap(QPixmap());
                    m_currentOrderProductImage->setVisible(false);
                }
            } else {
                m_currentOrderProductImage->setPixmap(QPixmap());
                m_currentOrderProductImage->setVisible(false);
            }
        }
        // 调整倒计时文本的位置，使其在订单图片和订单文本下方
        if (m_orderTimerTextItem && m_currentOrderTextItem) {
            qreal yPos = m_currentOrderTextItem->pos().y() + m_currentOrderTextItem->boundingRect().height() + 5;
            if (m_currentOrderProductImage->isVisible()) {
                yPos = qMax(yPos, m_currentOrderProductImage->pos().y() + m_currentOrderProductImage->boundingRect().height() + 5);
            } else if (m_currentOrderTextItem->toPlainText().simplified().isEmpty() || m_currentOrderTextItem->toPlainText().contains("所有订单完成!")) {
                m_orderTimerTextItem->setVisible(false);
            }
            m_orderTimerTextItem->setPos(10, yPos);
            if (!m_orderTimerTextItem->isVisible() && !currentOrderFullText.contains("所有订单完成!") && !currentOrderFullText.contains("订单超时:") && !currentOrderFullText.simplified().isEmpty()){
                m_orderTimerTextItem->setVisible(true);
            }
        }
    }
}

// 更新订单倒计时显示的方法实现
void GameScene::updateOrderTimerDisplay(int secondsLeft)
{
    if (m_orderTimerTextItem) {
        if (secondsLeft < 0) { // 特殊值表示计时结束或未开始
            m_orderTimerTextItem->setPlainText(QString("时间: --"));
            m_orderTimerTextItem->setDefaultTextColor(Qt::cyan);
        } else if (secondsLeft == 0) {
            m_orderTimerTextItem->setPlainText(QString("时间到!"));
            m_orderTimerTextItem->setDefaultTextColor(Qt::red);
        } else {
            m_orderTimerTextItem->setPlainText(QString("剩余时间: %1s").arg(secondsLeft));
            if (secondsLeft <= 5) { // 最后5秒变红
                m_orderTimerTextItem->setDefaultTextColor(Qt::red);
            } else if (secondsLeft <= 10) { // 最后10秒变橙色
                m_orderTimerTextItem->setDefaultTextColor(QColor(255, 165, 0)); // Orange
            }
            else {
                m_orderTimerTextItem->setDefaultTextColor(Qt::cyan);
            }
        }
    }
}
