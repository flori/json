require File.dirname(__FILE__) + '/constants'

Gem::Specification.new do |s|
  s.name = 'json'
  s.version = PKG_VERSION
  s.summary = PKG_TITLE
  s.description = "This is a JSON implementation as a Ruby extension in C."

  s.files = PKG_FILES

  s.extensions = Dir.glob('ext/**/extconf.rb')

  s.require_path = EXT_ROOT_DIR
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
  s.homepage = "http://flori.github.com/#{PKG_NAME}"
  s.rubyforge_project = "json"
end
