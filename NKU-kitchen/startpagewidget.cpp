#include "startpagewidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>
#include <QDebug>

StartPageWidget::StartPageWidget(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(30);
    mainLayout->setAlignment(Qt::AlignCenter);

    titleLabel = new QLabel("厨房历险记(NKU-kitchen)", this);
    QFont titleFont("Arial", 40, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    startButton = new QPushButton("开始游戏", this);
    QFont buttonFont("Arial", 20);
    startButton->setFont(buttonFont);
    startButton->setMinimumSize(200, 60); // 设置按钮最小尺寸

    mainLayout->addStretch(); // 添加弹性空间，将内容推向中间
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(startButton, 0, Qt::AlignHCenter); // 水平居中按钮
    mainLayout->addStretch(); // 添加弹性空间

    setLayout(mainLayout);

    connect(startButton, &QPushButton::clicked, this, &StartPageWidget::onStartButtonClicked);

    // 设置开始页面的背景颜色或图片
    // setStyleSheet("background-color: lightblue;");
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, QPixmap(":/images/start_background.jpg"));
    setAutoFillBackground(true);
    setPalette(pal);
}

StartPageWidget::~StartPageWidget()
{
    qDebug() << "StartPageWidget 销毁";
}

void StartPageWidget::onStartButtonClicked()
{
    qDebug() << "开始游戏按钮被点击！";
    emit startGameRequested();
}
