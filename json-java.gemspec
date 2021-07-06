# -*- encoding: utf-8 -*-

spec = Gem::Specification.new do |s|
  s.name = "json"
  s.version = File.read("VERSION").chomp

  s.summary = "JSON Implementation for Ruby"
  s.description = "A JSON implementation as a JRuby extension."
  s.licenses = ["Ruby"]
  s.author = "Daniel Luz"
  s.email = "dev+ruby@mernen.com"

  s.platform = 'java'

  s.files = Dir["lib/**/*", "LICENSE"]

  s.homepage = "http://flori.github.com/json"
  s.metadata = {
      'bug_tracker_uri'   => 'https://github.com/flori/json/issues',
      'changelog_uri'     => 'https://github.com/flori/json/blob/master/CHANGES.md',
      'documentation_uri' => 'http://flori.github.io/json/doc/index.html',
      'homepage_uri'      => 'http://flori.github.io/json/',
      'source_code_uri'   => 'https://github.com/flori/json',
      'wiki_uri'          => 'https://github.com/flori/json/wiki'
  }

  s.required_ruby_version = Gem::Requirement.new(">= 2.3")
end

if $0 == __FILE__
  Gem::Builder.new(spec).build
else
  spec
end
