#include <stdio.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define WIDTH 1200
#define HEIGHT 900

#define VERSIONNO "1.9"
#define BEATSDIR "./resources/beats/"

//------------------------------------------------------------------------------

typedef struct {
    Sound * sounds;
    int count;
} wrzBeatSounds; // return type for wrzLoadBeatSounds()

//------------------------------------------------------------------------------

// TODO: make this function skip non-audio files
wrzBeatSounds wrzLoadBeatSounds(const char * dir) {
    wrzBeatSounds output;

    if(!DirectoryExists(dir)) {
        printf("ERROR: Could not load beat sounds filepath \"%s!\" Check that this directory exists!\n", dir);
        exit(1);
    }

    // Raylib's supported filetypes -- wav, mp3, ogg, flac, qoa, xm, mod
    FilePathList wavs = LoadDirectoryFiles(dir);

    printf("INFO: BEATS: `%s` contains %d file(s).\n", dir, wavs.count);

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

int wrzSelectBeatSounds(int * primary, int * secondary, int count) {
    if( GuiButton((Rectangle) { 450, 850, 150, 40 }, TextFormat("%1d", (*primary + 1))) ) {
        // render the second button, because we won't get that far in the code
        GuiButton((Rectangle) { 600, 850, 150, 40 }, TextFormat("%1d", (*secondary + 1)));
        *primary = ((((int) *primary + 1)) % count); // increment primary counter with overflow
        return 1; // inform that the primary has changed
    }
    if( GuiButton((Rectangle) { 600, 850, 150, 40 }, TextFormat("%1d", (*secondary + 1))) ) {
        // first button has already rendered, so no need to repeat it here
        *secondary = ((((int) *secondary + 1)) % count); // increment secondary counter with overflow
        return 2; // inform that the secondary has changed
    }
    return 0;
}

//------------------------------------------------------------------------------

void wrzDrawStaticElements() {
    const char * title = TextFormat("WRZ: Metronome v.%s -- %03d FPS", VERSIONNO, GetFPS());
    // "static" of course meaning non-user-interactable, not completely unchanging.
    const char * to_exit = "Press ESC to exit.";

    int to_exit_width = MeasureText(to_exit, 20);

    //------------------------------------------------------------------------------
    DrawRectangle(0, 0, WIDTH, 50, Fade(LIGHTGRAY, .50f)); // background for title text
    DrawRectangle(0, 0, WIDTH, 40, LIGHTGRAY);

    DrawText(title, 10, 10, 20, BLACK);
    DrawText(to_exit, (WIDTH - 10 - to_exit_width), 10, 20, BLACK);
    //------------------------------------------------------------------------------
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 460, 30.0f, RAYWHITE); // main metronome background triangle
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
// TODO: make this less janky
void wrzSpeedInputBox(char ** input_buffer, int input_buffer_size, float * bpm) {
    snprintf(*input_buffer, input_buffer_size, "%03d", (int) (*bpm)); // this makes the text box kind of annoying but it seems to be necessary
    GuiTextBox((Rectangle) { 300, 800, 50, 40 }, *input_buffer, 120, true);
    *bpm = (float) atoi(*input_buffer); // i've heard atoi is no good, maybe i'll change this
}

// used in wrzSpeedSelectionButtons(), just some common tempi to be rendered in button form for ease of use
// these are customizable, just edit this list. max bpm is 300, min is 1
int left_side_numbers[9] = { 108, 120, 128, 132, 136, 140, 144, 148, 152 };
int right_side_numbers[9] = { 100, 96, 92, 88, 80, 72, 66, 60, 52 };

// NOTE: modifies `bpm`
void wrzSpeedSelectionButtons(float * bpm) {
    // left side, dx = -35, dy = 60
    for(int i = 0; i < 9; i++) {
        int render_bpm = left_side_numbers[i];
        if( GuiButton((Rectangle) { 480 - (35 * i), 180 + (60 * i), 100, 50 }, TextFormat((render_bpm > 99) ? "%03d      " : "%02d       ", render_bpm)) ) *bpm = (float) render_bpm;    
    }

    // right side, now, dx = 35
    for(int j = 0; j < 9; j++) {
        int render_bpm = right_side_numbers[j];
        if( GuiButton((Rectangle) { 620 + (35 * j), 180 + (60 * j), 100, 50 }, TextFormat((render_bpm > 99) ? "      %03d" : "       %02d", render_bpm)) ) *bpm = (float) render_bpm;
    }
}

//------------------------------------------------------------------------------

void wrzSubdivisionSelectionButton(int * subdivision) {
    // render button with overflow
    if( GuiButton((Rectangle) { 850, 800, 50, 40 }, TextFormat("%1d", (int) *subdivision))) *subdivision = ((*subdivision) % 6) + 1;
}

//------------------------------------------------------------------------------

// TODO: create an alternate animation for sub-beats
void wrzBeatAnimation(float deltaTime, float spb) {
    float raw_scale = 1.1f * (spb - (deltaTime)) / spb; 
    // calculate what fraction of time have been in between this and the next beat
    // multiplied by 1.1f and then clamped so that the triangle stays at its widest for the briefest instant
    // this helps with establishing the visual timing cue if it just stays still for a small amount of time 
    float scale = Clamp(raw_scale, 0.0f, 1.0f);
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 400 * scale, 30.0f, Fade(GRAY, 0.2f * scale));
}

