#include "PT2.3A_replay_cia.h"
#include "PlatformCTR.h"
#include <stdio.h>
#include <list>
#include <array>

#define JOYSTICK_AXIS_THRESHOLD 25000

#ifdef PLATFORM_MODULE_BASED_AUDIO
#define LARGEST_MODULE_SIZE 105654
#define TOTAL_SAMPLE_SIZE 75755
#endif

#ifdef PLATFORM_SPRITE_SUPPORT
static int8_t tileSpriteMap[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1, 49, 50, 57, 58, 59, 60, -1, -1, -1, -1, -1, -1, -1, 48,
    -1, -1, -1, 73, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     1,  0,  3, -1, 53, 54, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 76, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
#endif
static int8_t animTileMap[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16,
    -1, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1,  8, 10, -1, -1, 12, 14, -1, -1, 20, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static char MAPNAME[] = "romfs:/3DS/level-a";
#ifdef PLATFORM_MODULE_BASED_AUDIO
static const char* moduleFilenames[] = {
    "romfs:/Music/mod.soundfx",
    "romfs:/Music/mod.metal heads",
    "romfs:/Music/mod.win",
    "romfs:/Music/mod.lose",
    "romfs:/Music/mod.metallic bop amiga",
    "romfs:/Music/mod.get psyched",
    "romfs:/Music/mod.robot attack",
    "romfs:/Music/mod.rushin in"
};
static const char* sampleFilenames[] = {
    "romfs:/Sounds/sounds_dsbarexp.raw",
    "romfs:/Sounds/SOUND_MEDKIT.raw",
    "romfs:/Sounds/SOUND_EMP.raw",
    "romfs:/Sounds/SOUND_MAGNET2.raw",
    "romfs:/Sounds/SOUND_SHOCK.raw",
    "romfs:/Sounds/SOUND_MOVE.raw",
    "romfs:/Sounds/SOUND_PLASMA_FASTER.raw",
    "romfs:/Sounds/sounds_dspistol.raw",
    "romfs:/Sounds/SOUND_FOUND_ITEM.raw",
    "romfs:/Sounds/SOUND_ERROR.raw",
    "romfs:/Sounds/SOUND_CYCLE_WEAPON.raw",
    "romfs:/Sounds/SOUND_CYCLE_ITEM.raw",
    "romfs:/Sounds/SOUND_DOOR_FASTER.raw",
    "romfs:/Sounds/SOUND_BEEP2.raw",
    "romfs:/Sounds/SOUND_BEEP.raw",
    "romfs:/Sounds/SquareWave.raw"
};
#endif

static uint8_t standardControls[] = {
    0, // MOVE UP orig: 56 (8)
    0, // MOVE DOWN orig: 50 (2)
    0, // MOVE LEFT orig: 52 (4)
    0, // MOVE RIGHT orig: 54 (6)
    0, // FIRE UP
    0, // FIRE DOWN
    0, // FIRE LEFT
    0, // FIRE RIGHT
    0, // CYCLE WEAPONS
    0, // CYCLE ITEMS
    0, // USE ITEM
    0, // SEARCH OBJECT
    0, // MOVE OBJECT
    0, // LIVE MAP
    0, // LIVE MAP ROBOTS
    0, // PAUSE
    0, // MUSIC
    0, // CHEAT
    0, // CURSOR UP
    0, // CURSOR DOWN
    0, // CURSOR LEFT
    0, // CURSOR RIGHT
    0, // SPACE
    0, // RETURN
    0, // YES
    0 // NO
};

static uint32_t paletteIntro[] = {
    0xff000000,
    0xff443300,
    0xff775533,
    0xff997755,
    0xffccaa88,
    0xff882222,
    0xffcc7766,
    0xffee8888,
    0xffaa5577,
    0xff3311aa,
    0xff6644cc,
    0xff4488ee,
    0xff33bbee,
    0xff88eeee,
    0xffeeeeee,
    0xff55bb77
};

static uint32_t paletteGame[] = {
    0xff000000,
    0xffffffff,
    0xff775544,
    0xff998877,
    0xffccbbaa,
    0xff993300,
    0xffbb6633,
    0xffffaa00,
    0xff006655,
    0xff009977,
    0xff00ddaa,
    0xff004477,
    0xff0077bb,
    0xff00ccff,
    0xff99aaee,
    0xff0000ee
};

static uint8_t tileLiveMap[] = {
    0,  0, 15, 15, 15, 15, 15, 15, 15,  2, 15, 15, 15, 15,  0, 15,
    15, 15, 15, 15, 15, 15, 15,  6,  4, 15, 15, 15, 15,  3, 10, 10,
    3, 13, 10,  3,  3, 13,  2,  3,  6, 10, 10,  6,  6, 10, 10,  6,
    15, 15, 15,  6, 15,  4,  4,  6, 15,  4,  6,  6,  3,  3, 15,  3,
    15, 15,  3, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 14, 14, 14, 14, 14,
    6,  6,  0,  0,  0,  0,  0,  0, 15, 15, 15, 13, 14, 14, 14,  0,
    15, 15, 14, 14, 14,  2, 14, 15, 15,  6,  6,  6,  6,  6,  6,  6,
    15, 15,  0, 13, 15, 15,  0,  4, 15, 15, 14,  2,  0,  0,  0, 14,
    15, 15, 15, 15, 10, 14, 10, 10, 15, 15,  6,  6,  0,  0,  6,  2,
    6,  6,  6,  2,  0,  0, 15, 14, 14,  6, 15, 14, 15, 15, 15,  0,
    15, 15, 15,  0, 15, 15, 15,  6,  1,  1,  1, 15,  1,  6,  6, 15,
    1,  6,  6, 15, 14, 14,  4,  4, 14, 14, 14, 14,  2, 10, 10,  8,
    4,  4,  4,  5, 14, 14,  1,  5, 15, 15, 15,  5, 15, 15, 15,  5,
    15, 15, 15, 13,  4,  4, 13, 13,  4,  4, 13, 13,  1,  1,  0,  0,
    0,  0, 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

uint32_t mapPalette[] = {
    0xff000000,
    0xff555555,
    0xffaa0000,
    0xffff5555,
    0xff00aa00,
    0xff55ff55,
    0xffaaaa00,
    0xffffff55,
    0xff0000aa,
    0xff0055ff,
    0xffaa00aa,
    0xffff55ff,
    0xff0055aa,
    0xff55ffff,
    0xffaaaaaa,
    0xffffffff
};

std::list<Sprite> renderQueueTop;
std::list<Sprite> renderQueueBot;
C2D_Sprite gameOverlay;

static bool shake = false;
static int shakeTimer = 0;

static u32 tileMap[PLATFORM_MAP_WINDOW_TILES_HEIGHT * PLATFORM_MAP_WINDOW_TILES_WIDTH]; // really inefficient bc we only need 17 bits

#define draw(s) if(queue) renderQueueTop.emplace_back(s); else C2D_DrawSpriteTinted(&s.sprite, &s.tint)
#define shake(s, timer) s.sprite.params.pos.x = (timer % 6) ? s.sprite.params.pos.x - 8 : s.sprite.params.pos.x; 

void PlatformCTR::decodeTiles(u32* tileMap, int x, int y, bool queue)
{
    if(tileMap[(y) * PLATFORM_MAP_WINDOW_TILES_WIDTH + x] >= (1 << 18 ) && !(tileMap[(y) * PLATFORM_MAP_WINDOW_TILES_WIDTH + x] & 0x3FFFF) ) {
        Sprite sprite;
        C2D_SpriteFromSheet(&sprite.sprite, animTilesSpritesheet, tileMap[(y) * PLATFORM_MAP_WINDOW_TILES_WIDTH + x] >> 18);
        C2D_SpriteSetPos(&sprite.sprite, (x) *24, (y) *24);
        shake(sprite, shakeTimer);
        draw(sprite);
        return;
    }

    if(tileMap[(y) * PLATFORM_MAP_WINDOW_TILES_WIDTH + x] > 255 ) {
        const u32 tile = tileMap[(y) * PLATFORM_MAP_WINDOW_TILES_WIDTH + x];
        Sprite background;
        C2D_SpriteFromSheet(&background.sprite, tile & 0x1 ? animTilesSpritesheet : tileSpritesheet, tile >> 2 & 0b11111111);
        C2D_SpriteSetPos(&background.sprite, (x) *24, (y) *24);
        shake(background, shakeTimer);
        draw(background);
        Sprite foreground;
        C2D_SpriteFromSheet(&foreground.sprite, tile & 0x2 ? spritesSpritesheet : tileSpritesheet, tile >> 10 & 0b11111111);
        C2D_SpriteSetPos(&foreground.sprite, (x) *24, (y) *24);
        shake(foreground, shakeTimer);
        draw(foreground);
        return;
    }
        
    Sprite sprite;
    C2D_SpriteFromSheet(&sprite.sprite, tileSpritesheet, tileMap[(y) * PLATFORM_MAP_WINDOW_TILES_WIDTH + x]);
    C2D_SpriteSetPos(&sprite.sprite, (x) *24, (y) *24);
    shake(sprite, shakeTimer);
    draw(sprite);
}
#undef draw
#undef shake

#define LIVE_MAP_ORIGIN_X ((320 - 128 * 2) / 2)
#define LIVE_MAP_ORIGIN_Y 100

uint8_t unitTypes[48];
uint8_t unitX[48];
uint8_t unitY[48];

PlatformCTR::PlatformCTR() :
    interrupt(0),
    audioSpec({0}),
    audioDeviceID(0),
    palette(paletteIntro),
#ifdef PLATFORM_CURSOR_SUPPORT
    cursorRect({0}),
#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
    cursorShape(ShapeUse),
#endif
#endif
    cursorX(-1),
    cursorY(-1),
    framesPerSecond_(60),
#ifdef PLATFORM_MODULE_BASED_AUDIO
    moduleData(new uint8_t[LARGEST_MODULE_SIZE]),
    loadedModule(ModuleSoundFX),
    sampleData(new int8_t[TOTAL_SAMPLE_SIZE]),
    effectChannel(0),
#else
    audioAngle(0),
    audioFrequency(440),
    audioVolume(INT16_MAX >> 4),
#endif
#ifdef PLATFORM_IMAGE_SUPPORT
    loadedImage(ImageIntro),
#endif
    interruptIntervalInSamples(0),
    samplesSinceInterrupt(0),
    fadeBaseColor(0),
    fadeIntensity(0),
    joystickStateToReturn(0),
    joystickState(0),
    pendingState(0),
    keyToReturn(0xff),
    downKey(0xff),
    shift(0)
{
    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS * 3);
    C2D_SetTintMode(C2D_TintMult);
    osSetSpeedupEnable(true);
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    }

    topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    C2D_TargetClear(topScreen, 0);
    C2D_TargetClear(bottomScreen, 0);
    memset(tileMap, 0, PLATFORM_MAP_WINDOW_TILES_HEIGHT * PLATFORM_MAP_WINDOW_TILES_WIDTH * sizeof(tileMap[0]));
    memset(SCREEN_MEMORY, 32, SCREEN_WIDTH_IN_CHARACTERS * SCREEN_HEIGHT_IN_CHARACTERS);

    SDL_AudioSpec requestedAudioSpec;
    // memset(&requestedAudioSpec, 0, sizeof(requestedAudioSpec));
    SDL_zero(requestedAudioSpec);
    requestedAudioSpec.freq = 44100;
    requestedAudioSpec.format = AUDIO_S16LSB;
    requestedAudioSpec.channels = 1;
    requestedAudioSpec.samples = 512;
    requestedAudioSpec.callback = audioCallback;
    requestedAudioSpec.userdata = this;
    audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &requestedAudioSpec, &audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    // SDL_OpenAudio(&requestedAudioSpec, &audioSpec);
    if (!audioDeviceID) {
        fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
    }

    interruptIntervalInSamples = audioSpec.freq / framesPerSecond_;
    samplesSinceInterrupt = interruptIntervalInSamples;
    SDL_PauseAudioDevice(audioDeviceID, 0);
    // SDL_PauseAudio(0);
    
    fontSpritesheet = C2D_SpriteSheetLoad("romfs:/c64font.t3x");
    tileSpritesheet = C2D_SpriteSheetLoad("romfs:/tiles.t3x");
    imageSpritesheet = C2D_SpriteSheetLoad("romfs:/images.t3x");
    itemsSpritesheet = C2D_SpriteSheetLoad("romfs:/items.t3x");
    keysSpritesheet = C2D_SpriteSheetLoad("romfs:/keys.t3x");
    healthSpritesheet = C2D_SpriteSheetLoad("romfs:/health.t3x");
    facesSpritesheet = C2D_SpriteSheetLoad("romfs:/faces.t3x");
    animTilesSpritesheet = C2D_SpriteSheetLoad("romfs:/animtiles.t3x");
    spritesSpritesheet = C2D_SpriteSheetLoad("romfs:/sprites.t3x");
    textSpritesheet = C2D_SpriteSheetLoad("romfs:/text.t3x");
    C2D_SpriteFromSheet(&gameOverlay, imageSpritesheet, 1);

#ifdef PLATFORM_MODULE_BASED_AUDIO
    int sample = 0;
    int8_t* destination = sampleData;
    soundExplosion = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundMedkit = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundEMP = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundMagnet = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundShock = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundMove = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundPlasma = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundPistol = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundItemFound = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundError = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundCycleWeapon = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundCycleItem = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundDoor = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundMenuBeep = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundShortBeep = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    squareWave = destination;
    load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);

    // Clear the first two bytes of effect samples to enable the 2-byte no-loop loop
    *((uint16_t*)soundExplosion) = 0;
    *((uint16_t*)soundMedkit) = 0;
    *((uint16_t*)soundEMP) = 0;
    *((uint16_t*)soundMagnet) = 0;
    *((uint16_t*)soundShock) = 0;
    *((uint16_t*)soundMove) = 0;
    *((uint16_t*)soundPlasma) = 0;
    *((uint16_t*)soundPistol) = 0;
    *((uint16_t*)soundItemFound) = 0;
    *((uint16_t*)soundError) = 0;
    *((uint16_t*)soundCycleWeapon) = 0;
    *((uint16_t*)soundCycleItem) = 0;
    *((uint16_t*)soundDoor) = 0;
    *((uint16_t*)soundMenuBeep) = 0;
    *((uint16_t*)soundShortBeep) = 0;
#endif

    platform = this;
}

