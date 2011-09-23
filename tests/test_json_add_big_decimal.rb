#!/usr/bin/env ruby
# -*- coding:utf-8 -*-

require 'test/unit'
require File.join(File.dirname(__FILE__), 'setup_variant')
require 'json/add/big_decimal'

class TC_JSONAddBigDecimal < Test::Unit::TestCase

  def test_to_json
    assert_equal 2, BigDecimal.new('2').to_json
  end

end

