module JSON
  # JSON version
  VERSION         = '1.8.3' unless defined? VERSION
  VERSION_ARRAY   = VERSION.split(/./).map { |x| x.to_i } unless defined? VERSION_ARRAY # :nodoc:
  VERSION_MAJOR   = VERSION_ARRAY[0] unless defined? VERSION_MAJOR # :nodoc:
  VERSION_MINOR   = VERSION_ARRAY[1] unless defined? VERSION_MINOR # :nodoc:
  VERSION_BUILD   = VERSION_ARRAY[2] unless defined? VERSION_BUILD # :nodoc:
end
