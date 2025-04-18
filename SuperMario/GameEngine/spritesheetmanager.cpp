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
    // Validate inputs
    if (name.isEmpty() || path.isEmpty()) {
        qWarning() << "Invalid parameters for loadSpriteSheet - name:" << name << "path:" << path;
        return false;
    }

    // Check if already loaded
    if (m_spriteSheets.contains(name)) {
        return true;
    }

    // Create new sprite sheet
    SpriteSheet *sheet = new SpriteSheet();
    if (!sheet->loadFromFile(path)) {
        qWarning() << "Failed to load sprite sheet from path:" << path;
        delete sheet;
        return false;
    }

    // Add to cache
    m_spriteSheets[name] = sheet;
    return true;
}

SpriteSheet* SpriteSheetManager::getSpriteSheet(const QString &name) const
{
    if (!m_spriteSheets.contains(name)) {
        qWarning() << "Sprite sheet not found:" << name;
        return nullptr;
    }
    return m_spriteSheets.value(name);
}

Animation* SpriteSheetManager::createRowAnimation(const QString &sheetName, int startX, int startY,
                                                  int width, int height, int frameCount,
                                                  int fps, bool loop, int spacing)
{
    // Parameter validation
    if (sheetName.isEmpty() || width <= 0 || height <= 0 || frameCount <= 0 || fps <= 0) {
        qWarning() << "Invalid parameters for createRowAnimation";
        return nullptr;
    }

    SpriteSheet* sheet = getSpriteSheet(sheetName);
    if (!sheet) {
        qWarning() << "Sheet not found:" << sheetName;
        return nullptr;
    }

    Animation* animation = new Animation();
    animation->setFrameRate(fps);
    animation->loopAnimation(loop);

    QVector<QPixmap> frames = sheet->getSpriteRow(startX, startY, width, height, frameCount, spacing);
    if (frames.isEmpty()) {
        qWarning() << "Failed to extract frames from sprite sheet:" << sheetName;
        delete animation;
        return nullptr;
    }

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
    // Parameter validation
    if (sheetName.isEmpty() || animationFrames.isEmpty()) {
        qWarning() << "Invalid parameters for createCharacterSprite";
        return nullptr;
    }

    SpriteSheet* sheet = getSpriteSheet(sheetName);
    if (!sheet) {
        qWarning() << "Sheet not found:" << sheetName;
        return nullptr;
    }

    Sprite* sprite = new Sprite();
    bool anyAnimationAdded = false;

    // For each animation state
    for (auto animName = animationFrames.begin(); animName != animationFrames.end(); ++animName) {
        QString name = animName.key();
        QRect frameRect = animName.value();

        // Validate frame rectangle
        if (frameRect.width() <= 0 || frameRect.height() <= 0) {
            qWarning() << "Invalid frame rectangle for animation:" << name;
            continue;
        }

        // Get frames count and rate with default values
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

        if (frames.isEmpty()) {
            qWarning() << "No frames extracted for animation:" << name;
            delete anim;
            continue;
        }

        for (const QPixmap& frame : frames) {
            if (frame.isNull()) {
                qWarning() << "Null frame in animation:" << name;
                continue;
            }
            anim->addFrame(frame);
        }

        // Add to sprite if animation has frames
        if (anim->getFrameCount() > 0) {
            sprite->addAnimation(name, anim);
            anyAnimationAdded = true;
        } else {
            qWarning() << "Animation has no frames:" << name;
            delete anim;
        }
    }

    // If no animations were added, clean up and return null
    if (!anyAnimationAdded) {
        qWarning() << "No valid animations created for sprite from sheet:" << sheetName;
        delete sprite;
        return nullptr;
    }

    return sprite;
}

