require 'mkmf'

$CFLAGS << ' -O3'
if CONFIG['CC'] =~ /gcc/
  $CFLAGS << ' -Wall'
  $CFLAGS << ' -O0 -ggdb' if $DEBUG
end

$defs << "-DJSON_GENERATOR"
create_makefile 'json/ext/generator'
