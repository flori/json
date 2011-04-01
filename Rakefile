begin
  require 'rake/gempackagetask'
rescue LoadError
end

begin
  require 'rake/extensiontask'
rescue LoadError
  warn "WARNING: rake-compiler is not installed. You will not be able to build the json gem until you install it."
end

require 'rbconfig'
include Config

require 'rake/clean'
CLOBBER.include Dir['benchmarks/data/*.{dat,log}']
CLEAN.include FileList['diagrams/*.*'], 'doc', 'coverage', 'tmp',
  FileList["ext/**/{Makefile,mkmf.log}"], 'build', 'dist', FileList['**/*.rbc'],
  FileList["{ext,lib}/**/*.{so,bundle,#{CONFIG['DLEXT']},o,obj,pdb,lib,manifest,exp,def,jar,class}"],
  FileList['java/src/**/*.class']

MAKE = ENV['MAKE'] || %w[gmake make].find { |c| system(c, '-v') }
require File.dirname(__FILE__) + '/constants'

def myruby(*args, &block)
  @myruby ||= File.join(CONFIG['bindir'], CONFIG['ruby_install_name'])
  options = (Hash === args.last) ? args.pop : {}
  if args.length > 1 then
    sh(*([@myruby] + args + [options]), &block)
  else
    sh("#{@myruby} #{args.first}", options, &block)
  end
end

desc "Installing library (pure)"
task :install_pure => :version do
  myruby 'install.rb'
end

task :install_ext_really do
  sitearchdir = CONFIG["sitearchdir"]
  cd 'ext' do
    for file in Dir["json/ext/*.#{CONFIG['DLEXT']}"]
      d = File.join(sitearchdir, file)
      mkdir_p File.dirname(d)
      install(file, d)
    end
  end
end

desc "Installing library (extension)"
task :install_ext => [ :compile_ext, :install_pure, :install_ext_really ]

desc "Installing library (extension)"
if RUBY_PLATFORM =~ /java/
  task :install => :install_pure
else
  task :install => :install_ext
end

if defined?(Gem) and defined?(Rake::GemPackageTask)
  spec_pure = Gem::Specification.load(File.dirname(__FILE__) + '/json_pure.gemspec')
  
  Rake::GemPackageTask.new(spec_pure) do |pkg|
      pkg.need_tar = true
      pkg.package_files = PKG_FILES
  end
end

if defined?(Gem) and defined?(Rake::GemPackageTask) and defined?(Rake::ExtensionTask)
  spec_ext = Gem::Specification.load(File.dirname(__FILE__) + '/json.gemspec')

  Rake::GemPackageTask.new(spec_ext) do |pkg|
    pkg.need_tar      = true
    pkg.package_files = PKG_FILES
  end

  Rake::ExtensionTask.new do |ext|
    ext.name            = 'parser'
    ext.gem_spec        = spec_ext
    ext.cross_compile   = true
    ext.cross_platform  = %w[i386-mswin32 i386-mingw32]
    ext.ext_dir         = 'ext/json/ext/parser'
    ext.lib_dir         = 'lib/json/ext'
  end

  Rake::ExtensionTask.new do |ext|
    ext.name            = 'generator'
    ext.gem_spec        = spec_ext
    ext.cross_compile   = true
    ext.cross_platform  = %w[i386-mswin32 i386-mingw32]
    ext.ext_dir         = 'ext/json/ext/generator'
    ext.lib_dir         = 'lib/json/ext'
  end
end

desc m = "Writing version information for #{PKG_VERSION}"
task :version do
  puts m
  File.open(File.join('lib', 'json', 'version.rb'), 'w') do |v|
    v.puts <<EOT
module JSON
  # JSON version
  VERSION         = '#{PKG_VERSION}'
  VERSION_ARRAY   = VERSION.split(/\\./).map { |x| x.to_i } # :nodoc:
  VERSION_MAJOR   = VERSION_ARRAY[0] # :nodoc:
  VERSION_MINOR   = VERSION_ARRAY[1] # :nodoc:
  VERSION_BUILD   = VERSION_ARRAY[2] # :nodoc:
end
EOT
  end
end

