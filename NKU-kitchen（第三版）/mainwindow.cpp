#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "startpagewidget.h"
#include "gamescene.h"       // GameScene 的定义需要被包含
#include "player.h"
#include "choppingstationitem.h"
#include "saladassemblystationitem.h"
#include "servinghatchitem.h"
#include <QKeyEvent>
#include <QDebug>
#include <QGraphicsPolygonItem>
#include <QPolygonF>
#include <QGraphicsPixmapItem>
#include <QColor>
#include <QVBoxLayout>

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
    , m_gameIsRunning(false)
{
    stackedWidget = new QStackedWidget(this);
    startPage = new StartPageWidget(this);
    gamePageWidget = new QWidget(this);

    connect(startPage, &StartPageWidget::startGameRequested, this, &MainWindow::switchToGamePage);

    stackedWidget->addWidget(startPage);
    stackedWidget->addWidget(gamePageWidget);

    setCentralWidget(stackedWidget);
    stackedWidget->setCurrentWidget(startPage);

    resize(820, 620);
    // updateScoreDisplay(); // 构造时调用可能过早，移到 switchToGamePage 和 setupGameUI 中
    this->setFocusPolicy(Qt::StrongFocus);
    setWindowTitle(tr("厨房历险记 (NKU-kitchen)")); // 初始窗口标题
}

