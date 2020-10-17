MRuby::Build.new do |conf|
  toolchain :gcc
  conf.gembox 'default'
  conf.cc.flags = %W(-Os -I /home/trandat/emsdk/upstream/emscripten/system/include)
end

MRuby::CrossBuild.new("emscripten") do |conf| 
  toolchain :clang
  conf.gembox 'default'
  conf.cc.command = 'emcc'
  conf.linker.command = 'emcc'
  conf.linker.flags = ["-Os" ,
    %q(-s ASYNCIFY -s 'ASYNCIFY_IMPORTS=["js_sleep","browser_prompt","browser_confirm","bluetooth_list_devices","bluetooth_write","bluetooth_connect","bluetooth_available","bluetooth_read", "bluetooth_disconnect"]')]
  conf.archiver.command = 'emar'
end