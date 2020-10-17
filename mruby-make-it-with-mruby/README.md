# mruby-make-it-with-mruby   [![Build Status](https://travis-ci.org/TheEEs/mruby-make-it-with-mruby.svg?branch=master)](https://travis-ci.org/TheEEs/mruby-make-it-with-mruby)
MIWMrb class
## install by mrbgems
- add conf.gem line to `build_config.rb`

```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem :github => 'TheEEs/mruby-make-it-with-mruby'
end
```
## example
```ruby
p MIWMrb.hi
#=> "hi!!"
t = MIWMrb.new "hello"
p t.hello
#=> "hello"
p t.bye
#=> "hello bye"
```

## License
under the MIT License:
- see LICENSE file