void MainWindow::setupGameUI()
{
    // 清理旧的游戏资源
    if(player && player->scene()) { player->scene()->removeItem(player); } delete player; player = nullptr;
    if(m_servingHatch && m_servingHatch->scene()) { m_servingHatch->scene()->removeItem(m_servingHatch); } delete m_servingHatch; m_servingHatch = nullptr;
    // 注意：GameScene 内部创建的 m_scoreTextItem 会随着 scene 的删除而被删除
    delete scene; scene = nullptr;


    qDebug() << "正在初始化游戏界面...";

    scene = new GameScene(this); // GameScene 构造函数中会创建 m_scoreTextItem
    scene->setSceneRect(0, 0, 800, 600);

    // ... (您原有的障碍物、工作站、玩家等创建代码保持不变) ...
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
    ChoppingStationItem *choppingStation = nullptr;
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

    // 创建沙拉组装台
    QString assemblyTablePath = ":/images/assembly_table.png";
    QPixmap assemblyTablePixmap(assemblyTablePath);
    SaladAssemblyStationItem *saladStation = nullptr;
    if (assemblyTablePixmap.isNull()) {
        qWarning() << "警告：无法加载沙拉组装台图片 (" << assemblyTablePath << ")。将使用占位符。";
        QPixmap fallbackAssemblyPixmap(150, 100);
        fallbackAssemblyPixmap.fill(QColor(128, 128, 128, 10));
        saladStation = new SaladAssemblyStationItem(fallbackAssemblyPixmap);
        if(saladStation) saladStation->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    } else {
        saladStation = new SaladAssemblyStationItem(assemblyTablePixmap.scaled(150, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (saladStation) {
        saladStation->setPos(540,100);
        scene->addItem(saladStation);
    }

    // 创建上菜口
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
        disconnect(m_servingHatch, &ServingHatchItem::orderServed, this, &MainWindow::handleOrderServed);
        connect(m_servingHatch, &ServingHatchItem::orderServed, this, &MainWindow::handleOrderServed);
    }

    // 创建蔬菜源
    QString tomatoRawPathUser = Player::getImagePathForItem(RAW_TOMATO);
    QPixmap tomatoSourcePixmapUser(tomatoRawPathUser);
    if (!tomatoSourcePixmapUser.isNull()) {
        QGraphicsPixmapItem *tomatoSourceItem = new QGraphicsPixmapItem(tomatoSourcePixmapUser.scaled(50, 50));
        tomatoSourceItem->setPos(100, 100);
        tomatoSourceItem->setData(ITEM_TYPE_ROLE, TYPE_COLLECTIBLE_VEGETABLE_SOURCE);
        tomatoSourceItem->setData(ITEM_NAME_ROLE, RAW_TOMATO);
        tomatoSourceItem->setData(ITEM_IMAGE_PATH_ROLE, tomatoRawPathUser);
        scene->addItem(tomatoSourceItem);
    } else { qWarning() << "警告：无法加载西红柿源图片 (" << tomatoRawPathUser << ")"; }

    QString lettuceRawPathUser = Player::getImagePathForItem(RAW_LETTUCE);
    QPixmap lettuceSourcePixmapUser(lettuceRawPathUser);
    if (!lettuceSourcePixmapUser.isNull()) {
        QGraphicsPixmapItem *lettuceSourceItem = new QGraphicsPixmapItem(lettuceSourcePixmapUser.scaled(45, 45));
        lettuceSourceItem->setPos(600, 100);
        lettuceSourceItem->setData(ITEM_TYPE_ROLE, TYPE_COLLECTIBLE_VEGETABLE_SOURCE);
        lettuceSourceItem->setData(ITEM_NAME_ROLE, RAW_LETTUCE);
        lettuceSourceItem->setData(ITEM_IMAGE_PATH_ROLE, lettuceRawPathUser);
        scene->addItem(lettuceSourceItem);
    } else { qWarning() << "警告：无法加载生菜源图片 (" << lettuceRawPathUser << ")"; }

    // 创建玩家
    QString playerImagePath = ":/images/player_character.png";
    player = new Player(playerImagePath, 10.0);
    player->setPos(200,300);
    scene->addItem(player);

    if (choppingStation && player) {
        disconnect(choppingStation, &ChoppingStationItem::choppingCompleted, player, &Player::onVegetableChopped);
        connect(choppingStation, &ChoppingStationItem::choppingCompleted, player, &Player::onVegetableChopped);
        qDebug() << "已连接 ChoppingStationItem::choppingCompleted 到 Player::onVegetableChopped";
    } else {
        qWarning() << "MainWindow::setupGameUI - choppingStation 或 player 未能成功创建，无法连接切菜信号！";
    }
    // ... (结束您原有的场景元素创建代码) ...


    //初始化游戏视图
    QLayout* oldLayout = gamePageWidget->layout();
    if (oldLayout) {
        QLayoutItem* itemL;
        while ((itemL = oldLayout->takeAt(0)) != nullptr) {
            delete itemL->widget();
            delete itemL;
        }
        delete oldLayout;
    }
    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setFixedSize(800, 600);

    QVBoxLayout *gameLayout = new QVBoxLayout(gamePageWidget);
    gameLayout->setContentsMargins(0,0,0,0);
    gameLayout->addWidget(view);
    gamePageWidget->setLayout(gameLayout);

    m_score = 0; // 重置分数
    updateScoreDisplay(); // 初始化游戏内和窗口标题的分数显示
    m_gameIsRunning = true;
    qDebug() << "游戏界面初始化完毕。";
    this->setFocus();
}

void MainWindow::switchToGamePage()
{
    qDebug() << "切换到游戏页面...";
    setupGameUI();
    stackedWidget->setCurrentWidget(gamePageWidget);
    // updateScoreDisplay(); // 已在 setupGameUI 末尾调用
    this->setFocus();
}

MainWindow::~MainWindow()
{
    if (ui) { delete ui; }
    qDebug() << "MainWindow 销毁";
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_gameIsRunning && stackedWidget && stackedWidget->currentWidget() == gamePageWidget) {
        if (!player || !scene) { QMainWindow::keyPressEvent(event); return; }
        bool keyWasProcessed = true;
        switch (event->key()) {
        case Qt::Key_A: case Qt::Key_Left: case Qt::Key_D: case Qt::Key_Right:
        case Qt::Key_W: case Qt::Key_Up:   case Qt::Key_S: case Qt::Key_Down:
            player->handleMovementKey(static_cast<Qt::Key>(event->key()), scene->sceneRect());
            break;
        case Qt::Key_Space: player->attemptPickupFromSource(); break;
        case Qt::Key_E:     player->attemptInteractWithStation(); break;
        default: keyWasProcessed = false; break;
        }
        if (!keyWasProcessed) { QMainWindow::keyPressEvent(event); }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::handleOrderServed(const QString& itemName, int points) {
    if (!m_gameIsRunning) return;
    m_score += points;
    qDebug() << "上菜成功: " << itemName << "! 得分: " << points << "。总分: " << m_score;
    updateScoreDisplay(); // 更新分数显示（包括游戏内和窗口标题）
}

void MainWindow::updateScoreDisplay() {
    QString scoreTextForTitle = QString("得分: %1").arg(m_score);

    if (m_gameIsRunning && stackedWidget && stackedWidget->currentWidget() == gamePageWidget) {
        setWindowTitle(QString("厨房游戏 - %1").arg(scoreTextForTitle));
        if (scene) { // 确保 scene 对象存在
            scene->updateScoreDisplay(m_score); // 调用 GameScene 的方法来更新场景内的分数文本
        }
    } else {
        setWindowTitle(tr("厨房历险记 (NKU-kitchen)"));
        // 如果不在游戏页面，也可以考虑让 GameScene 中的分数显示重置或显示特定文本
        if (scene) {
            scene->updateScoreDisplay(0); // 例如，不在游戏时显示0分
        }
    }
}
