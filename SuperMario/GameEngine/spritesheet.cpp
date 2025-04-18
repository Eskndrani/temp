#include "spritesheet.h"
#include <QDebug>
#include <QImage>

SpriteSheet::SpriteSheet() : m_isLoaded(false)
{
}

SpriteSheet::SpriteSheet(const QString &path) : m_isLoaded(false)
{
    loadFromFile(path);
}

SpriteSheet::SpriteSheet(const QPixmap &pixmap) : m_isLoaded(false)
{
    loadFromPixmap(pixmap);
}

SpriteSheet::~SpriteSheet()
{
    // No heap allocations to clean up
}

bool SpriteSheet::loadFromFile(const QString &path)
{
    m_isLoaded = m_spriteSheet.load(path);
    if (!m_isLoaded) {
        qWarning() << "Failed to load sprite sheet from:" << path; // Changed to qWarning for better visibility
    }
    return m_isLoaded;
}

bool SpriteSheet::loadFromPixmap(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        m_isLoaded = false;
        return false;
    }

    m_spriteSheet = pixmap;
    m_isLoaded = true;
    return true;
}

QPixmap SpriteSheet::getSprite(int x, int y, int width, int height) const
{
    if (!m_isLoaded) {
        return QPixmap();
    }

    // Ensure the region is within the bounds of the sprite sheet
    if (x >= 0 && y >= 0 && x + width <= m_spriteSheet.width() && y + height <= m_spriteSheet.height()) {
        return m_spriteSheet.copy(x, y, width, height);
    } else {
        qWarning() << "Sprite region out of bounds: (" << x << "," << y << "," << width << "," << height << ")";
        return QPixmap();
    }
}

QVector<QPixmap> SpriteSheet::getSprites(int startX, int startY, int width, int height,
                                         int columns, int count, int spacing) const
{
    QVector<QPixmap> sprites;
    if (!m_isLoaded) {
        return sprites;
    }

    sprites.reserve(count); // Pre-allocate for better performance

    for (int i = 0; i < count; ++i) {
        int col = i % columns;
        int row = i / columns;
        int x = startX + col * (width + spacing);
        int y = startY + row * (height + spacing);

        QPixmap sprite = getSprite(x, y, width, height);
        if (!sprite.isNull()) {
            sprites.push_back(sprite);
        }
    }

    return sprites;
}

QVector<QPixmap> SpriteSheet::getSpriteRow(int startX, int startY, int width, int height,
                                           int count, int spacing) const
{
    QVector<QPixmap> sprites;
    if (!m_isLoaded) {
        return sprites;
    }

    sprites.reserve(count); // Pre-allocate for better performance

    for (int i = 0; i < count; ++i) {
        int x = startX + i * (width + spacing);

        QPixmap sprite = getSprite(x, startY, width, height);
        if (!sprite.isNull()) {
            sprites.push_back(sprite);
        }
    }

    return sprites;
}

QVector<QPixmap> SpriteSheet::getSpriteColumn(int startX, int startY, int width, int height,
                                              int count, int spacing) const
{
    QVector<QPixmap> sprites;
    if (!m_isLoaded) {
        return sprites;
    }

    sprites.reserve(count); // Pre-allocate for better performance

    for (int i = 0; i < count; ++i) {
        int y = startY + i * (height + spacing);

        QPixmap sprite = getSprite(startX, y, width, height);
        if (!sprite.isNull()) {
            sprites.push_back(sprite);
        }
    }

    return sprites;
}

QVector<QRect> SpriteSheet::detectSprites(int minWidth, int minHeight) const
{
    QVector<QRect> sprites;
    if (!m_isLoaded) {
        return sprites;
    }

    // Convert sheet to image to access pixel data
    QImage image = m_spriteSheet.toImage();

    // Simple sprite detection logic:
    // Scan the image for non-transparent regions
    int width = image.width();
    int height = image.height();

    // Create visited array only once (more efficient for large images)
    QVector<QVector<bool>> visited(width, QVector<bool>(height, false));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Skip if already visited or pixel is transparent
            if (visited[x][y] || qAlpha(image.pixel(x, y)) == 0) {
                continue;
            }

            // Found a non-transparent pixel - start flood fill to find the sprite bounds
            int minX = x, maxX = x, minY = y, maxY = y;
            QQueue<QPoint> queue;
            queue.enqueue(QPoint(x, y));
            visited[x][y] = true;

            while (!queue.isEmpty()) {
                QPoint p = queue.dequeue();

                // Update bounds
                minX = qMin(minX, p.x());
                maxX = qMax(maxX, p.x());
                minY = qMin(minY, p.y());
                maxY = qMax(maxY, p.y());

                // Check neighbors (4-way connectivity)
                static const int dx[] = {-1, 0, 1, 0};
                static const int dy[] = {0, -1, 0, 1};

                for (int i = 0; i < 4; ++i) {
                    int nx = p.x() + dx[i];
                    int ny = p.y() + dy[i];

                    if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                        !visited[nx][ny] && qAlpha(image.pixel(nx, ny)) > 0) {
                        visited[nx][ny] = true;
                        queue.enqueue(QPoint(nx, ny));
                    }
                }
            }

            // If the sprite is big enough, add it
            int spriteWidth = maxX - minX + 1;
            int spriteHeight = maxY - minY + 1;

            if (spriteWidth >= minWidth && spriteHeight >= minHeight) {
                sprites.append(QRect(minX, minY, spriteWidth, spriteHeight));
            }
        }
    }

    return sprites;
}

SpriteSheet* SpriteSheet::extractSubSheet(int x, int y, int width, int height) const
{
    if (!m_isLoaded) {
        return nullptr;
    }

    QPixmap subPixmap = getSprite(x, y, width, height);
    if (subPixmap.isNull()) {
        return nullptr;
    }

    SpriteSheet* subSheet = new SpriteSheet();
    if (!subSheet->loadFromPixmap(subPixmap)) {
        delete subSheet; // Clean up if loading fails
        return nullptr;
    }
    return subSheet;
}

bool SpriteSheet::isLoaded() const
{
    return m_isLoaded;
}

QSize SpriteSheet::size() const
{
    return m_spriteSheet.size();
}

QPixmap SpriteSheet::pixmap() const
{
    return m_spriteSheet;
}
