#include "choppingstationitem.h"
#include <QPixmap>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QBrush>
#include <QPen>
#include "item_definitions.h"

// 进度条更新频率 (毫秒)
const int PROGRESS_UPDATE_INTERVAL_MS = 50;
// 进度条的尺寸和位置参数
const qreal PROGRESS_BAR_HEIGHT = 8.0;
const qreal PROGRESS_BAR_Y_OFFSET = 5.0;
const qreal PROGRESS_BAR_WIDTH_PERCENTAGE = 0.8;


ChoppingStationItem::ChoppingStationItem(const QPixmap& pixmap, QGraphicsItem *parent)
    : QObject(), QGraphicsPixmapItem(pixmap, parent),
    m_visualVegetableOnBoard(nullptr), m_isCut(false), m_isChopping(false),
    m_progressBarBackground(nullptr), m_progressBarForeground(nullptr),
    m_progressUpdateTimer(nullptr), m_totalChoppingTimeMs(0), m_elapsedChoppingTimeMs(0)
{
    this->setData(ITEM_TYPE_ROLE, TYPE_CHOPPING_STATION);
    this->setData(ITEM_NAME_ROLE, "切菜板");
    qDebug() << "ChoppingStationItem created at pos:" << this->pos();

    m_choppingTimer = new QTimer(this);
    m_choppingTimer->setSingleShot(true);
    connect(m_choppingTimer, &QTimer::timeout, this, &ChoppingStationItem::onChoppingTimerTimeout);

    m_progressUpdateTimer = new QTimer(this);
    connect(m_progressUpdateTimer, &QTimer::timeout, this, &ChoppingStationItem::updateChoppingProgress);

    setupProgressBar();
}

ChoppingStationItem::~ChoppingStationItem() {
    qDebug() << "ChoppingStationItem destroyed.";
    if (m_choppingTimer->isActive()) {
        m_choppingTimer->stop();
    }
    if (m_progressUpdateTimer && m_progressUpdateTimer->isActive()) {
        m_progressUpdateTimer->stop();
    }
}

void ChoppingStationItem::setupProgressBar() {
    qreal stationWidth = this->boundingRect().width();
    qreal stationHeight = this->boundingRect().height();

    qreal progressBarWidth = stationWidth * PROGRESS_BAR_WIDTH_PERCENTAGE;
    qreal progressBarX = (stationWidth - progressBarWidth) / 2.0;
    qreal progressBarY = stationHeight + PROGRESS_BAR_Y_OFFSET;

    m_progressBarBackground = new QGraphicsRectItem(progressBarX, progressBarY, progressBarWidth, PROGRESS_BAR_HEIGHT, this);
    m_progressBarBackground->setBrush(QBrush(Qt::darkGray));
    m_progressBarBackground->setPen(Qt::NoPen);
    m_progressBarBackground->setVisible(false);

    m_progressBarForeground = new QGraphicsRectItem(progressBarX, progressBarY, 0, PROGRESS_BAR_HEIGHT, this);
    m_progressBarForeground->setBrush(QBrush(Qt::green));
    m_progressBarForeground->setPen(Qt::NoPen);
    m_progressBarForeground->setVisible(false);
}

void ChoppingStationItem::resetProgressBar() {
    qDebug() << "ChoppingStationItem::resetProgressBar called.";
    if (m_progressUpdateTimer->isActive()) {
        m_progressUpdateTimer->stop();
    }
    if (m_progressBarBackground) {
        m_progressBarBackground->setVisible(false);
    }
    if (m_progressBarForeground) {
        m_progressBarForeground->setRect(m_progressBarForeground->rect().x(), m_progressBarForeground->rect().y(), 0, PROGRESS_BAR_HEIGHT);
        m_progressBarForeground->setVisible(false);
    }
    m_elapsedChoppingTimeMs = 0;
    m_totalChoppingTimeMs = 0;
}

void ChoppingStationItem::showProgressBar() {
    qDebug() << "ChoppingStationItem::showProgressBar called.";
    if (m_progressBarBackground && m_progressBarForeground) {
        m_progressBarBackground->setVisible(true);
        m_progressBarForeground->setRect(m_progressBarForeground->rect().x(), m_progressBarForeground->rect().y(), 0, PROGRESS_BAR_HEIGHT);
        m_progressBarForeground->setVisible(true);
        m_progressUpdateTimer->start(PROGRESS_UPDATE_INTERVAL_MS);
    } else {
        qWarning() << "ChoppingStationItem::showProgressBar - Progress bar items not initialized!";
    }
}


