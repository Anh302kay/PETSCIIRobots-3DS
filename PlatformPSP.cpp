#include <iostream>
#include <exception>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include <errno.h>
#include <fatms.h>
#include <libgu.h>
#include <libwave.h>
#include <displaysvc.h>
#include <ctrlsvc.h>
#include <libfpu.h>
#include <impose.h>
#include <utility/utility_common.h>
#include <psperror.h>
#include <malloc.h>
#include "PT2.3A_replay_cia.h"
#include "PlatformPSP.h"

SCE_MODULE_INFO(petrobots, 0, 1, 0 );
int sce_newlib_heap_kb_size = 18430;
unsigned int sce_user_main_thread_stack_kb_size = 16;
unsigned int sce_user_main_thread_attribute = SCE_KERNEL_TH_USE_VFPU;
static const SceChar8 *SOUND_THREAD_NAME = "Sound";

#define DISPLAYLIST_SIZE (1179648/sizeof(int))
#define CACHE_SIZE 131072

static char cache[CACHE_SIZE];
static int cacheSize = 0;

#define LARGEST_MODULE_SIZE 105654
#define TOTAL_SAMPLE_SIZE 75755
#define AUDIO_BUFFER_SIZE 1024
#define SAMPLERATE 44100

extern uint32_t font[];
extern uint32_t tiles[];
extern uint32_t introScreen[];
extern uint32_t gameScreen[];
extern uint32_t gameOver[];
extern uint8_t levelA[];
extern uint8_t levelAEnd[];
extern uint8_t moduleSoundFX[];
extern uint8_t moduleSoundFXEnd[];
extern uint8_t moduleMetalHeads[];
extern uint8_t moduleMetalHeadsEnd[];
extern int8_t soundExplosion[];
extern int8_t soundExplosionEnd[];
extern int8_t soundMedkit[];
extern int8_t soundMedkitEnd[];
extern int8_t soundEMP[];
extern int8_t soundEMPEnd[];
extern int8_t soundMagnet[];
extern int8_t soundMagnetEnd[];
extern int8_t soundShock[];
extern int8_t soundShockEnd[];
extern int8_t soundMove[];
extern int8_t soundMoveEnd[];
extern int8_t soundPlasma[];
extern int8_t soundPlasmaEnd[];
extern int8_t soundPistol[];
extern int8_t soundPistolEnd[];
extern int8_t soundItemFound[];
extern int8_t soundItemFoundEnd[];
extern int8_t soundError[];
extern int8_t soundErrorEnd[];
extern int8_t soundCycleWeapon[];
extern int8_t soundCycleWeaponEnd[];
extern int8_t soundCycleItem[];
extern int8_t soundCycleItemEnd[];
extern int8_t soundDoor[];
extern int8_t soundDoorEnd[];
extern int8_t soundMenuBeep[];
extern int8_t soundMenuBeepEnd[];
extern int8_t soundShortBeep[];
extern int8_t soundShortBeepEnd[];

uint32_t* images[] = { introScreen, gameScreen, gameOver };

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
static const char* moduleFilenames[] = {
    "mod.soundfx",
    "mod.metal heads",
    "mod.win",
    "mod.lose",
    "mod.metallic bop amiga",
    "mod.get psyched",
    "mod.robot attack",
    "mod.rushin in"
};

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
    0, // SEARCH OBEJCT
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

void debug(const char* message, ...)
{
    FILE* f = fopen(SCE_FATMS_ALIAS_NAME "/log.txt", "a");
    va_list argList;
    va_start(argList, message);
    vfprintf(f, message, argList);
    fflush(f);
    fclose(f);
    va_end(argList);
}