desc "Testing library (pure ruby)"
task :test_pure => :clean do
  ENV['JSON'] = 'pure'
  ENV['RUBYOPT'] = "-Ilib #{ENV['RUBYOPT']}"
  myruby '-S', 'testrb', *Dir['./tests/test_*.rb']
end

desc "Testing library (pure ruby and extension)"
task :test => [ :test_pure, :test_ext ]


if defined?(RUBY_ENGINE) and RUBY_ENGINE == 'jruby'
  file JAVA_PARSER_SRC => JAVA_RAGEL_PATH do
    cd JAVA_DIR do
      if RAGEL_CODEGEN == 'ragel'
        sh "ragel Parser.rl -J -o Parser.java"
      else
        sh "ragel -x Parser.rl | #{RAGEL_CODEGEN} -J"
      end
    end
  end

  desc "Generate parser for java with ragel"
  task :ragel => JAVA_PARSER_SRC

  desc "Delete the ragel generated Java source"
  task :ragel_clean do
    rm_rf JAVA_PARSER_SRC
  end

  JRUBY_JAR = File.join(Config::CONFIG["libdir"], "jruby.jar")
  if File.exist?(JRUBY_JAR)
    JAVA_SOURCES.each do |src|
      classpath = (Dir['java/lib/*.jar'] << 'java/src' << JRUBY_JAR) * ':'
      obj = src.sub(/\.java\Z/, '.class')
      file obj => src do
        sh 'javac', '-classpath', classpath, '-source', '1.5', src
      end
      JAVA_CLASSES << obj
    end
  else
    warn "WARNING: Cannot find jruby in path => Cannot build jruby extension!"
  end

  desc "Compiling jruby extension"
  task :compile_ext => JAVA_CLASSES

  desc "Package the jruby gem"
  task :jruby_gem => :create_jar do
    sh 'gem build json-java.gemspec'
    mkdir_p 'pkg'
    mv "json-#{PKG_VERSION}-java.gem", 'pkg'
  end

  desc "Testing library (jruby)"
  task :test_ext => :create_jar do
    ENV['JSON'] = 'ext'
    myruby '-S', 'testrb', '-Ilib', *Dir['./tests/test_*.rb']
  end

  file JRUBY_PARSER_JAR => :compile_ext do
    cd 'java/src' do
      parser_classes = FileList[
        "json/ext/ByteListTranscoder*.class",
        "json/ext/OptionsReader*.class",
        "json/ext/Parser*.class",
        "json/ext/RuntimeInfo*.class",
        "json/ext/StringDecoder*.class",
        "json/ext/Utils*.class"
      ]
      sh 'jar', 'cf', File.basename(JRUBY_PARSER_JAR), *parser_classes
      mv File.basename(JRUBY_PARSER_JAR), File.dirname(JRUBY_PARSER_JAR)
    end
  end

  desc "Create parser jar"
  task :create_parser_jar => JRUBY_PARSER_JAR

  file JRUBY_GENERATOR_JAR => :compile_ext do
    cd 'java/src' do
      generator_classes = FileList[
        "json/ext/ByteListTranscoder*.class",
        "json/ext/OptionsReader*.class",
        "json/ext/Generator*.class",
        "json/ext/RuntimeInfo*.class",
        "json/ext/StringEncoder*.class",
        "json/ext/Utils*.class"
      ]
      sh 'jar', 'cf', File.basename(JRUBY_GENERATOR_JAR), *generator_classes
      mv File.basename(JRUBY_GENERATOR_JAR), File.dirname(JRUBY_GENERATOR_JAR)
    end
  end

  desc "Create generator jar"
  task :create_generator_jar => JRUBY_GENERATOR_JAR

  desc "Create parser and generator jars"
  task :create_jar => [ :create_parser_jar, :create_generator_jar ]

  desc "Build all gems and archives for a new release of the jruby extension."
  task :release => [ :clean, :version, :jruby_gem ]
