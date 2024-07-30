#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define WIDTH 1200
#define HEIGHT 900

#define VERSIONNO "1.9"

#define CONFIGPATH "./metronome.config"

//------------------------------------------------------------------------------

typedef struct {
    Sound * sounds;
    int count;
} wrzBeatSounds; // return type for wrzLoadBeatSounds()

typedef struct {
    int primary_beat_no, secondary_beat_no; // suffixed "no" because this data is 1-indexed
    char * beats_directory;
    char * style_filepath;
} wrzProgramConfig;

//------------------------------------------------------------------------------

// TODO: make this function skip non-audio files
wrzBeatSounds wrzLoadBeatSounds(const char * dir) {
    wrzBeatSounds output;

    if(!DirectoryExists(dir)) {
        printf("ERROR: Could not load beat sounds filepath \"%s\"! Check that this directory exists!\n", dir);
        exit(1);
    }

    //------------------------------------------------------------------------------

    // Raylib's supported filetypes -- wav, mp3, ogg, flac, and a few more but if you're that kind of nerd you can do it yourself
    FilePathList wavs = LoadDirectoryFilesEx(dir, ".wav",  true);
    FilePathList mp3s = LoadDirectoryFilesEx(dir, ".mp3",  true);
    FilePathList oggs = LoadDirectoryFilesEx(dir, ".ogg",  true);
    FilePathList flac = LoadDirectoryFilesEx(dir, ".flac", true);
    // is it annoying that flac is not plural? less annoying than them not being all 4 characters, i think

    // monolithic filepath since i don't think i can filter for multiple filetypes
    FilePathList files = { 0 };

    files.count = wavs.count + mp3s.count + oggs.count + flac.count;
    files.paths = malloc(sizeof(char **) * files.count);

    // please let me know if this is disgusting and bad
    if(wavs.count > 0) for(int i = 0; i < wavs.count; i++) files.paths[i] = wavs.paths[i]; // copy wav paths
    if(mp3s.count > 0) for(int j = 0; j < mp3s.count; j++) files.paths[j + wavs.count] = mp3s.paths[j]; // copy mp3 paths
    if(oggs.count > 0) for(int k = 0; k < oggs.count; k++) files.paths[k + wavs.count + mp3s.count] = oggs.paths[k]; // ogg paths
    if(flac.count > 0) for(int l = 0; l < mp3s.count; l++) files.paths[l + wavs.count + mp3s.count + oggs.count] = flac.paths[l]; // and the flac paths

    free(wavs.paths); // discard the individual file lists
    free(mp3s.paths);
    free(oggs.paths);
    free(flac.paths);

    //------------------------------------------------------------------------------

    if(files.count == 0) printf("WARNING: No files found in \"%s\". Attempting to load defaults...\n", dir);
    else printf("INFO: `%s` contains %d file(s).\n", dir, files.count);

    if(files.count > 0) {
        output.sounds = malloc(files.count * sizeof(Sound));

        for(int i = 0; i < files.count; i++) {
            output.sounds[i] = LoadSound(files.paths[i]);
        }

        output.count = files.count;
    } else { // if no files are found in the beats directory, load the default one
        bool cooked = false;

        if(FileExists("./resources/beats/default-beat.wav")) { // check the default primary beat
            output.sounds = malloc(sizeof(Sound));
            output.sounds[0] = LoadSound("./resources/beats/default-beat.wav");
            output.count = 1;
        } else cooked = true; 
        // if the primary and secondary file don't exist, we're probably cooked
        
        // redundant checks kept for clarity's sake
        if(!FileExists("./resources/beats/default-beat.wav") && FileExists("./resources/beats/default-sub-beat.wav")) { 
            // if the default primary click is gone, load the default sub click in its place
            cooked = false; // we're not cooked
            output.sounds = malloc(sizeof(Sound));
            output.sounds[0] = LoadSound("./resources/beats/default-sub-beat.wav");
            output.count = 1;
        } else if(FileExists("./resources/beats/default-beat.wav") && FileExists("./resources/beats/default-sub-beat.wav")) {
            // if both default files are available, configure them correctly
            output.sounds = realloc(output.sounds, 2 * sizeof(Sound));
            output.sounds[1] = LoadSound("./resources/beats/default-sub-beat.wav");
            output.count = 2;
        } // else, cooked remains true, and neither file is loaded
        
        // if both default files are missing, then things are truly over
        if(cooked) {
            printf("ERROR: \"%s\" is empty and the defaults are missing. The West has fallen.\n", dir);
            exit(2);
        }
    }

    free(files.paths);

    return output;
}