PlatformPSP::PlatformPSP() :
    interrupt(0),
    framesPerSecond_(60),
    moduleData(new uint8_t[LARGEST_MODULE_SIZE]),
    loadedModule(ModuleSoundFX),
    effectChannel(0),
    audioBuffer(new int16_t[AUDIO_BUFFER_SIZE]),
    audioOutputBuffer(new SceShort16[AUDIO_BUFFER_SIZE * 2 * 2]),
    audioOutputBufferOffset(0),
    audioThreadId(0),
    interruptIntervalInSamples(SAMPLERATE / 60),
    samplesSinceInterrupt(SAMPLERATE / 60),
    displayList(new int[DISPLAYLIST_SIZE]),
    joystickStateToReturn(0),
    joystickState(0),
    pendingState(0)
{
    // Increase thread priority
    sceKernelChangeThreadPriority(SCE_KERNEL_TH_SELF, 40);

    // Disable all exceptions
    unsigned int FCR31 = sceFpuGetFCR31();
    FCR31 &= ~(SCE_ALLEGREX_FCR31_ENABLES_V |
                       SCE_ALLEGREX_FCR31_ENABLES_Z |
                       SCE_ALLEGREX_FCR31_ENABLES_O |
                       SCE_ALLEGREX_FCR31_ENABLES_U |
                       SCE_ALLEGREX_FCR31_ENABLES_I |
                       SCE_ALLEGREX_FCR31_CAUSE_E |
                       SCE_ALLEGREX_FCR31_CAUSE_V |
                       SCE_ALLEGREX_FCR31_CAUSE_Z |
                       SCE_ALLEGREX_FCR31_CAUSE_O |
                       SCE_ALLEGREX_FCR31_CAUSE_U |
                       SCE_ALLEGREX_FCR31_CAUSE_I |
                       SCE_ALLEGREX_FCR31_FLAGS_V |
                       SCE_ALLEGREX_FCR31_FLAGS_Z |
                       SCE_ALLEGREX_FCR31_FLAGS_O |
                       SCE_ALLEGREX_FCR31_FLAGS_U |
                       SCE_ALLEGREX_FCR31_FLAGS_I);
    sceFpuSetFCR31(FCR31);

    // Set the impose language
    sceImposeSetLanguageMode(SCE_UTILITY_LANG_ENGLISH, SCE_UTILITY_CTRL_ASSIGN_CROSS_IS_ENTER);

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALONLY);

    if (sceWaveInit() != SCE_OK) {
        debug("Couldn't initialize libwave\n");
    }

    int ret = sceWaveAudioSetFormat(0, SCE_WAVE_AUDIO_FMT_S16_STEREO);
    if (ret != SCE_OK && ret != (int)SCE_AUDIO_ERROR_OUTPUT_BUSY) {
        debug("Couldn't set audio format\n");
    }

    sceWaveAudioSetSample(0, AUDIO_BUFFER_SIZE);
    sceWaveAudioSetVolume(0, SCE_WAVE_AUDIO_VOL_MAX, SCE_WAVE_AUDIO_VOL_MAX);

    sceGuInit();

    sceGuStart(SCEGU_IMMEDIATE, displayList, DISPLAYLIST_SIZE * sizeof(int));

    sceGuDrawBuffer(SCEGU_PF8888, SCEGU_VRAM_BP32_0, SCEGU_VRAM_WIDTH);
    sceGuDispBuffer(SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT, SCEGU_VRAM_BP32_0, SCEGU_VRAM_WIDTH);
    sceGuDepthBuffer(SCEGU_VRAM_BP32_2, SCEGU_VRAM_WIDTH);

    sceGuOffset(SCEGU_SCR_OFFSETX, SCEGU_SCR_OFFSETY);
    sceGuViewport(2048, 2048, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT);
    sceGuDepthRange(100, 65535);

    sceGuScissor(0, 0, PLATFORM_SCREEN_WIDTH - 56, PLATFORM_SCREEN_HEIGHT - 32);
    sceGuDisable(SCEGU_SCISSOR_TEST);
    sceGuDisable(SCEGU_CLIP_PLANE);

    sceGuDepthFunc(SCEGU_LEQUAL);
    sceGuEnable(SCEGU_TEXTURE);

    sceGuColor(0xffffffff);
    sceGuClearColor(0xff000000);
    sceGuClearDepth(0);
    sceGuClearStencil(0);
    sceGuClear(SCEGU_CLEAR_ALL);

    sceGuTexEnvColor(0xffffff);
    sceGuTexScale(1.0f, 1.0f);
    sceGuTexOffset(0.0f, 0.0f);
    sceGuTexWrap(SCEGU_CLAMP, SCEGU_CLAMP);
    sceGuTexMapMode(SCEGU_UV_MAP, 0, 1);
    sceGuTexFilter(SCEGU_NEAREST, SCEGU_NEAREST);
    sceGuModelColor(0x00000000, 0xffffffff, 0xffffffff, 0xffffffff);

    sceGuFrontFace(SCEGU_CW);
    sceGuShadeModel(SCEGU_SMOOTH);
    sceGuDisable(SCEGU_DEPTH_TEST);
    sceGuEnable(SCEGU_BLEND);
    sceGuDisable(SCEGU_FOG);
    sceGuDisable(SCEGU_LIGHTING);
    sceGuBlendFunc(SCEGU_ADD, SCEGU_SRC_ALPHA, SCEGU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuTexFunc(SCEGU_TEX_MODULATE, SCEGU_RGBA);

    sceGuDisplay(SCEGU_DISPLAY_ON);

    sceGuFinish();
    sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);

    sceGuStart(SCEGU_IMMEDIATE, displayList, DISPLAYLIST_SIZE * sizeof(int));

    platform = this;

    PlatformPSP* p = this;
    audioThreadId = sceKernelCreateThread(SOUND_THREAD_NAME, audioThread, SCE_KERNEL_USER_HIGHEST_PRIORITY, 1024, 0, NULL);
    sceKernelStartThread(audioThreadId, sizeof(PlatformPSP *), &p);
    sceKernelStartThread(sceKernelCreateThread("update_thread", callbackThread, 0x11, 2048, 0, NULL), sizeof(PlatformPSP*), &p);
}