PlatformCTR::~PlatformCTR()
{
    C2D_SpriteSheetFree(fontSpritesheet);
    C2D_SpriteSheetFree(tileSpritesheet);
    C2D_SpriteSheetFree(imageSpritesheet);
    C2D_SpriteSheetFree(itemsSpritesheet);
    C2D_SpriteSheetFree(keysSpritesheet);
    C2D_SpriteSheetFree(healthSpritesheet);
    C2D_SpriteSheetFree(facesSpritesheet);
    C2D_SpriteSheetFree(animTilesSpritesheet);
    C2D_SpriteSheetFree(spritesSpritesheet);
    C2D_SpriteSheetFree(textSpritesheet);

    SDL_CloseAudioDevice(audioDeviceID);

    C2D_Fini();
    C3D_Fini();
    SDL_Quit();
    gfxExit();
    romfsExit();
#ifdef PLATFORM_MODULE_BASED_AUDIO
    delete[] sampleData;
    delete[] moduleData;
#endif
}

void PlatformCTR::audioCallback(void* data, uint8_t* stream, int bytes) {
    PlatformCTR* platform = (PlatformCTR*)data;
    int words = bytes >> 1;
    int16_t* output = (int16_t*)stream;
#ifdef PLATFORM_MODULE_BASED_AUDIO
    processAudio(output, words, platform->audioSpec.freq);
#else
    for (int i = 0; i < words; i++) {
        output[i] = platform->audioVolume * (sin(platform->audioAngle) >= 0 ? 1 : -1);
        platform->audioAngle += 2 * M_PI * platform->audioFrequency / platform->audioSpec.freq;
    }
#endif
    platform->samplesSinceInterrupt += words;
    while (platform->samplesSinceInterrupt >= platform->interruptIntervalInSamples) {
        if (platform->interrupt) {
            (*platform->interrupt)();
        }
        platform->samplesSinceInterrupt -= platform->interruptIntervalInSamples;
    }
}


