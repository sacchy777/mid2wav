require "Win32API"

if ARGV[0] == nil then
  print <<EOD
Usage: ruby testdll.rb hoge.mid
EOD
  exit
end

infile = ARGV[0]
outfile = infile.gsub(/\.mid/, '.wav')

myopen = Win32API.new("mid2wav.dll", "mid2wav_open", 'PP', 'L')
myprocess = Win32API.new("mid2wav.dll", "mid2wav_process", 'L', 'I')
myversion = Win32API.new("mid2wav.dll", "mid2wav_version", 'P', 'V')

m = myopen.call(infile, outfile)
prog = 0
buf = "\0" * 256
myversion.call(buf)


print "Now converting #{infile} to #{outfile} by using #{buf}\n."

while(prog >= 0 && prog < 100) do
  prog = myprocess.call(m)
  printf "%d%%\r", prog
end