PlatformPSP::~PlatformPSP()
{
    sceGuFinish();
    sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);
    sceGuTerm();

    if (audioThreadId != -1) {
        sceKernelWaitThreadEnd(audioThreadId, NULL);
    }

    sceWaveExit();

    delete[] displayList;
    delete[] audioOutputBuffer;
    delete[] audioBuffer;
    delete[] moduleData;

    sceKernelExitGame();
}

int PlatformPSP::callbackThread(SceSize args, void* argp)
{
    PlatformPSP* platform = *((PlatformPSP**)argp);
    sceKernelRegisterExitCallback(sceKernelCreateCallback("Exit Callback", exitCallback, platform));

    // Display callback notifications
    while (!platform->quit) {
        sceDisplayWaitVblankStartCB();
    }

    return 0;
}

int PlatformPSP::exitCallback(int arg1, int arg2, void* common)
{
    PlatformPSP* platform = (PlatformPSP*)common;
    platform->quit = true;

    return 0;
}

SceInt32 PlatformPSP::audioThread(SceSize args, SceVoid *argb)
{
    PlatformPSP* platform = *((PlatformPSP**)argb);

    // Render loop
    while (!platform->quit) {
        int16_t *bufferPosition = platform->audioBuffer;
        for (int samplesLeft = AUDIO_BUFFER_SIZE; samplesLeft > 0;) {
            // Number of samples to process before VBI
            int samplesToProcess = platform->samplesSinceInterrupt + samplesLeft >= platform->interruptIntervalInSamples ? platform->interruptIntervalInSamples - platform->samplesSinceInterrupt : samplesLeft;

            // Process each audio channel
            processAudio(bufferPosition, samplesToProcess, SAMPLERATE);
            bufferPosition += samplesToProcess;

            // Run the vertical blank interupt if required
            platform->samplesSinceInterrupt += samplesToProcess;
            if (platform->samplesSinceInterrupt >= platform->interruptIntervalInSamples) {
                if (platform->interrupt) {
                    (*platform->interrupt)();
                }
                platform->samplesSinceInterrupt -= platform->interruptIntervalInSamples;
            }

            // Samples left
            samplesLeft -= samplesToProcess;
        }

        // Render to the actual output buffer
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
            platform->audioOutputBuffer[platform->audioOutputBufferOffset + i * 2] = platform->audioBuffer[i];
            platform->audioOutputBuffer[platform->audioOutputBufferOffset + i * 2 + 1] = platform->audioBuffer[i];
        }

        // Queue the output buffer for playback
        sceWaveAudioWriteBlocking(0, SCE_WAVE_AUDIO_VOL_MAX, SCE_WAVE_AUDIO_VOL_MAX, platform->audioOutputBuffer + platform->audioOutputBufferOffset);

        // Write the next data to the other part of the output buffer
        platform->audioOutputBufferOffset ^= AUDIO_BUFFER_SIZE * 2;
    }

    return 0;
}

