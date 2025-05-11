#ifndef STARTSCREENWIDGET_H
#define STARTSCREENWIDGET_H

#include <QWidget>
#include <QPushButton>

class StartScreenWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StartScreenWidget(QWidget *parent = nullptr);

signals:
    void startGameClicked(); // 当按钮被点击时发射此信号

private:
    QPushButton *startButton;
};

#endif