void wrzDestroyBeatSounds(wrzBeatSounds * b) {
    free(b->sounds);
}

//------------------------------------------------------------------------------

wrzProgramConfig wrzLoadProgramConfig(const char * filepath) {
    //------------------------------------------------------------------------------

    wrzProgramConfig output = { 0 };

    if(!FileExists(filepath)) { // if the file does not exist, crash
        // TODO: make this create and load a default configuration file
        printf("WARNING: CONFIG: Config file \"%s\" not found! Creating \"./metronome.config\" with default settings.\n", filepath);

        FILE * default_config = fopen("./metronome.config", "w"); // create the default config file

        fprintf_s(default_config, "PRIMARY = 1\nSECONDARY = 2\n"); // write default beat configuration
        fprintf_s(default_config, "BEATSDIR = \"./resources/beats/\"\nSTYLEPATH = \"\""); // intentionally empty stylepath by default

        fclose(default_config);
    }

    //------------------------------------------------------------------------------

    // if filepath does not exist, load the default that we just created
    FILE * config_file = fopen((FileExists(filepath)) ? filepath : "./metronome.config", "r");

    int imported_beat_idx = -1; // holder variables that file data will be loaded into before processing
    int imported_sub_beat_idx = -1;

    char imported_beats_dir_buffer[255]; // holder buffers as above
    char imported_style_path_buffer[255];

    memset(imported_beats_dir_buffer, '\0', 255); // pre-set memory to avoid funny business 
    memset(imported_style_path_buffer, '\0', 255);

    char config_file_line[255]; // line buffer for when the file is read line-by-line
    memset(config_file_line, '\0', 255);

    // typically, fgets() is used in a while loop ie. while(fgets(...)); this is just an unwrapped version of such a loop
    fgets(config_file_line, 255, config_file);
    sscanf_s(config_file_line, "PRIMARY = %d", &imported_beat_idx); // load config file data into the holder variable
    printf("INFO: CONFIG: Loaded config option, primary beat sound set to #%d.\n", imported_beat_idx);

    memset(config_file_line, '\0', 255); // clear the line buffer to avoid funny business

    // every fgets() call advances by one line
    fgets(config_file_line, 255, config_file);
    sscanf_s(config_file_line, "SECONDARY = %d", &imported_sub_beat_idx); // load config data as above
    printf("INFO: CONFIG: Loaded config option, secondary beat sound set to #%d.\n", imported_sub_beat_idx);

    memset(config_file_line, '\0', 255);

    fgets(config_file_line, 255, config_file);
    sscanf_s(config_file_line, "BEATSDIR = \"%s\"", &imported_beats_dir_buffer);
    imported_beats_dir_buffer[strlen(imported_beats_dir_buffer) - 1] = '\0'; // chop off trailing \" character that gets pulled in by fgets()

    if(!DirectoryExists(imported_beats_dir_buffer)) { // check the imported data for cogency
        printf("WARNING: CONFIG: Provided beats directory \"%s\" does not exist! Attemping to load default directory...\n", imported_beats_dir_buffer);

        if(!DirectoryExists("./resources/beats/")) { // if the default directory is gone
            printf("ERROR: CONFIG: No beats directory found!\n"); // that means neither the specified nor default directory exists
            exit(4); // can't do much with no beats, so we crash
        }
        
        strcpy(imported_beats_dir_buffer, "./resources/beats"); // otherwise, use the default beat directory (not guaranteed to have anything inside it, though)
    }
    
    printf("INFO: CONFIG: Loaded config option, beats directory is \"%s\".\n", imported_beats_dir_buffer);

    memset(config_file_line, '\0', 255);

    fgets(config_file_line, 255, config_file);
    sscanf_s(config_file_line, "STYLEPATH = \"%s\"", &imported_style_path_buffer);
    imported_style_path_buffer[strlen(imported_style_path_buffer) - 1] = '\0'; // chop off trailing \" character that gets pulled in by fgets()

    // if it does not exist, we will set it to NULL below
    if(FileExists(imported_style_path_buffer)) printf("INFO: CONFIG: Loaded config option, style path is \"%s\".\n", imported_style_path_buffer);

    fclose(config_file);

    //------------------------------------------------------------------------------

    output.primary_beat_no = imported_beat_idx; // 1-idx
    output.secondary_beat_no = imported_sub_beat_idx; // 1-idx

    output.beats_directory = malloc(strlen(imported_beats_dir_buffer)); // load directory that we now know exists into config options
    strcpy(output.beats_directory, imported_beats_dir_buffer); // note that this directory could be empty, wrzLoadBeatSounds() performs that check

    // note that the reason there is no default style path/folder like there is for the beats is that raygui.h packages the default style in the code itself
    if(FileExists(imported_style_path_buffer)) { // if we have a style path
        output.style_filepath = malloc(strlen(imported_style_path_buffer));
        strcpy(output.style_filepath, imported_style_path_buffer);
    } else { // use the raygui default one, which comes in raygui.h
        output.style_filepath = NULL;
        printf("WARNING: CONFIG: Style \"%s\" not found! Loading raygui default style.\n", imported_style_path_buffer);
    }

    // we did not malloc() the input buffers, so no need to free them
    return output;
}