void PlatformPSP::drawRectangle(uint32_t* texture, uint32_t color, uint16_t tx, uint16_t ty, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    sceGuEnable(SCEGU_TEXTURE);
    sceGuTexImage(0, texture[2], texture[3], texture[2], texture + 5);
    sceGuTexMode(SCEGU_PF8888, 0, 0, SCEGU_TEXBUF_NORMAL);
    sceGuColor(color);

    int oldCacheSize = cacheSize;
    uint16_t* data = (uint16_t*)(cache + cacheSize);
    data[0 * 5 + 0] = tx;
    data[0 * 5 + 1] = ty;
    data[0 * 5 + 2] = x;
    data[0 * 5 + 3] = y;
    data[0 * 5 + 4] = 0;
    data[1 * 5 + 0] = tx + width;
    data[1 * 5 + 1] = ty + height;
    data[1 * 5 + 2] = x + width;
    data[1 * 5 + 3] = y + height;
    data[1 * 5 + 4] = 0;
    cacheSize += 2 * 5 * sizeof(uint16_t);

    sceKernelDcacheWritebackRange(data, cacheSize - oldCacheSize);
    sceGuDrawArray(SCEGU_PRIM_RECTANGLES, SCEGU_TEXTURE_USHORT | SCEGU_VERTEX_SHORT | SCEGU_THROUGH, 2, 0, data);
}

void PlatformPSP::undeltaSamples(uint8_t* module, uint32_t moduleSize)
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

void PlatformPSP::setSampleData(uint8_t* module)
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
    putWord((uint8_t*)&sampleData[15 + 0].length, 0, (uint16_t)(soundExplosionEnd - soundExplosion) >> 1);
    putWord((uint8_t*)&sampleData[15 + 1].length, 0, (uint16_t)(soundShortBeepEnd - soundShortBeep) >> 1);
    putWord((uint8_t*)&sampleData[15 + 2].length, 0, (uint16_t)(soundMedkitEnd - soundMedkit) >> 1);
    putWord((uint8_t*)&sampleData[15 + 3].length, 0, (uint16_t)(soundEMPEnd - soundEMP) >> 1);
    putWord((uint8_t*)&sampleData[15 + 4].length, 0, (uint16_t)(soundMagnetEnd - soundMagnet) >> 1);
    putWord((uint8_t*)&sampleData[15 + 5].length, 0, (uint16_t)(soundShockEnd - soundShock) >> 1);
    putWord((uint8_t*)&sampleData[15 + 6].length, 0, (uint16_t)(soundMoveEnd - soundMove) >> 1);
    putWord((uint8_t*)&sampleData[15 + 7].length, 0, (uint16_t)(soundShockEnd - soundShock) >> 1);
    putWord((uint8_t*)&sampleData[15 + 8].length, 0, (uint16_t)(soundPlasmaEnd - soundPlasma) >> 1);
    putWord((uint8_t*)&sampleData[15 + 9].length, 0, (uint16_t)(soundPistolEnd - soundPistol) >> 1);
    putWord((uint8_t*)&sampleData[15 + 10].length, 0, (uint16_t)(soundItemFoundEnd - soundItemFound) >> 1);
    putWord((uint8_t*)&sampleData[15 + 11].length, 0, (uint16_t)(soundErrorEnd - soundError) >> 1);
    putWord((uint8_t*)&sampleData[15 + 12].length, 0, (uint16_t)(soundCycleWeaponEnd - soundCycleWeapon) >> 1);
    putWord((uint8_t*)&sampleData[15 + 13].length, 0, (uint16_t)(soundCycleItemEnd - soundCycleItem) >> 1);
    putWord((uint8_t*)&sampleData[15 + 14].length, 0, (uint16_t)(soundDoorEnd - soundDoor) >> 1);
    putWord((uint8_t*)&sampleData[15 + 15].length, 0, (uint16_t)(soundMenuBeepEnd - soundMenuBeep) >> 1);
    for (int i = 0; i < 16; i++) {
        sampleData[15 + i].volume = 64;
    }
}

