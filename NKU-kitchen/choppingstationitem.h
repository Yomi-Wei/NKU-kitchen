#ifndef CHOPPINGSTATIONITEM_H
#define CHOPPINGSTATIONITEM_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QString>
#include <QTimer>
#include "item_definitions.h"


class QGraphicsRectItem;

class ChoppingStationItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit ChoppingStationItem(const QPixmap& pixmap, QGraphicsItem *parent = nullptr);
    ~ChoppingStationItem();

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
    void onChoppingTimerTimeout();
    void updateChoppingProgress();

private:
    QString m_placedVegetableName;
    QString m_placedVegetableImagePath;
    QGraphicsPixmapItem* m_visualVegetableOnBoard;
    bool m_isCut;
    bool m_isChopping;
    QTimer* m_choppingTimer;
    QString m_pendingCutImagePath;

    // 进度条相关成员
    QGraphicsRectItem* m_progressBarBackground;
    QGraphicsRectItem* m_progressBarForeground;
    QTimer* m_progressUpdateTimer;
    int m_totalChoppingTimeMs;
    int m_elapsedChoppingTimeMs;

    void setupProgressBar();
    void resetProgressBar();
    void showProgressBar();
};

#endif
