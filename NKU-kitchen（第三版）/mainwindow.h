#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QGraphicsView>
#include "item_definitions.h"

class GameScene;
class Player;
class ServingHatchItem;
class StartPageWidget; // << 前向声明开始页面

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // keyPressEvent 将只在游戏页面激活时才应该处理游戏逻辑
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void handleOrderServed(const QString& itemName, int points);
    void switchToGamePage(); // 新增：切换到游戏页面的槽

private:
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget; // << 管理不同页面的堆叠部件
    StartPageWidget *startPage;
    QWidget *gamePageWidget;      // 游戏内容将放在这个QWidget上（包含QGraphicsView）

    // 游戏相关的成员变量，现在可能在 switchToGamePage 时初始化
    GameScene *scene;
    QGraphicsView *view;
    Player *player;
    ServingHatchItem *m_servingHatch;
    int m_score;

    void setupGameUI();        // 用于初始化游戏界面的函数
    void updateScoreDisplay();
    bool m_gameIsRunning;      // 标记游戏是否正在运行
};
#endif