#ifdef PLATFORM_MODULE_BASED_AUDIO
void PlatformCTR::undeltaSamples(uint8_t* module, uint32_t moduleSize)
{
    uint8_t numPatterns = 0;
    for (int i = 0; i < module[950]; i++) {
        numPatterns = MAX(numPatterns, module[952 + i]);
    }
    numPatterns++;

    int8_t* samplesStart = (int8_t*)(module + 1084 + (numPatterns << 10));
    int8_t* samplesEnd = (int8_t*)(module + moduleSize);

    int8_t sample = 0;
    for (int8_t* sampleData = samplesStart; sampleData < samplesEnd; sampleData++) {
        int8_t delta = *sampleData;
        sample += delta;
        *sampleData = sample;
    }
}

void PlatformCTR::setSampleData(uint8_t* module)
{
    mt_SampleStarts[15 + 0] = soundExplosion;
    mt_SampleStarts[15 + 1] = soundShortBeep;
    mt_SampleStarts[15 + 2] = soundMedkit;
    mt_SampleStarts[15 + 3] = soundEMP;
    mt_SampleStarts[15 + 4] = soundMagnet;
    mt_SampleStarts[15 + 5] = soundShock;
    mt_SampleStarts[15 + 6] = soundMove;
    mt_SampleStarts[15 + 7] = soundShock;
    mt_SampleStarts[15 + 8] = soundPlasma;
    mt_SampleStarts[15 + 9] = soundPistol;
    mt_SampleStarts[15 + 10] = soundItemFound;
    mt_SampleStarts[15 + 11] = soundError;
    mt_SampleStarts[15 + 12] = soundCycleWeapon;
    mt_SampleStarts[15 + 13] = soundCycleItem;
    mt_SampleStarts[15 + 14] = soundDoor;
    mt_SampleStarts[15 + 15] = soundMenuBeep;

    SampleData* sampleData = (SampleData*)(module + 20);
    putWord((uint8_t*)&sampleData[15 + 0].length, 0, (uint16_t)(soundMedkit - soundExplosion) >> 1);
    putWord((uint8_t*)&sampleData[15 + 1].length, 0, (uint16_t)(squareWave - soundShortBeep) >> 1);
    putWord((uint8_t*)&sampleData[15 + 2].length, 0, (uint16_t)(soundEMP - soundMedkit) >> 1);
    putWord((uint8_t*)&sampleData[15 + 3].length, 0, (uint16_t)(soundMagnet - soundEMP) >> 1);
    putWord((uint8_t*)&sampleData[15 + 4].length, 0, (uint16_t)(soundShock - soundMagnet) >> 1);
    putWord((uint8_t*)&sampleData[15 + 5].length, 0, (uint16_t)(soundMove - soundShock) >> 1);
    putWord((uint8_t*)&sampleData[15 + 6].length, 0, (uint16_t)(soundPlasma - soundMove) >> 1);
    putWord((uint8_t*)&sampleData[15 + 7].length, 0, (uint16_t)(soundMove - soundShock) >> 1);
    putWord((uint8_t*)&sampleData[15 + 8].length, 0, (uint16_t)(soundPistol - soundPlasma) >> 1);
    putWord((uint8_t*)&sampleData[15 + 9].length, 0, (uint16_t)(soundItemFound - soundPistol) >> 1);
    putWord((uint8_t*)&sampleData[15 + 10].length, 0, (uint16_t)(soundError - soundItemFound) >> 1);
    putWord((uint8_t*)&sampleData[15 + 11].length, 0, (uint16_t)(soundCycleWeapon - soundError) >> 1);
    putWord((uint8_t*)&sampleData[15 + 12].length, 0, (uint16_t)(soundCycleItem - soundCycleWeapon) >> 1);
    putWord((uint8_t*)&sampleData[15 + 13].length, 0, (uint16_t)(soundDoor - soundCycleItem) >> 1);
    putWord((uint8_t*)&sampleData[15 + 14].length, 0, (uint16_t)(soundMenuBeep - soundDoor) >> 1);
    putWord((uint8_t*)&sampleData[15 + 15].length, 0, (uint16_t)(soundShortBeep - soundMenuBeep) >> 1);
    for (int i = 0; i < 16; i++) {
        sampleData[15 + i].volume = 64;
    }
}
#endif

