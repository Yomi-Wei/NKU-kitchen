#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startpagewidget.h"
#include "characterselectionwidget.h"
#include "gamescene.h"
#include "player.h"
#include "choppingstationitem.h"
#include "saladassemblystationitem.h"
#include "servinghatchitem.h"
#include "fryingstationitem.h"
#include "item_definitions.h"

#include <QKeyEvent>
#include <QDebug>
#include <QGraphicsPolygonItem>
#include <QPolygonF>
#include <QGraphicsPixmapItem>
#include <QColor>
#include <QVBoxLayout>
#include <QList>


// 订单默认时限（毫秒）
const int DEFAULT_ORDER_TIME_LIMIT_MS = 40000;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(nullptr)
    , stackedWidget(nullptr)
    , startPage(nullptr)
    , characterSelectionPage(nullptr)
    , gamePageWidget(nullptr)
    , scene(nullptr)
    , view(nullptr)
    , player(nullptr)
    , m_servingHatch(nullptr)
    , m_score(0)
    , m_currentOrder("")
    , m_currentOrderIndex(0)
    , m_orderTimer(nullptr)
    , m_orderDisplayUpdateTimer(nullptr)
    , m_currentOrderTimeLimitMs(0)
    , m_currentOrderTimeLeftSeconds(0)
    , m_selectedCharacterSpritePath(CHARACTER_A_SPRITE_PATH) // 默认选择角色A
    , m_gameIsRunning(false)
{
    stackedWidget = new QStackedWidget(this);
    startPage = new StartPageWidget(this);
    characterSelectionPage = new CharacterSelectionWidget(this); // 创建角色选择页面实例
    gamePageWidget = new QWidget(this);

    // StartPage 的 "开始游戏" 按钮现在会触发角色选择页面
    connect(startPage, &StartPageWidget::startGameRequested, this, &MainWindow::switchToCharacterSelectionPage);
    // 连接角色选择页面的信号
    connect(characterSelectionPage, &CharacterSelectionWidget::characterSelectedAndStartGame, this, &MainWindow::handleCharacterSelectedAndStartGame);
    connect(characterSelectionPage, &CharacterSelectionWidget::backToStartPageRequested, this, &MainWindow::switchToStartPage);

    stackedWidget->addWidget(startPage);
    stackedWidget->addWidget(characterSelectionPage); // 将角色选择页面添加到堆叠部件
    stackedWidget->addWidget(gamePageWidget);

    setCentralWidget(stackedWidget);
    stackedWidget->setCurrentWidget(startPage); // 默认显示开始页面

    resize(820, 620);
    this->setFocusPolicy(Qt::StrongFocus);
    setWindowTitle(tr("NKU-kitchen"));

    initializeOrderQueue(); // 初始化预设订单

    // 初始化订单计时器
    m_orderTimer = new QTimer(this);
    m_orderTimer->setSingleShot(true);
    connect(m_orderTimer, &QTimer::timeout, this, &MainWindow::onOrderTimerTimeout);

    // 初始化订单显示更新计时器
    m_orderDisplayUpdateTimer = new QTimer(this);
    connect(m_orderDisplayUpdateTimer, &QTimer::timeout, this, &MainWindow::updateOrderCountdownDisplay);
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow 销毁";
}

void MainWindow::initializeOrderQueue() {
    m_orderQueue.clear();
    m_orderQueue << PRODUCT_SALAD;
    m_orderQueue << PRODUCT_BURGER;
    m_orderQueue << PRODUCT_SALAD;
     m_orderQueue << PRODUCT_BURGER;
     m_orderQueue << PRODUCT_BURGER;
     m_orderQueue << PRODUCT_SALAD;
     m_orderQueue << PRODUCT_SALAD;
    qDebug() << "预设订单队列已初始化，包含" << m_orderQueue.size() << "个订单。";
}

// 新增槽函数：切换到角色选择页面
void MainWindow::switchToCharacterSelectionPage() {
    qDebug() << "切换到角色选择页面...";
    stackedWidget->setCurrentWidget(characterSelectionPage);
    this->setFocus();
}

