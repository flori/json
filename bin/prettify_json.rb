#!/usr/bin/env ruby

require 'json'
require 'fileutils'
include FileUtils
require 'spruz/go'
include Spruz::GO

opts = go 'slhi:', args = ARGV.dup
if opts['h'] || opts['l'] && opts['s']
  puts <<EOT
Usage: #{File.basename($0)} [OPTION] [FILE]

If FILE is skipped, this scripts waits for input from STDIN. Otherwise
FILE is opened, read, and used as input for the prettifier.

OPTION can be
  -s     to output the shortest possible JSON (precludes -l)
  -l     to output a longer, better formatted JSON (precludes -s)
  -i EXT prettifies FILE in place, saving a backup to FILE.EXT
  -h     this help
EOT
  exit 0
end

json_opts = { :max_nesting => false, :create_additions => false }

document =
  if filename = args.first or filename == '-'
    File.read(filename)
  else
    STDIN.read
  end

json = JSON.parse document, json_opts

output = if opts['s']
  JSON.fast_generate json, json_opts
else # default is -l
  JSON.pretty_generate json, json_opts
end

if opts['i'] && filename
  cp filename, "#{filename}.#{opts['i']}"
  File.open(filename, 'w') { |f| f.puts output }
else
  puts output
end
