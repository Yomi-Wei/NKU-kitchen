#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include "item_definitions.h" // 包含常量定义

class GameScene;
class Player;
class ServingHatchItem; // 前向声明

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

private slots: // << 新增槽
    void handleOrderServed(const QString& itemName, int points);

private:
    Ui::MainWindow *ui;
    GameScene *scene;
    QGraphicsView *view;
    Player *player;
    ServingHatchItem *m_servingHatch; // << (可选) 如果需要直接访问上菜口
    int m_score; // << 新增分数

    void updateScoreDisplay(); // 用于更新分数显示
};
#endif // MAINWINDOW_H