uint8_t* PlatformCTR::standardControls() const
{
    return ::standardControls;
}

void PlatformCTR::setInterrupt(void (*interrupt)(void))
{
    this->interrupt = interrupt;
}

int PlatformCTR::framesPerSecond()
{
    return framesPerSecond_;
}

void PlatformCTR::chrout(uint8_t character)
{
    putchar(character == 0x0d ? 0x0a : character);
}

uint8_t PlatformCTR::readKeyboard()
{
    return 0xff;
}

void PlatformCTR::keyRepeat()
{
    // keyToReturn = downKey;
    joystickStateToReturn = joystickState;
}

void PlatformCTR::clearKeyBuffer()
{
    // SDL_Event event;
    // while (SDL_PollEvent(&event));
    // keyToReturn = 0xff;
    // downKey = 0xff;
    // pendingState = 0;
    joystickStateToReturn = 0;
}

bool PlatformCTR::isKeyOrJoystickPressed(bool gamepad)
{
    // return downKey != 0xff || (joystickState != 0 && joystickState != JoystickPlay);
    return joystickState != 0 && joystickState != JoystickPlay;
}

static u64 touchTime;

uint16_t PlatformCTR::readJoystick(bool gamepad)
{
    hidScanInput();
    u32 kHeld = hidKeysHeld();
    uint16_t state = 0;

    this->quit = !aptMainLoop();

    if(kHeld & KEY_LEFT) {
        state |= JoystickLeft;
    }
    if(kHeld & KEY_RIGHT) {
        state |= JoystickRight;
    }
    if(kHeld & KEY_UP) {
        state |= JoystickUp;
    }
    if(kHeld & KEY_DOWN) {
        state |= JoystickDown;
    }
    if(kHeld & KEY_Y) {
        state |= JoystickGreen;
    }
    if(kHeld & KEY_A) {
        state |= JoystickBlue;
    }
    if(kHeld & KEY_X) {
        state |= JoystickYellow;
    }
    if(kHeld & KEY_B) {
        state |= JoystickRed;
    }
    if(kHeld & KEY_L) {
        state |= JoystickReverse;
    }
    if(kHeld & KEY_R) {
        state |= JoystickForward;
    }
    if(kHeld & KEY_START) {
        state |= JoystickExtra;
    }
    if(hidKeysDown() & KEY_TOUCH) {
        touchTime = osGetTime() + 250;
    }
    if(hidKeysUp() & KEY_TOUCH && touchTime > osGetTime()) {
        state |= JoystickTouch;
    }
    if(kHeld & KEY_SELECT || (kHeld & KEY_TOUCH && touchTime < osGetTime())) {
        state |= JoystickPlay;
    }

    if(joystickState != state) {
        joystickStateToReturn = state != JoystickPlay ? state : 0;
        joystickState = state;
    }
  
    

    uint16_t result = joystickStateToReturn;
    joystickStateToReturn = 0;
    return result;
}


uint32_t PlatformCTR::load(const char* filename, uint8_t* destination, uint32_t size)
{
    uint32_t bytesRead = 0;

    FILE* file = fopen(filename, "r");
    if (file) {
        bytesRead = (uint32_t)fread(destination, 1, size, file);

        fclose(file);
    }

    return bytesRead;
}

void PlatformCTR::loadMap(Map map, uint8_t* destination)
{
    MAPNAME[17] = 'a' + map;

    load(MAPNAME, destination, 8960);
}

uint8_t* PlatformCTR::loadTileset()
{
    uint8_t* tileset = new uint8_t[2818];
    load("romfs:/tileset.amiga", tileset, 2818);
    return tileset;
}

#ifdef PLATFORM_IMAGE_SUPPORT
void PlatformCTR::displayImage(Image image)
{

    if (image == ImageGame) {
        palette = paletteGame;
        Sprite sprite;
        C2D_SpriteFromSheet(&sprite.sprite, imageSpritesheet, image);
        C2D_SpriteScale(&sprite.sprite, 1.f, 1.f);
        renderQueueTop.emplace_back(sprite);
        for(int i = 0; i < 4; i++) {
            C2D_SpriteFromSheet(&sprite.sprite, textSpritesheet, i);
            C2D_SpriteSetPos(&sprite.sprite, 28 + 72 * i, 14);
            renderQueueBot.emplace_back(sprite);
        }

    } else {
        palette = paletteIntro;
        Sprite sprite;
        C2D_SpriteFromSheet(&sprite.sprite, imageSpritesheet, image);
        if(image == ImageGameOver) {
            C2D_SpriteScale(&sprite.sprite, 1.f, 1.f);
            memset(tileMap, 0, PLATFORM_MAP_WINDOW_TILES_HEIGHT * PLATFORM_MAP_WINDOW_TILES_WIDTH * sizeof(tileMap[0]));
        }
        else {
            C2D_SpriteScale(&sprite.sprite, 1.25f, 1.25f);
        }
        renderQueueTop.emplace_back(sprite);

    }

    loadedImage = image;
}
#endif

