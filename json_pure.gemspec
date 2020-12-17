# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = "json_pure".freeze
  s.version = File.read("VERSION").chomp

  s.summary = "JSON Implementation for Ruby".freeze
  s.description = "This is a JSON implementation in pure Ruby.".freeze
  s.licenses = ["Ruby".freeze]
  s.authors = ["Florian Frank".freeze]
  s.email = "flori@ping.de".freeze

  s.extra_rdoc_files = ["README.md".freeze]
  s.rdoc_options = ["--title".freeze, "JSON implemention for ruby".freeze, "--main".freeze, "README.md".freeze]
  s.files = [
    "CHANGES.md".freeze,
    "Gemfile".freeze,
    "LICENSE".freeze,
    "README.md".freeze,
    "VERSION".freeze,
    "json_pure.gemspec".freeze,
    "lib/json.rb".freeze,
    "lib/json/add/bigdecimal.rb".freeze,
    "lib/json/add/complex.rb".freeze,
    "lib/json/add/core.rb".freeze,
    "lib/json/add/date.rb".freeze,
    "lib/json/add/date_time.rb".freeze,
    "lib/json/add/exception.rb".freeze,
    "lib/json/add/ostruct.rb".freeze,
    "lib/json/add/range.rb".freeze,
    "lib/json/add/rational.rb".freeze,
    "lib/json/add/regexp.rb".freeze,
    "lib/json/add/set.rb".freeze,
    "lib/json/add/struct.rb".freeze,
    "lib/json/add/symbol.rb".freeze,
    "lib/json/add/time.rb".freeze,
    "lib/json/common.rb".freeze,
    "lib/json/ext.rb".freeze,
    "lib/json/ext/.keep".freeze,
    "lib/json/generic_object.rb".freeze,
    "lib/json/pure.rb".freeze,
    "lib/json/pure/generator.rb".freeze,
    "lib/json/pure/parser.rb".freeze,
    "lib/json/version.rb".freeze,
  ] + Dir["tests/**/*"]
  s.homepage = "http://flori.github.com/json".freeze
  s.metadata = {
    'bug_tracker_uri'   => 'https://github.com/flori/json/issues',
    'changelog_uri'     => 'https://github.com/flori/json/blob/master/CHANGES.md',
    'documentation_uri' => 'http://flori.github.io/json/doc/index.html',
    'homepage_uri'      => 'http://flori.github.io/json/',
    'source_code_uri'   => 'https://github.com/flori/json',
    'wiki_uri'          => 'https://github.com/flori/json/wiki'
  }

  s.required_ruby_version = Gem::Requirement.new(">= 2.0".freeze)
  s.test_files = ["./tests/test_helper.rb".freeze]

  s.add_development_dependency "rake"
  s.add_development_dependency "test-unit"
end
