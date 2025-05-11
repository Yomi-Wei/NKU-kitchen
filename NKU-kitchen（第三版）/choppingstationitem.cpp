#include "choppingstationitem.h"
#include <QPixmap>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsRectItem> // 用于进度条
#include <QBrush>            // 用于设置颜色
#include <QPen>              // 用于设置边框

// 进度条更新频率 (毫秒)
const int PROGRESS_UPDATE_INTERVAL_MS = 50;
// 进度条的尺寸和位置参数
const qreal PROGRESS_BAR_HEIGHT = 8.0;
const qreal PROGRESS_BAR_Y_OFFSET = 5.0; // 进度条在砧板图像下方的偏移量
const qreal PROGRESS_BAR_WIDTH_PERCENTAGE = 0.8; // 进度条宽度占砧板宽度的百分比


ChoppingStationItem::ChoppingStationItem(const QPixmap& pixmap, QGraphicsItem *parent)
    : QObject(), QGraphicsPixmapItem(pixmap, parent),
    m_visualVegetableOnBoard(nullptr), m_isCut(false), m_isChopping(false),
    m_progressBarBackground(nullptr), m_progressBarForeground(nullptr), // 初始化进度条指针
    m_progressUpdateTimer(nullptr), m_totalChoppingTimeMs(0), m_elapsedChoppingTimeMs(0)
{
    this->setData(ITEM_TYPE_ROLE, TYPE_CHOPPING_STATION);
    this->setData(ITEM_NAME_ROLE, "切菜板");

    m_choppingTimer = new QTimer(this);
    m_choppingTimer->setSingleShot(true);
    connect(m_choppingTimer, &QTimer::timeout, this, &ChoppingStationItem::onChoppingTimerTimeout);

    // 初始化进度条更新定时器
    m_progressUpdateTimer = new QTimer(this);
    connect(m_progressUpdateTimer, &QTimer::timeout, this, &ChoppingStationItem::updateChoppingProgress);

    setupProgressBar(); // 创建进度条图形项
}

ChoppingStationItem::~ChoppingStationItem() {
    if (m_choppingTimer->isActive()) {
        m_choppingTimer->stop();
    }
    if (m_progressUpdateTimer && m_progressUpdateTimer->isActive()) {
        m_progressUpdateTimer->stop();
    }
    // QGraphicsItem 的子项 (m_progressBarBackground, m_progressBarForeground, m_visualVegetableOnBoard)
    // 会被其父项(this)或场景自动管理和删除，通常不需要手动 delete。
    // delete m_progressUpdateTimer; // QTimer作为QObject子对象也会被自动管理
}

void ChoppingStationItem::setupProgressBar() {
    qreal stationWidth = this->boundingRect().width();
    qreal stationHeight = this->boundingRect().height();

    qreal progressBarWidth = stationWidth * PROGRESS_BAR_WIDTH_PERCENTAGE;
    qreal progressBarX = (stationWidth - progressBarWidth) / 2.0; // 居中
    qreal progressBarY = stationHeight + PROGRESS_BAR_Y_OFFSET;   // 砧板下方

    // 创建进度条背景
    m_progressBarBackground = new QGraphicsRectItem(progressBarX, progressBarY, progressBarWidth, PROGRESS_BAR_HEIGHT, this);
    m_progressBarBackground->setBrush(QBrush(Qt::darkGray));
    m_progressBarBackground->setPen(Qt::NoPen);
    m_progressBarBackground->setVisible(false); // 初始隐藏

    // 创建进度条前景
    m_progressBarForeground = new QGraphicsRectItem(progressBarX, progressBarY, 0, PROGRESS_BAR_HEIGHT, this); // 初始宽度为0
    m_progressBarForeground->setBrush(QBrush(Qt::green));
    m_progressBarForeground->setPen(Qt::NoPen);
    m_progressBarForeground->setVisible(false); // 初始隐藏
}

