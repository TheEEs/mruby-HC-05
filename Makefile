build: build-libmruby.a build-mruby
	cp ./mruby-interpreter-emscripten/dist/mruby.js ./code-editor/assets/js/mruby.js
	cd ./code-editor && mint build
	cp ./mruby-interpreter-emscripten/dist/mruby.wasm ./code-editor/dist/mruby.wasm
	cd ./code-editor && npx cap sync
build-libmruby.a:
	cd ./mruby && rake
build-mruby:
	cd ./mruby-interpreter-emscripten && make