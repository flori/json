#!/usr/bin/env ruby

require 'rbconfig'
require 'fileutils'
include FileUtils::Verbose

include Config

sitelibdir = CONFIG["sitelibdir"]
cd 'lib' do
  install('json.rb', sitelibdir)
  mkdir_p File.join(sitelibdir, 'json')
  for file in Dir['json/**/*}']
    d = File.join(sitelibdir, file)
    mkdir_p File.dirname(d)
    install(file, d)
  end
end
warn " *** Installed PURE ruby library."
