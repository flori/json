require 'ostruct'

module JSON
  class LightObject < OpenStruct
    class << self
      alias [] new

      def json_create(data)
        data = data.dup
        data.delete JSON.create_id
        self[data]
      end
    end

    def to_hash
      table
    end

    def [](name)
      to_hash[name.to_sym]
    end

    def []=(name, value)
      modifiable[name.to_sym] = value
    end

    def |(other)
      self.class[other.to_hash.merge(to_hash)]
    end

    def as_json(*)
      to_hash | { JSON.create_id => self.class.name }
    end

    def to_json(*a)
      as_json.to_json(*a)
    end

    def method_missing(*a, &b)
      to_hash.__send__(*a, &b)
    rescue NoMethodError
      super
    end
  end
end