void PlatformCTR::generateTiles(uint8_t* tileData, uint8_t* tileAttributes)
{

}

#ifndef PLATFORM_IMAGE_BASED_TILES
void PlatformCTR::updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles)
{

}
#endif

SDL_Rect clipRect = { 0, 0, PLATFORM_SCREEN_WIDTH - 56, PLATFORM_SCREEN_HEIGHT - 32 };

void PlatformCTR::renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent)
{
    if (transparent) {
        if (tileSpriteMap[tile] >= 0) {
            renderSprite(tileSpriteMap[tile] + variant, x, y);
            return;
        }
    } else {
        if (animTileMap[tile] >= 0) {
            renderAnimTile(animTileMap[tile] + variant, x, y);
            return;
        }
    }

    Sprite sprite;
    C2D_SpriteFromSheet(&sprite.sprite, tileSpritesheet, tile);
    C2D_SpriteSetPos(&sprite.sprite, x, y);
    renderQueueTop.emplace_back(sprite);
    tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] = tile;
}

void PlatformCTR::renderTiles(uint8_t backgroundTile, uint8_t foregroundTile, uint16_t x, uint16_t y, uint8_t backgroundVariant, uint8_t foregroundVariant)
{
    tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] = 0;
    if (animTileMap[backgroundTile] >= 0) {
        backgroundTile = animTileMap[backgroundTile] + backgroundVariant;
        Sprite sprite;
        C2D_SpriteFromSheet(&sprite.sprite, animTilesSpritesheet, backgroundTile);
        C2D_SpriteSetPos(&sprite.sprite, x, y);
        renderQueueTop.emplace_back(sprite);
        tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] |= 0x1;
        tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] |= (backgroundTile << 2);

    } else {
        Sprite sprite;
        C2D_SpriteFromSheet(&sprite.sprite, tileSpritesheet, backgroundTile);
        C2D_SpriteSetPos(&sprite.sprite, x, y);
        renderQueueTop.emplace_back(sprite);
        tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] |= (backgroundTile << 2);
    }

    if (tileSpriteMap[foregroundTile] >= 0) {
        uint8_t sprite = tileSpriteMap[foregroundTile] + foregroundVariant;
        Sprite Ssprite;
        C2D_SpriteFromSheet(&Ssprite.sprite, spritesSpritesheet, sprite);
        C2D_SpriteSetPos(&Ssprite.sprite, x, y);
        renderQueueTop.emplace_back(Ssprite);
        tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] |= 0x2;
        tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] |= sprite << 10;
        if(sprite == 0) 
            tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] |= 1 << 18;
    } else {
        Sprite Ssprite;
        C2D_SpriteFromSheet(&Ssprite.sprite, tileSpritesheet, foregroundTile);
        C2D_SpriteSetPos(&Ssprite.sprite, x, y);
        renderQueueTop.emplace_back(Ssprite);
        tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] |= foregroundTile << 10;
        if(foregroundTile == 0) 
            tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] |= 1 << 18;
    }
}

#ifdef PLATFORM_SPRITE_SUPPORT
void PlatformCTR::renderSprite(uint8_t sprite, uint16_t x, uint16_t y)
{
    Sprite Ssprite;
    C2D_SpriteFromSheet(&Ssprite.sprite, spritesSpritesheet, sprite);
    C2D_SpriteSetPos(&Ssprite.sprite, x, y);
    renderQueueTop.emplace_back(Ssprite);
}
#endif

#ifdef PLATFORM_IMAGE_BASED_TILES
void PlatformCTR::renderAnimTile(uint8_t animTile, uint16_t x, uint16_t y)
{
    Sprite sprite;
    C2D_SpriteFromSheet(&sprite.sprite, animTilesSpritesheet, animTile);
    C2D_SpriteSetPos(&sprite.sprite, x, y);
    renderQueueTop.emplace_back(sprite);
    tileMap[PLATFORM_MAP_WINDOW_TILES_WIDTH * (y/24) + (x/24)] = animTile << 18;
}
#endif

#ifdef PLATFORM_IMAGE_SUPPORT
void PlatformCTR::renderItem(uint8_t item, uint16_t x, uint16_t y)
{
    Sprite sprite;
    C2D_SpriteFromSheet(&sprite.sprite, itemsSpritesheet, item);
    C2D_SpriteSetPos(&sprite.sprite, x, y);
    renderQueueBot.emplace_back(sprite);
}

void PlatformCTR::renderKey(uint8_t key, uint16_t x, uint16_t y)
{
    Sprite sprite;
    C2D_SpriteFromSheet(&sprite.sprite, keysSpritesheet, key);
    C2D_SpriteSetPos(&sprite.sprite, x, y);
    renderQueueBot.emplace_back(sprite);
}

void PlatformCTR::renderHealth(uint8_t health, uint16_t x, uint16_t y)
{
    Sprite sprite;
    C2D_SpriteFromSheet(&sprite.sprite, healthSpritesheet, health);
    C2D_SpriteSetPos(&sprite.sprite, x, y);
    renderQueueBot.emplace_back(sprite);
}

void PlatformCTR::renderFace(uint8_t face, uint16_t x, uint16_t y)
{
    Sprite sprite;
    C2D_SpriteFromSheet(&sprite.sprite, facesSpritesheet, face);
    C2D_SpriteScale(&sprite.sprite, 1.25, 1.25);
    C2D_SpriteSetPos(&sprite.sprite, x * 1.25, y * 1.25);
    renderQueueTop.emplace_back(sprite);
}
#endif

#ifdef PLATFORM_LIVE_MAP_SUPPORT
void PlatformCTR::renderLiveMap(uint8_t* map)
{
    for (int mapY = 0; mapY < 64; mapY++) {
        for (int mapX = 0; mapX < 128; mapX++) {
            Sprite sprite;
            sprite.flags = DRAWRECT;
            sprite.colour = mapPalette[tileLiveMap[*map++]];
            sprite.rect = { LIVE_MAP_ORIGIN_X + mapX * 2, LIVE_MAP_ORIGIN_Y + mapY * 2, 2, 2 };
            // C2D_SpriteFromSheet(&sprite.sprite, tileSpritesheet, *map++);
            // C2D_SpriteSetPos(&sprite.sprite, LIVE_MAP_ORIGIN_X + mapX * 2, LIVE_MAP_ORIGIN_Y + (mapY * 2));
            // sprite.sprite.params.pos.w = 2;
            // sprite.sprite.params.pos.h = 2;
            // *map++;
            renderQueueBot.emplace_back(sprite);
        }
    }

    for (int i = 0; i < 48; i++) {
        unitTypes[i] = 255;
    }
}

