#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startpagewidget.h"
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(nullptr)
    , stackedWidget(nullptr)
    , startPage(nullptr)
    , gamePageWidget(nullptr)
    , scene(nullptr)
    , view(nullptr)
    , player(nullptr)
    , m_servingHatch(nullptr)
    , m_score(0)
    , m_currentOrder("")
    , m_currentOrderIndex(0)
    , m_gameIsRunning(false)
{
    // stackedWidget 和 startPage 的初始化
    stackedWidget = new QStackedWidget(this);
    startPage = new StartPageWidget(this);
    gamePageWidget = new QWidget(this); // 游戏视图的容器

    connect(startPage, &StartPageWidget::startGameRequested, this, &MainWindow::switchToGamePage);

    stackedWidget->addWidget(startPage);
    stackedWidget->addWidget(gamePageWidget);

    setCentralWidget(stackedWidget);
    stackedWidget->setCurrentWidget(startPage);

    resize(820, 620);                           // 调整窗口大小以适应内容
    this->setFocusPolicy(Qt::StrongFocus);
    setWindowTitle(tr("厨房历险记 (NKU-kitchen)")); // 初始窗口标题

    initializeOrderQueue(); // 初始化预设订单
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow 销毁";
}

void MainWindow::initializeOrderQueue() {
    m_orderQueue.clear();
    // 在这里添加预设的订单
    m_orderQueue << PRODUCT_SALAD;
    m_orderQueue << PRODUCT_BURGER; // 添加汉堡到订单
    m_orderQueue << PRODUCT_SALAD;
    // 可以添加更多不同类型的订单
    qDebug() << "预设订单队列已初始化，包含" << m_orderQueue.size() << "个订单。";
}