void ChoppingStationItem::resetProgressBar() {
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
    if (m_progressBarBackground && m_progressBarForeground) {
        m_progressBarBackground->setVisible(true);
        m_progressBarForeground->setRect(m_progressBarForeground->rect().x(), m_progressBarForeground->rect().y(), 0, PROGRESS_BAR_HEIGHT);
        m_progressBarForeground->setVisible(true);
        m_progressUpdateTimer->start(PROGRESS_UPDATE_INTERVAL_MS);
    }
}


bool ChoppingStationItem::placeVegetable(const QString& vegetableName, const QString& vegetableImagePath)
{
    if (!isEmpty() || m_isChopping) {
        qDebug() << "切菜板忙碌或已有物品：" << getCurrentVisualNameOnBoard();
        return false;
    }
    resetProgressBar(); // 如果之前有中断的进度条，清除掉

    QPixmap vegPixmap(vegetableImagePath);
    if (vegPixmap.isNull()) {
        qWarning() << "ChoppingStationItem Error: 无法为 '"<< vegetableName <<"' 加载图片: " << vegetableImagePath;
        return false;
    }
    if(m_visualVegetableOnBoard) { delete m_visualVegetableOnBoard; m_visualVegetableOnBoard = nullptr;}
    m_visualVegetableOnBoard = new QGraphicsPixmapItem(vegPixmap.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation), this);
    qreal boardWidth = this->boundingRect().width(); qreal boardHeight = this->boundingRect().height();
    qreal vegWidth = m_visualVegetableOnBoard->boundingRect().width(); qreal vegHeight = m_visualVegetableOnBoard->boundingRect().height();
    m_visualVegetableOnBoard->setPos( (boardWidth - vegWidth) / 2.0, (boardHeight - vegHeight) / 2.0 );
    m_placedVegetableName = vegetableName;
    m_placedVegetableImagePath = vegetableImagePath;
    m_isCut = false;
    qDebug() << "'" << vegetableName << "' 已放置在切菜板上 (未切)。";
    return true;
}

QString ChoppingStationItem::getPlacedVegetableName() const { return m_placedVegetableName; }

QString ChoppingStationItem::getCurrentVisualNameOnBoard() const {
    if (m_placedVegetableName.isEmpty()) return QString();
    if (m_isChopping) return m_placedVegetableName + " (正在切...)";
    return m_placedVegetableName + (m_isCut ? "_已切" : "");
}

QString ChoppingStationItem::takeVegetable() {
    if(m_isChopping) {
        qDebug() << "正在切菜，无法取走！将中断切菜。";
        m_choppingTimer->stop(); // 停止主计时器
        resetProgressBar();      // 重置并隐藏进度条
        m_isChopping = false;    // 更新状态
        // 蔬菜还是未切好的状态
    } else {
        resetProgressBar(); // 如果不是在切菜，也确保进度条被清除
    }

    QString itemNameTaken = getCurrentVisualNameOnBoard(); // 获取当前状态的物品名称
    if (m_isChopping) { // 如果是因为中断而取走
        itemNameTaken = m_placedVegetableName; // 应该返回原始未切的蔬菜名
    }


    if (m_visualVegetableOnBoard) { delete m_visualVegetableOnBoard; m_visualVegetableOnBoard = nullptr; }
    m_placedVegetableName.clear(); m_placedVegetableImagePath.clear();
    bool wasCut = m_isCut; // 保存一下是否已切的状态
    m_isCut = false;       // 重置已切状态
    // m_isChopping 已经在上面处理了

    qDebug() << "从切菜板上取走了：" << itemNameTaken;
    return wasCut ? (itemNameTaken) : (itemNameTaken.remove("_已切")); // 确保返回正确的名称
}

bool ChoppingStationItem::isEmpty() const { return m_placedVegetableName.isEmpty(); }
bool ChoppingStationItem::isCut() const { return m_isCut; }
bool ChoppingStationItem::isChopping() const { return m_isChopping; }

