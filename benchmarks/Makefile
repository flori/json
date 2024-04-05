RUBYLIB := $(dir $(CURDIR))ext$(if $(RUBYLIB),:$(RUBYLIB))
export RUBYLIB

all: summary.md
.PHONY: all

update-README.md: summary.md
	{ \
	  sed -n '1,/BEGIN benchmarks\/summary\.md/p'  <../README.md && \
	  cat summary.md && \
	  sed -n '/END benchmarks\/summary.md/,$$p'  <../README.md ; \
	} > ../README.md.tmp
	mv -f ../README.md.tmp ../README.md
.PHONY: update-README.md

.NOTINTERMEDIATE:
.DELETE_ON_ERROR:
.NOTPARALLEL:

summary.md: data/GeneratorBenchmarkComparison.log
summary.md: data/Generator2BenchmarkComparison.log
summary.md: data/ParserBenchmarkComparison.log
summary.md: data/Parser2BenchmarkComparison.log
summary.md: summarize.rb
	bundle exec ruby ./summarize.rb >$@

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
