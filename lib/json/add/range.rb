#frozen_string_literal: false
unless defined?(::JSON::JSON_LOADED) and ::JSON::JSON_LOADED
  require 'json'
end

class Range

  # Returns a new \Range object constructed from the given argument,
  # which must be a hash containing the entry <tt>{'a' => _array_}</tt>,
  # where +array+ contains values suitable as arguments for a call
  # to Range.new[https://docs.ruby-lang.org/en/master/Range.html#method-c-new]:
  #
  #   require 'json/add/range'
  #   Range.json_create({'a' => [1, 4]})       # => 1..4
  #   Range.json_create({'a' => [1, 4, true]}) # => 1...4
  #   Range.json_create({'a' => ['a', 'd']})   # => "a".."d"
  #
  # Note that other entries in the hash are ignored,
  # so the hash can be one returned by #as_json.
  #
  def self.json_create(object)
    new(*object['a'])
  end

  # Returns a 2-element hash representing +self+:
  #
  #   require 'json/add/range'
  #   (1..4).as_json     # => {"json_class"=>"Range", "a"=>[1, 4, false]}
  #   (1...4).as_json    # => {"json_class"=>"Range", "a"=>[1, 4, true]}
  #   ('a'..'d').as_json # => {"json_class"=>"Range", "a"=>["a", "d", false]}
  #
  def as_json(*)
    {
      JSON.create_id  => self.class.name,
      'a'             => [ first, last, exclude_end? ]
    }
  end

  # Returns a JSON string representing +self+:
  #
  #   require 'json/add/range'
  #   (1..4).to_json     # => "{\"json_class\":\"Range\",\"a\":[1,4,false]}"
  #   (1...4).to_json    # => "{\"json_class\":\"Range\",\"a\":[1,4,true]}"
  #   ('a'..'d').to_json # => "{\"json_class\":\"Range\",\"a\":[\"a\",\"d\",false]}"
  #
  def to_json(*args)
    as_json.to_json(*args)
  end
end
