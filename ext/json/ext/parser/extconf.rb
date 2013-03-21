require 'mkmf'

$CFLAGS << ' -O3'
if CONFIG['CC'] =~ /gcc/
  $CFLAGS << ' -Wall'
  $CFLAGS << ' -O0 -ggdb' if $DEBUG
end

create_makefile 'json/ext/parser'