void wrzDestroyProgramConfig(wrzProgramConfig * c) {
    free(c->beats_directory);
    free(c->style_filepath);
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

void wrzDrawStaticElements(Font font, float text_spacing, Color bgc, Color c, Color txtc) {

    const char * title = TextFormat("WRZ: Metronome v.%s -- %03d FPS", VERSIONNO, GetFPS());
    // "static" of course meaning non-user-interactable, not completely unchanging.
    const char * to_exit = "Press ESC to exit.";

    int to_exit_width = (int) (MeasureTextEx(font, to_exit, 20, text_spacing)).x;

    //------------------------------------------------------------------------------
    DrawRectangle(0, 0, WIDTH, 50, Fade(c, .50f)); // background for title text
    DrawRectangle(0, 0, WIDTH, 40, c);

    DrawTextEx(font, title, (Vector2) { 10, 10 }, 20, text_spacing, txtc);
    DrawTextEx(font, to_exit, (Vector2) { (WIDTH - 10 - to_exit_width), 10 }, 20, text_spacing, txtc);
    //------------------------------------------------------------------------------
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 460, 30.0f, bgc); // main metronome background triangle
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 440, 30.0f, Fade(c, .25f));
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 420, 30.0f, Fade(c, .50f));
    DrawPoly((Vector2) { WIDTH / 2, 550 }, 3, 400, 30.0f, c);
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

// TODO: put this in the configuration file
// used in wrzSpeedSelectionButtons(), just some common tempi to be rendered in button form for ease of use
// these are customizable, just edit this list. max bpm is 300, min is 1
int left_side_numbers[9] =  { 108, 120, 128, 132, 136, 140, 144, 148, 152 };
int right_side_numbers[9] = { 100, 96,  92,  88,  80,  72,  66,  60,  52 };

