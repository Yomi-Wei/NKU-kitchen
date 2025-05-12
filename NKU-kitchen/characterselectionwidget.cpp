#include "characterselectionwidget.h"
#include <QPixmap>
#include <QFont>
#include <QDebug>

CharacterSelectionWidget::CharacterSelectionWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();

    // 连接信号槽
    connect(m_playerAButton, &QPushButton::clicked, this, &CharacterSelectionWidget::onPlayerASelected);
    connect(m_playerBButton, &QPushButton::clicked, this, &CharacterSelectionWidget::onPlayerBSelected);
    connect(m_backButton, &QPushButton::clicked, this, &CharacterSelectionWidget::onBackButtonClicked);

    // 可选：设置背景
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, QPixmap(":/images/character_select_background.jpg")); // 背景图
    setAutoFillBackground(true);
    setPalette(pal);
}

CharacterSelectionWidget::~CharacterSelectionWidget()
{
    qDebug() << "CharacterSelectionWidget 销毁";
}

void CharacterSelectionWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setAlignment(Qt::AlignCenter);

    m_titleLabel = new QLabel("选择你的厨师", this);
    QFont titleFont("Arial", 36, QFont::Bold);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("color: white;");
    mainLayout->addWidget(m_titleLabel);

    QHBoxLayout *selectionLayout = new QHBoxLayout(); // 水平布局放两个角色选项
    selectionLayout->setSpacing(50);

    // 角色A选项
    QVBoxLayout *playerALayout = new QVBoxLayout();
    m_playerAPreviewLabel = new QLabel(this);
    QPixmap pA(CHARACTER_A_SPRITE_PATH);
    if (!pA.isNull()) {
        m_playerAPreviewLabel->setPixmap(pA.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_playerAPreviewLabel->setText("角色 A\n(图片丢失)");
        m_playerAPreviewLabel->setFixedSize(150,150);
        m_playerAPreviewLabel->setStyleSheet("border: 1px solid white; color: white;");
    }
    m_playerAPreviewLabel->setAlignment(Qt::AlignCenter);
    m_playerAButton = new QPushButton("选择厨师 A", this);
    QFont buttonFont("Arial", 16);
    m_playerAButton->setFont(buttonFont);
    m_playerAButton->setMinimumHeight(50);

    playerALayout->addWidget(m_playerAPreviewLabel, 0, Qt::AlignHCenter);
    playerALayout->addWidget(m_playerAButton, 0, Qt::AlignHCenter);
    selectionLayout->addLayout(playerALayout);

    // 角色B选项
    QVBoxLayout *playerBLayout = new QVBoxLayout();
    m_playerBPreviewLabel = new QLabel(this);
    QPixmap pB(CHARACTER_B_SPRITE_PATH);
    if (!pB.isNull()) {
        m_playerBPreviewLabel->setPixmap(pB.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_playerBPreviewLabel->setText("角色 B\n(图片丢失)");
        m_playerBPreviewLabel->setFixedSize(150,150);
        m_playerBPreviewLabel->setStyleSheet("border: 1px solid white; color: white;");
    }
    m_playerBPreviewLabel->setAlignment(Qt::AlignCenter);
    m_playerBButton = new QPushButton("选择厨师 B", this);
    m_playerBButton->setFont(buttonFont);
    m_playerBButton->setMinimumHeight(50);

    playerBLayout->addWidget(m_playerBPreviewLabel, 0, Qt::AlignHCenter);
    playerBLayout->addWidget(m_playerBButton, 0, Qt::AlignHCenter);
    selectionLayout->addLayout(playerBLayout);

    mainLayout->addLayout(selectionLayout);

    m_backButton = new QPushButton("返回", this);
    m_backButton->setFont(buttonFont);
    m_backButton->setMinimumSize(150, 40);
    mainLayout->addWidget(m_backButton, 0, Qt::AlignHCenter);
    mainLayout->addStretch();


    setLayout(mainLayout);
}

void CharacterSelectionWidget::onPlayerASelected()
{
    qDebug() << "角色 A 被选择";
    emit characterSelectedAndStartGame(CHARACTER_A_SPRITE_PATH);
}

void CharacterSelectionWidget::onPlayerBSelected()
{
    qDebug() << "角色 B 被选择";
    emit characterSelectedAndStartGame(CHARACTER_B_SPRITE_PATH);
}

void CharacterSelectionWidget::onBackButtonClicked()
{
    qDebug() << "返回按钮被点击";
    emit backToStartPageRequested();
}