// maybe this and wrzDrawSubBPM() should just be one function?
void wrzDrawBPM(int bpm) {
    const char * text = TextFormat("%d", bpm);
    int width = MeasureText(text, 140);

    DrawText(text, (WIDTH - width) / 2, 450, 140, RAYWHITE);
}

// NOTE: takes subdivided bpm
void wrzDrawSubBPM(int bpm) {
    const char * text = TextFormat("%d", bpm);
    int width = MeasureText(text, 40);
    DrawText(text, (WIDTH - width) / 2, 575, 40, RAYWHITE);
}

//------------------------------------------------------------------------------

int main(void) {
    //------------------------------------------------------------------------------

    InitWindow(WIDTH, HEIGHT, "WRZ: Metronome v." VERSIONNO);

    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20); // raygui, set the default style's text size to 20
    // this should not interfere with any custom raygui styles, though i would recommend any styles to use font size 20

    //------------------------------------------------------------------------------

    // Load icon image
    Image icon_img = LoadImage("./resources/icon.png");

    // Set icon 
    SetWindowIcon(icon_img);

    // Unload icon image from memory
    UnloadImage(icon_img);

    //------------------------------------------------------------------------------

    InitAudioDevice();
    wrzBeatSounds sounds = wrzLoadBeatSounds(BEATSDIR); // load beat sounds from filesystem
    // NOTE: this function should capture errors with missing files by itself, but i have not thoroughly tested this

    Sound beat_sound = { 0 };
    Sound sub_beat_sound = { 0 };

    // which beat and sub-beat are to be played, indexing into sounds.sounds[]
    int beat_idx;
    int sub_beat_idx;

    beat_sound = sounds.sounds[0];
    beat_idx = 0;

    if(sounds.count > 1) { // if there is more than one sound
        sub_beat_sound = sounds.sounds[1];
        sub_beat_idx = 1;
    } else { // otherwise, duplicate it
        sub_beat_sound = sounds.sounds[0];
        sub_beat_idx = 0;
    }

    //------------------------------------------------------------------------------

    // raygui sliders work in floats, not integers
    float bpm = 60.0f;

    // change in time since the last frame
    double deltaTime = 0.0f;

    // prepare speed input buffer
    int input_buffer_size = 4; // max input is '3' '0' '0' '\0'
    char * input_buffer = malloc(input_buffer_size);
    memset(input_buffer, ' ', input_buffer_size);
    input_buffer[input_buffer_size - 1] = '\0';

    // subdivision counter
    int subdivision = 1;
    int sub_play_counter = 0;

    while(!WindowShouldClose()) {
        BeginDrawing();

            ClearBackground(RAYWHITE);

            //------------------------------------------------------------------------------

            wrzSpeedSelectionButtons(&bpm); // draw speed selection buttons below background triangle + get bpm

            wrzDrawStaticElements(); // draw the title and background triangle

            wrzSpeedSelectionSlider(&bpm); // draw the slider + get/set bpm

            wrzSpeedInputBox(&input_buffer, input_buffer_size, &bpm); // draw the text input box + get/set bpm

            wrzSubdivisionSelectionButton(&subdivision); // draw the subdivision button + get subdivision

            // beat change is 0 normally, 1 if the primary has changed, and 2 if the secondary has changed
            int beat_change = wrzSelectBeatSounds(&beat_idx, &sub_beat_idx, sounds.count);

            if(beat_change == 2) {
                sub_beat_sound = sounds.sounds[sub_beat_idx];
            } else if(beat_change == 1) {
                beat_sound = sounds.sounds[beat_idx];
            } // else, no change

            //------------------------------------------------------------------------------
            
            float spb = 60 * (1 / (float) (bpm * subdivision)); // convert from beats-per-minute to seconds-per-beat

            deltaTime += GetFrameTime(); // add to the change in time since last click

            if(deltaTime >= spb) { // if it has been long enough for the next beat
                if(sub_play_counter > 0) PlaySound(sub_beat_sound); // play the sub beat sound, if it's time to
                else PlaySound(beat_sound); // otherwise, play the primary beat sound

                deltaTime = 0.0f; // reset the timer
                if(subdivision > 1) sub_play_counter = (sub_play_counter + 1) % subdivision; // increment the subdivision counter with overflow
                else sub_play_counter = 0; // if subdivision is 1, always play the main beat sound
            }

            //------------------------------------------------------------------------------

            wrzBeatAnimation(deltaTime, spb); // play the beating animation

            wrzDrawBPM((int) floor(bpm)); // draw the bpm text over the beating animation
            if(subdivision > 1) wrzDrawSubBPM((int) floor(bpm) * subdivision); // draw subdivided BPM if there is subdivision

        EndDrawing();
    }

    wrzDestroyBeatSounds(&sounds); // free sounds->sounds
    free(input_buffer); // bpm text input buffer

    CloseWindow();

    CloseAudioDevice();

    return 0;
}

// FIN. :)