// NOTE: modifies `bpm`
void wrzSpeedSelectionButtons(float * bpm) {
    // left side, dx = -35, dy = 60
    for(int i = 0; i < (sizeof(left_side_numbers) / sizeof(int)); i++) {
        int render_bpm = left_side_numbers[i]; //                                                                  ------         ------- ternary checks for necessary offset
        if( GuiButton((Rectangle) { 480 - (35 * i), 180 + (60 * i), 100, 50 }, TextFormat((render_bpm > 99) ? "%03d      " : "%02d       ", render_bpm)) ) *bpm = (float) render_bpm;    
    }

    // right side, now, dx = 35
    for(int j = 0; j < (sizeof(right_side_numbers) / sizeof(int)); j++) { // i like to use i, j, k, l when i'm making two loops that do pretty much the same thing; this is not necessary
        int render_bpm = right_side_numbers[j]; //                                                             ------         ------- ternary checks for necessary offset 
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
void wrzDrawBPM(int bpm, int subdivision, Font font, float text_spacing, Color text_color) {
    const char * text = TextFormat("%d", bpm);
    int width = (MeasureTextEx(font, text,     140, text_spacing)).x;
    DrawTextEx(font, text, (Vector2) { (WIDTH - width) / 2, 450 }, 140, text_spacing, text_color);

    if(subdivision > 1) { // do not render sub-bpm text if it's the same (bpm * 1 = bpm) as the base bpm
        int sub_bpm = bpm * subdivision;

        const char * sub_text = TextFormat("%d", sub_bpm);    
        int sub_width = (MeasureTextEx(font, sub_text, 40,  text_spacing)).x;
        DrawTextEx(font, sub_text, (Vector2) { (WIDTH - sub_width) / 2, 575 }, 40, text_spacing, text_color);
    }
}

//------------------------------------------------------------------------------

int main(void) {
    //------------------------------------------------------------------------------

    InitWindow(WIDTH, HEIGHT, "WRZ: Metronome v." VERSIONNO);

    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------

    // Load icon image
    Image icon_img = LoadImage("./resources/icon.png");

    // Set icon 
    SetWindowIcon(icon_img);

    // Unload icon image from memory
    UnloadImage(icon_img);

    //------------------------------------------------------------------------------

    wrzProgramConfig config = wrzLoadProgramConfig(CONFIGPATH);

    //------------------------------------------------------------------------------

    // if there is a specified style in the config file, use it
    if(config.style_filepath != NULL) GuiLoadStyle(config.style_filepath);
    // else, use the default one, wrzLoadProgramConfig() sets the value to NULL if the file does not exist/is not specified

    // pre-get the colors to pass to the draw functions, because these need not be read more than once
    Color clear_color = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
    Color fill_color = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));
    Color text_color = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));

    Font font = GuiGetFont(); // pre-load the font as were the colors above
    float text_spacing = GuiGetStyle(DEFAULT, TEXT_SPACING); // pre-load as above

    if(config.style_filepath == NULL) text_spacing *= 2; // this is just to fix the default style's spacing being too small for my eyes

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20); // raygui, set the style's text size to 20

    //------------------------------------------------------------------------------

    InitAudioDevice();

    wrzBeatSounds sounds = wrzLoadBeatSounds(config.beats_directory); // load beat sounds from filesystem
    // NOTE: this function SHOULD capture errors with missing files by itself

    Sound beat_sound = { 0 };
    Sound sub_beat_sound = { 0 };

    // which beat and sub-beat are to be played, indexing into sounds.sounds[]
    // note that we subtract one because the rendered indices in the GUI are 1-indexed, while in sounds.sounds[] they are 0-indexed
    int beat_idx = (config.primary_beat_no != -1 && (config.primary_beat_no - 1) < sounds.count) ? config.primary_beat_no - 1 : 0;
    int sub_beat_idx = (config.secondary_beat_no != -1 && (config.secondary_beat_no - 1) < sounds.count) ? config.secondary_beat_no - 1 : 0;

    beat_sound = sounds.sounds[beat_idx]; // load the specified sounds
    sub_beat_sound = sounds.sounds[sub_beat_idx];

    //------------------------------------------------------------------------------

    // raygui sliders work in floats, not integers, so this must be a float, and is converted to int when necessary
    float bpm = 60.0f;

    // change in time since the last frame
    double deltaTime = 0.0f;

    // prepare speed input buffer
    int input_buffer_size = 4; // max input is { '3', '0', '0', '\0' }
    char * input_buffer = malloc(input_buffer_size);
    memset(input_buffer, '\0', input_buffer_size); // memset to avoid funny business

    // subdivision counters
    int subdivision = 1; // denotes which fraction (1 / subdivision) of the beat we are using
    int sub_play_counter = 0; // counts how many times in a single beat the sound has been played, used for tracking which sound to play

    while(!WindowShouldClose()) {
        BeginDrawing();

            ClearBackground(clear_color);

            //------------------------------------------------------------------------------

            wrzSpeedSelectionButtons(&bpm); // draw speed selection buttons below background triangle + get bpm

            wrzDrawStaticElements(font, text_spacing, clear_color, fill_color, text_color); // draw the title and background triangle

            wrzSpeedSelectionSlider(&bpm); // draw the slider + get/set bpm

            wrzSpeedInputBox(&input_buffer, input_buffer_size, &bpm); // draw the text input box + get/set bpm

            wrzSubdivisionSelectionButton(&subdivision); // draw the subdivision button + get subdivision

            // beat change is 0 normally, 1 if the primary has changed, and 2 if the secondary has changed
            int beat_change = wrzSelectBeatSounds(&beat_idx, &sub_beat_idx, sounds.count);

            // it should not be possible to click both buttons in the same frame
            if(beat_change == 2) { // the second beat button has been changed
                sub_beat_sound = sounds.sounds[sub_beat_idx];
            } else if(beat_change == 1) { // the first beat button has been changed
                beat_sound = sounds.sounds[beat_idx];
            } // else, no change

            //------------------------------------------------------------------------------
            
            double spb = 60 * (1 / (double) (bpm * subdivision)); // convert from beats-per-minute to seconds-per-beat

            deltaTime += (double) GetFrameTime(); // add to the change in time since last click

            if(deltaTime >= spb) { // if it has been long enough for the next beat
                if(sub_play_counter > 0) PlaySound(sub_beat_sound); // play the sub beat sound, if it's time to
                else PlaySound(beat_sound); // otherwise, play the primary beat sound

                deltaTime = 0.0f; // reset the timer
                if(subdivision > 1) sub_play_counter = (sub_play_counter + 1) % subdivision; // increment the subdivision counter with overflow
                else sub_play_counter = 0; // if subdivision is 1, always play the main beat sound
            }

            //------------------------------------------------------------------------------

            wrzBeatAnimation((float) deltaTime, (float) spb); // play the beating animation

            wrzDrawBPM((int) floor(bpm), subdivision, font, text_spacing, text_color); // draw the bpm text over the beating animation

        EndDrawing();
    }

    wrzDestroyBeatSounds(&sounds); // free sounds->sounds
    free(input_buffer); // free the input buffer that is used by wrzSpeedInputBox()

    CloseWindow();

    CloseAudioDevice();

    //------------------------------------------------------------------------------

    /* file format is such that
    PRIMARY = INT
    SECONDARY = INT
    BEATSDIR = "..."
    STYLEPATH = "..."
    */
    
    // TODO: investigate whether this works on Linux ie. whether this works over both \r\n and \n systems

    // note that this deletes previous config, we will rewrite it
    FILE * config_file = fopen("./metronome.config", "w+");

    // write to the newly created config file
    fprintf_s(config_file, "PRIMARY = %d\nSECONDARY = %d\n", beat_idx + 1, sub_beat_idx + 1); // adding one to from 0-idx to 1-idx

    printf("INFO: CONFIG: Updated config file, primary = %d and secondary = %d.\n", beat_idx + 1, sub_beat_idx + 1); // same here

    fprintf_s(config_file, "BEATSDIR = \"%s\"\nSTYLEPATH = \"%s\"", config.beats_directory, config.style_filepath);

    fclose(config_file);

    wrzDestroyProgramConfig(&config); // after saving the config to disk, free the strings that were malloced()

    //------------------------------------------------------------------------------

    return 0;
}

// FIN. :)