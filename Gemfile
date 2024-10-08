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
gem "benchmark-ips"

group :benchmark, optional: true do
  gem "bullshit", "~> 0.1.2"
  gem "activesupport", "~> 7.1"
  gem "yajl-ruby", "~> 1.4"
end
