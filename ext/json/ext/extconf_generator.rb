require 'mkmf'
require 'rbconfig'

unless $CFLAGS.gsub!(/ -O[\dsz]?/, ' -O3')
  $CFLAGS << ' -O3'
end
if CONFIG['CC'] =~ /gcc/
  $CFLAGS << ' -Wall'
  #unless $CFLAGS.gsub!(/ -O[\dsz]?/, ' -O0 -ggdb')
  #  $CFLAGS << ' -O0 -ggdb'
  #end
end
if RUBY_VERSION >= '1.9'
  $CFLAGS << ' -DRUBY_19'
end

have_header("ruby/re.h") || have_header("re.h")
have_header("ruby/encoding.h")
create_makefile 'generator'
