#ifndef CHARACTERSELECTIONWIDGET_H
#define CHARACTERSELECTIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "item_definitions.h"

class CharacterSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CharacterSelectionWidget(QWidget *parent = nullptr);
    ~CharacterSelectionWidget();

signals:
    void characterSelectedAndStartGame(const QString& characterSpritePath); // 传递选定角色的路径
    void backToStartPageRequested(); // 返回到开始页面的请求

private slots:
    void onPlayerASelected();
    void onPlayerBSelected();
    void onBackButtonClicked();

private:
    QLabel *m_titleLabel;
    QPushButton *m_playerAButton;
    QPushButton *m_playerBButton;
    QLabel *m_playerAPreviewLabel; // 用于显示角色A预览（可选）
    QLabel *m_playerBPreviewLabel; // 用于显示角色B预览（可选）
    QPushButton *m_backButton;

    void setupUi();
};

#endif
