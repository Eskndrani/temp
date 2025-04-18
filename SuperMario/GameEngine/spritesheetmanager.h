#ifndef SPRITESHEETMANAGER_H
#define SPRITESHEETMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include "spritesheet.h"
#include "animation.h"
#include "sprite.h"

// This utility class helps with managing and extracting sprites from sprite sheets
class SpriteSheetManager : public QObject
{
    Q_OBJECT

public:
    static SpriteSheetManager& instance();

    // Load a sprite sheet and cache it by name
    bool loadSpriteSheet(const QString &name, const QString &path);

    // Get a cached sprite sheet
    SpriteSheet* getSpriteSheet(const QString &name) const;

    // Create an animation from a row of sprites
    Animation* createRowAnimation(const QString &sheetName, int startX, int startY,
                                  int width, int height, int frameCount,
                                  int fps = 10, bool loop = true, int spacing = 0);

    // Create a sprite with multiple animations from a character sheet
    Sprite* createCharacterSprite(const QString &sheetName,
                                  const QMap<QString, QRect> &animationFrames,
                                  const QMap<QString, int> &frameCountsPerRow,
                                  const QMap<QString, int> &frameRates,
                                  int spacing = 0);

    // Create a simple sprite with a single animation
    Sprite* createSimpleSprite(const QString &sheetName, int x, int y, int width, int height,
                               int frameCount = 1, int fps = 10, bool loop = true, int spacing = 0);

    // Extract specific character from a multi-character sheet
    SpriteSheet* extractCharacterSheet(const QString &sheetName, int characterIndex,
                                       int characterWidth, int characterHeight);

    // Game-specific character sprite loaders
    Sprite* loadCharacterSprite(const QString &sheetName, int characterRow, int paletteRow,
                                int frameWidth, int frameHeight, int framesCount);

    Sprite* loadMarioSprite(const QString &sheetName, int paletteIndex);

    // Tile set loading
    QMap<int, QPixmap> loadTileSet(const QString &sheetName, int tileWidth, int tileHeight);

    // Cleanup resources
    void cleanup();

private:
    SpriteSheetManager(QObject *parent = nullptr);
    ~SpriteSheetManager();

    // Private constructor/destructor for singleton
    SpriteSheetManager(const SpriteSheetManager&) = delete;
    SpriteSheetManager& operator=(const SpriteSheetManager&) = delete;

    QMap<QString, SpriteSheet*> m_spriteSheets;
};

#endif // SPRITESHEETMANAGER_H