void MainWindow::setupGameUI()
{
    if(player) {
        if(player->scene()) player->scene()->removeItem(player);
        delete player;
        player = nullptr;
    }
    if(m_servingHatch) {
        if(m_servingHatch->scene()) m_servingHatch->scene()->removeItem(m_servingHatch);
        delete m_servingHatch;
        m_servingHatch = nullptr;
    }
    delete scene;
    scene = nullptr;

    qDebug() << "正在初始化游戏界面...";

    scene = new GameScene(this);
    scene->setSceneRect(0, 0, 800, 600); // 设置场景大小

    // 创建游戏元素

    // 创建障碍物
    QPolygonF rightPointingTriangleShape;
    rightPointingTriangleShape << QPointF(0, -55) << QPointF(0, 55) << QPointF(90, 0);
    QGraphicsPolygonItem *rightTriangle = new QGraphicsPolygonItem(rightPointingTriangleShape);
    rightTriangle->setPos(400,335);
    rightTriangle->setBrush(Qt::NoBrush);
    rightTriangle->setPen(Qt::NoPen);
    rightTriangle->setData(ITEM_TYPE_ROLE, TYPE_OBSTACLE);
    scene->addItem(rightTriangle);

    QPolygonF leftPointingTriangleShape;
    qreal triangleWidth = 90.0;
    qreal triangleBaseHeight = 110.0;
    leftPointingTriangleShape << QPointF(0, 0) << QPointF(triangleWidth, -triangleBaseHeight / 2.0) << QPointF(triangleWidth, triangleBaseHeight / 2.0);
    QGraphicsPolygonItem *leftTriangle = new QGraphicsPolygonItem(leftPointingTriangleShape);
    leftTriangle->setPos(330, 340);
    leftTriangle->setBrush(Qt::NoBrush);
    leftTriangle->setPen(Qt::NoPen);
    leftTriangle->setData(ITEM_TYPE_ROLE, TYPE_OBSTACLE);
    scene->addItem(leftTriangle);

    // 创建切菜站
    QString cuttingBoardPath = ":/images/cutting_board.png";
    QPixmap cuttingBoardPixmap(cuttingBoardPath);
    ChoppingStationItem *choppingStation = nullptr; // 局部指针
    if (cuttingBoardPixmap.isNull()) {
        qWarning() << "警告：无法加载砧板图片 (" << cuttingBoardPath << ")。将使用占位符。";
        QPixmap fallbackPixmapCS(120, 80);
        fallbackPixmapCS.fill(QColor(128, 128, 128, 10));
        choppingStation = new ChoppingStationItem(fallbackPixmapCS);
        if(choppingStation) choppingStation->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    } else {
        choppingStation = new ChoppingStationItem(cuttingBoardPixmap.scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (choppingStation) {
        choppingStation->setPos(140, 480);
        scene->addItem(choppingStation);
    }

    // 创建煎锅工作站
    QString fryingPanPath = ":/images/frying_pan.png";
    QPixmap fryingPanPixmap(fryingPanPath);
    FryingStationItem *fryingStation = nullptr;
    if (fryingPanPixmap.isNull()) {
        qWarning() << "警告：无法加载煎锅图片 (" << fryingPanPath << ")。将使用占位符。";
        QPixmap fallbackPixmapFS(100, 100);
        fallbackPixmapFS.fill(QColor(100, 100, 100, 10));
        fryingStation = new FryingStationItem(fallbackPixmapFS);
        if(fryingStation) fryingStation->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    } else {
        fryingStation = new FryingStationItem(fryingPanPixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (fryingStation) {
        fryingStation->setPos(200,100);
        scene->addItem(fryingStation);
    }

    // 创建组装台
    QString assemblyTablePath = ":/images/assembly_table.png";
    QPixmap assemblyTablePixmap(assemblyTablePath);
    SaladAssemblyStationItem *assemblyStation = nullptr;
    if (assemblyTablePixmap.isNull()) {
        qWarning() << "警告：无法加载组装台图片 (" << assemblyTablePath << ")。将使用占位符。";
        QPixmap fallbackAssemblyPixmap(150, 100);
        fallbackAssemblyPixmap.fill(QColor(128, 128, 128, 10));
        assemblyStation = new SaladAssemblyStationItem(fallbackAssemblyPixmap);
        if(assemblyStation) assemblyStation->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    } else {
        assemblyStation = new SaladAssemblyStationItem(assemblyTablePixmap.scaled(150, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (assemblyStation) {
        assemblyStation->setPos(540,100);
        scene->addItem(assemblyStation);
    }

    // 创建上菜口 (m_servingHatch 是成员变量，在清理时已处理)
    QString servingHatchPath = ":/images/serving_hatch.png";
    QPixmap servingHatchPixmap(servingHatchPath);
    if (servingHatchPixmap.isNull()) {
        qWarning() << "警告：无法加载上菜口图片 (" << servingHatchPath << ")。将使用占位符。";
        QPixmap fallbackPixmapSH(80, 100);
        fallbackPixmapSH.fill(QColor(128, 128, 128,10));
        m_servingHatch = new ServingHatchItem(fallbackPixmapSH);
        if(m_servingHatch) m_servingHatch->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    } else {
        m_servingHatch = new ServingHatchItem(servingHatchPixmap.scaled(80, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (m_servingHatch) {
        m_servingHatch->setPos(630,450);
        scene->addItem(m_servingHatch);
        // 确保先断开旧连接
        disconnect(m_servingHatch, &ServingHatchItem::orderServed, this, &MainWindow::handleOrderServed);
        connect(m_servingHatch, &ServingHatchItem::orderServed, this, &MainWindow::handleOrderServed);
    }

    // 创建蔬菜源 (西红柿)
    QString tomatoRawPathUser = Player::getImagePathForItem(RAW_TOMATO);
    if (!tomatoRawPathUser.isEmpty()) {
        QPixmap tomatoSourcePixmapUser(tomatoRawPathUser);
        if (!tomatoSourcePixmapUser.isNull()) {
            QGraphicsPixmapItem *tomatoSourceItem = new QGraphicsPixmapItem(tomatoSourcePixmapUser.scaled(50, 50));
            tomatoSourceItem->setPos(100, 100);
            tomatoSourceItem->setData(ITEM_TYPE_ROLE, TYPE_COLLECTIBLE_VEGETABLE_SOURCE);
            tomatoSourceItem->setData(ITEM_NAME_ROLE, RAW_TOMATO);
            tomatoSourceItem->setData(ITEM_IMAGE_PATH_ROLE, tomatoRawPathUser);
            scene->addItem(tomatoSourceItem);
        } else { qWarning() << "警告：无法加载西红柿源图片 (" << tomatoRawPathUser << ")"; }
    } else { qWarning() << "警告：未找到西红柿的图片路径。"; }


    // 创建蔬菜源 (生菜)
    QString lettuceRawPathUser = Player::getImagePathForItem(RAW_LETTUCE);
    if (!lettuceRawPathUser.isEmpty()){
        QPixmap lettuceSourcePixmapUser(lettuceRawPathUser);
        if (!lettuceSourcePixmapUser.isNull()) {
            QGraphicsPixmapItem *lettuceSourceItem = new QGraphicsPixmapItem(lettuceSourcePixmapUser.scaled(45, 45));
            lettuceSourceItem->setPos(100, 160); // 西红柿下方
            lettuceSourceItem->setData(ITEM_TYPE_ROLE, TYPE_COLLECTIBLE_VEGETABLE_SOURCE);
            lettuceSourceItem->setData(ITEM_NAME_ROLE, RAW_LETTUCE);
            lettuceSourceItem->setData(ITEM_IMAGE_PATH_ROLE, lettuceRawPathUser);
            scene->addItem(lettuceSourceItem);
        } else { qWarning() << "警告：无法加载生菜源图片 (" << lettuceRawPathUser << ")"; }
    } else { qWarning() << "警告：未找到生菜的图片路径。"; }


    // 创建肉的来源点
    QString meatRawPathUser = Player::getImagePathForItem(RAW_MEAT);
    if (!meatRawPathUser.isEmpty()){
        QPixmap meatSourcePixmapUser(meatRawPathUser);
        if (!meatSourcePixmapUser.isNull()) {
            QGraphicsPixmapItem *meatSourceItem = new QGraphicsPixmapItem(meatSourcePixmapUser.scaled(60, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            meatSourceItem->setPos(100, 220); // 生菜下方
            meatSourceItem->setData(ITEM_TYPE_ROLE, TYPE_COLLECTIBLE_VEGETABLE_SOURCE);
            meatSourceItem->setData(ITEM_NAME_ROLE, RAW_MEAT);
            meatSourceItem->setData(ITEM_IMAGE_PATH_ROLE, meatRawPathUser);
            scene->addItem(meatSourceItem);
        } else { qWarning() << "警告：无法加载生肉源图片 (" << meatRawPathUser << ")"; }
    } else { qWarning() << "警告：未找到生肉的图片路径。"; }


    // 创建面包片来源点
    QString breadSlicePathUser = Player::getImagePathForItem(BREAD_SLICE);
    if (!breadSlicePathUser.isEmpty()) {
        QPixmap breadSourcePixmapUser(breadSlicePathUser);
        if (!breadSourcePixmapUser.isNull()) {
            QGraphicsPixmapItem *breadSourceItem = new QGraphicsPixmapItem(breadSourcePixmapUser.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            breadSourceItem->setPos(100, 280); // 肉源下方
            breadSourceItem->setData(ITEM_TYPE_ROLE, TYPE_COLLECTIBLE_VEGETABLE_SOURCE);
            breadSourceItem->setData(ITEM_NAME_ROLE, BREAD_SLICE);
            breadSourceItem->setData(ITEM_IMAGE_PATH_ROLE, breadSlicePathUser);
            scene->addItem(breadSourceItem);
        } else { qWarning() << "警告：无法加载面包片源图片 (" << breadSlicePathUser << ")"; }
    } else { qWarning() << "警告：未找到面包片的图片路径。"; }


    // 创建玩家 (player 是成员变量，在清理时已处理)
    QString playerImagePath = ":/images/player_character.png";
    player = new Player(playerImagePath, 10.0);
    player->setPos(200,300);
    scene->addItem(player);

    // 连接信号和槽
    if (choppingStation && player) {
        // 先断开旧连接，以防重复
        disconnect(choppingStation, &ChoppingStationItem::choppingCompleted, player, &Player::onVegetableChopped);
        connect(choppingStation, &ChoppingStationItem::choppingCompleted, player, &Player::onVegetableChopped);
    }
    if (fryingStation && player) {
        disconnect(fryingStation, &FryingStationItem::fryingCompleted, player, &Player::onMeatFried);
        connect(fryingStation, &FryingStationItem::fryingCompleted, player, &Player::onMeatFried);
    }


    // 初始化游戏视图
    // 先清理旧的 gamePageWidget 布局和 QGraphicsView
    QLayout* oldLayout = gamePageWidget->layout();
    if (oldLayout) {
        QLayoutItem* itemL;
        while ((itemL = oldLayout->takeAt(0)) != nullptr) {
            if (itemL->widget()) { // 确保 widget 存在
                itemL->widget()->deleteLater(); // 安全删除 widget (例如旧的 QGraphicsView)
            }
            delete itemL; // 删除布局项
        }
        delete oldLayout; // 删除旧布局本身
    }
    view = new QGraphicsView(scene, gamePageWidget); // 将 gamePageWidget 作为父对象
    view->setRenderHint(QPainter::Antialiasing);
    view->setFixedSize(800, 600);

    QVBoxLayout *gameLayout = new QVBoxLayout(gamePageWidget); // gamePageWidget 会管理 gameLayout
    gameLayout->setContentsMargins(0,0,0,0);
    gameLayout->addWidget(view); // view 会被 gameLayout 管理


    m_score = 0;
    m_currentOrderIndex = 0; // 游戏开始时，从第一个订单开始
    updateScoreDisplay();    // 更新分数显示（窗口标题）
    generateNewOrder();

    m_gameIsRunning = true;
    qDebug() << "游戏界面初始化完毕。";
    this->setFocus(); // 主窗口获取焦点以便接收键盘事件
}

void MainWindow::switchToGamePage()
{
    qDebug() << "切换到游戏页面...";
    setupGameUI(); // 每次切换都重新设置UI，确保游戏状态重置
    stackedWidget->setCurrentWidget(gamePageWidget);
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
    if (!m_gameIsRunning) return;

    if (itemName == m_currentOrder && !m_currentOrder.isEmpty()) {
        m_score += points;
        qDebug() << "订单 '" << itemName << "' 完成! 得分: " << points << "。总分: " << m_score;
        updateScoreDisplay(); // 更新窗口标题分数和场景内分数
        m_currentOrderIndex++;
        generateNewOrder();
    } else if (!m_currentOrder.isEmpty()) { // 有活动订单但上错菜
        qDebug() << "上菜错误! 当前订单是 '" << m_currentOrder << "', 但提交的是 '" << itemName << "'";
        // 此处可以添加惩罚或提示
    } else { // 没有活动订单（例如所有订单已完成）
        qDebug() << "收到意外的上菜 '" << itemName << "'，当前没有活动订单。";
    }
}

void MainWindow::generateNewOrder() {
    if (m_currentOrderIndex < m_orderQueue.size()) {
        m_currentOrder = m_orderQueue.at(m_currentOrderIndex);
        qDebug() << "新订单生成 (来自队列): " << m_currentOrder << " (订单 " << m_currentOrderIndex + 1 << "/" << m_orderQueue.size() << ")";
        if (scene) { // 确保场景存在
            QString displayText = QString("当前订单: %1").arg(m_currentOrder);
            // m_currentOrder 存储的是产品键名，例如 PRODUCT_SALAD, PRODUCT_BURGER
            scene->updateCurrentOrderDisplay(displayText, m_currentOrder);
        }
    } else {
        m_currentOrder = ""; // 表示没有活动的产品订单
        qDebug() << "所有预设订单已完成!";
        if (scene) { // 确保场景存在
            scene->updateCurrentOrderDisplay("所有订单完成!", ""); \
        }
        m_gameIsRunning = false; // 游戏结束
        // 可选: 显示游戏结束画面等
    }
}

void MainWindow::updateScoreDisplay() {
    QString scoreTextForTitle = QString("得分: %1").arg(m_score);

    if (m_gameIsRunning && stackedWidget && stackedWidget->currentWidget() == gamePageWidget) {
        setWindowTitle(QString("厨房游戏 - %1").arg(scoreTextForTitle));
        if (scene) { // 确保场景存在
            scene->updateScoreDisplay(m_score);
        }
    } else { // 不在游戏页面时（例如开始页面，或游戏结束后）
        setWindowTitle(tr("厨房历险记 (NKU-kitchen)"));
        if (scene) { // 如果场景还存在（例如从游戏页切换出去，或者游戏结束但场景未销毁）
            scene->updateScoreDisplay(0); // 重置场景内分数
            scene->updateCurrentOrderDisplay(" ", ""); // 清空场景内订单显示
        }
    }
}