bool ChoppingStationItem::startChoppingProcess(const QString& rawVegNameCurrentlyOnBoard, const QString& cutVegImagePath, int durationMs) {
    if (isEmpty() || m_isCut || m_isChopping) {
        qDebug() << "无法开始切菜：砧板为空、已切或已在切菜。当前：" << getCurrentVisualNameOnBoard();
        return false;
    }
    if (m_placedVegetableName != rawVegNameCurrentlyOnBoard) {
        qWarning() << "ChoppingStationItem Logic Error: 尝试切的蔬菜与砧板上的不符！砧板：" << m_placedVegetableName << "，尝试切：" << rawVegNameCurrentlyOnBoard;
        return false;
    }

    m_isChopping = true;
    m_pendingCutImagePath = cutVegImagePath;
    m_totalChoppingTimeMs = durationMs;
    m_elapsedChoppingTimeMs = 0;

    m_choppingTimer->start(durationMs);
    showProgressBar(); // 显示进度条并开始更新

    qDebug() << "开始切 '" << m_placedVegetableName << "'，持续 " << durationMs << "ms。";
    // 可以选择在这里改变蔬菜图片透明度等视觉效果
    // if(m_visualVegetableOnBoard) { m_visualVegetableOnBoard->setOpacity(0.7); }
    return true;
}


void ChoppingStationItem::updateChoppingProgress() {
    if (!m_isChopping || !m_progressBarForeground || !m_progressBarBackground) {
        resetProgressBar(); // 如果状态不对，停止并重置
        return;
    }

    m_elapsedChoppingTimeMs += PROGRESS_UPDATE_INTERVAL_MS;
    if (m_elapsedChoppingTimeMs > m_totalChoppingTimeMs) {
        m_elapsedChoppingTimeMs = m_totalChoppingTimeMs; // 防止超出
    }

    float progressRatio = 0.0f;
    if (m_totalChoppingTimeMs > 0) {
        progressRatio = static_cast<float>(m_elapsedChoppingTimeMs) / m_totalChoppingTimeMs;
    }
    progressRatio = qBound(0.0f, progressRatio, 1.0f); // 确保比例在0到1之间

    qreal fullWidth = m_progressBarBackground->rect().width();
    m_progressBarForeground->setRect(
        m_progressBarBackground->rect().x(),
        m_progressBarBackground->rect().y(),
        fullWidth * progressRatio,
        PROGRESS_BAR_HEIGHT
        );

    if (m_elapsedChoppingTimeMs >= m_totalChoppingTimeMs) {
        // 理论上此时 onChoppingTimerTimeout 应该很快会被调用
        // 但为保险起见，可以停止更新定时器
        if (m_progressUpdateTimer->isActive()) {
            m_progressUpdateTimer->stop();
        }
    }
}


void ChoppingStationItem::onChoppingTimerTimeout() {
    resetProgressBar(); // 停止并隐藏进度条

    if (!m_isChopping || m_placedVegetableName.isEmpty()) {
        m_isChopping = false;
        return;
    }

    QPixmap cutPixmap(m_pendingCutImagePath);
    if (cutPixmap.isNull()) {
        qWarning() << "ChoppingStationItem Error: 定时器到，但无法加载已切蔬菜图片 '"
                   << m_placedVegetableName << "': " << m_pendingCutImagePath;
        m_isChopping = false;
        // if(m_visualVegetableOnBoard) { m_visualVegetableOnBoard->setOpacity(1.0); }
        return;
    }

    if (m_visualVegetableOnBoard) {
        m_visualVegetableOnBoard->setPixmap(cutPixmap.scaled(35, 35, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // m_visualVegetableOnBoard->setOpacity(1.0);
        qreal boardWidth = this->boundingRect().width(); qreal boardHeight = this->boundingRect().height();
        qreal vegWidth = m_visualVegetableOnBoard->boundingRect().width(); qreal vegHeight = m_visualVegetableOnBoard->boundingRect().height();
        m_visualVegetableOnBoard->setPos( (boardWidth - vegWidth) / 2.0, (boardHeight - vegHeight) / 2.0 );
    }

    m_isCut = true;
    m_isChopping = false;
    QString originalName = m_placedVegetableName;
    qDebug() << "'" << originalName << "' 已在砧板上切好！现在是 '" << getCurrentVisualNameOnBoard() << "'";

    emit choppingCompleted(originalName, m_pendingCutImagePath);
}
