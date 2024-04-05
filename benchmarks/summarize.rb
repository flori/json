#!/usr/bin/env ruby
# CODING: UTF-8

def summarize(data_dir)
  pkg_data = {
    :json => {
      :ver => `git describe --tags`.strip,
    },
    :yajl => {
      :ver => Gem.loaded_specs['yajl-ruby'].version.to_s,
      :url => Gem.loaded_specs['yajl-ruby'].homepage,
    },
    :rails => {
      :ver => Gem.loaded_specs['activesupport'].version.to_s,
      :url => Gem.loaded_specs['activesupport'].homepage,
    }
  }

  puts "    Ruby: " + `ruby --version`.strip
  puts "    CPU : " + File.readlines("/proc/cpuinfo", chomp: true).find{|line| line.start_with? "model name"}.split(":", 2)[1].strip

  cols = [
    {:hdr => "Package", :align => :left, :val => lambda { |key, val|
       ugly = key.split('#', 2)[0]
       case ugly
       when "Ext"
         "`json/ext` #{pkg_data[:json][:ver]}"
       when "Pure"
         "`json_pure` #{pkg_data[:json][:ver]}"
       when "Rails"
         "[Rails #{pkg_data[:rails][:ver]}](#{pkg_data[:rails][:url]})"
       when "Yajl"
         "[YAJL #{pkg_data[:yajl][:ver]}](#{pkg_data[:yajl][:url]})"
       when "YAML"
         "YAML (Ruby #{RUBY_VERSION} builtin)"
       else
         ugly
       end
     }},
    {:hdr => "Function", :align => :left, :val => lambda { |key, val| '`'+key.split('#', 2)[1]+'`' }},
    {:hdr => "mean µs/call", :align => :right, :val => lambda { |key, val| sprintf "%.2f", val['mean']['µs/call'] }},
    {:hdr => "median µs/call", :align => :right, :val => lambda { |key, val| sprintf "%.2f", val['median']['µs/call'] }},
    {:hdr => "stddev %", :align => :right, :val => lambda { |key, val| sprintf "%.2f", val['mean']['stddev%'] }},
  ]

  Dir.each_child(data_dir) do |filename|
    next unless filename.end_with? "Comparison.log"

    benchmark_name = filename[0, filename.length-"Comparison.log".length]
    benchmark_filename = benchmark_name.gsub(/(.)([A-Z])/){$1+"_"+$2}.downcase + ".rb"

    data = {}
    lines = File.readlines(File.join(data_dir, filename), chomp: true)
    while lines.length > 0
      line = lines.shift
      if m = /^Comparing times \((.*)\):$/.match(line)
        method = m[1]
        method.slice! "call_time_"
      elsif m = /^\s*[0-9]+ ([A-Z]\S+) *[0-9]+ repeats:$/.match(line)
        name = m[1]
        name.slice! benchmark_name
        lines.shift
        secs_per_call, stddev, _ = lines.shift.split

        data[name] = {} unless data.include? name
        data[name][method] = {
          'µs/call' => secs_per_call.to_f * 1e6,
          'stddev%' => stddev.to_f,
        }
      end
    end

    puts
    puts "### `#{benchmark_filename}`"
    puts
    col_widths = cols.map{|col| col[:hdr].length }
    data.each do |key, val|
      col_widths = (col_widths.zip cols.map{|col|col[:val].call(key, val).length}).map(&:max)
    end
    fmt = "  | " + cols.each_with_index.map{|col, i| col[:align] == :left ? "%-#{col_widths[i]}s" : "%#{col_widths[i]}s"}.join(" | ")+" |\n"
    sep = "  |" + cols.each_with_index.map{|col, i|sprintf (col[:align] == :left ? ":%s-" : "-%s:"), "-"*col_widths[i]}.join("|")+"|\n"

    printf fmt, *(cols.map{|col| col[:hdr]})
    prev = ''
    for key in data.keys.sort
      pkg = key.split('#', 2)[0]
      if pkg != prev
        printf sep
        _key = key
      else
        _key = key[pkg.length,key.length]
      end
      printf fmt, *(cols.map{|col| col[:val].call(_key, data[key])})
      prev = pkg
    end
  end
end

if $0 == __FILE__
  summarize(File.join(File.dirname(__FILE__), 'data'))
end
