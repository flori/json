# vim: set ft=ruby:

source 'https://rubygems.org'

case ENV['JSON']
when 'ext', nil
  if ENV['RUBY_ENGINE'] == 'jruby'
    gemspec :name => 'json-java'
  else
    gemspec :name => 'json'
  end
when 'pure'
  gemspec :name => 'json_pure'
end

gem "rake"
gem "test-unit"
gem "test-unit-ruby-core"
gem "all_images", "~> 0" unless RUBY_PLATFORM =~ /java/

group :benchmark do
  gem "benchmark-ips"
  unless RUBY_PLATFORM =~ /java/
    gem "oj"
    gem "rapidjson"
  end
end
