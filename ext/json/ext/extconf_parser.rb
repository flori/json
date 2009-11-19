require 'mkmf'
require 'rbconfig'

unless $CFLAGS.gsub!(/ -O[\dsz]?/, ' -O3')
  $CFLAGS << ' -O3'
end
if CONFIG['CC'] =~ /gcc/
  $CFLAGS << ' -Wall'
  #$CFLAGS.gsub!(/ -O[\dsz]?/, ' -O0 -ggdb')
end
if RUBY_VERSION >= '1.9'
  $CFLAGS << ' -DRUBY_19'
end

have_header("ruby/st.h") || have_header("st.h")
have_header("re.h")
create_makefile 'parser'
