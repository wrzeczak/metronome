# WRZ: Metronome

A basic metronome utility.

---

### USAGE: 

Compilation should be as simple as `make build`. The only dependency is raylib, including `raygui.h`.

Click and drag on the blue slider to change the tempo. Press `ESC` to exit.

Any Raylib-supported audio file in `./resources/beats/` will be loaded as a click sound. Press space to cycle through them. Anything that's not audio will be "loaded," the app will simply play nothing there because Raylib's sound loader will only warn about such things. Just be sure to keep that folder cogent.

To use a different beats folder, edit the line towards the top of `metronome.c` that reads as follows:
```c
#define BEATSDIR "..."
```
And in between the quotes put the file directory. It can be relative or explicit. If no beats are provided, the app will load `./resources/beats/default-beat.wav`. Be aware that file loading can take a few seconds, so if you have many clicks, be patient.

---

Click sound source: [errorjones via Reddit](https://www.reddit.com/r/audioengineering/comments/kg8gth/free_click_track_sound_archive/?rdt=32981) -- [direct archive link](https://stash.reaper.fm/40824/Metronomes.zip)