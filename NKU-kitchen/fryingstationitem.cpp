#include "fryingstationitem.h"
#include <QPixmap>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QBrush>
#include <QPen>
#include "item_definitions.h"
// 进度条更新频率 (毫秒)
const int FRYING_PROGRESS_UPDATE_INTERVAL_MS = 50;
// 进度条的尺寸和位置参数
const qreal FRYING_PROGRESS_BAR_HEIGHT = 8.0;
const qreal FRYING_PROGRESS_BAR_Y_OFFSET = 5.0;
const qreal FRYING_PROGRESS_BAR_WIDTH_PERCENTAGE = 0.8;


FryingStationItem::FryingStationItem(const QPixmap& pixmap, QGraphicsItem *parent)
    : QObject(), QGraphicsPixmapItem(pixmap, parent),
    m_visualItemOnBoard(nullptr), m_isFried(false), m_isFrying(false),
    m_progressBarBackground(nullptr), m_progressBarForeground(nullptr),
    m_progressUpdateTimer(nullptr), m_totalFryingTimeMs(0), m_elapsedFryingTimeMs(0)
{
    this->setData(ITEM_TYPE_ROLE, TYPE_FRYING_STATION);
    this->setData(ITEM_NAME_ROLE, "煎锅");
    qDebug() << "FryingStationItem created at pos:" << this->pos();


    m_fryingTimer = new QTimer(this);
    m_fryingTimer->setSingleShot(true);
    connect(m_fryingTimer, &QTimer::timeout, this, &FryingStationItem::onFryingTimerTimeout);

    m_progressUpdateTimer = new QTimer(this);
    connect(m_progressUpdateTimer, &QTimer::timeout, this, &FryingStationItem::updateFryingProgress);

    setupProgressBar();
}

FryingStationItem::~FryingStationItem() {
    qDebug() << "FryingStationItem destroyed.";
    if (m_fryingTimer->isActive()) {
        m_fryingTimer->stop();
    }
    if (m_progressUpdateTimer && m_progressUpdateTimer->isActive()) {
        m_progressUpdateTimer->stop();
    }
}

void FryingStationItem::setupProgressBar() {
    qreal stationWidth = this->boundingRect().width();
    qreal stationHeight = this->boundingRect().height();

    qreal progressBarWidth = stationWidth * FRYING_PROGRESS_BAR_WIDTH_PERCENTAGE;
    qreal progressBarX = (stationWidth - progressBarWidth) / 2.0;
    qreal progressBarY = stationHeight + FRYING_PROGRESS_BAR_Y_OFFSET;

    m_progressBarBackground = new QGraphicsRectItem(progressBarX, progressBarY, progressBarWidth, FRYING_PROGRESS_BAR_HEIGHT, this);
    m_progressBarBackground->setBrush(QBrush(Qt::darkGray));
    m_progressBarBackground->setPen(Qt::NoPen);
    m_progressBarBackground->setVisible(false);

    m_progressBarForeground = new QGraphicsRectItem(progressBarX, progressBarY, 0, FRYING_PROGRESS_BAR_HEIGHT, this);
    m_progressBarForeground->setBrush(QBrush(Qt::red)); // 煎炸用红色进度条
    m_progressBarForeground->setPen(Qt::NoPen);
    m_progressBarForeground->setVisible(false);
}

void FryingStationItem::resetProgressBar() {
    qDebug() << "FryingStationItem::resetProgressBar called.";
    if (m_progressUpdateTimer && m_progressUpdateTimer->isActive()) {
        m_progressUpdateTimer->stop();
    }
    if (m_progressBarBackground) m_progressBarBackground->setVisible(false);
    if (m_progressBarForeground) {
        m_progressBarForeground->setRect(m_progressBarForeground->rect().x(), m_progressBarForeground->rect().y(), 0, FRYING_PROGRESS_BAR_HEIGHT);
        m_progressBarForeground->setVisible(false);
    }
    m_elapsedFryingTimeMs = 0;
    m_totalFryingTimeMs = 0;
}

void FryingStationItem::showProgressBar() {
    qDebug() << "FryingStationItem::showProgressBar called.";
    if (m_progressBarBackground && m_progressBarForeground) {
        m_progressBarBackground->setVisible(true);
        m_progressBarForeground->setRect(m_progressBarForeground->rect().x(), m_progressBarForeground->rect().y(), 0, FRYING_PROGRESS_BAR_HEIGHT);
        m_progressBarForeground->setVisible(true);
        if (m_progressUpdateTimer) {
            m_progressUpdateTimer->start(FRYING_PROGRESS_UPDATE_INTERVAL_MS);
        } else {
            qWarning() << "FryingStationItem::showProgressBar - m_progressUpdateTimer is null!";
        }
    } else {
        qWarning() << "FryingStationItem::showProgressBar - Progress bar items not initialized!";
    }
}

