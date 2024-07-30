# WRZ: Metronome

A basic metronome utility.


https://github.com/user-attachments/assets/020f7987-a645-4d39-af3d-a4fd3e894850


---

### Usage

Compilation should be as simple as `make build`. The only dependency is raylib. Use the included version of `raygui.h` to avoid an error if you're using Raylib 5.0 (see below).

Click and drag on the blue slider to change the tempo, or use the text input box, or click one of the pre-written tempi.

Click on the button to the right of the slider to set the subdivision from 1 (ie. no subdivision) to 6.

Click the left and right buttons below the slider to set the primary and secondary (ie. subdivision) click sounds, respectively.

 Press `ESC` to exit.

 ### Customization

Any Raylib-supported audio file in `./resources/beats/` will be loaded as a click sound. Press space to cycle through them. Anything that's not audio will be "loaded," the app will simply play nothing there because Raylib's sound loader will only warn about such things. Just be sure to keep that folder cogent.

To use a different beats folder, edit the line towards the top of `metronome.c` that reads as follows:
```c
#define BEATSDIR "..."
```
And in between the quotes put the file directory. It can be relative or explicit. If no beats are provided, the app will try load `./resources/beats/default-beat.wav`. Be aware that file loading can take a few seconds, so if you have many sounds, be patient.

### Error compiling?

`undefined reference to TextToFloat()`: this will happen if you use Raylib 5.0 and the latest `raygui.h`, their APIs are not totally in line. Move `TextToFloat()` above `GuiValueBoxFloat()` in the code. If you use the provided `raygui.h` you should not encounter this issue.

### Intended Features

- ~~Text input for bpm (v.1.0)~~
- ~~Easier, more user-friendly system for selecting click sounds (v.1.1)~~
- ~~Subdivisions~~ (v.1.9) with swing percentage (v.2.0)
- Time signatures (v.2.1)
- 
---

Click sound source: [errorjones via Reddit](https://www.reddit.com/r/audioengineering/comments/kg8gth/free_click_track_sound_archive/?rdt=32981) -- [direct archive link](https://stash.reaper.fm/40824/Metronomes.zip)


