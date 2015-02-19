

         ctplay ver. 0.3
                           2015/1/20

-----------
What's this
-----------
This is a midi player which generates old
console game sound. ctplay has 2 types of  
synthesizer.

wavetable synth:
  can generate Square, Saw, and Triange waves. 
  has 16 presets.

noise rhythm synth:
  can generate rhythm sound.
  has 4 presets(kick, snare, open-hihat,
  closed-hihat)


-------------------
System Requirements
-------------------
OS: MS Windows7 or higher
CPU: Fast processors


----------
How to use
----------
Drop a midi file into the ctplay window.


--------------
User Interface
--------------
The intruments name is clickable.
  Left click : make the track mute or not.
  Right Clock  : make the track solo or not.

Music repeats at default.
You can toggle it by
clicking "repeat".

Transpose can be done by clicking
< or > besides "Key".


--------
Features
--------
16 multi tracks MIDI sequencer
Track 1-9,11-16 are for musical instruments.
Track 10 is dedicated for the rhythm.
maximum 124 voices
  120 wavetable synth, 8 voices for each track
  4 for rhythm


---------------
Version History
---------------
2015/2/19 ver 0.5
  - Pitchbend support(Bendrange=2 for now)
  - Velocity curve changed to power of 2 from linear.
  - Decreased loudness of high tones

2015/1/20 ver 0.4
  - Added SDL_Delay to reduce cpu usage.


2013/11/10 ver 0.2
  - Added a function that you can drop a directory
    which has midi files into the app to play.
  - Fixed some thread related issues.

2013/11/2 ver 0.1
  - Initial release


-----------
Source code
-----------
https://github.com/sadagussy/mid2wav


-----------
Acknowledge
-----------
ctplay owes following libraries and tools.

- SDL2/SDL2_ttf .. Simple Direct Media Layer version 2
http://www.libsdl.org/

- EDGE .. for drawing bitmaps
http://takabosoft.com/edge

- Inkscape .. for creating an icon
http://inkscape.org/

- MS Paint .. for creating an icon 

- MSYS/MinGW and other gnu stuffs(compiler, debugger, etc..)
http://www.mingw.org/

- The FontStruction for linesquare_rounded_extended.ttf
http://fontstruct.fontshop.com/fontstructions/show/164768

------
Author
------
sada.gussy

Bug reports are welcome!
 to sada.gussy at gmail.

-------
LICENSE
-------

 * Copyright (c) 2013 sada.gussy (sada dot gussy at gmail dot com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