uint8_t* PlatformPSP::standardControls() const
{
    return ::standardControls;
}

void PlatformPSP::setInterrupt(void (*interrupt)(void))
{
    this->interrupt = interrupt;
}

int PlatformPSP::framesPerSecond()
{
    return framesPerSecond_;
}

uint8_t PlatformPSP::readKeyboard()
{
    return 0xff;
}

void PlatformPSP::keyRepeat()
{
    joystickStateToReturn = joystickState;
}

void PlatformPSP::clearKeyBuffer()
{
    joystickStateToReturn = 0;
}

bool PlatformPSP::isKeyOrJoystickPressed(bool gamepad)
{
    return joystickState != 0 && joystickState != JoystickPlay;
}

uint16_t PlatformPSP::readJoystick(bool gamepad)
{
    uint16_t state = 0;

    // Read new input
    SceCtrlData ct;
    if (sceCtrlReadBufferPositive(&ct, 1) >= 0) {
        if (ct.Buttons & SCE_CTRL_LEFT) {
            state |= JoystickLeft;
        }
        if (ct.Buttons & SCE_CTRL_RIGHT) {
            state |= JoystickRight;
        }
        if (ct.Buttons & SCE_CTRL_UP) {
            state |= JoystickUp;
        }
        if (ct.Buttons & SCE_CTRL_DOWN) {
            state |= JoystickDown;
        }
        if (ct.Buttons & SCE_CTRL_SQUARE) {
            state |= JoystickGreen;
        }
        if (ct.Buttons & SCE_CTRL_CIRCLE) {
            state |= JoystickBlue;
        }
        if (ct.Buttons & SCE_CTRL_TRIANGLE) {
            state |= JoystickYellow;
        }
        if (ct.Buttons & SCE_CTRL_CROSS) {
            state |= JoystickRed;
        }
        if (ct.Buttons & SCE_CTRL_L) {
            state |= JoystickReverse;
        }
        if (ct.Buttons & SCE_CTRL_R) {
            state |= JoystickForward;
        }
        if (ct.Buttons & SCE_CTRL_START) {
            state |= JoystickPlay;
        }
    }

    if (joystickState != state) {
        if (joystickState == 0) {
            pendingState = state == JoystickPlay ? JoystickPlay : 0;
        } else if (state != 0) {
            pendingState &= state == JoystickPlay ? JoystickPlay : 0;
        }

        // Return Play button press only when released
        if (state != 0) {
            joystickStateToReturn = state != (gamepad ? JoystickPlay : JoystickBlue) ? state : 0;
        } else {
            joystickStateToReturn = pendingState ? pendingState : state;
        }
        joystickState = state;
    }

    uint16_t result = joystickStateToReturn;
    joystickStateToReturn = 0;
    return result;
}

