#!/usr/bin/env ruby
require 'rubygems'
require 'rake/packagetask'
require 'rbconfig'

spec_pure = Gem::Specification.new do |s|
  s.name = 'json_pure'
  s.version = File.read("VERSION").chomp
  s.summary = 'JSON Implementation for Ruby'
  s.description = "This is a JSON implementation in pure Ruby."

  s.files = FileList["**/*"].exclude(/CVS|pkg|tmp|coverage|Makefile|\.nfs\.|\.iml\Z/).exclude(/\.(so|bundle|o|class|#{RbConfig::CONFIG['DLEXT']})$/)

  s.require_path = 'lib'

  s.bindir = "bin"
  s.executables = [ "edit_json.rb", "prettify_json.rb" ]
  s.default_executable = "edit_json.rb"

  s.has_rdoc = true
  s.extra_rdoc_files << 'README'
  s.rdoc_options <<
    '--title' <<  'JSON implemention for ruby' << '--main' << 'README'
  s.test_files.concat Dir['./tests/test_*.rb']

  s.author = "Florian Frank"
  s.email = "flori@ping.de"
  s.homepage = "http://flori.github.com/json"
  s.rubyforge_project = "json"
end

if $0 == __FILE__
  Gem::Builder.new(spec_pure).build
end
spec_pure