void PlatformCTR::renderLiveMapTile(uint8_t* map, uint8_t mapX, uint8_t mapY)
{
    Sprite sprite;
    sprite.flags = DRAWRECT;
    sprite.colour = mapPalette[tileLiveMap[map[(mapY << 7) + mapX]]];
    sprite.rect = { LIVE_MAP_ORIGIN_X + mapX * 2, LIVE_MAP_ORIGIN_Y + mapY * 2, 2, 2 };
    // C2D_SpriteFromSheet(&sprite.sprite, tileSpritesheet, map[(mapY << 7) + mapX]);
    // C2D_SpriteSetPos(&sprite.sprite, LIVE_MAP_ORIGIN_X + mapX * 2, LIVE_MAP_ORIGIN_Y + mapY * 2);
    // sprite.sprite.params.pos.w = 2;
    // sprite.sprite.params.pos.h = 2;
    renderQueueBot.emplace_back(sprite);
}

void PlatformCTR::renderLiveMapUnits(uint8_t* map, uint8_t* unitTypes, uint8_t* unitX, uint8_t* unitY, uint8_t playerColor, bool showRobots)
{
    for (int i = 0; i < 48; i++) {
        if ((i < 28 || unitTypes[i] == 22) && (unitX[i] != ::unitX[i] || unitY[i] != ::unitY[i] || (i > 0 && (!showRobots || unitTypes[i] == 22 || unitTypes[i] != ::unitTypes[i])) || (i == 0 && playerColor != ::unitTypes[i]))) {
            // Remove old dot if any
            if (::unitTypes[i] != 255) {
                renderLiveMapTile(map, ::unitX[i], ::unitY[i]);

                if (i > 0 && !showRobots) {
                    ::unitTypes[i] = 255;
                }
            }

            if (i == 0 ||
                (unitTypes[i] == 22 && (unitX[i] != unitX[0] || unitY[i] != unitY[0])) ||
                (showRobots &&
                 (unitTypes[i] == 1 ||
                 (unitTypes[i] >= 2 && unitTypes[i] <= 5) ||
                 (unitTypes[i] >= 17 && unitTypes[i] <= 18) ||
                 unitTypes[i] == 9))) {
                // Render new dot
                int x = unitX[i];
                int y = unitY[i];
                Sprite sprite;
                sprite.flags = DRAWRECT;
                sprite.rect = { LIVE_MAP_ORIGIN_X + x * 2, LIVE_MAP_ORIGIN_Y + y * 2, 2, 2 };
                sprite.colour = palette[(i > 0 || playerColor == 1) ? 1 : 0];
                renderQueueBot.emplace_back(sprite);
                // SDL_FillRect(bufferSurface, &clearRect, SDL_MapRGB(bufferSurface->format, color & 0xff, (color >> 8) & 0xff, (color >> 16)  & 0xff));

                ::unitTypes[i] = i == 0 ? playerColor : unitTypes[i];
                ::unitX[i] = unitX[i];
                ::unitY[i] = unitY[i];
            }
        }
    }
}
#endif

static bool cursorCover = false;
static s16 oldcX = -1;
static s16 oldcY = -1;

void PlatformCTR::showCursor(uint16_t x, uint16_t y)
{
    if(oldcX == -1)
    {
        oldcX = cursorX;
        oldcY = cursorY;
    }
    cursorX = x * 24 - 2;
    cursorY = y * 24 - 1;
    // cursorCover = false;
}

void PlatformCTR::hideCursor()
{
    oldcX = cursorX;
    oldcY = cursorY;
    if(cursorX != -1)
        cursorX = -1;
    cursorCover = true;
}

#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
void PlatformCTR::setCursorShape(CursorShape shape)
{
    cursorShape = shape;
}
#endif

void PlatformCTR::copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height)
{
    // Sprite copyRect;
    // copyRect.flags = COPYRECT;
    // copyRect.copyRect.x = sourceX;
    // copyRect.copyRect.y = sourceY;
    // copyRect.copyRect.w = width;
    // copyRect.copyRect.h = height;
    // copyRect.copyPos.x = destinationX;
    // copyRect.copyPos.y = destinationY;
    // renderQueueTop.emplace_back(copyRect);
    //SDL_BlitSurface(bufferSurface, &sourceRect, bufferSurface, &destinationRect);
}

void PlatformCTR::clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    Sprite clearRect;
    clearRect.rect.x = x;
    clearRect.rect.y = y;
    clearRect.rect.w = width;
    clearRect.rect.h = height;
    clearRect.colour = 0xFF000000;
    clearRect.flags = DRAWRECT;
    if(y >= 240) {
        clearRect.rect.y -= 240;
        renderQueueBot.emplace_back(clearRect);
        return;
    }
    renderQueueTop.emplace_back(clearRect);
}

void PlatformCTR::startShakeScreen()
{
    shake = true;
}

void PlatformCTR::shakeScreen()
{
    // copyRect(8, 0, 0, 0, 256, 168);
    shakeTimer += 1;
}

void PlatformCTR::stopShakeScreen()
{
    shake = false;
    shakeTimer = 0;
    Sprite gameOverlay;
    C2D_SpriteSetPos(&gameOverlay.sprite, (shakeTimer % 6) ? -8 : 0, 0);
    C2D_SpriteFromSheet(&gameOverlay.sprite, imageSpritesheet, 1);
    renderQueueTop.emplace_back(gameOverlay);
    for (int X = 0; X != 33; X++) {
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 3) * SCREEN_WIDTH_IN_CHARACTERS + X, SCREEN_MEMORY[0x546 + X], 10, 0);
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 2) * SCREEN_WIDTH_IN_CHARACTERS + X, SCREEN_MEMORY[0x578 + X], 10, 0);
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + X, SCREEN_MEMORY[0x5AA + X], 10, 0); // BOTTOM ROW
    }
    for(int y = 0; y < PLATFORM_MAP_WINDOW_TILES_HEIGHT; y ++) 
        for(int x = 0; x < PLATFORM_MAP_WINDOW_TILES_WIDTH; x ++) {
            if(tileMap[(y) * PLATFORM_MAP_WINDOW_TILES_WIDTH + x] == 0 )
                continue;
    
            decodeTiles(tileMap, x, y, true);
        }
}

