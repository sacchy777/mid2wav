#
# requires ruby-2.0.0 or higher to run this script.
#

require "fiddle/import"

module DLL
  extend Fiddle::Importer
  dlload "mid2wav.dll"
  extern "void* mid2wav_open(char*, char*)"
  extern "int mid2wav_process(void*)"
  extern "void mid2wav_version(char*)"
end

if ARGV[0] == nil then
  print <<EOD
Usage: ruby mid2wav_fiddle.rb hoge.mid
EOD
  exit
end

infile = ARGV[0]
outfile = infile.gsub(/\.mid/, '.wav')

m = DLL.mid2wav_open(infile, outfile)
prog = 0
buf = "\0" * 256
DLL.mid2wav_version(buf)

print "Now converting #{infile} to #{outfile} by using #{buf}\n."

while(prog >= 0 && prog < 100) do
  prog = DLL.mid2wav_process(m)
  printf "%d%%\r", prog
  STDOUT.flush
end

