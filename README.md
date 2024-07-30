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

Any `.wav`, `.mp3`, .`ogg`, or `.flac` in `./resources/beats/` will be loaded as a click sound. Press space to cycle through them. By default, the first alphabetically will be the primary click sound, and the second the secondary. I don't yet have a mechanism to save which sounds should be primary and secondary, so plan accordingly.

To use a different beats folder, edit the line towards the top of `metronome.c` that reads as follows:
```c
#define BEATSDIR "..."
```
and in between the quotes put the file directory. It can be relative or explicit. If no beats are provided, the app will try to load `./resources/beats/default-beat.wav`. Be aware that file loading can take a few seconds, so if you have many sounds, be patient.

You can also change what common tempi are shown on either side of the triangle. Edit the lines above `wrzSpeedSelectionButtons()` that read as follows:
```c
int left_side_numbers[9] = { ... };
int right_side_numbers[9] = { ... };
```
with any replacement numbers (integers only) you want, so long as they are between 300 and 1, inclusive. There can be fewer than 9 numbers (the number between the square brackets [] must match the number of elements in the list), but there cannot be more than 9, and there must be at least 1. If you want to remove the common tempi, then comment out the two loops inside `wrzSpeedSelectionButtons()`, and for thoroughness' sake the two lists of numbers. The compiler might warn you about an empty function if you do this.

This program supports using custom raygui styles. To set a custom style, edit the line that reads as follows:
```c
#define CUSTOMSTYLE "..."
```
with the filepath (relative or absolute) to your preferred .rgs file. The line might be commented out.

### Error compiling?

`undefined reference to TextToFloat()`: this will happen if you use Raylib 5.0 and the latest `raygui.h` (as of July 2024). Move `TextToFloat()` above `GuiValueBoxFloat()` in the code. **If you use the provided `raygui.h` you should not encounter this issue.**

### Intended Features

- ~~Text input for bpm (v.1.0)~~
- ~~Easier, more user-friendly system for selecting click sounds (v.1.1)~~
- ~~Subdivisions~~ (v.1.9) with swing percentage (v.2.0)
- Time signatures (v.2.1)
- 
---

Click sound source: [errorjones via Reddit](https://www.reddit.com/r/audioengineering/comments/kg8gth/free_click_track_sound_archive/?rdt=32981) -- [direct archive link](https://stash.reaper.fm/40824/Metronomes.zip)


