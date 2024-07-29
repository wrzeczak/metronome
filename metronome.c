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
} BeatSounds;

//------------------------------------------------------------------------------

void wrzBeatAnimation(float deltaTime, float spb);
void wrzDestroyBeatSounds(BeatSounds * b);
void wrzDrawBPM(int bpm);
void wrzDrawStaticElements();
void wrzSpeedSelectionSlider(float * bpm);

BeatSounds wrzLoadBeatSounds(const char * dir);

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
    BeatSounds sounds = wrzLoadBeatSounds(BEATSDIR);

    Sound beat_sound = { 0 };

    if(sounds.err == 0) {
        beat_sound = sounds.sounds[0];
    } // error handling should be handled by wrzLoadBeatSounds() itself, so no else is needed

    //------------------------------------------------------------------------------

    // GUI sliders work in floats -- musical bpms are almost always integers
    float bpm = 60.0f;

    // change in time since the last frame
    double deltaTime = 0.0f;

    // which beat the app is using ie. index in sounds.sounds[]
    int beat_idx = 0;

    while(!WindowShouldClose()) {
        BeginDrawing();
            // rotate through beat sounds when space is pressed
            if(IsKeyPressed(KEY_SPACE)) {
                beat_idx = (beat_idx + 1) % sounds.count;
                beat_sound = sounds.sounds[beat_idx];
            }

            ClearBackground(RAYWHITE);

            wrzDrawStaticElements(); // draw the title and background triangle

            wrzSpeedSelectionSlider(&bpm); // get the bpm (float) from the slider
            float spb = 60 * (1 / (float) bpm); // convert from beats-per-minute to seconds-per-beat

            deltaTime += GetFrameTime(); // add to the deltaTime

            if(deltaTime >= spb) { // if it has been long enough for the next beat
                PlaySound(beat_sound);
                deltaTime = 0.0f; // reset the timer
            }

            wrzBeatAnimation(deltaTime, spb); // play the beating animation

            wrzDrawBPM((int) floor(bpm)); // draw the bpm text over the beating animation

        EndDrawing();
    }

    wrzDestroyBeatSounds(&sounds);

    CloseWindow();

    CloseAudioDevice();

    return 0;
}

//------------------------------------------------------------------------------

BeatSounds wrzLoadBeatSounds(const char * dir) {
    BeatSounds output;

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

void wrzDestroyBeatSounds(BeatSounds * b) {
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
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 440, 30.0f, Fade(LIGHTGRAY, .25f));
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 420, 30.0f, Fade(LIGHTGRAY, .50f));
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 400, 30.0f, LIGHTGRAY);
}

//------------------------------------------------------------------------------

// NOTE: modifies `bpm`
void wrzSpeedSelectionSlider(float * bpm) {
    GuiSliderBar((Rectangle) { 400, 800, 400, 40 }, NULL, NULL, bpm, 1, 300);
}

void wrzDrawBPM(int bpm) {
    const char * text = TextFormat("%d", bpm);
    int width = MeasureText(text, 140);

    DrawText(text, (WIDTH - width) / 2, 450, 140, RAYWHITE);
}

//------------------------------------------------------------------------------

void wrzBeatAnimation(float deltaTime, float spb) {
    float raw_scale = 1.1f * (spb - deltaTime) / spb; // what fraction of time have been in between this and the next beat
    // multiplied by 1.1f and then clamped so that the triangle stays at its widest for the briefest instant
    // this helps with establishing the visual timing cue if it just stays still for a small amount of time 
    float scale = Clamp(raw_scale, 0.0f, 1.0f);
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 400 * scale, 30.0f, Fade(GRAY, 0.2f * scale));
}

//------------------------------------------------------------------------------

// FIN. :)