#ifndef STARTPAGEWIDGET_H
#define STARTPAGEWIDGET_H

#include <QWidget>

class QPushButton; // 前向声明
class QVBoxLayout; // 前向声明
class QLabel;      // 前向声明 (用于标题)

class StartPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StartPageWidget(QWidget *parent = nullptr);
    ~StartPageWidget();

signals:
    void startGameRequested(); // 当 "开始游戏" 按钮被点击时发出此信号

private slots:
    void onStartButtonClicked();

private:
    QLabel *titleLabel;
    QPushButton *startButton;
    QVBoxLayout *mainLayout;
};

#endif