bool FryingStationItem::placeItem(const QString& itemName, const QString& itemImagePath)
{
    qDebug() << "FryingStationItem::placeItem called with item:" << itemName << "Path:" << itemImagePath
             << "Current state: isEmpty=" << isEmpty() << "isFrying=" << m_isFrying;
    if (!isEmpty() || m_isFrying) {
        qDebug() << "煎锅忙碌或已有物品：" << getCurrentVisualNameOnBoard() << "Cannot place.";
        return false;
    }
    if (itemName != CUT_MEAT) {
        qDebug() << "煎锅只接受已切的肉 (" << CUT_MEAT << ")，收到：" << itemName << ". Cannot place.";
        return false;
    }

    resetProgressBar();

    QPixmap itemPixmap(itemImagePath);
    if (itemPixmap.isNull()) {
        qWarning() << "FryingStationItem Error: 无法为 '"<< itemName <<"' 加载图片: " << itemImagePath;
        return false;
    }
    if(m_visualItemOnBoard) {
        qDebug() << "FryingStationItem::placeItem - Deleting old visual item.";
        delete m_visualItemOnBoard;
        m_visualItemOnBoard = nullptr;
    }
    m_visualItemOnBoard = new QGraphicsPixmapItem(itemPixmap.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation), this);
    qreal stationWidth = this->boundingRect().width(); qreal stationHeight = this->boundingRect().height();
    qreal itemVisWidth = m_visualItemOnBoard->boundingRect().width(); qreal itemVisHeight = m_visualItemOnBoard->boundingRect().height();
    m_visualItemOnBoard->setPos( (stationWidth - itemVisWidth) / 2.0, (stationHeight - itemVisHeight) / 2.0 - 5 );
    m_placedItemName = itemName;
    m_placedItemImagePath = itemImagePath;
    m_isFried = false; // 新放置的肯定是未煎的
    qDebug() << "'" << itemName << "' 已放置在煎锅上 (未煎)。Returning true.";
    return true;
}

QString FryingStationItem::getPlacedItemName() const { return m_placedItemName; }

QString FryingStationItem::getCurrentVisualNameOnBoard() const {
    if (m_placedItemName.isEmpty()) return QString();
    if (m_isFrying) return m_placedItemName + " (正在煎...)";
    // m_placedItemName 在煎好后会变成 FRIED_MEAT，所以这里不需要再加 _已煎
    return m_placedItemName; // 如果是 FRIED_MEAT，直接返回；如果是 CUT_MEAT (未开始煎或中断)，也直接返回
}

QString FryingStationItem::takeItem() {
    qDebug() << "FryingStationItem::takeItem called. isFrying=" << m_isFrying << "isFried=" << m_isFried << "Item on board:" << m_placedItemName;
    QString itemNameTaken;

    if(m_isFrying) {
        qDebug() << "正在煎制，无法取走！将中断煎制。";
        m_fryingTimer->stop();
        resetProgressBar();
        m_isFrying = false;
        // 物品还是未煎好的状态 (CUT_MEAT)
        itemNameTaken = m_placedItemName;
    } else {
        resetProgressBar();
        itemNameTaken = m_placedItemName;
    }

    if (m_visualItemOnBoard) {
        qDebug() << "FryingStationItem::takeItem - Deleting visual item.";
        delete m_visualItemOnBoard;
        m_visualItemOnBoard = nullptr;
    }
    m_placedItemName.clear();
    m_placedItemImagePath.clear();
    m_isFried = false;
    m_isFrying = false;

    qDebug() << "从煎锅上取走了：" << itemNameTaken;
    return itemNameTaken;
}

bool FryingStationItem::isEmpty() const { return m_placedItemName.isEmpty(); }
bool FryingStationItem::isFried() const { return m_isFried; }
bool FryingStationItem::isFrying() const { return m_isFrying; }

