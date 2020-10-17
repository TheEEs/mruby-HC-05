/*
** mrb_miwmrb.c - MIWMrb class
**
** Copyright (c) TheEEs 2020
**
** See Copyright Notice in LICENSE
*/

#include "mruby.h"
#include "mruby/data.h"
#include "mrb_miwmrb.h"
#include "mruby/variable.h"
#include "mruby/compile.h"
#include "mruby/array.h"
#include "mruby/value.h"
#include "mruby/string.h"
#include <stdlib.h>


#define DONE mrb_gc_arena_restore(mrb, 0);


#define SET_MODE 0
#define READ 1
#define WRITE 2

#define MODE_INPUT 0
#define MODE_OUTPUT 1

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void *,bluetooth_list_devices,(),{
  return Asyncify.handleAsync(async ()=>{
    return new Promise((resolve)=>{
      bluetoothSerial.list((devices)=>{
        const device_names = _malloc(devices.length * 4);
        const device_addresses = _malloc(devices.length * 4);
        devices.forEach((device,index)=>{
          let name_b_length = lengthBytesUTF8(device.name) + 1;
          let address_b_length = lengthBytesUTF8(device.address) + 1;
          let name_mem = _malloc(name_b_length);
          let address_mem = _malloc(address_b_length);
          stringToUTF8(device.name,name_mem,name_b_length);
          stringToUTF8(device.address,address_mem,address_b_length);
          setValue(device_names + index * 4,name_mem,"*");
          setValue(device_addresses + index * 4,address_mem,"*");
        });
        const ret = _malloc(3 * 4);
        setValue(ret,devices.length,"i32");
        setValue(ret + 4,device_names,"*");
        setValue(ret + 8,device_addresses,"*");
        resolve(ret);
      },resolve);
    });
  });
});


mrb_value list_devices(mrb_state * mrb, mrb_value self) {
  char *** devices = (char***)bluetooth_list_devices();
  int length = *((int*)devices);
  char ** names = devices[1];
  char ** addresses = devices[2];
  mrb_value return_array = mrb_ary_new(mrb);
  for(int i = 0; i < length; i++){
    char * name = names[i];
    char * address = addresses[i];
    mrb_value device = mrb_ary_new(mrb);
    mrb_ary_push(mrb,device,mrb_str_new_cstr(mrb,name));
    mrb_ary_push(mrb,device,mrb_str_new_cstr(mrb,address));
    mrb_ary_push(mrb,return_array,device);
    free((void*)name);
    free((void*)address);
  }
  free((void*)names);
  free((void*)addresses);
  free((void*)devices);
  return return_array;
}

EM_JS(int,bluetooth_connect,(const char* address),{
  return Asyncify.handleAsync(async ()=>{
    return new Promise((resolve)=>{
      let addr = UTF8ToString(address);
      bluetoothSerial.connect(addr,() => {
        resolve(1);
      },()=>{
        resolve(0);
      });
    });
  });
});


mrb_value connect_device(mrb_state * mrb, mrb_value self){
  char * path; 
  mrb_int size;
  mrb_get_args(mrb,"s",&path,&size);
  int connected = bluetooth_connect(path);
  if(!connected){
    return mrb_false_value();
  }
  return mrb_true_value();
}

EM_JS(int, bluetooth_available,(),{
  return Asyncify.handleAsync(async ()=> {
    return new Promise((resolve)=>{
      bluetoothSerial.available((num_bytes)=>{
        resolve(num_bytes);
      },()=>{
        resolve(0);
      });
    });
  });
});

mrb_value data_available(mrb_state * mrb, mrb_value self){
  int available = bluetooth_available();
  return mrb_fixnum_value((mrb_int)available);
} 

EM_JS(int,bluetooth_write,(uint8_t * data, size_t size),{
  return Asyncify.handleAsync(async () => {
    return new Promise((resolve)=>{
        const package = new Uint8Array(size);
        for(let i = 0; i < size;i++){
          package[i] = getValue(data +i,"i8");
        }
        bluetoothSerial.write(package,()=> {resolve(1)}, ()=> {resolve(0)});
    });
  });
});


mrb_value data_write(mrb_state * mrb, mrb_value self){
  mrb_value * values;
  mrb_int size;
  mrb_get_args(mrb,"a",&values,&size);
  uint8_t * data_to_send = (uint8_t *)malloc(size);
  int8_t is_okay = 1;
  for(int i = 0;i < size;i++){
    if(values[i].tt != MRB_TT_FIXNUM){
      mrb_check_type(mrb,values[i],MRB_TT_FIXNUM);
      is_okay = 0;
      break;
    }
    data_to_send[i] = (uint8_t)(values[i].value.i);
  }

  if(is_okay){
    int result = bluetooth_write(data_to_send,size);
    if (result)
      return mrb_true_value();
    return mrb_false_value();
  }
  else
    return mrb_true_value();
}

EM_JS(void *, bluetooth_read,(),{
  return Asyncify.handleAsync(async () => {
    return new Promise((resolve)=>{
      bluetoothSerial.read((data)=>{
        const size = lengthBytesUTF8(data);
        const read_bytes = _malloc(size + 1);
        stringToUTF8(data,read_bytes,size + 1);
        const ret = _malloc(2 * 4);
        setValue(ret,size,"i32");
        setValue(ret + 4, read_bytes, "*");
        resolve(ret);
      },
      resolve);
    });
  });
});


