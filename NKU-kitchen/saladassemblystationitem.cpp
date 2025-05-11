#include "saladassemblystationitem.h"
#include "player.h"
#include "item_definitions.h"
#include <QDebug>
#include <QGraphicsScene>

SaladAssemblyStationItem::SaladAssemblyStationItem(const QPixmap& pixmap, QGraphicsItem *parent)
    : QObject(), QGraphicsPixmapItem(pixmap, parent)
{
    this->setData(ITEM_TYPE_ROLE, TYPE_SALAD_ASSEMBLY_STATION);
    this->setData(ITEM_NAME_ROLE, "组装台");

    initializeRecipes();
    checkAvailableRecipes();
    qDebug() << "SaladAssemblyStationItem created. Initial producible items:" << m_producibleProductNames
             << "Board state:" << m_ingredientsOnBoard;
}

void SaladAssemblyStationItem::initializeRecipes() {
    m_recipes.clear();

    // 沙拉配方 (保持不变)
    QMap<QString, int> saladRecipe;
    saladRecipe[CUT_TOMATO] = 1;
    saladRecipe[CUT_LETTUCE] = 1;
    m_recipes[PRODUCT_SALAD] = saladRecipe;

    // 新的汉堡配方：沙拉 + 面包 + 煎肉
    QMap<QString, int> newBurgerRecipe;
    newBurgerRecipe[PRODUCT_SALAD] = 1;   // 需要1份已做好的沙拉
    newBurgerRecipe[BREAD_SLICE] = 2;   // 需要2片面包
    newBurgerRecipe[FRIED_MEAT] = 1;    // 需要1份煎好的肉
    m_recipes[PRODUCT_BURGER] = newBurgerRecipe; // PRODUCT_BURGER 现在通过这个新配方制作

    qDebug() << "SaladAssemblyStationItem::initializeRecipes - Recipes initialized. Keys:" << m_recipes.keys();
    qDebug() << "Salad recipe ('" << PRODUCT_SALAD << "'):" << saladRecipe;
    qDebug() << "New Burger recipe ('" << PRODUCT_BURGER << "'):" << newBurgerRecipe;
}

// tryAddIngredient, checkAvailableRecipes, produceAndTakeItem,
// getCurrentIngredientsOnBoard, updateVisualIngredients, clearVisualIngredients
// 这些方法的现有逻辑应该能够处理新的配方，因为它们是基于 m_recipes 动态工作的。
// 我们需要确保 tryAddIngredient 能够接受 PRODUCT_SALAD 作为一种有效的 ingredientName。
// 当前的 tryAddIngredient 逻辑：
// for (auto recipeIt = m_recipes.constBegin(); recipeIt != m_recipes.constEnd(); ++recipeIt) {
//     const QMap<QString, int>& currentRecipeIngredients = recipeIt.value();
//     if (currentRecipeIngredients.contains(ingredientName)) { ... }
// }
// 这个逻辑应该可以正确处理 PRODUCT_SALAD 作为汉堡配方的一部分。

bool SaladAssemblyStationItem::tryAddIngredient(const QString& ingredientName)
{
    qDebug() << "SaladAssemblyStationItem::tryAddIngredient - Attempting to add:" << ingredientName
             << "Current board:" << m_ingredientsOnBoard;

    bool canBeAdded = false;
    bool partOfAnyKnownRecipe = false;

    for (auto recipeIt = m_recipes.constBegin(); recipeIt != m_recipes.constEnd(); ++recipeIt) {
        const QMap<QString, int>& currentRecipeIngredients = recipeIt.value();
        if (currentRecipeIngredients.contains(ingredientName)) {
            partOfAnyKnownRecipe = true;
            int neededForThisRecipe = currentRecipeIngredients.value(ingredientName);
            int haveOnBoard = m_ingredientsOnBoard.value(ingredientName, 0);
            if (haveOnBoard < neededForThisRecipe) {
                canBeAdded = true;
                qDebug() << "Ingredient" << ingredientName << "can be added for recipe" << recipeIt.key()
                         << "(Need:" << neededForThisRecipe
                         << ", Have:" << haveOnBoard << ")";
                // 对于一个配料，只要它对至少一个需要它的配方来说是不足的，就应该允许添加。
                // 如果它对所有需要它的配方都已经足够了，那么 canBeAdded 会保持 false。
            } else {
                qDebug() << "Ingredient" << ingredientName << "is at/over limit for recipe" << recipeIt.key()
                << "(Need:" << neededForThisRecipe
                << ", Have:" << haveOnBoard << ")";
            }
        }
    }

    if (!partOfAnyKnownRecipe) {
        qDebug() << "SaladAssemblyStationItem::tryAddIngredient - Ingredient" << ingredientName << "is not part of any known recipe. Rejecting.";
        return false;
    }

    if (!canBeAdded) {
        qDebug() << "SaladAssemblyStationItem::tryAddIngredient - Ingredient" << ingredientName
                 << "cannot be added (all needing recipes are full for this ingredient, or it's not needed more). Rejecting.";
        return false;
    }

    m_ingredientsOnBoard[ingredientName]++;
    qDebug() << "SaladAssemblyStationItem::tryAddIngredient - Added" << ingredientName << ". New board:" << m_ingredientsOnBoard;
    updateVisualIngredients();
    checkAvailableRecipes();
    return true;
}

