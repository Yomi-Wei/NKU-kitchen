// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gamescene.h"
#include "player.h"
#include "choppingstationitem.h"
#include "saladassemblystationitem.h"
#include "servinghatchitem.h"
#include <QKeyEvent>
#include <QDebug>
#include <QGraphicsPolygonItem>
#include <QPolygonF>
#include <QGraphicsPixmapItem>
#include <QLabel>               // 如果使用状态栏显示分数 (当前未使用)
#include<QColor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)    // 如果不使用 .ui 文件，可以注释掉或处理
    , scene(nullptr)
    , view(nullptr)
    , player(nullptr)
    , m_servingHatch(nullptr)   // 初始化成员变量
    , m_score(0)                // 初始化分数
{
    if (ui) { // 简单检查 ui 指针是否有效
        ui->setupUi(this);
    }

    scene = new GameScene(this);
    scene->setSceneRect(0, 0, 800, 600); // 定义场景边界

    // --- 创建障碍物 ---
    QPolygonF rightPointingTriangleShape;
    rightPointingTriangleShape << QPointF(0, -55) << QPointF(0, 55) << QPointF(90, 0);
    QGraphicsPolygonItem *rightTriangle = new QGraphicsPolygonItem(rightPointingTriangleShape);
    rightTriangle->setPos(440, 335); // 根据需要调整位置
    rightTriangle->setBrush(Qt::NoBrush);
    rightTriangle->setPen(Qt::NoPen);
    rightTriangle->setData(ITEM_TYPE_ROLE, TYPE_OBSTACLE);
    rightTriangle->setData(ITEM_NAME_ROLE, "粉三角障碍");
    scene->addItem(rightTriangle);

    QPolygonF leftPointingTriangleShape;
    qreal triangleWidth = 90.0;
    qreal triangleBaseHeight = 110.0;
    leftPointingTriangleShape << QPointF(0, 0) << QPointF(triangleWidth, -triangleBaseHeight / 2.0) << QPointF(triangleWidth, triangleBaseHeight / 2.0);
    QGraphicsPolygonItem *leftTriangle = new QGraphicsPolygonItem(leftPointingTriangleShape);
    leftTriangle->setPos(330, 340); // 根据你的需要调整位置
    leftTriangle->setBrush(Qt::NoBrush);
    leftTriangle->setPen(Qt::NoPen);
    leftTriangle->setData(ITEM_TYPE_ROLE, TYPE_OBSTACLE);
    leftTriangle->setData(ITEM_NAME_ROLE, "青三角障碍");
    scene->addItem(leftTriangle);


    // --- 创建切菜站 ---
    QString cuttingBoardPath = ":/images/cutting_board.png";
    QPixmap cuttingBoardPixmap(cuttingBoardPath);
    ChoppingStationItem *choppingStation = nullptr;

    if (cuttingBoardPixmap.isNull()) {
        qWarning() << "警告：无法加载砧板图片 (" << cuttingBoardPath << ")。将使用占位符。";
        QPixmap fallbackPixmapCS(120, 80);
        fallbackPixmapCS.fill(QColor(128, 128, 128, 10));
        choppingStation = new ChoppingStationItem(fallbackPixmapCS);
    } else {
        choppingStation = new ChoppingStationItem(cuttingBoardPixmap.scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (choppingStation) {
        choppingStation->setPos(140, 480); // 你之前代码中的备用板位置，或调整
        scene->addItem(choppingStation);
        qDebug() << "切菜站 '" << choppingStation->data(ITEM_NAME_ROLE).toString() << "' 已添加在: " << choppingStation->pos();
    }

    // --- 创建沙拉组装台 ---
    QString assemblyTablePath = ":/images/assembly_table.png";
    QPixmap assemblyTablePixmap(assemblyTablePath);
    SaladAssemblyStationItem *saladStation = nullptr;

    if (assemblyTablePixmap.isNull()) {
        qWarning() << "警告：无法加载沙拉组装台图片 (" << assemblyTablePath << ")。将使用占位符。";
        QPixmap fallbackAssemblyPixmap(150,100);
        fallbackAssemblyPixmap.fill(QColor(128, 128, 128, 10));
        saladStation = new SaladAssemblyStationItem(fallbackAssemblyPixmap);
    } else {
        saladStation = new SaladAssemblyStationItem(assemblyTablePixmap.scaled(150, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (saladStation) {
        saladStation->setPos(540,100); // 示例位置
        scene->addItem(saladStation);
        qDebug() << "沙拉组装台 '" << saladStation->data(ITEM_NAME_ROLE).toString() << "' 已添加在: " << saladStation->pos();
    }

    // --- 创建上菜口 ---
    QString servingHatchPath = ":/images/serving_hatch.png";
    QPixmap servingHatchPixmap(servingHatchPath);
    // m_servingHatch 是 MainWindow 的成员变量，在 .h 中声明
    if (servingHatchPixmap.isNull()) {
        qWarning() << "警告：无法加载上菜口图片 (" << servingHatchPath << ")。将使用占位符。";
        QPixmap fallbackPixmapSH(100,100);
        fallbackPixmapSH.fill(QColor(128, 128, 128,10));
        m_servingHatch = new ServingHatchItem(fallbackPixmapSH);
    } else {
        m_servingHatch = new ServingHatchItem(servingHatchPixmap.scaled(80, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (m_servingHatch) {
        m_servingHatch->setPos(630,450); // 上菜口位置示例
        scene->addItem(m_servingHatch);
        // 连接信号槽
        connect(m_servingHatch, &ServingHatchItem::orderServed, this, &MainWindow::handleOrderServed);
        qDebug() << "上菜口 '"<< m_servingHatch->data(ITEM_NAME_ROLE).toString() <<"' 已添加在: " << m_servingHatch->pos();
    }

    // --- 创建蔬菜源 ---
    // 西红柿源
    QString tomatoRawPath = Player::getImagePathForItem("西红柿"); // 静态方法调用
    QPixmap tomatoSourcePixmap(tomatoRawPath);
    if (!tomatoSourcePixmap.isNull()) {
        QGraphicsPixmapItem *tomatoSourceItem = new QGraphicsPixmapItem(tomatoSourcePixmap.scaled(50, 50)); // 你之前代码是50,50
        tomatoSourceItem->setPos(100,300);
        tomatoSourceItem->setData(ITEM_TYPE_ROLE, TYPE_COLLECTIBLE_VEGETABLE_SOURCE);
        tomatoSourceItem->setData(ITEM_NAME_ROLE, "西红柿");
        tomatoSourceItem->setData(ITEM_IMAGE_PATH_ROLE, tomatoRawPath);
        scene->addItem(tomatoSourceItem);
    } else { qWarning() << "警告：无法加载西红柿源图片 (" << tomatoRawPath << ")"; }

    // 生菜源
    QString lettuceRawPath = Player::getImagePathForItem("生菜"); // 静态方法调用
    QPixmap lettuceSourcePixmap(lettuceRawPath);
    if (!lettuceSourcePixmap.isNull()) {
        QGraphicsPixmapItem *lettuceSourceItem = new QGraphicsPixmapItem(lettuceSourcePixmap.scaled(45, 45));
        lettuceSourceItem->setPos(735,250); // 根据你的布局调整
        lettuceSourceItem->setData(ITEM_TYPE_ROLE, TYPE_COLLECTIBLE_VEGETABLE_SOURCE);
        lettuceSourceItem->setData(ITEM_NAME_ROLE, "生菜");
        lettuceSourceItem->setData(ITEM_IMAGE_PATH_ROLE, lettuceRawPath);
        scene->addItem(lettuceSourceItem);
    } else { qWarning() << "警告：无法加载生菜源图片 (" << lettuceRawPath << ")"; }


    // 创建玩家
    QString playerImagePath = ":/images/player_character.png"; // 你的玩家图片路径
    player = new Player(playerImagePath, 15.0); // 你之前代码速度是15.0，这里使用10.0或你希望的
    player->setPos(200,300); // 你之前代码中的玩家初始位置
    scene->addItem(player);

    // 创建视图
    if (!view) { view = new QGraphicsView(scene, this); setCentralWidget(view); }
    else { view->setScene(scene); }
    view->setRenderHint(QPainter::Antialiasing);
    view->setFixedSize(800, 600);

    updateScoreDisplay(); // 初始化分数显示
    this->setFocusPolicy(Qt::StrongFocus); // 主窗口需要焦点来接收键盘事件
}

MainWindow::~MainWindow()
{
    if (ui) { // 如果使用了 .ui 文件
        delete ui;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!player || !scene) { // 确保 player 和 scene 对象有效
        QMainWindow::keyPressEvent(event); // 如果无效，调用基类处理
        return;
    }

    bool keyWasProcessed = true; // 假设按键会被处理
    // 将移动相关的按键事件委托给玩家对象处理
    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
    case Qt::Key_D:
    case Qt::Key_Right:
    case Qt::Key_W:
    case Qt::Key_Up:
    case Qt::Key_S:
    case Qt::Key_Down:
        player->handleMovementKey(static_cast<Qt::Key>(event->key()), scene->sceneRect());
        break;
    case Qt::Key_Space: // 空格键用于从源拾取
        player->attemptPickupFromSource();
        break;
    case Qt::Key_E:     // E键用于与工作站交互
        player->attemptInteractWithStation();
        break;
    default:
        keyWasProcessed = false; // 其他按键不由我们的游戏逻辑处理
        break;
    }

    if (!keyWasProcessed) { // 如果游戏逻辑未处理该按键
        QMainWindow::keyPressEvent(event); // 则传递给基类处理
    }
}

// 这个函数定义【只保留一次】
void MainWindow::handleOrderServed(const QString& itemName, int points) {
    m_score += points;
    qDebug() << "上菜成功: " << itemName << "! 得分: " << points << "。总分: " << m_score;
    updateScoreDisplay();
    // TODO: 可以在这里添加一些视觉或音效反馈
}

// 这个函数定义【只保留一次】
void MainWindow::updateScoreDisplay() {
    setWindowTitle(QString("厨房 - 得分: %1").arg(m_score));
    // 如果你有状态栏:
    // if (statusBar()) {
    //    statusBar()->showMessage(QString("得分: %1").arg(m_score));
    // }
}