void PlatformCTR::startFadeScreen(uint16_t color, uint16_t intensity)
{
    uint32_t r = (color & 0xf00) >> 8;
    uint32_t g = (color & 0x0f0) << 4;
    uint32_t b = (color & 0x00f) << 16;
    uint32_t bgr = r |  g | b;
    fadeBaseColor = bgr | (bgr << 4);
    fadeIntensity = intensity;
}

void PlatformCTR::fadeScreen(uint16_t intensity, bool immediate)
{
    if (fadeIntensity != intensity) {
        if (immediate) {
            fadeIntensity = intensity;
         } else {
            int16_t fadeDelta = intensity > fadeIntensity ? 1 : -1;
            do {
                fadeIntensity += fadeDelta;

                clearKeyBuffer();

                this->renderFrame(true);
            } while (fadeIntensity != intensity);
        }
    }
}

void PlatformCTR::stopFadeScreen()
{
    fadeIntensity = 15;
}

void PlatformCTR::writeToScreenMemory(address_t address, uint8_t value)
{
    Sprite sprite;
    const int x = (address % SCREEN_WIDTH_IN_CHARACTERS) << 3;
    const int y = ((address / SCREEN_WIDTH_IN_CHARACTERS) << 3);
    C2D_SpriteFromSheet(&sprite.sprite, fontSpritesheet, value);
    C2D_SpriteSetPos(&sprite.sprite, x, y);
    if(palette == paletteIntro)
    {
        C2D_SpriteScale(&sprite.sprite, 1.25, 1.25);
        C2D_SpriteSetPos(&sprite.sprite, x * 1.25, y * 1.25);
    }

    C2D_ImageTint& tint = sprite.tint;
    for(int i = 0; i < 4; i++) {
        tint.corners[i].blend = 1.f;
        tint.corners[i].color = C2D_Color32(0x77, 0xbb, 0x55, 0xff);
    }

    if(sprite.sprite.params.pos.y > 240)
    {
       sprite.sprite.params.pos.y -= 240;
       renderQueueBot.emplace_back(sprite); 
       return;
    }

    renderQueueTop.emplace_back(sprite);
}

void PlatformCTR::writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{

    Sprite sprite;
    const int x = (address % SCREEN_WIDTH_IN_CHARACTERS) << 3;
    const int y = ((address / SCREEN_WIDTH_IN_CHARACTERS) << 3) + yOffset;
    C2D_SpriteFromSheet(&sprite.sprite, fontSpritesheet, value);
    C2D_SpriteSetPos(&sprite.sprite, x, y);
    if(palette == paletteIntro)
    {
        C2D_SpriteScale(&sprite.sprite, 1.25, 1.25);
        C2D_SpriteSetPos(&sprite.sprite, x * 1.25, loadedImage == ImageGameOver ? y * 1.25 - 10 : y * 1.25);
    }

    C2D_ImageTint& tint = sprite.tint;
    for(int i = 0; i < 4; i++) {
        tint.corners[i].blend = 1.f;
        tint.corners[i].color = palette[color];
    }

    if(sprite.sprite.params.pos.y > 240)
    {
       sprite.sprite.params.pos.y -= 240;
       renderQueueBot.emplace_back(sprite); 
       return;
    }

    renderQueueTop.emplace_back(sprite);
}

#ifdef PLATFORM_MODULE_BASED_AUDIO
void PlatformCTR::loadModule(Module module)
{
    if (loadedModule != module) {
        uint32_t moduleSize = load(moduleFilenames[module], moduleData, LARGEST_MODULE_SIZE);
        undeltaSamples(moduleData, moduleSize);
        setSampleData(moduleData);
        loadedModule = module;
    }
}

void PlatformCTR::playModule(Module module)
{
    stopModule();
    stopSample();

    loadModule(module);
    mt_init(moduleData);

    mt_Enable = true;
}

void PlatformCTR::pauseModule()
{
    mt_speed = 0;
    mt_music();
    mt_Enable = false;
    channel0.volume = 0;
    channel1.volume = 0;
    channel2.volume = 0;
    channel3.volume = 0;
}

void PlatformCTR::stopModule()
{
    mt_end();
}

void PlatformCTR::playSample(uint8_t sample)
{
    ChanInput* input = &mt_chaninputs[effectChannel < 2 ? effectChannel : (5 - effectChannel)];

    effectChannel++;
    effectChannel &= 3;

    putWord((uint8_t*)&input->note, 0, 0x1000 + 320);
    if (sample < 16) {
        putWord((uint8_t*)&input->cmd, 0, sample << 12);
    } else if (sample == 16) {
        putWord((uint8_t*)&input->cmd, 0, 1 << 12);
    } else {
        putWord((uint8_t*)&input->cmd, 0, 15 << 12);
    }
}

void PlatformCTR::stopSample()
{
    for (int i = 0; i < 4; i++) {
        mt_chaninputs[i].note = 0;
        mt_chaninputs[i].cmd = 0;
    }
}
#else
static const float noteToFrequency[] = {
    0,
    246.94,
    261.63,
    277.18,
    293.66,
    311.13,
    329.63,
    349.23,
    369.99,
    392.00,
    415.30,
    440.00,
    466.16,
    493.88,
    523.25,
    554.37,
    587.33,
    622.25,
    659.25,
    698.46,
    739.99,
    783.99,
    830.61,
    880.00,
    932.33,
    987.77,
    1046.50,
    1108.73,
    1174.66,
    1244.51,
    1318.51,
    1396.91,
    1479.98,
    1567.98,
    1661.22,
    1760.00,
    1864.66,
    1975.53,
    0
};

void PlatformCTR::playNote(uint8_t note)
{
    audioFrequency = noteToFrequency[note];
    audioVolume = audioFrequency > 0 ? (INT16_MAX >> 4) : 0;
    audioAngle = 0;
}

void PlatformCTR::stopNote()
{
    audioVolume = 0;
}
#endif

