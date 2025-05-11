#ifndef CHOPPINGSTATIONITEM_H
#define CHOPPINGSTATIONITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QString>
#include <QTimer> // 为了切菜耗时
#include "item_definitions.h"

// 前向声明
class QGraphicsRectItem; // 用于进度条

class ChoppingStationItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit ChoppingStationItem(const QPixmap& pixmap, QGraphicsItem *parent = nullptr);
    ~ChoppingStationItem(); // 添加析构函数

    bool placeVegetable(const QString& vegetableName, const QString& vegetableImagePath);
    QString getPlacedVegetableName() const;
    QString takeVegetable();
    bool isEmpty() const;
    bool isCut() const;
    bool isChopping() const;

    bool startChoppingProcess(const QString& rawVegNameCurrentlyOnBoard, const QString& cutVegImagePath, int durationMs);
    QString getCurrentVisualNameOnBoard() const;

signals:
    // 当切菜动作（定时器结束）完成时发出
    void choppingCompleted(const QString& originalVegetableName, const QString& cutImagePathUsed);

private slots:
    void onChoppingTimerTimeout(); // 主定时器，用于切菜完成
    void updateChoppingProgress(); // 用于更新进度条视觉效果的定时器槽函数

private:
    QString m_placedVegetableName;
    QString m_placedVegetableImagePath; // 原始图片路径
    QGraphicsPixmapItem* m_visualVegetableOnBoard;
    bool m_isCut;
    bool m_isChopping;      // 标记是否正在切菜
    QTimer* m_choppingTimer;  // 切菜总时长定时器
    QString m_pendingCutImagePath; // 切好后的图片路径，供定时器使用

    // --- 进度条相关成员 ---
    QGraphicsRectItem* m_progressBarBackground; // 进度条背景
    QGraphicsRectItem* m_progressBarForeground; // 进度条前景（实际进度）
    QTimer* m_progressUpdateTimer;    // 用于更新进度条显示的定时器
    int m_totalChoppingTimeMs;      // 当前切菜任务的总时长
    int m_elapsedChoppingTimeMs;    // 当前切菜任务已用时长 (用于更新进度条)

    void setupProgressBar();          // 初始化进度条图形项
    void resetProgressBar();          // 重置并隐藏进度条
    void showProgressBar();           // 显示进度条并开始更新
};

#endif // CHOPPINGSTATIONITEM_H