bool FryingStationItem::startFryingProcess(const QString& cutMeatNameOnBoard, const QString& friedMeatImagePath, int durationMs) {
    qDebug() << "FryingStationItem::startFryingProcess called for:" << cutMeatNameOnBoard
             << "Target fried image:" << friedMeatImagePath << "Duration:" << durationMs
             << "Current state: isEmpty=" << isEmpty() << "isFried=" << m_isFried << "isFrying=" << m_isFrying
             << "Item on board:" << m_placedItemName;

    if (isEmpty() || m_isFried || m_isFrying) {
        qDebug() << "无法开始煎制：煎锅为空、已煎好或已在煎制。当前：" << getCurrentVisualNameOnBoard() << "Returning false.";
        return false;
    }
    if (m_placedItemName != CUT_MEAT || m_placedItemName != cutMeatNameOnBoard) { // 确保板上确实是未煎的 CUT_MEAT
        qWarning() << "FryingStationItem Logic Error: 尝试煎的物品与煎锅上的不符或不是已切的肉！煎锅："
                   << m_placedItemName << "，尝试煎：" << cutMeatNameOnBoard << "Returning false.";
        return false;
    }

    m_isFrying = true;
    m_isFried = false;
    m_pendingFriedItemImagePath = friedMeatImagePath;
    m_totalFryingTimeMs = durationMs;
    m_elapsedFryingTimeMs = 0;

    m_fryingTimer->start(durationMs);
    showProgressBar();

    qDebug() << "开始煎 '" << m_placedItemName << "'，持续 " << durationMs << "ms. Returning true.";
    if(m_visualItemOnBoard) { m_visualItemOnBoard->setOpacity(0.7); }
    return true;
}

void FryingStationItem::updateFryingProgress() {

    if (!m_isFrying || !m_progressBarForeground || !m_progressBarBackground) {
        resetProgressBar();
        return;
    }

    m_elapsedFryingTimeMs += FRYING_PROGRESS_UPDATE_INTERVAL_MS;
    if (m_elapsedFryingTimeMs > m_totalFryingTimeMs) {
        m_elapsedFryingTimeMs = m_totalFryingTimeMs;
    }

    float progressRatio = (m_totalFryingTimeMs > 0) ? (static_cast<float>(m_elapsedFryingTimeMs) / m_totalFryingTimeMs) : 0.0f;
    progressRatio = qBound(0.0f, progressRatio, 1.0f);

    qreal fullWidth = m_progressBarBackground->rect().width();
    m_progressBarForeground->setRect(
        m_progressBarBackground->rect().x(),
        m_progressBarBackground->rect().y(),
        fullWidth * progressRatio,
        FRYING_PROGRESS_BAR_HEIGHT
        );

    if (m_elapsedFryingTimeMs >= m_totalFryingTimeMs) {
        if (m_progressUpdateTimer && m_progressUpdateTimer->isActive()) { // 检查指针有效性
            m_progressUpdateTimer->stop();
        }
    }
}

void FryingStationItem::onFryingTimerTimeout() {
    qDebug() << "FryingStationItem::onFryingTimerTimeout called. Item was:" << m_placedItemName << "isFrying was:" << m_isFrying;
    resetProgressBar();

    if (!m_isFrying || m_placedItemName.isEmpty()) {
        qDebug() << "Frying timeout, but not in frying state or no item. Placed:" << m_placedItemName << "isFrying:" << m_isFrying;
        m_isFrying = false;
        if(m_visualItemOnBoard) { m_visualItemOnBoard->setOpacity(1.0); }
        return;
    }

    QPixmap friedPixmap(m_pendingFriedItemImagePath);
    if (friedPixmap.isNull()) {
        qWarning() << "FryingStationItem Error: 定时器到，但无法加载已煎好的肉图片 '"
                   << m_placedItemName << "': " << m_pendingFriedItemImagePath;
        m_isFrying = false;
        if(m_visualItemOnBoard) { m_visualItemOnBoard->setOpacity(1.0); }
        return;
    }

    QString originalName = m_placedItemName;

    if (m_visualItemOnBoard) {
        m_visualItemOnBoard->setPixmap(friedPixmap.scaled(45, 45, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_visualItemOnBoard->setOpacity(1.0);
        qreal stationWidth = this->boundingRect().width(); qreal stationHeight = this->boundingRect().height();
        qreal itemVisWidth = m_visualItemOnBoard->boundingRect().width(); qreal itemVisHeight = m_visualItemOnBoard->boundingRect().height();
        m_visualItemOnBoard->setPos( (stationWidth - itemVisWidth) / 2.0, (stationHeight - itemVisHeight) / 2.0 - 5 );
    }

    m_isFried = true;
    m_isFrying = false;
    m_placedItemName = FRIED_MEAT; // 更新板上的物品名为已煎状态
    m_placedItemImagePath = m_pendingFriedItemImagePath;

    qDebug() << "'" << originalName << "' 已在煎锅上煎好！现在是 '" << getCurrentVisualNameOnBoard() << "'. Emitting fryingCompleted.";

    emit fryingCompleted(originalName, m_pendingFriedItemImagePath);
}