void PlatformCTR::renderFrame(bool)
{
    if(!aptMainLoop()) {
        this->quit = false;
        return;
    }
    C2D_Prepare();
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    if(shake)
        C2D_TargetClear(topScreen, 0);
    C2D_SceneBegin(topScreen);

    if(cursorCover)
    {
        int cX = (oldcX + 2) / 24 - 2;
        int cY = (oldcY + 1) / 24 - 2;
        for(int y = 0; y <= 4; y++) {
            for(int x = 0; x <= 4; x++) {
                if(tileMap[(cY + y) * PLATFORM_MAP_WINDOW_TILES_WIDTH + cX + x] == 0 || cX+x <0 || cY+y < 0 || cX+x >= PLATFORM_MAP_WINDOW_TILES_WIDTH || cY+y >= PLATFORM_MAP_WINDOW_TILES_HEIGHT)
                    continue;

                decodeTiles(tileMap, cX + x, cY + y, false);
                
            }
        }
        cursorCover = false;
    }

    if(shake) {
        // Sprite gameOverlay;
        // C2D_SpriteSetPos(&gameOverlay.sprite, (shakeTimer % 6) ? -8 : 0, 0);
        // C2D_SpriteFromSheet(&gameOverlay.sprite, imageSpritesheet, 1);
        // renderQueueTop.emplace_back(gameOverlay);
        renderQueueTop.clear();
        for (int X = 0; X != 33; X++) {
            writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 3) * SCREEN_WIDTH_IN_CHARACTERS + X - (shakeTimer % 6 && X != 0), SCREEN_MEMORY[0x546 + X], 10, 0);
            writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 2) * SCREEN_WIDTH_IN_CHARACTERS + X - (shakeTimer % 6 && X != 0), SCREEN_MEMORY[0x578 + X], 10, 0);
            writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + X - (shakeTimer % 6 && X != 0), SCREEN_MEMORY[0x5AA + X], 10, 0); // BOTTOM ROW
        }
        for(int y = 0; y < PLATFORM_MAP_WINDOW_TILES_HEIGHT; y ++) 
            for(int x = 0; x < PLATFORM_MAP_WINDOW_TILES_WIDTH; x ++) {
                if(tileMap[(y) * PLATFORM_MAP_WINDOW_TILES_WIDTH + x] == 0 )
                    continue;
        
                decodeTiles(tileMap, x, y, true);
            }
    
    }

    if(!renderQueueTop.empty()) {
        
        for(auto& sprite : renderQueueTop) {
            switch (sprite.flags) {
                case DRAWRECT:
                    C2D_DrawRectSolid(sprite.rect.x, sprite.rect.y, 0, sprite.rect.w, sprite.rect.h, sprite.colour);
                    break;
                default:
                    C2D_DrawSpriteTinted(&sprite.sprite, &sprite.tint);
                    break;
            }
        }
        if(fadeIntensity == 15)
            renderQueueTop.clear();
    }



    if(cursorX != -1) {
        C2D_DrawLine(cursorX, cursorY, C2D_Color32(255,255,255,255), cursorX + 28, cursorY, C2D_Color32(255,255,255,255), 2, 0);
        C2D_DrawLine(cursorX, cursorY + 26, C2D_Color32(255,255,255,255), cursorX + 26, cursorY + 26, C2D_Color32(255,255,255,255), 2, 0);
        C2D_DrawLine(cursorX + 27, cursorY, C2D_Color32(255,255,255,255), cursorX + 27, cursorY + 27, C2D_Color32(255,255,255,255), 2, 0);
        C2D_DrawLine(cursorX+1, cursorY, C2D_Color32(255,255,255,255), cursorX+1, cursorY + 27, C2D_Color32(255,255,255,255), 2, 0);
        if (cursorShape != ShapeUse) {
            C2D_Sprite sprite;
            C2D_SpriteFromSheet(&sprite, spritesSpritesheet, cursorShape == ShapeSearch ? 83 : 85);
            C2D_SpriteSetPos(&sprite, cursorX + 2, cursorY + 1);
            C2D_DrawSprite(&sprite);
            if(cursorShape == ShapeSearch)
                cursorCover = true;
        }
    }

    if(loadedImage == ImageGame) {
        C2D_SpriteSetPos(&gameOverlay, (shakeTimer % 6) ? -8 : 0, 0);
        C2D_DrawSprite(&gameOverlay);
    }

    if (fadeIntensity != 15) {
        if(palette == paletteGame) {
            if(tileMap[0] != 0 ) {
                Sprite clear;
                clear.flags = DRAWRECT;
                clear.rect = {0, 216, 400, 24};
                clear.colour = 0xFF000000;
                renderQueueTop.emplace_back(clear);
                for (int X = 0; X != 33; X++) {
                    writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 3) * SCREEN_WIDTH_IN_CHARACTERS + X, SCREEN_MEMORY[0x546 + X], 10, 0);
                    writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 2) * SCREEN_WIDTH_IN_CHARACTERS + X, SCREEN_MEMORY[0x578 + X], 10, 0);
                    writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + X, SCREEN_MEMORY[0x5AA + X], 10, 0); // BOTTOM ROW
                }  
            }

            for(int y = 0; y < PLATFORM_MAP_WINDOW_TILES_HEIGHT; y ++) 
                for(int x = 0; x < PLATFORM_MAP_WINDOW_TILES_WIDTH; x ++) {
                    if(tileMap[(y) * PLATFORM_MAP_WINDOW_TILES_WIDTH + x] == 0 )
                        continue;
            
                    decodeTiles(tileMap, x, y, true);
                }
        }

        uint32_t intensity = (15 - fadeIntensity) << 24;
        uint32_t abgr = intensity | (intensity << 4) | fadeBaseColor;
        C2D_DrawRectSolid(0, 0, 0, 400, 240, abgr);
    }

    C2D_SceneBegin(bottomScreen);

    if(!renderQueueBot.empty()) {
        
        for(auto& sprite : renderQueueBot) {
            switch (sprite.flags)
            {
            case DRAWRECT:
                C2D_DrawRectSolid(sprite.rect.x, sprite.rect.y, 0, sprite.rect.w, sprite.rect.h, sprite.colour);
                break;
            default:
                C2D_DrawSpriteTinted(&sprite.sprite, &sprite.tint);
                break;
            }
        }
        renderQueueBot.clear();
    }

    C3D_FrameEnd(0);
    if(fadeIntensity != 15)
        svcSleepThread(44000000);
}
