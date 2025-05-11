#include "startscreenwidget.h"
#include <QVBoxLayout>
#include <QPalette>
#include <QColor>

StartScreenWidget::StartScreenWidget(QWidget *parent) : QWidget(parent)
{
    startButton = new QPushButton("开始游戏", this); // "开始游戏"
    startButton->setMinimumSize(150, 50); // 设置按钮最小尺寸以便看得清楚
    QFont buttonFont = startButton->font();
    buttonFont.setPointSize(16); // 设置字体大小
    startButton->setFont(buttonFont);

    connect(startButton, &QPushButton::clicked, this, &StartScreenWidget::startGameClicked);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(startButton);
    layout->setAlignment(Qt::AlignCenter); // 按钮居中显示
    setLayout(layout);

    //为起始页设置背景颜色
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(Qt::lightGray).lighter(120));
    setAutoFillBackground(true);
    setPalette(pal);
}
