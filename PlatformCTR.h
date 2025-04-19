#ifndef _PLATFORMCTR_H
#define _PLATFORMCTR_H

#define PlatformClass PlatformCTR

#include <SDL2/SDL.h>
#include <fstream>
#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include "Platform.h"

enum
{
    DRAWRECT = 1,
};

struct Sprite
{
    Sprite() {tint.corners[0].color = 0xffffffff; tint.corners[1].color = 0xffffffff; tint.corners[2].color = 0xffffffff; tint.corners[3].color = 0xffffffff;}
    union 
    {
        struct 
        {
            C2D_ImageTint tint;
            C2D_Sprite sprite;
        };
        struct 
        {
            SDL_Rect rect;
            u32 colour;
        };
    };
    u8 flags = 0;

    bool operator==(const Sprite& rhs) { return this->flags == rhs.flags && this->rect.x == rhs.rect.x && this->rect.y == rhs.rect.y && this->rect.w == rhs.rect.w && this->rect.h == rhs.rect.h; } 
};

class PlatformCTR : public Platform {
public:
    PlatformCTR();
    virtual ~PlatformCTR();

    virtual uint8_t* standardControls() const;
    virtual void setInterrupt(void (*interrupt)(void));
    virtual int framesPerSecond();
    virtual void chrout(uint8_t);
    virtual uint8_t readKeyboard();
    virtual void keyRepeat();
    virtual void clearKeyBuffer();
    virtual bool isKeyOrJoystickPressed(bool gamepad);
    virtual uint16_t readJoystick(bool gamepad);
    virtual void loadMap(Map map, uint8_t* destination);
    virtual uint8_t* loadTileset();
#ifdef PLATFORM_IMAGE_SUPPORT
    virtual void displayImage(Image image);
#endif
    virtual void generateTiles(uint8_t* tileData, uint8_t* tileAttributes);
#ifndef PLATFORM_IMAGE_BASED_TILES
    virtual void updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles);
#endif
    virtual void renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent);
    virtual void renderTiles(uint8_t backgroundTile, uint8_t foregroundTile, uint16_t x, uint16_t y, uint8_t backgroundVariant, uint8_t foregroundVariant);
#ifdef PLATFORM_IMAGE_SUPPORT
    virtual void renderItem(uint8_t item, uint16_t x, uint16_t y);
    virtual void renderKey(uint8_t key, uint16_t x, uint16_t y);
    virtual void renderHealth(uint8_t health, uint16_t x, uint16_t y);
    virtual void renderFace(uint8_t face, uint16_t x, uint16_t y);
#endif
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    virtual void renderLiveMap(uint8_t* map);
    virtual void renderLiveMapTile(uint8_t* map, uint8_t x, uint8_t y);
    virtual void renderLiveMapUnits(uint8_t* map, uint8_t* unitTypes, uint8_t* unitX, uint8_t* unitY, uint8_t playerColor, bool showRobots);
#endif
    virtual void showCursor(uint16_t x, uint16_t y);
    virtual void hideCursor();
    virtual void setCursorShape(CursorShape shape);

    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height);
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    virtual void startShakeScreen();
    virtual void shakeScreen();
    virtual void stopShakeScreen();

    virtual void startFadeScreen(uint16_t color, uint16_t intensity);
    virtual void fadeScreen(uint16_t intensity, bool immediate);
    virtual void stopFadeScreen();

    virtual void writeToScreenMemory(address_t address, uint8_t value);
    virtual void writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset);
#ifdef PLATFORM_MODULE_BASED_AUDIO
    virtual void loadModule(Module module);
    virtual void playModule(Module module);
    virtual void pauseModule();
    virtual void stopModule();
    virtual void playSample(uint8_t sample);
    virtual void stopSample();
#else
    virtual void playNote(uint8_t note);
    virtual void stopNote();
#endif
    virtual void renderFrame(bool waitForNextFrame);

    uint8_t SCREEN_MEMORY[SCREEN_WIDTH_IN_CHARACTERS * SCREEN_HEIGHT_IN_CHARACTERS];
    bool petMode = false;
private:
    uint32_t load(const char* filename, uint8_t* destination, uint32_t size);
    void decodeTiles(u32* tileMap, int x, int y, bool queue);
#ifdef PLATFORM_MODULE_BASED_AUDIO
    void undeltaSamples(uint8_t* module, uint32_t moduleSize);
    void setSampleData(uint8_t* module);
#endif
#ifdef PLATFORM_SPRITE_SUPPORT
    void renderSprite(uint8_t sprite, uint16_t x, uint16_t y);
#endif
#ifdef PLATFORM_IMAGE_BASED_TILES
    void renderAnimTile(uint8_t animTile, uint16_t x, uint16_t y);
#endif
    static void audioCallback(void* data, uint8_t* stream, int bytes);
    void (*interrupt)(void);
    SDL_AudioSpec audioSpec;
    SDL_AudioDeviceID audioDeviceID;
    C3D_RenderTarget* topScreen;
    C3D_RenderTarget* bottomScreen;
    C2D_SpriteSheet fontSpritesheet;
    C2D_SpriteSheet tileSpritesheet;
    C2D_SpriteSheet petTilesSpritesheet;
    C2D_SpriteSheet imageSpritesheet;
    C2D_SpriteSheet itemsSpritesheet;
    C2D_SpriteSheet keysSpritesheet;
    C2D_SpriteSheet healthSpritesheet;
    C2D_SpriteSheet facesSpritesheet;
    C2D_SpriteSheet animTilesSpritesheet;
    C2D_SpriteSheet textSpritesheet;
    uint32_t* palette;

    C2D_SpriteSheet spritesSpritesheet;

    C2D_Image mapImage;
    C2D_ImageTint imageTint;

    SDL_Rect cursorRect;
    CursorShape cursorShape;
    s16 cursorX = 0;
    s16 cursorY = 0;
    int framesPerSecond_;
#ifdef PLATFORM_MODULE_BASED_AUDIO
    uint8_t* moduleData;
    Module loadedModule;
    int8_t* sampleData;
    int8_t* soundExplosion;
    int8_t* soundMedkit;
    int8_t* soundEMP;
    int8_t* soundMagnet;
    int8_t* soundShock;
    int8_t* soundMove;
    int8_t* soundPlasma;
    int8_t* soundPistol;
    int8_t* soundItemFound;
    int8_t* soundError;
    int8_t* soundCycleWeapon;
    int8_t* soundCycleItem;
    int8_t* soundDoor;
    int8_t* soundMenuBeep;
    int8_t* soundShortBeep;
    int8_t* squareWave;
    uint8_t effectChannel;
#else
    float audioAngle;
    float audioFrequency;
    int16_t audioVolume;
#endif
#ifdef PLATFORM_IMAGE_SUPPORT
    Image loadedImage;
#endif
    uint16_t interruptIntervalInSamples;
    uint16_t samplesSinceInterrupt;
    uint32_t fadeBaseColor;
    uint16_t fadeIntensity;
    uint16_t joystickStateToReturn;
    uint16_t joystickState;
    uint16_t pendingState;
    uint8_t keyToReturn;
    uint8_t downKey;
    uint8_t shift;
};

#endif
