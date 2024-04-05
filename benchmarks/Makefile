RUBYLIB := $(dir $(CURDIR))ext$(if $(RUBYLIB),:$(RUBYLIB))
export RUBYLIB

all: data/GeneratorBenchmarkComparison.log
all: data/Generator2BenchmarkComparison.log
all: data/ParserBenchmarkComparison.log
all: data/Parser2BenchmarkComparison.log
.PHONY: all

.NOTINTERMEDIATE:
.DELETE_ON_ERROR:
.NOTPARALLEL:

data/GeneratorBenchmarkComparison.log: vendor
	rm -f -- data/GeneratorBenchmark*
	bundle exec ruby ./generator_benchmark.rb
data/Generator2BenchmarkComparison.log: vendor
	rm -f -- data/Generator2Benchmark*
	bundle exec ruby ./generator2_benchmark.rb
data/ParserBenchmarkComparison.log: vendor
	rm -f -- data/ParserBenchmark*
	bundle exec ruby ./parser_benchmark.rb
data/Parser2BenchmarkComparison.log: vendor
	rm -f -- data/Parser2Benchmark*
	bundle exec ruby ./parser2_benchmark.rb

../.bundle/config:
	bundle config set path $(CURDIR)/vendor
	bundle config set with benchmark
vendor: ../Gemfile $(wildcard ../Gemfile.lock) ../.bundle/config
	bundle install
	touch --no-create $@
