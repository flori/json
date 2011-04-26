require 'rbconfig'
include Config

PKG_NAME    = 'json'
PKG_TITLE   = 'JSON Implementation for Ruby'
PKG_VERSION = File.read('VERSION').chomp
PKG_FILES   = Dir.glob("**/**").reject { |file|
                file =~ /CVS|pkg|tmp|coverage|constants|Makefile|\.nfs\.|\.iml\Z/ ||
                file =~ /\.(so|bundle|o|class|#{CONFIG['DLEXT']})$/
              }

EXT_ROOT_DIR      = 'ext/json/ext'
EXT_PARSER_DIR    = "#{EXT_ROOT_DIR}/parser"
EXT_PARSER_DL     = "#{EXT_PARSER_DIR}/parser.#{CONFIG['DLEXT']}"
RAGEL_PATH        = "#{EXT_PARSER_DIR}/parser.rl"
EXT_PARSER_SRC    = "#{EXT_PARSER_DIR}/parser.c"
PKG_FILES << EXT_PARSER_SRC
EXT_GENERATOR_DIR = "#{EXT_ROOT_DIR}/generator"
EXT_GENERATOR_DL  = "#{EXT_GENERATOR_DIR}/generator.#{CONFIG['DLEXT']}"
EXT_GENERATOR_SRC = "#{EXT_GENERATOR_DIR}/generator.c"

JAVA_DIR            = "java/src/json/ext"
JAVA_RAGEL_PATH     = "#{JAVA_DIR}/Parser.rl"
JAVA_PARSER_SRC     = "#{JAVA_DIR}/Parser.java"
JAVA_SOURCES        = Dir.glob("#{JAVA_DIR}/*.java")
JAVA_CLASSES        = []
JRUBY_PARSER_JAR    = File.expand_path("lib/json/ext/parser.jar")
JRUBY_GENERATOR_JAR = File.expand_path("lib/json/ext/generator.jar")

RAGEL_CODEGEN     = %w[rlcodegen rlgen-cd ragel].find { |c| `which #{c}` != "" }
RAGEL_DOTGEN      = %w[rlgen-dot rlgen-cd ragel].find { |c| `which #{c}` != "" }