bool ChoppingStationItem::placeVegetable(const QString& vegetableName, const QString& vegetableImagePath)
{
    qDebug() << "ChoppingStationItem::placeVegetable called with item:" << vegetableName << "Path:" << vegetableImagePath
             << "Current state: isEmpty=" << isEmpty() << "isChopping=" << m_isChopping;
    if (!isEmpty() || m_isChopping) {
        qDebug() << "切菜板忙碌或已有物品：" << getCurrentVisualNameOnBoard() << "Cannot place.";
        return false;
    }
    resetProgressBar();

    QPixmap vegPixmap(vegetableImagePath);
    if (vegPixmap.isNull()) {
        qWarning() << "ChoppingStationItem Error: 无法为 '"<< vegetableName <<"' 加载图片: " << vegetableImagePath;
        return false;
    }
    if(m_visualVegetableOnBoard) {
        qDebug() << "ChoppingStationItem::placeVegetable - Deleting old visual item.";
        delete m_visualVegetableOnBoard;
        m_visualVegetableOnBoard = nullptr;
    }
    m_visualVegetableOnBoard = new QGraphicsPixmapItem(vegPixmap.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation), this);
    qreal boardWidth = this->boundingRect().width(); qreal boardHeight = this->boundingRect().height();
    qreal vegWidth = m_visualVegetableOnBoard->boundingRect().width(); qreal vegHeight = m_visualVegetableOnBoard->boundingRect().height();
    m_visualVegetableOnBoard->setPos( (boardWidth - vegWidth) / 2.0, (boardHeight - vegHeight) / 2.0 );
    m_placedVegetableName = vegetableName;
    m_placedVegetableImagePath = vegetableImagePath;
    m_isCut = false; // 新放置的物品肯定是未切的
    qDebug() << "'" << vegetableName << "' 已放置在切菜板上 (未切)。Returning true.";
    return true;
}

QString ChoppingStationItem::getPlacedVegetableName() const {
    return m_placedVegetableName;
}

QString ChoppingStationItem::getCurrentVisualNameOnBoard() const {
    if (m_placedVegetableName.isEmpty()) return QString();
    if (m_isChopping) return m_placedVegetableName + " (正在切...)";
    return m_placedVegetableName + (m_isCut ? "_已切" : "");
}

QString ChoppingStationItem::takeVegetable() {
    qDebug() << "ChoppingStationItem::takeVegetable called. isChopping=" << m_isChopping;
    if(m_isChopping) {
        qDebug() << "正在切菜，无法取走！将中断切菜。";
        m_choppingTimer->stop();
        resetProgressBar();
        m_isChopping = false;
    } else {
        resetProgressBar();
    }

    QString itemNameTaken = getCurrentVisualNameOnBoard();
    if (m_isChopping && !m_placedVegetableName.isEmpty()) { // 如果是因为中断而取走, 应该返回原始未切的
        itemNameTaken = m_placedVegetableName;
    }


    if (m_visualVegetableOnBoard) {
        qDebug() << "ChoppingStationItem::takeVegetable - Deleting visual item.";
        delete m_visualVegetableOnBoard;
        m_visualVegetableOnBoard = nullptr;
    }
    QString originalPlacedName = m_placedVegetableName; // 用于debug
    m_placedVegetableName.clear();
    m_placedVegetableImagePath.clear();
    bool wasCut = m_isCut;
    m_isCut = false;


    qDebug() << "从切菜板上取走了：" << itemNameTaken << "(Original on board was:" << originalPlacedName << ", wasCut:" << wasCut << ")";

    return itemNameTaken;
}

bool ChoppingStationItem::isEmpty() const {
    return m_placedVegetableName.isEmpty();
}
bool ChoppingStationItem::isCut() const {
    return m_isCut;
}
bool ChoppingStationItem::isChopping() const {
    return m_isChopping;
}

