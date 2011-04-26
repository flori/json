require File.dirname(__FILE__) + '/constants'

Gem::Specification.new do |s|
  s.name = 'json_pure'
  s.version = PKG_VERSION
  s.summary = PKG_TITLE
  s.description = "This is a JSON implementation in pure Ruby."

  s.files = PKG_FILES

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
  s.homepage = "http://flori.github.com/#{PKG_NAME}"
  s.rubyforge_project = "json"
end
