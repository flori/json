#! /usr/bin/env jruby
require "rubygems"
require 'rake/gempackagetask'
require 'rbconfig'

spec_ext = Gem::Specification.new do |s|
  s.name = 'json'
  s.version = File.read("VERSION").chomp
  s.summary = 'JSON Implementation for Ruby'
  s.description = "This is a JSON implementation as a Ruby extension in C."

  s.files = FileList["**/*"].exclude(/CVS|pkg|tmp|coverage|Makefile|\.nfs\.|\.iml\Z/).exclude(/\.(so|bundle|o|class|#{RbConfig::CONFIG['DLEXT']})$/)

  s.extensions = FileList['ext/**/extconf.rb']

  s.require_path = 'ext/json/ext'
  s.require_paths << 'ext'
  s.require_paths << 'lib'

  s.bindir = "bin"
  s.executables = [ "edit_json.rb", "prettify_json.rb" ]
  s.default_executable = "edit_json.rb"

  s.has_rdoc = true
  s.extra_rdoc_files << 'README'
  s.rdoc_options <<
    '--title' <<  'JSON implemention for Ruby' << '--main' << 'README'
  s.test_files.concat Dir['./tests/test_*.rb']

  s.author = "Florian Frank"
  s.email = "flori@ping.de"
  s.homepage = "http://flori.github.com/json"
  s.rubyforge_project = "json"
end

if $0 == __FILE__
  Gem::Builder.new(spec_ext).build
end
spec_ext