void SaladAssemblyStationItem::checkAvailableRecipes()
{
    m_producibleProductNames.clear();
    qDebug() << "SaladAssemblyStationItem::checkAvailableRecipes START - Current ingredients on board:" << m_ingredientsOnBoard;

    for (auto recipeIt = m_recipes.constBegin(); recipeIt != m_recipes.constEnd(); ++recipeIt) {
        const QString& productName = recipeIt.key();
        const QMap<QString, int>& currentRecipeRequirements = recipeIt.value();
        qDebug() << "SaladAssemblyStationItem::checkAvailableRecipes - Checking recipe for:" << productName << "Requires:" << currentRecipeRequirements;

        bool recipeMet = true;
        if (currentRecipeRequirements.isEmpty() && !productName.isEmpty()) {
            qWarning() << "Recipe for" << productName << "is empty, but product name exists. Skipping.";
            recipeMet = false;
        }

        for (auto ingredientIt = currentRecipeRequirements.constBegin(); ingredientIt != currentRecipeRequirements.constEnd(); ++ingredientIt) {
            const QString& requiredIngredientName = ingredientIt.key();
            int requiredAmount = ingredientIt.value();
            int amountOnBoard = m_ingredientsOnBoard.value(requiredIngredientName, 0);

            if (amountOnBoard < requiredAmount) {
                qDebug() << "Recipe for" << productName << "NOT MET. Ingredient:" << requiredIngredientName
                         << "Need:" << requiredAmount << "Have:" << amountOnBoard;
                recipeMet = false;
                break;
            } else {
                qDebug() << "Recipe for" << productName << "MET for ingredient:" << requiredIngredientName
                         << "Need:" << requiredAmount << "Have:" << amountOnBoard;
            }
        }

        if (recipeMet) {
            qDebug() << "RECIPE MET for" << productName << ". Adding to producible list.";
            m_producibleProductNames.append(productName);
        } else {
            qDebug() << "RECIPE NOT MET for" << productName;
        }
    }
    qDebug() << "SaladAssemblyStationItem::checkAvailableRecipes END - Final producible list:" << m_producibleProductNames;
}

QList<QString> SaladAssemblyStationItem::getProducibleItems() const
{
    return m_producibleProductNames;
}

QString SaladAssemblyStationItem::produceAndTakeItem(const QString& productNameToProduce)
{
    qDebug() << "SaladAssemblyStationItem::produceAndTakeItem called for:" << productNameToProduce
             << ". Current producible list:" << m_producibleProductNames
             << ". Current board:" << m_ingredientsOnBoard;

    if (!m_producibleProductNames.contains(productNameToProduce)) {
        qDebug() << "SaladAssemblyStation: Cannot produce '" << productNameToProduce << "' with current ingredients. Producible list:" << m_producibleProductNames;
        return QString();
    }

    if (!m_recipes.contains(productNameToProduce)) {
        qWarning() << "SaladAssemblyStation: Recipe for '" << productNameToProduce << "' not found!";
        return QString();
    }
    const QMap<QString, int>& recipe = m_recipes.value(productNameToProduce);
    qDebug() << "SaladAssemblyStation: Using recipe for" << productNameToProduce << "Recipe details:" << recipe;


    for (auto it = recipe.constBegin(); it != recipe.constEnd(); ++it) {
        m_ingredientsOnBoard[it.key()] -= it.value();
        if (m_ingredientsOnBoard.value(it.key()) <= 0) {
            m_ingredientsOnBoard.remove(it.key());
        }
    }
    qDebug() << "SaladAssemblyStation: Ingredients after producing" << productNameToProduce << ":" << m_ingredientsOnBoard;

    clearVisualIngredients();
    updateVisualIngredients();
    checkAvailableRecipes(); // Update producible list

    qDebug() << "SaladAssemblyStation: Successfully produced and took" << productNameToProduce
             << ". New producible list:" << m_producibleProductNames;
    return productNameToProduce;
}

QMap<QString, int> SaladAssemblyStationItem::getCurrentIngredientsOnBoard() const
{
    return m_ingredientsOnBoard;
}

void SaladAssemblyStationItem::updateVisualIngredients() {
    clearVisualIngredients();


    qreal nextX = 5;
    qreal nextY = 5;
    const qreal spacing = 2;
    const int itemsPerRow = 3;
    int currentItemInRow = 0;
    const qreal itemVisualSize = 20.0;

    for (auto it = m_ingredientsOnBoard.constBegin(); it != m_ingredientsOnBoard.constEnd(); ++it) {
        QString itemName = it.key();
        int count = it.value();
        QString imagePath = Player::getImagePathForItem(itemName);
        if (imagePath.isEmpty()) {
            continue;
        }

        for (int i = 0; i < count; ++i) {
            QPixmap pixmap(imagePath);
            if (pixmap.isNull()) {
                continue;
            }

            QGraphicsPixmapItem* visualIngredient = new QGraphicsPixmapItem(pixmap.scaled(itemVisualSize, itemVisualSize, Qt::KeepAspectRatio, Qt::SmoothTransformation), this);
            visualIngredient->setPos(nextX, nextY);
            m_visualIngredients.append(visualIngredient);

            nextX += itemVisualSize + spacing;
            currentItemInRow++;
            if (currentItemInRow >= itemsPerRow) {
                currentItemInRow = 0;
                nextX = 5;
                nextY += itemVisualSize + spacing;
            }
        }
    }
}

void SaladAssemblyStationItem::clearVisualIngredients() {
    for (QGraphicsPixmapItem* item : m_visualIngredients) {
        if (item && item->scene()) {
            item->scene()->removeItem(item);
        }
        delete item;
    }
    m_visualIngredients.clear();
}