bool ChoppingStationItem::startChoppingProcess(const QString& rawVegNameCurrentlyOnBoard, const QString& cutVegImagePath, int durationMs) {
    qDebug() << "ChoppingStationItem::startChoppingProcess called for:" << rawVegNameCurrentlyOnBoard
             << "Target cut image:" << cutVegImagePath << "Duration:" << durationMs
             << "Current state: isEmpty=" << isEmpty() << "isCut=" << m_isCut << "isChopping=" << m_isChopping
             << "Item on board:" << m_placedVegetableName;

    if (isEmpty() || m_isCut || m_isChopping) {
        qDebug() << "无法开始切菜：砧板为空、已切或已在切菜。当前：" << getCurrentVisualNameOnBoard() << "Returning false.";
        return false;
    }
    if (m_placedVegetableName != rawVegNameCurrentlyOnBoard) {
        qWarning() << "ChoppingStationItem Logic Error: 尝试切的蔬菜与砧板上的不符！砧板：" << m_placedVegetableName << "，尝试切：" << rawVegNameCurrentlyOnBoard << "Returning false.";
        return false;
    }

    m_isChopping = true;
    m_isCut = false;
    m_pendingCutImagePath = cutVegImagePath;
    m_totalChoppingTimeMs = durationMs;
    m_elapsedChoppingTimeMs = 0;

    m_choppingTimer->start(durationMs);
    showProgressBar();

    qDebug() << "开始切 '" << m_placedVegetableName << "'，持续 " << durationMs << "ms. Returning true.";
    return true;
}


void ChoppingStationItem::updateChoppingProgress() {
    // qDebug() << "ChoppingStationItem::updateChoppingProgress called. Elapsed:" << m_elapsedChoppingTimeMs << "/" << m_totalChoppingTimeMs; // 会产生大量输出
    if (!m_isChopping || !m_progressBarForeground || !m_progressBarBackground) {
        resetProgressBar();
        return;
    }

    m_elapsedChoppingTimeMs += PROGRESS_UPDATE_INTERVAL_MS;
    if (m_elapsedChoppingTimeMs > m_totalChoppingTimeMs) {
        m_elapsedChoppingTimeMs = m_totalChoppingTimeMs;
    }

    float progressRatio = 0.0f;
    if (m_totalChoppingTimeMs > 0) {
        progressRatio = static_cast<float>(m_elapsedChoppingTimeMs) / m_totalChoppingTimeMs;
    }
    progressRatio = qBound(0.0f, progressRatio, 1.0f);

    qreal fullWidth = m_progressBarBackground->rect().width();
    m_progressBarForeground->setRect(
        m_progressBarBackground->rect().x(),
        m_progressBarBackground->rect().y(),
        fullWidth * progressRatio,
        PROGRESS_BAR_HEIGHT
        );

    if (m_elapsedChoppingTimeMs >= m_totalChoppingTimeMs) {
        if (m_progressUpdateTimer->isActive()) {
            m_progressUpdateTimer->stop();
        }
    }
}


void ChoppingStationItem::onChoppingTimerTimeout() {
    qDebug() << "ChoppingStationItem::onChoppingTimerTimeout called. Item was:" << m_placedVegetableName << "isChopping was:" << m_isChopping;
    resetProgressBar();

    if (!m_isChopping || m_placedVegetableName.isEmpty()) {
        qDebug() << "Chopping timeout, but not in chopping state or no item. Placed:" << m_placedVegetableName << "isChopping:" << m_isChopping;
        m_isChopping = false;
        return;
    }

    QPixmap cutPixmap(m_pendingCutImagePath);
    if (cutPixmap.isNull()) {
        qWarning() << "ChoppingStationItem Error: 定时器到，但无法加载已切蔬菜图片 '"
                   << m_placedVegetableName << "': " << m_pendingCutImagePath;
        m_isChopping = false;
        return;
    }

    if (m_visualVegetableOnBoard) {
        m_visualVegetableOnBoard->setPixmap(cutPixmap.scaled(35, 35, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        qreal boardWidth = this->boundingRect().width(); qreal boardHeight = this->boundingRect().height();
        qreal vegWidth = m_visualVegetableOnBoard->boundingRect().width(); qreal vegHeight = m_visualVegetableOnBoard->boundingRect().height();
        m_visualVegetableOnBoard->setPos( (boardWidth - vegWidth) / 2.0, (boardHeight - vegHeight) / 2.0 );
    }

    m_isCut = true;
    m_isChopping = false;
    QString originalName = m_placedVegetableName;

    qDebug() << "'" << originalName << "' 已在砧板上切好！现在是 '" << getCurrentVisualNameOnBoard() << "'. Emitting choppingCompleted.";

    emit choppingCompleted(originalName, m_pendingCutImagePath);
}