// 新增槽函数：处理角色选择并开始游戏
void MainWindow::handleCharacterSelectedAndStartGame(const QString& characterSpritePath) {
    qDebug() << "角色已选择，精灵路径：" << characterSpritePath << "。开始游戏...";
    m_selectedCharacterSpritePath = characterSpritePath;
    setupGameUI(); // 使用选中的角色设置游戏
    stackedWidget->setCurrentWidget(gamePageWidget);
    this->setFocus();
}

// 新增槽函数：从角色选择返回到开始页面
void MainWindow::switchToStartPage() {
    qDebug() << "返回到开始页面...";
    stackedWidget->setCurrentWidget(startPage);
    this->setFocus();
}

void MainWindow::setupGameUI()
{
    // 清理旧的游戏资源
    if(player) { if(player->scene()) player->scene()->removeItem(player); delete player; player = nullptr; }
    if(m_servingHatch) { if(m_servingHatch->scene()) m_servingHatch->scene()->removeItem(m_servingHatch); delete m_servingHatch; m_servingHatch = nullptr; }
    delete scene; scene = nullptr;

    // 停止可能正在运行的计时器
    if (m_orderTimer && m_orderTimer->isActive()) { m_orderTimer->stop(); }
    if (m_orderDisplayUpdateTimer && m_orderDisplayUpdateTimer->isActive()) { m_orderDisplayUpdateTimer->stop(); }
    m_currentOrderTimeLeftSeconds = 0;

    qDebug() << "正在初始化游戏界面... 使用角色: " << m_selectedCharacterSpritePath;
    scene = new GameScene(this);
    scene->setSceneRect(0, 0, 800, 600);

    //创建游戏元素
    // 创建障碍物
    QPolygonF rightPointingTriangleShape;
    rightPointingTriangleShape << QPointF(0, -55) << QPointF(0, 55) << QPointF(90, 0);
    QGraphicsPolygonItem *rightTriangle = new QGraphicsPolygonItem(rightPointingTriangleShape);
    rightTriangle->setPos(400,335);
    rightTriangle->setBrush(Qt::NoBrush); rightTriangle->setPen(Qt::NoPen);
    rightTriangle->setData(ITEM_TYPE_ROLE, TYPE_OBSTACLE); scene->addItem(rightTriangle);

    QPolygonF leftPointingTriangleShape;
    qreal triangleWidth = 90.0; qreal triangleBaseHeight = 110.0;
    leftPointingTriangleShape << QPointF(0, 0) << QPointF(triangleWidth, -triangleBaseHeight / 2.0) << QPointF(triangleWidth, triangleBaseHeight / 2.0);
    QGraphicsPolygonItem *leftTriangle = new QGraphicsPolygonItem(leftPointingTriangleShape);
    leftTriangle->setPos(330, 340);
    leftTriangle->setBrush(Qt::NoBrush); leftTriangle->setPen(Qt::NoPen);
    leftTriangle->setData(ITEM_TYPE_ROLE, TYPE_OBSTACLE); scene->addItem(leftTriangle);

    // 创建切菜站
    QString cuttingBoardPath = ":/images/cutting_board.png";
    QPixmap cuttingBoardPixmap(cuttingBoardPath);
    ChoppingStationItem *choppingStation = nullptr;
    if (cuttingBoardPixmap.isNull()) {
        qWarning() << "警告：无法加载砧板图片 (" << cuttingBoardPath << ")。将使用占位符。";
        QPixmap fallbackPixmapCS(120, 80); fallbackPixmapCS.fill(QColor(128,128,128,10));
        choppingStation = new ChoppingStationItem(fallbackPixmapCS);
        if(choppingStation) choppingStation->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    } else { choppingStation = new ChoppingStationItem(cuttingBoardPixmap.scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation)); }
    if (choppingStation) { choppingStation->setPos(140, 480); scene->addItem(choppingStation); }

    // 创建煎锅工作站
    QString fryingPanPath = ":/images/frying_pan.png";
    QPixmap fryingPanPixmap(fryingPanPath);
    FryingStationItem *fryingStation = nullptr;
    if (fryingPanPixmap.isNull()) {
        qWarning() << "警告：无法加载煎锅图片 (" << fryingPanPath << ")。将使用占位符。";
        QPixmap fallbackPixmapFS(100,100); fallbackPixmapFS.fill(QColor(100,100,100,10));
        fryingStation = new FryingStationItem(fallbackPixmapFS);
        if(fryingStation) fryingStation->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    } else { fryingStation = new FryingStationItem(fryingPanPixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation)); }
    if (fryingStation) { fryingStation->setPos(200,100); scene->addItem(fryingStation); }

    // 创建组装台
    QString assemblyTablePath = ":/images/assembly_table.png";
    QPixmap assemblyTablePixmap(assemblyTablePath);
    SaladAssemblyStationItem *assemblyStation = nullptr;
    if (assemblyTablePixmap.isNull()) {
        qWarning() << "警告：无法加载组装台图片 (" << assemblyTablePath << ")。将使用占位符。";
        QPixmap fallbackAssemblyPixmap(150,100); fallbackAssemblyPixmap.fill(QColor(128,128,128,10));
        assemblyStation = new SaladAssemblyStationItem(fallbackAssemblyPixmap);
        if(assemblyStation) assemblyStation->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    } else { assemblyStation = new SaladAssemblyStationItem(assemblyTablePixmap.scaled(150, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation)); }
    if (assemblyStation) { assemblyStation->setPos(540,100); scene->addItem(assemblyStation); }

    // 创建上菜口
    QString servingHatchPath = ":/images/serving_hatch.png";
    QPixmap servingHatchPixmap(servingHatchPath);
    if (servingHatchPixmap.isNull()) {
        qWarning() << "警告：无法加载上菜口图片 (" << servingHatchPath << ")。将使用占位符。";
        QPixmap fallbackPixmapSH(80,100); fallbackPixmapSH.fill(QColor(128,128,128,10));
        m_servingHatch = new ServingHatchItem(fallbackPixmapSH);
        if(m_servingHatch) m_servingHatch->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    } else { m_servingHatch = new ServingHatchItem(servingHatchPixmap.scaled(80, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation)); }
    if (m_servingHatch) {
        m_servingHatch->setPos(630,450); scene->addItem(m_servingHatch);
        disconnect(m_servingHatch, &ServingHatchItem::orderServed, this, &MainWindow::handleOrderServed);
        connect(m_servingHatch, &ServingHatchItem::orderServed, this, &MainWindow::handleOrderServed);
    }

    // 创建原料来源点
    QStringList rawItemNames = {RAW_TOMATO, RAW_LETTUCE, RAW_MEAT, BREAD_SLICE};
    QList<QPointF> rawItemPositions = {QPointF(100,100), QPointF(100,160), QPointF(100,220), QPointF(100,280)};
    QList<QSize> rawItemSizes = {QSize(50,50), QSize(45,45), QSize(60,40), QSize(40,40)};
    for(int i=0; i < rawItemNames.size(); ++i) {
        QString path = Player::getImagePathForItem(rawItemNames[i]);
        if(!path.isEmpty()){ QPixmap pix(path); if(!pix.isNull()){
                QGraphicsPixmapItem* sourceItem = new QGraphicsPixmapItem(pix.scaled(rawItemSizes[i], Qt::KeepAspectRatio, Qt::SmoothTransformation));
                sourceItem->setPos(rawItemPositions[i]);
                sourceItem->setData(ITEM_TYPE_ROLE, TYPE_COLLECTIBLE_VEGETABLE_SOURCE);
                sourceItem->setData(ITEM_NAME_ROLE, rawItemNames[i]);
                sourceItem->setData(ITEM_IMAGE_PATH_ROLE, path);
                scene->addItem(sourceItem);
            } else { qWarning() << "警告：无法加载图片" << path << "for" << rawItemNames[i];}}
        else { qWarning() << "警告：未找到图片路径 for" << rawItemNames[i];}
    }

    // 创建玩家时使用选中的角色路径
    player = new Player(m_selectedCharacterSpritePath, 15.0);
    player->setPos(200,300);
    scene->addItem(player);

    // 连接信号槽
    if (choppingStation && player) {
        disconnect(choppingStation, &ChoppingStationItem::choppingCompleted, player, &Player::onVegetableChopped);
        connect(choppingStation, &ChoppingStationItem::choppingCompleted, player, &Player::onVegetableChopped);
    }
    if (fryingStation && player) {
        disconnect(fryingStation, &FryingStationItem::fryingCompleted, player, &Player::onMeatFried);
        connect(fryingStation, &FryingStationItem::fryingCompleted, player, &Player::onMeatFried);
    }
    // 游戏元素创建结束

    // 初始化游戏视图
    QLayout* oldLayout = gamePageWidget->layout();
    if (oldLayout) { QLayoutItem* itemL; while ((itemL = oldLayout->takeAt(0)) != nullptr) { if (itemL->widget()) { itemL->widget()->deleteLater(); } delete itemL; } delete oldLayout; }
    view = new QGraphicsView(scene, gamePageWidget);
    view->setRenderHint(QPainter::Antialiasing);
    view->setFixedSize(800, 600);
    QVBoxLayout *gameLayout = new QVBoxLayout(gamePageWidget);
    gameLayout->setContentsMargins(0,0,0,0);
    gameLayout->addWidget(view);

    m_score = 0;
    m_currentOrderIndex = 0;
    updateScoreDisplay();
    generateNewOrder(); //启动第一个订单的计时

    m_gameIsRunning = true;
    qDebug() << "游戏界面初始化完毕。";
    this->setFocus();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_gameIsRunning && stackedWidget && stackedWidget->currentWidget() == gamePageWidget) {
        if (!player || !scene) {
            qDebug() << "MainWindow::keyPressEvent - Player or Scene is null.";
            QMainWindow::keyPressEvent(event);
            return;
        }
        bool keyWasProcessed = true;
        switch (event->key()) {
        case Qt::Key_A: case Qt::Key_Left:
        case Qt::Key_D: case Qt::Key_Right:
        case Qt::Key_W: case Qt::Key_Up:
        case Qt::Key_S: case Qt::Key_Down:
            player->handleMovementKey(static_cast<Qt::Key>(event->key()), scene->sceneRect());
            break;
        case Qt::Key_Space:
            player->attemptPickupFromSource();
            break;
        case Qt::Key_E:
            player->attemptInteractWithStation();
            break;
        default:
            keyWasProcessed = false;
            break;
        }
        if (!keyWasProcessed) {
            QMainWindow::keyPressEvent(event);
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::handleOrderServed(const QString& itemName, int points) {
    if (!m_gameIsRunning || m_currentOrder.isEmpty()) {
        qDebug() << "HandleOrderServed: No active game or current order. Item served:" << itemName;
        return;
    }

    if (itemName == m_currentOrder) {
        if (m_orderTimer->isActive()) { // 检查是否在时限内
            m_orderTimer->stop();
            m_orderDisplayUpdateTimer->stop();
            m_score += points;
            qDebug() << "订单 '" << itemName << "' 限时完成! 得分: " << points << "。总分: " << m_score;
            updateScoreDisplay();
            m_currentOrderIndex++;
            generateNewOrder();
        } else {
            qDebug() << "订单 '" << itemName << "' 正确，但已超时。不得分。";
        }
    } else {
        qDebug() << "上菜错误! 当前订单是 '" << m_currentOrder << "', 但提交的是 '" << itemName << "'";
    }
}

void MainWindow::generateNewOrder() {
    // 停止旧订单的计时器
    if (m_orderTimer && m_orderTimer->isActive()) {
        m_orderTimer->stop();
    }
    if (m_orderDisplayUpdateTimer && m_orderDisplayUpdateTimer->isActive()) {
        m_orderDisplayUpdateTimer->stop();
    }

    if (m_currentOrderIndex < m_orderQueue.size()) {
        m_currentOrder = m_orderQueue.at(m_currentOrderIndex);

        // 为特定订单设置不同时限
        if (m_currentOrder == PRODUCT_BURGER) {
            m_currentOrderTimeLimitMs = 60000; // 汉堡给60秒
        } else if (m_currentOrder == PRODUCT_SALAD) {
            m_currentOrderTimeLimitMs = 40000; // 沙拉给40秒
        } else {
            m_currentOrderTimeLimitMs = DEFAULT_ORDER_TIME_LIMIT_MS; // 其他订单使用默认时长
        }
        m_currentOrderTimeLeftSeconds = m_currentOrderTimeLimitMs / 1000;

        qDebug() << "新订单生成: " << m_currentOrder << " (订单 " << m_currentOrderIndex + 1 << "/" << m_orderQueue.size() << ")"
                 << "时限: " << m_currentOrderTimeLeftSeconds << "s";

        if (scene) {
            QString displayText = QString("当前订单: %1").arg(m_currentOrder);
            scene->updateCurrentOrderDisplay(displayText, m_currentOrder);
            scene->updateOrderTimerDisplay(m_currentOrderTimeLeftSeconds); // 立即更新倒计时显示
        }
        m_orderTimer->start(m_currentOrderTimeLimitMs);
        m_orderDisplayUpdateTimer->start(1000); // 每秒更新一次显示
    } else {
        m_currentOrder = "";
        m_currentOrderTimeLeftSeconds = 0;
        qDebug() << "所有预设订单已完成!";
        if (scene) {
            scene->updateCurrentOrderDisplay("所有订单完成!", "");
            scene->updateOrderTimerDisplay(-1); // -1 表示计时结束
        }
        m_gameIsRunning = false; // 可选：游戏结束
    }
}

void MainWindow::updateScoreDisplay() {
    QString scoreTextForTitle = QString("得分: %1").arg(m_score);
    if (m_gameIsRunning && stackedWidget && stackedWidget->currentWidget() == gamePageWidget) {
        setWindowTitle(QString("厨房游戏 - %1").arg(scoreTextForTitle));
        if (scene) {
            scene->updateScoreDisplay(m_score);
        }
    } else {
        setWindowTitle(tr("NKU-kitchen"));
        if (scene) { // 如果场景还存在
            scene->updateScoreDisplay(0);
            scene->updateCurrentOrderDisplay(" ", ""); // 清空订单显示
            scene->updateOrderTimerDisplay(-1);      // 清空计时器显示
        }
    }
}

void MainWindow::onOrderTimerTimeout() {
    if (!m_gameIsRunning || m_currentOrder.isEmpty()) {
        if(m_orderDisplayUpdateTimer && m_orderDisplayUpdateTimer->isActive()) m_orderDisplayUpdateTimer->stop();
        return;
    }
    qDebug() << "订单 '" << m_currentOrder << "' 超时!";
    if(m_orderDisplayUpdateTimer && m_orderDisplayUpdateTimer->isActive()) m_orderDisplayUpdateTimer->stop();
    m_currentOrderTimeLeftSeconds = 0;

    if (scene) {
        scene->updateOrderTimerDisplay(0); // 显示时间为0
        scene->updateCurrentOrderDisplay(QString("订单超时: %1").arg(m_currentOrder), "");
    }

    m_currentOrderIndex++; // 移动到下一个订单
    generateNewOrder();    // 生成新订单
}

void MainWindow::updateOrderCountdownDisplay() {
    if (m_currentOrderTimeLeftSeconds > 0) {
        m_currentOrderTimeLeftSeconds--;
        if (scene) {
            scene->updateOrderTimerDisplay(m_currentOrderTimeLeftSeconds);
        }
    } else {
        if (m_orderDisplayUpdateTimer && m_orderDisplayUpdateTimer->isActive()) {
            m_orderDisplayUpdateTimer->stop();
        }
    }
}
