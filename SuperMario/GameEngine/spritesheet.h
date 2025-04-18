#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <QPixmap>
#include <QVector>
#include <QSize>
#include <QRect>
#include <QString>
#include <QPainter>
#include <QDir>
#include <QQueue>
#include <QPoint>

class SpriteSheet
{
public:
    SpriteSheet();
    explicit SpriteSheet(const QString &path);
    explicit SpriteSheet(const QPixmap &pixmap);
    ~SpriteSheet();

    // Load a sprite sheet from a file path
    bool loadFromFile(const QString &path);

    // Load from existing pixmap
    bool loadFromPixmap(const QPixmap &pixmap);

    // Extract a single sprite from the sheet
    QPixmap getSprite(int x, int y, int width, int height) const;

    // Extract multiple sprites using a uniform grid
    QVector<QPixmap> getSprites(int startX, int startY, int width, int height,
                                int columns, int count, int spacing = 0) const;

    // Extract a row of sprites
    QVector<QPixmap> getSpriteRow(int startX, int startY, int width, int height,
                                  int count, int spacing = 0) const;

    // Extract a column of sprites
    QVector<QPixmap> getSpriteColumn(int startX, int startY, int width, int height,
                                     int count, int spacing = 0) const;

    // Auto-detect sprites in a sheet based on non-transparent pixels
    QVector<QRect> detectSprites(int minWidth = 8, int minHeight = 8) const;

    // Extract a sub-region as a new sprite sheet
    SpriteSheet* extractSubSheet(int x, int y, int width, int height) const;

    // Check if sprite sheet is loaded
    bool isLoaded() const;

    // Get dimensions
    QSize size() const;

    // Get the raw pixmap
    QPixmap pixmap() const;

private:
    QPixmap m_spriteSheet;
    bool m_isLoaded;
};

#endif // SPRITESHEET_H
