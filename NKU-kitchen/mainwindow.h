#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QGraphicsView>
#include <QString>
#include <QList>
#include <QTimer>

#include "item_definitions.h" // 包含角色定义常量

class GameScene;
class Player;
class ServingHatchItem;
class StartPageWidget;
class CharacterSelectionWidget;

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
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void handleOrderServed(const QString& itemName, int points);
    // void switchToGamePage(); 这个槽不再直接由 StartPageWidget 调用
    void onOrderTimerTimeout();
    void updateOrderCountdownDisplay();

    void switchToCharacterSelectionPage(); // 切换到角色选择页面的槽
    void handleCharacterSelectedAndStartGame(const QString& characterSpritePath);
    void switchToStartPage(); // 从角色选择返回到开始页面

private:
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    StartPageWidget *startPage;
    CharacterSelectionWidget *characterSelectionPage; //角色选择页面实例
    QWidget *gamePageWidget;

    GameScene *scene;
    QGraphicsView *view;
    Player *player;
    ServingHatchItem *m_servingHatch;
    int m_score;
    QString m_currentOrder;

    QList<QString> m_orderQueue;
    int m_currentOrderIndex;

    QTimer* m_orderTimer;
    QTimer* m_orderDisplayUpdateTimer;
    int m_currentOrderTimeLimitMs;
    int m_currentOrderTimeLeftSeconds;

    QString m_selectedCharacterSpritePath; //

    void setupGameUI();
    void updateScoreDisplay();
    void initializeOrderQueue();
    void generateNewOrder();

    bool m_gameIsRunning;
};
#endif