else
  desc "Compiling extension"
  task :compile_ext => [ EXT_PARSER_DL, EXT_GENERATOR_DL ]

  file EXT_PARSER_DL => EXT_PARSER_SRC do
    cd EXT_PARSER_DIR do
      myruby 'extconf.rb'
      sh MAKE
    end
    cp "#{EXT_PARSER_DIR}/parser.#{CONFIG['DLEXT']}", EXT_ROOT_DIR
  end

  file EXT_GENERATOR_DL => EXT_GENERATOR_SRC do
    cd EXT_GENERATOR_DIR do
      myruby 'extconf.rb'
      sh MAKE
    end
    cp "#{EXT_GENERATOR_DIR}/generator.#{CONFIG['DLEXT']}", EXT_ROOT_DIR
  end

  desc "Testing library (extension)"
  task :test_ext => :compile_ext do
    ENV['JSON'] = 'ext'
    ENV['RUBYOPT'] = "-Iext:lib #{ENV['RUBYOPT']}"
    myruby '-S', 'testrb', *Dir['./tests/test_*.rb']
  end

  desc "Benchmarking parser"
  task :benchmark_parser do
    ENV['RUBYOPT'] = "-Ilib:ext #{ENV['RUBYOPT']}"
    myruby 'benchmarks/parser_benchmark.rb'
    myruby 'benchmarks/parser2_benchmark.rb'
  end

  desc "Benchmarking generator"
  task :benchmark_generator do
    ENV['RUBYOPT'] = "-Ilib:ext #{ENV['RUBYOPT']}"
    myruby 'benchmarks/generator_benchmark.rb'
    myruby 'benchmarks/generator2_benchmark.rb'
  end

  desc "Benchmarking library"
  task :benchmark => [ :benchmark_parser, :benchmark_generator ]

  desc "Create RDOC documentation"
  task :doc => [ :version, EXT_PARSER_SRC ] do
    sh "sdoc -o doc -t '#{PKG_TITLE}' -m README README lib/json.rb #{FileList['lib/json/**/*.rb']} #{EXT_PARSER_SRC} #{EXT_GENERATOR_SRC}"
  end

  desc "Generate parser with ragel"
  task :ragel => EXT_PARSER_SRC

  desc "Delete the ragel generated C source"
  task :ragel_clean do
    rm_rf EXT_PARSER_SRC
  end

  file EXT_PARSER_SRC => RAGEL_PATH do
    cd EXT_PARSER_DIR do
      if RAGEL_CODEGEN == 'ragel'
        sh "ragel parser.rl -G2 -o parser.c"
      else
        sh "ragel -x parser.rl | #{RAGEL_CODEGEN} -G2"
      end
    end
  end

  desc "Generate diagrams of ragel parser (ps)"
  task :ragel_dot_ps do
    root = 'diagrams'
    specs = []
    File.new(RAGEL_PATH).grep(/^\s*machine\s*(\S+);\s*$/) { specs << $1 }
    for s in specs
      if RAGEL_DOTGEN == 'ragel'
        sh "ragel #{RAGEL_PATH} -S#{s} -p -V | dot -Tps -o#{root}/#{s}.ps"
      else
        sh "ragel -x #{RAGEL_PATH} -S#{s} | #{RAGEL_DOTGEN} -p|dot -Tps -o#{root}/#{s}.ps"
      end
    end
  end

  desc "Generate diagrams of ragel parser (png)"
  task :ragel_dot_png do
    root = 'diagrams'
    specs = []
    File.new(RAGEL_PATH).grep(/^\s*machine\s*(\S+);\s*$/) { specs << $1 }
    for s in specs
      if RAGEL_DOTGEN == 'ragel'
        sh "ragel #{RAGEL_PATH} -S#{s} -p -V | dot -Tpng -o#{root}/#{s}.png"
      else
        sh "ragel -x #{RAGEL_PATH} -S#{s} | #{RAGEL_DOTGEN} -p|dot -Tpng -o#{root}/#{s}.png"
      end
    end
  end

  desc "Generate diagrams of ragel parser"
  task :ragel_dot => [ :ragel_dot_png, :ragel_dot_ps ]

  task :environment do
    ENV['RUBY_CC_VERSION'] = '1.8.7:1.9.2'
  end

  desc "Build all gems and archives for a new release of json and json_pure."
  task :release => [ :clean, :version, :environment, :cross, :native, :gem, ] do
    sh "#$0 clean native gem"
    sh "#$0 clean package"
  end
end

desc "Compile in the the source directory"
task :default => [ :version ]
