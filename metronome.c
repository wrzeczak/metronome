#include <stdio.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define WIDTH 1200
#define HEIGHT 900

#define VERSIONNO "0.9"
#define BEATSDIR "./resources/beats/"

//------------------------------------------------------------------------------

typedef struct {
    Sound * sounds;
    int err, count;
} wrzBeatSounds;

//------------------------------------------------------------------------------

wrzBeatSounds wrzLoadBeatSounds(const char * dir) {
    wrzBeatSounds output;

    output.err = 0; // no error

    if(!DirectoryExists(dir)) {
        printf("ERROR: Could not load beat sounds filepath \"%s!\" Check that this directory exists!\n", dir);
        exit(1);
    }

    // Raylib's supported filetypes -- wav, mp3, ogg, flac, qoa, xm, mod
    FilePathList wavs = LoadDirectoryFiles(dir);

    printf("INFO: BEATS: `%s` contains %d files.\n", dir, wavs.count);

    if(wavs.count > 0) {
        output.sounds = malloc(wavs.count * sizeof(Sound));

        for(int i = 0; i < wavs.count; i++) {
            output.sounds[i] = LoadSound(wavs.paths[i]);
        }

        output.count = wavs.count;
    } else { // if no files are found in the beats directory, load the default one
        output.sounds = malloc(sizeof(Sound));
        output.sounds[0] = LoadSound("./resources/default-beat.wav");
        output.count = 1;
    }

    free(wavs.paths);

    return output;
}

void wrzDestroyBeatSounds(wrzBeatSounds * b) {
    free(b->sounds);
}

//------------------------------------------------------------------------------

void wrzDrawStaticElements() {
    const char * title = TextFormat("WRZ: Metronome v.%s -- %03d FPS", VERSIONNO, GetFPS());
    // "static" of course meaning non-user-interacteable, not completely unchanging.
    const char * to_exit = "Press ESC to exit.";

    int to_exit_width = MeasureText(to_exit, 20);

    //------------------------------------------------------------------------------
    DrawRectangle(0, 0, WIDTH, 50, Fade(LIGHTGRAY, .50f));
    DrawRectangle(0, 0, WIDTH, 40, LIGHTGRAY);

    DrawText(title, 10, 10, 20, BLACK);
    DrawText(to_exit, (WIDTH - 10 - to_exit_width), 10, 20, BLACK);
    //------------------------------------------------------------------------------
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 460, 30.0f, RAYWHITE);
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 440, 30.0f, Fade(LIGHTGRAY, .25f));
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 420, 30.0f, Fade(LIGHTGRAY, .50f));
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 400, 30.0f, LIGHTGRAY);
}

//------------------------------------------------------------------------------

// NOTE: modifies `bpm`
void wrzSpeedSelectionSlider(float * bpm) {
    GuiSliderBar((Rectangle) { 400, 800, 400, 40 }, NULL, NULL, bpm, 1, 300);
}

// NOTE: modifies `bpm`
void wrzSpeedInputBox(char ** input_buffer, int input_buffer_size, float * bpm) {
    bool secret = false;
    snprintf(*input_buffer, input_buffer_size, "%03d", (int) (*bpm));
    GuiTextBox((Rectangle) { 300, 800, 50, 40 }, *input_buffer, 120, true);
    *bpm = (float) atoi(*input_buffer);
}

void wrzSpeedSelectionButtons(float * bpm) {
    // TODO: there's probably a better way to do this, probably with a list of tempi and a loop...
    // NOTE: dx = -35, dy = 60, strlen = 13
    //------------------------------------------------------------------------------
    // left side
    if( GuiButton((Rectangle) { 480, 180, 100, 50 }, "108      ") ) *bpm = 108.0f;
    if( GuiButton((Rectangle) { 445, 240, 100, 50 }, "120      ") ) *bpm = 120.0f;
    if( GuiButton((Rectangle) { 410, 300, 100, 50 }, "128      ") ) *bpm = 128.0f;
    if( GuiButton((Rectangle) { 375, 360, 100, 50 }, "132      ") ) *bpm = 132.0f;
    if( GuiButton((Rectangle) { 340, 420, 100, 50 }, "136      ") ) *bpm = 136.0f;
    if( GuiButton((Rectangle) { 305, 480, 100, 50 }, "140      ") ) *bpm = 140.0f;
    if( GuiButton((Rectangle) { 270, 540, 100, 50 }, "144      ") ) *bpm = 144.0f;
    if( GuiButton((Rectangle) { 235, 600, 100, 50 }, "148      ") ) *bpm = 148.0f;
    if( GuiButton((Rectangle) { 200, 660, 100, 50 }, "152      ") ) *bpm = 152.0f;
    //------------------------------------------------------------------------------
    // now, dx = 35, dy = 60, strlen = 13
    if( GuiButton((Rectangle) { 620, 180, 100, 50 }, "      100") ) *bpm = 100.0f;
    if( GuiButton((Rectangle) { 655, 240, 100, 50 }, "       96") ) *bpm =  96.0f;
    if( GuiButton((Rectangle) { 690, 300, 100, 50 }, "       92") ) *bpm =  92.0f;
    if( GuiButton((Rectangle) { 725, 360, 100, 50 }, "       88") ) *bpm =  88.0f;
    if( GuiButton((Rectangle) { 760, 420, 100, 50 }, "       80") ) *bpm =  80.0f;
    if( GuiButton((Rectangle) { 795, 480, 100, 50 }, "       72") ) *bpm =  72.0f;
    if( GuiButton((Rectangle) { 830, 540, 100, 50 }, "       66") ) *bpm =  66.0f;
    if( GuiButton((Rectangle) { 865, 600, 100, 50 }, "       60") ) *bpm =  60.0f;
    if( GuiButton((Rectangle) { 895, 660, 100, 50 }, "       52") ) *bpm =  52.0f;
}