Sprite* SpriteSheetManager::createSimpleSprite(const QString &sheetName, int x, int y,
                                               int width, int height, int frameCount,
                                               int fps, bool loop, int spacing)
{
    Animation* anim = createRowAnimation(sheetName, x, y, width, height, frameCount, fps, loop, spacing);
    if (!anim) {
        qWarning() << "Failed to create animation for simple sprite";
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
    // Parameter validation
    if (sheetName.isEmpty() || characterIndex < 0 || characterWidth <= 0 || characterHeight <= 0) {
        qWarning() << "Invalid parameters for extractCharacterSheet";
        return nullptr;
    }

    SpriteSheet* sheet = getSpriteSheet(sheetName);
    if (!sheet) {
        qWarning() << "Sheet not found:" << sheetName;
        return nullptr;
    }

    // Check if the sheet is big enough
    QSize sheetSize = sheet->size();
    if (sheetSize.width() < characterWidth || sheetSize.height() < characterHeight) {
        qWarning() << "Sheet size is too small for character dimensions";
        return nullptr;
    }

    // Calculate character position in the sheet
    int charactersPerRow = sheetSize.width() / characterWidth;
    if (charactersPerRow == 0) {
        qWarning() << "Invalid character width - larger than sheet width";
        return nullptr;
    }

    int row = characterIndex / charactersPerRow;
    int col = characterIndex % charactersPerRow;

    int x = col * characterWidth;
    int y = row * characterHeight;

    // Check if the character position is within the sheet
    if (x + characterWidth > sheetSize.width() || y + characterHeight > sheetSize.height()) {
        qWarning() << "Character position is outside sheet boundaries";
        return nullptr;
    }

    // Extract the character region as a new sprite sheet
    SpriteSheet* result = sheet->extractSubSheet(x, y, characterWidth, characterHeight);
    if (!result) {
        qWarning() << "Failed to extract character sub-sheet";
    }
    return result;
}

Sprite* SpriteSheetManager::loadCharacterSprite(const QString &sheetName, int characterRow, int paletteRow,
                                               int frameWidth, int frameHeight, int framesCount)
{
    // Parameter validation
    if (sheetName.isEmpty() || characterRow < 0 || paletteRow < 0 || 
        frameWidth <= 0 || frameHeight <= 0 || framesCount <= 0) {
        qWarning() << "Invalid parameters for loadCharacterSprite";
        return nullptr;
    }

    SpriteSheet* sheet = getSpriteSheet(sheetName);
    if (!sheet) {
        qWarning() << "Failed to get sprite sheet:" << sheetName;
        return nullptr;
    }

    // Check if the sheet is big enough
    QSize sheetSize = sheet->size();
    if (sheetSize.width() < frameWidth || sheetSize.height() < frameHeight) {
        qWarning() << "Sheet size is too small for frame dimensions";
        return nullptr;
    }

    // Create a new sprite
    Sprite* characterSprite = new Sprite();
    bool anyAnimationAdded = false;

    // Define different animation states with their configurations
    struct AnimConfig {
        QString name;
        int startFrame;
        int frameCount;
        int fps;
    };

    QVector<AnimConfig> animConfigs = {
        {"idle", 0, 1, 5},
        {"walk", 1, 3, 10},
        {"run", 5, 3, 15},
        {"jump", 10, 1, 5},
        {"fall", 12, 1, 5}
    };

    // Calculate the Y position based on the character row and palette row
    int yOffset = characterRow * frameHeight;
    
    // Avoid making assumptions about exact layout - use a parameter for palette height multiplier
    // Default to 1/6 of sheet height if palette row > 0
    if (paletteRow > 0) {
        int paletteHeight = sheet->size().height() / 6;  // Calculate palette height
        yOffset = paletteRow * paletteHeight;
    }

    // Create animations for each state
    for (const AnimConfig& config : animConfigs) {
        // Check if we have enough frames in the sheet for this animation
        if (config.startFrame + config.frameCount > framesCount) {
            qWarning() << "Not enough frames for animation:" << config.name;
            continue;
        }

        // Create animation
        Animation* anim = createRowAnimation(
            sheetName,
            config.startFrame * frameWidth, yOffset,
            frameWidth, frameHeight,
            config.frameCount, config.fps, true, 0
        );

        if (anim) {
            characterSprite->addAnimation(config.name, anim);
            anyAnimationAdded = true;
        } else {
            qWarning() << "Failed to create animation for state:" << config.name;
        }
    }

    // If no animations were added, clean up and return null
    if (!anyAnimationAdded) {
        qWarning() << "No valid animations created for character sprite";
        delete characterSprite;
        return nullptr;
    }

    // Set default animation
    characterSprite->playAnimation("idle");

    return characterSprite;
}

Sprite* SpriteSheetManager::loadMarioSprite(const QString &sheetName, int paletteIndex)
{
    // Parameter validation
    if (sheetName.isEmpty() || paletteIndex < 0) {
        qWarning() << "Invalid parameters for loadMarioSprite";
        return nullptr;
    }

    // Load the sprite sheet if not already loaded
    if (!m_spriteSheets.contains(sheetName)) {
        // Here we're using sheetName both as the key and the file path
        // This could be confusing - consider separating these parameters
        if (!loadSpriteSheet(sheetName, sheetName)) {
            qWarning() << "Failed to load Mario sprite sheet";
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

    // Create the sprite
    Sprite* sprite = createCharacterSprite(sheetName, marioAnimations, marioFrameCounts, marioFrameRates);
    if (!sprite) {
        qWarning() << "Failed to create Mario sprite";
    }
    
    return sprite;
}

QMap<int, QPixmap> SpriteSheetManager::loadTileSet(const QString &sheetName, int tileWidth, int tileHeight)
{
    QMap<int, QPixmap> tiles;
    
    // Parameter validation
    if (sheetName.isEmpty() || tileWidth <= 0 || tileHeight <= 0) {
        qWarning() << "Invalid parameters for loadTileSet";
        return tiles;
    }

    // Get or load the sheet
    SpriteSheet* sheet = getSpriteSheet(sheetName);
    if (!sheet) {
        if (!loadSpriteSheet(sheetName, sheetName)) {
            qWarning() << "Failed to load tile sheet:" << sheetName;
            return tiles;
        }
        sheet = getSpriteSheet(sheetName);
    }

    // Get sheet dimensions
    QSize sheetSize = sheet->size();
    if (sheetSize.width() < tileWidth || sheetSize.height() < tileHeight) {
        qWarning() << "Sheet size is too small for tile dimensions";
        return tiles;
    }

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
