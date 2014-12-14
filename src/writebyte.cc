#include <errno.h>
#include <node.h>
#include <nan.h>
#include "./i2c-dev.h"
#include "./writebyte.h"

static __s32 WriteByte(int fd, __u8 byte) {
  return i2c_smbus_write_byte(fd, byte);
}

class WriteByteWorker : public NanAsyncWorker {
public:
  WriteByteWorker(NanCallback *callback, int fd, __u8 byte)
    : NanAsyncWorker(callback), fd(fd), byte(byte) {}
  ~WriteByteWorker() {}

  void Execute() {
    __s32 ret = WriteByte(fd, byte);
    if (ret == -1) {
      SetErrorMessage(strerror(errno));
    }
  }

  void HandleOKCallback() {
    NanScope();

    v8::Local<v8::Value> argv[] = {
      NanNull()
    };

    callback->Call(1, argv);
  }

private:
  int fd;
  __u8 byte;
};

NAN_METHOD(WriteByteAsync) {
  NanScope();

  if (args.Length() < 3 || !args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsFunction()) {
    return NanThrowError("incorrect arguments passed to writeByte(int fd, int val, function cb)");
  }

  int fd = args[0]->Int32Value();
  __u8 byte = args[1]->Int32Value();
  NanCallback *callback = new NanCallback(args[2].As<v8::Function>());

  NanAsyncQueueWorker(new WriteByteWorker(callback, fd, byte));
  NanReturnUndefined();
}

NAN_METHOD(WriteByteSync) {
  NanScope();

  if (args.Length() < 2 || !args[0]->IsInt32() || !args[1]->IsInt32()) {
    return NanThrowError("incorrect arguments passed to writeByteSync(int fd, int val)");
  }

  int fd = args[0]->Int32Value();
  __u8 byte = args[1]->Int32Value();

  __s32 ret = WriteByte(fd, byte);
  if (ret == -1) {
    return NanThrowError(strerror(errno), errno);
  }

  NanReturnUndefined();
}

