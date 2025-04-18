#include "spritesheetmanager.h"
#include <QDebug>

SpriteSheetManager& SpriteSheetManager::instance()
{
    static SpriteSheetManager instance;
    return instance;
}

SpriteSheetManager::SpriteSheetManager(QObject *parent) : QObject(parent)
{
}

SpriteSheetManager::~SpriteSheetManager()
{
    cleanup();
}

void SpriteSheetManager::cleanup()
{
    // Clean up sprite sheets
    for (auto sheet : m_spriteSheets.values()) {
        delete sheet;
    }
    m_spriteSheets.clear();
}

bool SpriteSheetManager::loadSpriteSheet(const QString &name, const QString &path)
{
    // Check if already loaded
    if (m_spriteSheets.contains(name)) {
        return true;
    }

    // Create new sprite sheet
    SpriteSheet *sheet = new SpriteSheet();
    if (!sheet->loadFromFile(path)) {
        delete sheet;
        return false;
    }

    // Add to cache
    m_spriteSheets[name] = sheet;
    return true;
}

SpriteSheet* SpriteSheetManager::getSpriteSheet(const QString &name) const
{
    return m_spriteSheets.value(name, nullptr);
}

Animation* SpriteSheetManager::createRowAnimation(const QString &sheetName, int startX, int startY,
                                                  int width, int height, int frameCount,
                                                  int fps, bool loop, int spacing)
{
    SpriteSheet* sheet = getSpriteSheet(sheetName);
    if (!sheet) {
        qDebug() << "Sheet not found:" << sheetName;
        return nullptr;
    }

    Animation* animation = new Animation();
    animation->setFrameRate(fps);
    animation->loopAnimation(loop);

    QVector<QPixmap> frames = sheet->getSpriteRow(startX, startY, width, height, frameCount, spacing);
    for (const QPixmap& frame : frames) {
        animation->addFrame(frame);
    }

    return animation;
}

Sprite* SpriteSheetManager::createCharacterSprite(const QString &sheetName,
                                                  const QMap<QString, QRect> &animationFrames,
                                                  const QMap<QString, int> &frameCountsPerRow,
                                                  const QMap<QString, int> &frameRates,
                                                  int spacing)
{
    SpriteSheet* sheet = getSpriteSheet(sheetName);
    if (!sheet) {
        qDebug() << "Sheet not found:" << sheetName;
        return nullptr;
    }

    Sprite* sprite = new Sprite();

    // For each animation state
    for (auto animName = animationFrames.begin(); animName != animationFrames.end(); ++animName) {
        QString name = animName.key();
        QRect frameRect = animName.value();

        // Get frames count and rate
        int frameCount = frameCountsPerRow.value(name, 1);
        int frameRate = frameRates.value(name, 10);

        // Create animation
        Animation* anim = new Animation();
        anim->setFrameRate(frameRate);
        anim->loopAnimation(true); // Most character animations loop

        // Get frames
        QVector<QPixmap> frames = sheet->getSpriteRow(
            frameRect.x(), frameRect.y(),
            frameRect.width(), frameRect.height(),
            frameCount, spacing
            );

        for (const QPixmap& frame : frames) {
            anim->addFrame(frame);
        }

        // Add to sprite
        sprite->addAnimation(name, anim);
    }

    return sprite;
}

Sprite* SpriteSheetManager::createSimpleSprite(const QString &sheetName, int x, int y,
                                               int width, int height, int frameCount,
                                               int fps, bool loop, int spacing)
{
    Animation* anim = createRowAnimation(sheetName, x, y, width, height, frameCount, fps, loop, spacing);
    if (!anim) {
        return nullptr;
    }

    Sprite* sprite = new Sprite();
    sprite->addAnimation("default", anim);
    sprite->playAnimation("default");

    return sprite;
}

SpriteSheet* SpriteSheetManager::extractCharacterSheet(const QString &sheetName, int characterIndex,
                                                       int characterWidth, int characterHeight)
{
    SpriteSheet* sheet = getSpriteSheet(sheetName);
    if (!sheet) {
        qDebug() << "Sheet not found:" << sheetName;
        return nullptr;
    }

    // Calculate character position in the sheet
    QSize sheetSize = sheet->size();
    int charactersPerRow = sheetSize.width() / characterWidth;

    int row = characterIndex / charactersPerRow;
    int col = characterIndex % charactersPerRow;

    int x = col * characterWidth;
    int y = row * characterHeight;

    // Extract the character region as a new sprite sheet
    return sheet->extractSubSheet(x, y, characterWidth, characterHeight);
}

