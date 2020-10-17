all: make_output_dir
	emcc -Os -o dist/mruby.js \
		-I "../mruby/include" -L "../mruby/build/emscripten/lib" -lmruby \
		-s EXPORTED_FUNCTIONS='[]'\
		-s ASYNCIFY \
		-s EXIT_RUNTIME=1 \
		-s 'ASYNCIFY_IMPORTS=["js_sleep","browser_prompt","browser_confirm","bluetooth_list_devices","bluetooth_write","bluetooth_connect","bluetooth_available","bluetooth_read", "bluetooth_disconnect"]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' mruby.c
make_output_dir:
	mkdir -p ./dist
	rm ./dist/*