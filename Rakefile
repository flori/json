require 'flott'
require 'json'
include Flott

$meta = JSON.parse(File.read('meta.json'))

task :default => [:doc, :homepage]

desc "Create the project documentation."
task :doc do
  if File.directory?('doc')
    sh 'git rm -r doc'
  end
  sh 'git commit -m "deleted documentation" doc'
  sh 'git checkout master'
  rm_rf 'doc'
  sh 'rake doc'
  sh 'git checkout gh-pages'
  sh 'git add doc'
  sh 'git commit -m "generated documentation" doc'
end

desc "Compile the homepage."
task :compile_homepage do
  env = Environment.new
  env.update($meta)
  for tmpl in Dir['*.tmpl']
    ext = File.extname(tmpl)
    out_name = tmpl.sub(/#{ext}$/, '.html')
    warn "Compiling '#{tmpl}' -> '#{out_name}'."
    File.open(out_name, 'w') do |o|
      env.output = o
      fp = Parser.from_filename(tmpl)
      fp.evaluate(env)
    end
  end
end

desc "Check the homepage with tidy."
task :tidy_homepage do
  sh "tidy -e *index.html"
end

desc "Compile and check the homepage."
task :homepage => [ :compile_homepage, :tidy_homepage ]

desc "Publish the homepage to rubyforge."
task :publish_rubyforge => :homepage do
  sh "scp -r rubyforge_index.html rubyforge.org:/var/www/gforge-projects/#{$meta['project_unixname']}/index.html"
end
  # vim: set et sw=2 ts=2:
