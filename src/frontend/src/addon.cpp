// addon.cpp
#include <napi.h>
#include "hello.h"
#include <string>

// This function will be exposed to JavaScript.
Napi::String ProcessAudio(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  // Call your C++ function. If dsp() returns a value, you can capture it.
  // Here we assume dsp() returns void.
  print_hello();
  
  // Return a status message.
  return Napi::String::New(env, "Processing complete");
}

// Module initialization.
Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("processAudio", Napi::Function::New(env, ProcessAudio));
  return exports;
}

NODE_API_MODULE(addon, Init)
