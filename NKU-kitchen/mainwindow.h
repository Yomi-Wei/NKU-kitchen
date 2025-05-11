#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QGraphicsView>
#include <QString>
#include <QList>
#include "item_definitions.h"

class GameScene;
class Player;
class ServingHatchItem;
class StartPageWidget;

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
    void switchToGamePage();

private:
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    StartPageWidget *startPage;
    QWidget *gamePageWidget;

    GameScene *scene;
    QGraphicsView *view;
    Player *player;
    ServingHatchItem *m_servingHatch;
    int m_score;
    QString m_currentOrder; // 存储当前订单的产品键名，例如 PRODUCT_SALAD

    QList<QString> m_orderQueue;
    int m_currentOrderIndex;

    void setupGameUI();
    void updateScoreDisplay();
    void initializeOrderQueue();
    void generateNewOrder();


    bool m_gameIsRunning;
};
#endif