mrb_value data_read(mrb_state * mrb, mrb_value self){
  int * received_data = (int*)bluetooth_read();
  int data_size = *received_data;
  uint8_t * actual_data = (uint8_t *)(received_data[1]);
  mrb_value ret_array = mrb_ary_new(mrb);
  for(int i = 0;i < data_size;i++){
    mrb_ary_push(mrb,ret_array, mrb_fixnum_value(actual_data[i]));
  }
  free(actual_data);
  free(received_data);
  return ret_array;
}


EM_JS(int,bluetooth_disconnect,(),{
  return Asyncify.handleAsync(async () => {
    return new Promise((resolve)=>{
      bluetoothSerial.disconnect(()=> resolve(1),()=> resolve(0));
    });
  });
});



mrb_value disconnect_device(mrb_state * mrb, mrb_value self){
  int disconnected = bluetooth_disconnect();
  return disconnected ? mrb_true_value() : mrb_false_value();
}


EM_JS(int, browser_confirm, (char * text, char* cancel_button, char* ok_button),{
  return Asyncify.handleAsync(async ()=>{
    if(typeof(swal) !== "function"){
      const confirmation = window.confirm(UTF8ToString(text));
      return confirmation ? 1 : 0;
    }else{
      return swal(UTF8ToString(text),{
        buttons: [UTF8ToString(cancel_button),UTF8ToString(ok_button)]
      }).then(r => r ? 1 : 0);
    }
  })
})

mrb_value confirm(mrb_state * mrb, mrb_value self){
  mrb_value text,cancel_text,ok_text;
  mrb_bool cancel_given , ok_given;
  mrb_get_args(mrb,"S|S?S?",&text,&cancel_text,&cancel_given,&ok_text,&ok_given);
  char * cancel_text_cstr, *text_cstr, *ok_text_cstr;
  text_cstr = mrb_str_to_cstr(mrb,text);
  if(cancel_given)
    cancel_text_cstr = mrb_str_to_cstr(mrb,cancel_text);
  else
    cancel_text_cstr = (char*)"No";
  if(ok_given) 
    ok_text_cstr = mrb_str_to_cstr(mrb,ok_text);
  else 
    ok_text_cstr = (char*)"Yes";
  int confirmation = browser_confirm(text_cstr,cancel_text_cstr,ok_text_cstr);
  return mrb_fixnum_value(confirmation);
}

EM_JS(char*, browser_prompt, (char * text, char* button_text),{
  return Asyncify.handleAsync(async ()=>{
    if(typeof(swal) !== "function"){
      const input = window.prompt(UTF8ToString(text));
      let size = lengthBytesUTF8(input) + 1;
      let ret = _malloc(size);
      stringToUTF8(input,ret,size);
      return ret;
    }else{
      let input = await swal({
        text: UTF8ToString(text),
        content: "input",
        button:{
          text: UTF8ToString(button_text)
        }
      });
      input = input ? input : "";
      let size = lengthBytesUTF8(input) + 1;
      let ret = _malloc(size);
      stringToUTF8(input,ret,size);
      return ret;
    }
  })
})

mrb_value prompt(mrb_state * mrb, mrb_value self){
  mrb_value text,button_text;
  mrb_bool text_given , button_text_given;
  mrb_get_args(mrb,"|S?S?",&text,&text_given,&button_text,&button_text_given);
  char  *text_cstr, *button_text_cstr;
  if(text_given)
    text_cstr = mrb_str_to_cstr(mrb,text);
  else
    text_cstr = (char*)"Input";
  if(button_text_given) 
    button_text_cstr = mrb_str_to_cstr(mrb,button_text);
  else 
    button_text_cstr = (char*)"Done";
  char* input = browser_prompt(text_cstr,button_text_cstr);
  mrb_value ret =  mrb_str_new_cstr(mrb,input);
  free(input);
  return ret;
}

EM_JS(void,js_sleep,(mrb_int milliseconds),{
  return Asyncify.handleAsync(async ()=> {
    return new Promise((resolve)=>{
      setTimeout(resolve,milliseconds);
    });
  });
});


mrb_value mrb_sleep_ms(mrb_state * mrb, mrb_value self){
  mrb_int ms;
  mrb_get_args(mrb,"i",&ms);
  js_sleep(ms);
  return mrb_nil_value();
}


void mrb_mruby_make_it_with_mruby_gem_init(mrb_state *mrb)
{
  struct RClass * miwmrb_module = mrb_define_module(mrb,"MIWMrb");
  mrb_define_module_function(mrb,miwmrb_module,"list_devices",list_devices,MRB_ARGS_NONE());
  mrb_define_module_function(mrb,miwmrb_module,"data_write",data_write,MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb,miwmrb_module,"data_read",data_read,MRB_ARGS_NONE());
  mrb_define_module_function(mrb,miwmrb_module,"connect_device",connect_device,MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb,miwmrb_module,"data_available",data_available,MRB_ARGS_NONE());
  mrb_define_module_function(mrb,miwmrb_module,"disconnect_device",disconnect_device,MRB_ARGS_NONE());
  mrb_define_module_function(mrb,miwmrb_module,"confirm",confirm,MRB_ARGS_REQ(1)|MRB_ARGS_OPT(2));
  mrb_define_module_function(mrb,miwmrb_module,"prompt",prompt,MRB_ARGS_OPT(2));
  mrb_define_module_function(mrb,mrb->kernel_module,"sleep_ms",mrb_sleep_ms,MRB_ARGS_REQ(1));
}

void mrb_mruby_make_it_with_mruby_gem_final(mrb_state *mrb)
{
  
}


#else
void mrb_mruby_make_it_with_mruby_gem_init(mrb_state *mrb)
{

}

void mrb_mruby_make_it_with_mruby_gem_final(mrb_state *mrb)
{
}

#endif