struct FilenameMapping {
    const char* filename;
    uint8_t* data;
    uint32_t size;
};

#define FILENAME_MAPPINGS 2

static FilenameMapping filenameMappings[FILENAME_MAPPINGS] = {
    { "level-A", levelA, levelAEnd - levelA },
    { "mod.metal heads", moduleMetalHeads, moduleMetalHeadsEnd - moduleMetalHeads }
};

uint32_t PlatformPSP::load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset)
{
    for (int i = 0; i < FILENAME_MAPPINGS; i++) {
        if (strcmp(filename, filenameMappings[i].filename) == 0) {
            uint32_t availableSize = MIN(size, filenameMappings[i].size - offset);
            memcpy(destination, filenameMappings[i].data + offset, availableSize);
            return availableSize;
        }
    }

    return 0;
}

uint8_t* PlatformPSP::loadTileset(const char* filename)
{
    return 0;
}

void PlatformPSP::displayImage(Image image)
{
    this->clearRect(0, 0, PLATFORM_SCREEN_WIDTH - 1, PLATFORM_SCREEN_HEIGHT - 1);

    if (image == ImageGame) {
        drawRectangle(images[image], 0xffffffff, 320 - 56, 0, PLATFORM_SCREEN_WIDTH - 56, 0, 56, 128);

        for (int y = 128; y < (PLATFORM_SCREEN_HEIGHT - 32); y += 40) {
            drawRectangle(images[image], 0xffffffff, 320 - 56, 128, PLATFORM_SCREEN_WIDTH - 56, y, 56, MIN(40, PLATFORM_SCREEN_HEIGHT - 32 - y));
        }

        drawRectangle(images[image], 0xffffffff, 320 - 56, 168, PLATFORM_SCREEN_WIDTH - 56, PLATFORM_SCREEN_HEIGHT - 32, 56, 32);

        drawRectangle(images[image], 0xffffffff, 0, 168, 0, PLATFORM_SCREEN_HEIGHT - 32, 104, 8);

        for (int x = 104; x < (PLATFORM_SCREEN_WIDTH - 56); x += 160) {
            drawRectangle(images[image], 0xffffffff, 104, 168, x, PLATFORM_SCREEN_HEIGHT - 32, MIN(160, PLATFORM_SCREEN_WIDTH - 56 - x), 8);
        }
    } else {
        drawRectangle(images[image], 0xffffffff, 0, 0, 0, 0, images[image][0], images[image][1]);

        if (image == ImageIntro) {
            this->clearRect(32, 36, 80, 8);
        }
    }
}

void PlatformPSP::generateTiles(uint8_t* tileData, uint8_t* tileAttributes)
{
}

void PlatformPSP::renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent)
{
    sceGuEnable(SCEGU_SCISSOR_TEST);

    /*
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
    */

    drawRectangle(tiles, 0xffffffff, (tile & 15) * 24, (tile >> 4) * 24, x, y, 24, 24);

    sceGuDisable(SCEGU_SCISSOR_TEST);
}

void PlatformPSP::copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height)
{
    sceGuEnable(SCEGU_TEXTURE);
//    sceGuCopyImage(SCEGU_PF8888, sourceX, sourceY, width, height, SCEGU_VRAM_WIDTH, SCEGU_VRAM_BP32_0, destinationX, destinationY, SCEGU_VRAM_WIDTH, SCEGU_VRAM_BP32_0);
    sceGuFlush();
}

void PlatformPSP::clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    sceGuDisable(SCEGU_TEXTURE);
    sceGuColor(0xff000000);

    int oldCacheSize = cacheSize;
    uint16_t* data = (uint16_t*)(cache + cacheSize);
    data[0 * 3 + 0] = x;
    data[0 * 3 + 1] = y;
    data[0 * 3 + 2] = 0;
    data[1 * 3 + 0] = data[0] + width;
    data[1 * 3 + 1] = data[1] + height;
    data[1 * 3 + 2] = 0;
    cacheSize += 2 * 3 * sizeof(uint16_t);

    sceKernelDcacheWritebackRange(data, cacheSize - oldCacheSize);
    sceGuDrawArray(SCEGU_PRIM_RECTANGLES, SCEGU_VERTEX_SHORT | SCEGU_THROUGH, 2, 0, data);
}