Sprite* SpriteSheetManager::loadCharacterSprite(const QString &sheetName, int characterRow, int paletteRow,
                                                int frameWidth, int frameHeight, int framesCount)
{
    SpriteSheet* sheet = getSpriteSheet(sheetName);
    if (!sheet) {
        qDebug() << "Failed to get sprite sheet:" << sheetName;
        return nullptr;
    }

    // Create a new sprite
    Sprite* characterSprite = new Sprite();

    // Define different animation states
    QStringList animationStates = {"idle", "walk", "run", "jump", "fall"};
    QMap<QString, int> frameStartPositions;
    QMap<QString, int> frameCountsMap;
    QMap<QString, int> frameSpeeds;

    // Configure animations based on the sprite sheet layout
    frameStartPositions["idle"] = 0;
    frameStartPositions["walk"] = 1;
    frameStartPositions["run"] = 5;
    frameStartPositions["jump"] = 10;
    frameStartPositions["fall"] = 12;

    frameCountsMap["idle"] = 1;
    frameCountsMap["walk"] = 3;
    frameCountsMap["run"] = 3;
    frameCountsMap["jump"] = 1;
    frameCountsMap["fall"] = 1;

    frameSpeeds["idle"] = 5;
    frameSpeeds["walk"] = 10;
    frameSpeeds["run"] = 15;
    frameSpeeds["jump"] = 5;
    frameSpeeds["fall"] = 5;

    // Calculate the Y position based on the character row and palette row
    int yOffset = characterRow * frameHeight;
    if (paletteRow > 0) {
        // If we're selecting a specific palette (color variation)
        yOffset = paletteRow * sheet->size().height() / 6;  // Assuming there are 6 palettes in the sheet
    }

    // Create animations for each state
    for (const QString& state : animationStates) {
        if (frameStartPositions.contains(state) && frameCountsMap.contains(state)) {
            int startFrame = frameStartPositions[state];
            int frameCount = frameCountsMap[state];
            int fps = frameSpeeds.value(state, 10);

            // Create animation for this state
            Animation* anim = createRowAnimation(
                sheetName,
                startFrame * frameWidth, yOffset,
                frameWidth, frameHeight,
                frameCount, fps, true, 0
                );

            if (anim) {
                characterSprite->addAnimation(state, anim);
            } else {
                qDebug() << "Failed to create animation for state:" << state;
            }
        }
    }

    // Set default animation
    characterSprite->playAnimation("idle");

    return characterSprite;
}

Sprite* SpriteSheetManager::loadMarioSprite(const QString &sheetName, int paletteIndex)
{
    // Load the sprite sheet if not already loaded
    if (!m_spriteSheets.contains(sheetName)) {
        if (!loadSpriteSheet(sheetName, sheetName)) {
            qDebug() << "Failed to load Mario sprite sheet";
            return nullptr;
        }
    }

    // Setup Mario animations with proper configurations
    QMap<QString, QRect> marioAnimations;
    QMap<QString, int> marioFrameCounts;
    QMap<QString, int> marioFrameRates;

    // Based on the sprite sheet layout
    // Small Mario animations
    marioAnimations["idle"] = QRect(0, paletteIndex * 32, 16, 16);
    marioAnimations["walk"] = QRect(16, paletteIndex * 32, 16, 16);
    marioAnimations["run"] = QRect(48, paletteIndex * 32, 16, 16);
    marioAnimations["jump"] = QRect(80, paletteIndex * 32, 16, 16);
    marioAnimations["die"] = QRect(96, paletteIndex * 32, 16, 16);

    marioFrameCounts["idle"] = 1;
    marioFrameCounts["walk"] = 3;
    marioFrameCounts["run"] = 3;
    marioFrameCounts["jump"] = 1;
    marioFrameCounts["die"] = 1;

    marioFrameRates["idle"] = 5;
    marioFrameRates["walk"] = 10;
    marioFrameRates["run"] = 15;
    marioFrameRates["jump"] = 5;
    marioFrameRates["die"] = 5;

    return createCharacterSprite(sheetName, marioAnimations, marioFrameCounts, marioFrameRates);
}

QMap<int, QPixmap> SpriteSheetManager::loadTileSet(const QString &sheetName, int tileWidth, int tileHeight)
{
    QMap<int, QPixmap> tiles;

    // Get or load the sheet
    SpriteSheet* sheet = getSpriteSheet(sheetName);
    if (!sheet) {
        if (!loadSpriteSheet(sheetName, sheetName)) {
            qDebug() << "Failed to load tile sheet:" << sheetName;
            return tiles;
        }
        sheet = getSpriteSheet(sheetName);
    }

    // Get sheet dimensions
    QSize sheetSize = sheet->size();
    int tilesPerRow = sheetSize.width() / tileWidth;
    int rowCount = sheetSize.height() / tileHeight;

    // Extract all tiles
    int tileIndex = 0;
    for (int row = 0; row < rowCount; row++) {
        for (int col = 0; col < tilesPerRow; col++) {
            QPixmap tile = sheet->getSprite(col * tileWidth, row * tileHeight, tileWidth, tileHeight);

            // Skip completely transparent tiles
            if (!tile.isNull()) {
                QImage img = tile.toImage();
                bool hasVisiblePixel = false;

                // Check if the tile has any non-transparent pixels
                for (int y = 0; y < tileHeight && !hasVisiblePixel; y++) {
                    for (int x = 0; x < tileWidth && !hasVisiblePixel; x++) {
                        if (qAlpha(img.pixel(x, y)) > 0) {
                            hasVisiblePixel = true;
                            break;
                        }
                    }
                }

                if (hasVisiblePixel) {
                    tiles[tileIndex] = tile;
                }
            }
            tileIndex++;
        }
    }

    return tiles;
}