//------------------------------------------------------------------------------

void wrzBeatAnimation(float deltaTime, float spb) {
    float raw_scale = 1.1f * (spb - deltaTime) / spb; // what fraction of time have been in between this and the next beat
    // multiplied by 1.1f and then clamped so that the triangle stays at its widest for the briefest instant
    // this helps with establishing the visual timing cue if it just stays still for a small amount of time 
    float scale = Clamp(raw_scale, 0.0f, 1.0f);
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 400 * scale, 30.0f, Fade(GRAY, 0.2f * scale));
}

void wrzDrawBPM(int bpm) {
    const char * text = TextFormat("%d", bpm);
    int width = MeasureText(text, 140);

    DrawText(text, (WIDTH - width) / 2, 450, 140, RAYWHITE);
}

//------------------------------------------------------------------------------

int main(void) {
    //------------------------------------------------------------------------------
    InitWindow(WIDTH, HEIGHT, "WRZ: Metronome");

    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    //------------------------------------------------------------------------------

    // Load icon image
    Image icon_img = LoadImage("./resources/icon.png");

    // Set icon 
    SetWindowIcon(icon_img);

    // Unload icon image from memory
    UnloadImage(icon_img);
    //------------------------------------------------------------------------------

    InitAudioDevice();
    wrzBeatSounds sounds = wrzLoadBeatSounds(BEATSDIR);

    Sound beat_sound = { 0 };

    if(sounds.err == 0) {
        beat_sound = sounds.sounds[0];
    } // error handling should be handled by wrzLoadwrzBeatSounds() itself, so no else is needed

    //------------------------------------------------------------------------------

    // GUI sliders work in floats -- musical bpms are almost always integers
    float bpm = 60.0f;

    // change in time since the last frame
    double deltaTime = 0.0f;

    // which beat the app is using ie. index in sounds.sounds[]
    int beat_idx = 0;

    int input_buffer_size = 4; // max input is '3' '0' '0' '\0'
    char * input_buffer = malloc(input_buffer_size);
    memset(input_buffer, ' ', input_buffer_size);
    input_buffer[input_buffer_size - 1] = '\0';

    while(!WindowShouldClose()) {
        BeginDrawing();
            // rotate through beat sounds when space is pressed
            if(IsKeyPressed(KEY_SPACE)) {
                beat_idx = (beat_idx + 1) % sounds.count;
                beat_sound = sounds.sounds[beat_idx];
            }

            ClearBackground(RAYWHITE);

            //------------------------------------------------------------------------------

            wrzSpeedSelectionButtons(&bpm);

            wrzDrawStaticElements(); // draw the title and background triangle

            wrzSpeedSelectionSlider(&bpm); // get the bpm (float) from the slider

            wrzSpeedInputBox(&input_buffer, input_buffer_size, &bpm);

            //------------------------------------------------------------------------------
            
            float spb = 60 * (1 / (float) bpm); // convert from beats-per-minute to seconds-per-beat

            deltaTime += GetFrameTime(); // add to the deltaTime

            if(deltaTime >= spb) { // if it has been long enough for the next beat
                PlaySound(beat_sound);
                deltaTime = 0.0f; // reset the timer
            }

            //------------------------------------------------------------------------------

            wrzBeatAnimation(deltaTime, spb); // play the beating animation

            wrzDrawBPM((int) floor(bpm)); // draw the bpm text over the beating animation

        EndDrawing();
    }

    wrzDestroyBeatSounds(&sounds);
    free(input_buffer);

    CloseWindow();

    CloseAudioDevice();

    return 0;
}

// FIN. :)