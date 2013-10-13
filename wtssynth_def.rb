=begin
 * wtssynth_def.c generator
 *
 * freq_table.rb
 *
 *
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
=end

SOURCE_FILE = "wtssynth_def.c"

def square(duty)
  res = ""
  res += "{"
  32.times do |i|
    res += i < duty ? "1.0, " : "-1.0, "
  end
  res += "}, "
  res
end

def tria(duty) # duty 1..30
  if duty == 0 or duty == 31
    saw
    return
  end
  r = -1.0
  res = ""
  res += "{"
  32.times do |i|
    if i < duty
      res += (2.0 * i / duty - 1.0).to_s + ", "
    elsif i > duty
      res += (1.0 - 2.0 * (i - duty ) / (31 - duty) ).to_s + ", "
    else
      res += "1.0, "
    end
  end
  res += "}, "
  res
end

def saw
  i = -1.0

  res = ""
  res += "{"
  32.times do |i|
    res += (- 2.0 * i/31.0 + 1.0).to_s + ", "
  end
  res += "}, "
  res
end

def sin
  res = ""
  res += "{"
  32.times do |i|
    res += Math.sin(Math::PI * 2.0 * i /32).to_s + ", "
  end
  res += "}, "
  res
end

def noise
  res = ""
  res += "{"
  32.times do |i|
    res += (rand()*2.0-1.0).to_s + ", "
  end
  res += "}, "
  res
end


source_file = <<EOD
#include "wtssynth.h"
const wtstone_t wtstone_wave_presets[WTSTONE_NUM_PRESETS] = {
EOD

source_file +=        # at    tl   dt      sl    rt     dummy
  "  {" + tria(3)    + "0,    0.5, 40000,  0.01, 20000,  0" + " },\n" + # @1-8, piano
  "  {" + square(15) + "0,    0.3, 4000,   0.01, 4000,  0" + " },\n" + # @9-16, chromatic percussion
  "  {" + tria(15)   + "0,    0.5, 0,      0.5,  4000,  0" + " },\n" + # @17-24, organ
  "  {" + square(3)  + "400,  0.5, 60000,  0.05, 10000,  0" + " },\n" + # @25-32, guitar
  "  {" + saw        + "0,    0.7, 20000,  0.05, 10000,  0" + " },\n" + # @33-40, bass
  "  {" + tria(3)    + "400,  0.5, 100000, 0.10, 30000,  0" + " },\n" + # @41-48, strings
  "  {" + saw        + "4000, 0.3, 200000, 0.05, 80000, 0" + " },\n" + # @49-56, ensamble
  "  {" + square(3)  + "4000, 0.3, 200000, 0.15, 40000, 0" + " },\n" + # @57-64, brass
  "  {" + square(15) + "4000, 0.3, 200000, 0.05, 40000, 0" + " },\n" + # @65-72, reed
  "  {" + tria(15)   + "400,  0.5, 100000, 0.1,  10000,  0" + " },\n" + # @73-80, woodwind
  "  {" + square(7)  + "4000, 0.3, 200000, 0.1,  10000,  0" + " },\n" + # @81-88, synth lead
  "  {" + tria(7)    + "4000, 0.5, 200000, 0.05, 40000, 0" + " },\n" + # @89-96, synth pad
  "  {" + noise      + "4000, 0.5, 1000,   0.1,  4000,  0" + " },\n" + # @97-104, synth effect
  "  {" + square(3)  + "400,  0.5, 1000,   0.01, 40000, 0" + " },\n" + # @105-112, ethnic
  "  {" + noise      + "0,    0.5, 10000,  0.0,  400,   0" + " },\n" + # @113-128, purcussive
  "" # end of tone parameters

source_file += <<EOD
};
EOD

File.write(SOURCE_FILE, source_file)