void PlatformPSP::writeToScreenMemory(address_t address, uint8_t value)
{
    drawRectangle(font, 0xff55bb77, (value >> 2) & 0x0010, (value << 3) & 0xfff8, (address % SCREEN_WIDTH_IN_CHARACTERS) << 3, (address / SCREEN_WIDTH_IN_CHARACTERS) << 3, 8, 8);
}

void PlatformPSP::writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{
    drawRectangle(font, color == 14 ? 0xffffffff : 0xff55bb77, (value >> 2) & 0x0010, (value << 3) & 0xfff8, (address % SCREEN_WIDTH_IN_CHARACTERS) << 3, ((address / SCREEN_WIDTH_IN_CHARACTERS) << 3) + yOffset, 8, 8);
}

void PlatformPSP::loadModule(Module module)
{
    if (loadedModule != module) {
        uint32_t moduleSize = load(moduleFilenames[module], moduleData, LARGEST_MODULE_SIZE, 0);
        undeltaSamples(moduleData, moduleSize);
        setSampleData(moduleData);
        loadedModule = module;
    }
}

void PlatformPSP::playModule(Module module)
{
    stopModule();
    stopSample();

    loadModule(module);
    mt_init(moduleData);

    mt_Enable = true;
}

void PlatformPSP::pauseModule()
{
    mt_speed = 0;
    mt_music();
    mt_Enable = false;
    if (mt_chan1temp.n_start < soundExplosion || mt_chan1temp.n_start >= soundExplosionEnd) {
        channel0.volume = 0;
    }
    if (mt_chan2temp.n_start < soundExplosion || mt_chan2temp.n_start >= soundExplosionEnd) {
        channel1.volume = 0;
    }
    if (mt_chan3temp.n_start < soundExplosion || mt_chan3temp.n_start >= soundExplosionEnd) {
        channel2.volume = 0;
    }
    if (mt_chan4temp.n_start < soundExplosion || mt_chan4temp.n_start >= soundExplosionEnd) {
        channel3.volume = 0;
    }
}

void PlatformPSP::stopModule()
{
    mt_end();
}

void PlatformPSP::playSample(uint8_t sample)
{
    ChanInput* input = loadedModule == ModuleIntro ? &mt_chan2input : &mt_chan4input;
    if (loadedModule == ModuleSoundFX) {
        input = &mt_chan1input + (effectChannel < 2 ? effectChannel : (5 - effectChannel));

        effectChannel++;
        effectChannel &= 3;
    }

    putWord((uint8_t*)&input->note, 0, 0x1000 + 320);
    if (sample < 16) {
        putWord((uint8_t*)&input->cmd, 0, sample << 12);
    } else if (sample == 16) {
        putWord((uint8_t*)&input->cmd, 0, 1 << 12);
    } else {
        putWord((uint8_t*)&input->cmd, 0, 15 << 12);
    }
}

void PlatformPSP::stopSample()
{
    mt_chan1input.note = 0;
    mt_chan1input.cmd = 0;
    mt_chan2input.note = 0;
    mt_chan2input.cmd = 0;
    mt_chan3input.note = 0;
    mt_chan3input.cmd = 0;
    mt_chan4input.note = 0;
    mt_chan4input.cmd = 0;
}

void PlatformPSP::renderFrame(bool waitForNextFrame)
{
    sceGuFinish();
    sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);

    if (waitForNextFrame) {
        sceDisplayWaitVblankStart();
    }

    cacheSize = 0;

    sceGuStart(SCEGU_IMMEDIATE, displayList, DISPLAYLIST_SIZE * sizeof(int));
}
