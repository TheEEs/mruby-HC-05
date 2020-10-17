#include <mruby.h>
#include <mruby/compile.h>
#include <stdlib.h>
#include <emscripten.h>

void EMSCRIPTEN_KEEPALIVE run(const char* code){
    mrb_state * mrb = mrb_open();
    mrb_load_string(mrb, code);
    mrb_close(mrb);
}
