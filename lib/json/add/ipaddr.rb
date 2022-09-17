#frozen_string_literal: false
unless defined?(::JSON::JSON_LOADED) and ::JSON::JSON_LOADED
  require 'json'
end
require 'ipaddr'

class IPAddr

  # Deserializes JSON string by constructing new IPAddr object with address
  # <tt>a</tt> and prefix <tt>p</tt> serialized with <tt>to_json</tt>
  def self.json_create(object)
    addr = IPAddr.new(object['a'])
    addr.mask(object['p'])
    addr
  end

  # Returns a hash, that will be turned into a JSON object and represent this
  # object.
  def as_json(*)
    if Gem::Version.new(RUBY_VERSION) < Gem::Version.new("2.5.0")
      prefix = prefixlen
    end

    {
      JSON.create_id => self.class.name,
      'a' => to_s,
      'p' => prefix,
    }
  end

  # Stores class name (IPAddr) with address <tt>a</tt> and prefix
  # <tt>p</tt> as JSON string
  def to_json(*args)
    as_json.to_json(*args)
  end

  # for ruby version <= 2.4
  def prefixlen
    case @family
    when Socket::AF_INET
      n = IN4MASK ^ @mask_addr
      i = 32
    when Socket::AF_INET6
      n = IN6MASK ^ @mask_addr
      i = 128
    else
      raise AddressFamilyError, "unsupported address family"
    end
    while n.positive?
      n >>= 1
      i -= 1
    end
    i
  end
end
