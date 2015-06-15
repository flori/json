require 'test_helper'
require 'stringio'
require 'tempfile'

class JSONParserTest < Test::Unit::TestCase
  include JSON

  def setup
    @hash = {
      'a' => 2,
      'b' => 3.141,
      'c' => 'c',
      'd' => [ 1, "b", 3.14 ],
      'e' => { 'foo' => 'bar' },
      'g' => "\"\0\037",
      'h' => 1000.0,
      'i' => 0.001
    }
    @json = '{"a":2,"b":3.141,"c":"c","d":[1,"b",3.14],"e":{"foo":"bar"},'\
      '"g":"\\"\\u0000\\u001f","h":1.0E3,"i":1.0E-3}'
  end


  def test_index
  end

  def test_parser
  end

  def test_generator
  end

  def test_state
  end

  def test_create_id
  end

  def test_parse
  end

  def test_parse_bang
  end

  def test_generate
  end

  def test_fast_generate
  end

  def test_pretty_generate
  end

  def test_load
    assert_equal @hash, JSON.load(@json)
    tempfile = Tempfile.open('@json')
    tempfile.write @json
    tempfile.rewind
    assert_equal @hash, JSON.load(tempfile)
    stringio = StringIO.new(@json)
    stringio.rewind
    assert_equal @hash, JSON.load(stringio)
    assert_equal nil, JSON.load(nil)
    assert_equal nil, JSON.load('')
  ensure
    tempfile.close!
  end

  def test_load_with_options
    small_hash  = JSON("foo" => 'bar')
    symbol_hash = { :foo => 'bar' }
    assert_equal symbol_hash,
      JSON.load(small_hash, nil, :symbolize_names => true)
  end

  def test_dump
    too_deep = '[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]'
    assert_equal too_deep, dump(eval(too_deep))
    assert_kind_of String, Marshal.dump(eval(too_deep))
    assert_raises(ArgumentError) { dump(eval(too_deep), 100) }
    assert_raises(ArgumentError) { Marshal.dump(eval(too_deep), 100) }
    assert_equal too_deep, dump(eval(too_deep), 101)
    assert_kind_of String, Marshal.dump(eval(too_deep), 101)
    output = StringIO.new
    dump(eval(too_deep), output)
    assert_equal too_deep, output.string
    output = StringIO.new
    dump(eval(too_deep), output, 101)
    assert_equal too_deep, output.string
  end

  def test_dump_should_modify_defaults
    max_nesting = JSON.dump_default_options[:max_nesting]
    dump([], StringIO.new, 10)
    assert_equal max_nesting, JSON.dump_default_options[:max_nesting]
  end


  def test_JSON
  end
end
