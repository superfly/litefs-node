#include <napi.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

using namespace Napi;

// LiteFS lock offsets
const int HaltByte = 72;

// Use Open File Decription Locks, if available
#ifndef F_OFD_SETLKW
#define F_OFD_SETLKW F_SETLKW
#endif

// Halt locks the HALT lock on the file handle to the LiteFS database lock file.
// This causes writes to be halted on the primary node so that this replica can
// perform writes until Unhalt() is invoked.
//
// The HALT lock will automatically be released when the file descriptor is closed.
//
// This function is a no-op if the current node is the primary.
Napi::Value Halt(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() != 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "Wrong argument type").ThrowAsJavaScriptException();
    return env.Null();
  }

  int fd = info[0].As<Napi::Number>().Int32Value();

  struct flock lock;
  lock.l_type    = F_WRLCK;
  lock.l_start   = HaltByte;
  lock.l_whence  = SEEK_SET;
  lock.l_len     = 1;

  int rc = fcntl(fd, F_OFD_SETLKW, &lock);

  return Napi::Number::New(env, rc);
}

// Unhalt releases the HALT lock on the file handle to the LiteFS database lock
// file. This allows writes to resume on the primary node. This replica will
// not be able to perform any more writes until Halt() is called again.
//
// This function is a no-op if the current node is the primary or if the
// lock expired.
Napi::Value Unhalt(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() != 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "Wrong argument type").ThrowAsJavaScriptException();
    return env.Null();
  }

  int fd = info[0].As<Napi::Number>().Int32Value();

  struct flock lock;
  lock.l_type    = F_UNLCK;
  lock.l_start   = HaltByte;
  lock.l_whence  = SEEK_SET;
  lock.l_len     = 1;

  int rc = fcntl(fd, F_OFD_SETLKW, &lock);

  return Napi::Number::New(env, rc);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  Napi::Object obj = Napi::Object::New(env);
  obj.Set("Halt", Napi::Function::New(env, Halt));
  obj.Set("Unhalt", Napi::Function::New(env, Unhalt));

  exports.Set(Napi::String::New(env, "LitefsNode"), obj);
  return exports;
}

NODE_API_MODULE(addon, Init)
