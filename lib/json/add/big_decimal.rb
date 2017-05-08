unless defined?(::JSON::JSON_LOADED) and ::JSON::JSON_LOADED
  require 'json'
end
require 'bigdecimal' unless defined? BigDecimal

class BigDecimal

  def to_json
    to_i
  end

end

