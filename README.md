# WRZ: Metronome

A basic metronome utility.



https://github.com/user-attachments/assets/f9a8662f-9e3e-4445-8ce0-31b3c52c93c2



---

### Usage

Compilation should be as simple as `make build`. The only dependency is raylib. Use the included version of `raygui.h` to avoid an error if you're using Raylib 5.0 (see below).

Click and drag on the blue slider to change the tempo, or use the text input box, or click one of the pre-written tempi.

Click on the button to the right of the slider to set the subdivision from 1 (ie. no subdivision) to 6.

Click the left and right buttons below the slider to set the primary and secondary (ie. subdivision) click sounds, respectively.

 Press `ESC` to exit.

 ### Customization

This program uses a custom config file format. By default, it will look for `./metronome.config`, and will create that file if it cannot find it. To use a custom config file, change the line in `metronome.c` that reads as follows:
```c
#define CONFIGPATH "..."
```
to the (relative or absolute) path to your file. If this path does not exist, the program will automatically warn you about it, create a default config file `./metronome.config`, and load that.

**Be careful when manually editing the config file to keep the format the same as it came by default. The config file reader does not perform very much error checking. The file MUST have all four config options, and there MUST be two quotation marks for the string values; they can be empty strings.**

To use a different beats folder, change the value of `BEATSDIR = "..."`. If that directory does not exist, the program will try to load `./resources/beats`, the default; if that does not exist, the program will error and exit. If no files are found in the specified folder, the program will try to load `./resources/beats/default-beat.wav`, then try `./resources/beats/defualt-sub-beat.wav`, and if neither of those exist, it will error and exit.

Any `.wav`, `.mp3`, .`ogg`, or `.flac` in the specified beats directory (`BEATSDIR`) will be loaded as a click sound. By default, the first loaded alphabetically will be the primary click sound, and the second loaded the secondary. Files are loaded grouped by file extension in the order given previously, `.wav`, `.mp3`, .`ogg`, then `.flac`, then alphabetically within each file type group. The program will automatically save your beat sound configuration in your config file. 

This program supports using custom raygui styles. To set a custom style, change the value of `STYLEPATH = "..."` in your config file. If that file does not exist, the program will warn you about it and use the default raygui style.

You can also change what common tempi are shown on either side of the triangle. Edit the lines in `metronome.c` above `wrzSpeedSelectionButtons()` that read as follows:
```c
int left_side_numbers[9] = { ... };
int right_side_numbers[9] = { ... };
```
with any replacement numbers (integers only) you want, so long as they are between 300 and 1, inclusive. There can be fewer than 9 numbers (the number between the square brackets [] must match the number of elements in the list), but there cannot be more than 9, and there must be at least 1. If you want to remove the common tempi, then comment out the two loops `for(...) { ... }` inside `wrzSpeedSelectionButtons()`, and for thoroughness' sake the two lists of numbers. The compiler might warn you about an empty function if you do this.

### Error compiling?

`undefined reference to TextToFloat()`: this will happen if you use Raylib 5.0 and the latest `raygui.h` (as of July 2024). Move `TextToFloat()` above `GuiValueBoxFloat()` in the code. **If you use the provided `raygui.h` you should not encounter this issue.**

### Intended Features

- ~~Text input for bpm (v.1.0)~~
- ~~Easier, more user-friendly system for selecting click sounds (v.1.1)~~
- ~~Subdivisions~~ (v.1.9) with swing percentage (v.2.0)
- Time signatures (v.2.1)
  
---

Click sound source: [errorjones via Reddit](https://www.reddit.com/r/audioengineering/comments/kg8gth/free_click_track_sound_archive/?rdt=32981) -- [direct archive link](https://stash.reaper.fm/40824/Metronomes.zip)


