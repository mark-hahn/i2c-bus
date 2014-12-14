#include <errno.h>
#include <node.h>
#include <nan.h>
#include "./i2c-dev.h"
#include "./writei2cblockdata.h"

static __s32 WriteI2cBlockData(int fd, __u8 cmd, __u8 length, const __u8 *data) {
  return i2c_smbus_write_i2c_block_data(fd, cmd, length, data);
}

class WriteI2cBlockDataWorker : public NanAsyncWorker {
public:
  WriteI2cBlockDataWorker(
    NanCallback *callback,
    int fd,
    __u8 cmd,
    __u32 length,
    const __u8* data,
    v8::Local<v8::Object> &bufferHandle
  ) : NanAsyncWorker(callback), fd(fd), cmd(cmd), length(length), data(data) {
    SaveToPersistent("buffer", bufferHandle);
  }

  ~WriteI2cBlockDataWorker() {}

  void Execute() {
    __s32 ret = WriteI2cBlockData(fd, cmd, length, data);
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
  __u8 cmd;
  __u32 length;
  const __u8* data;
};

NAN_METHOD(WriteI2cBlockDataAsync) {
  NanScope();

  if (args.Length() < 5 ||
      !args[0]->IsInt32() ||
      !args[1]->IsInt32() ||
      !args[2]->IsUint32() ||
      !args[3]->IsObject() ||
      !args[4]->IsFunction()) {
    return NanThrowError("incorrect arguments passed to writeI2cBlockData"
      "(int fd, int cmd, int length, Buffer buffer, function cb)");
  }

  int fd = args[0]->Int32Value();
  __u8 cmd = args[1]->Int32Value();
  __u32 length = args[2]->Uint32Value();
  v8::Local<v8::Object> bufferHandle = args[3].As<v8::Object>();
  NanCallback *callback = new NanCallback(args[4].As<v8::Function>());

  const __u8* bufferData = (const __u8*) node::Buffer::Data(bufferHandle);
  size_t bufferLength = node::Buffer::Length(bufferHandle);

  if (length > I2C_SMBUS_I2C_BLOCK_MAX) {
    return NanThrowError("writeI2cBlockData can't write blocks "
      "with more than 32 characters");
  }

  if (length > bufferLength) {
    return NanThrowError("buffer passed to writeI2cBlockData "
      "contains less data than expected");
  }

  NanAsyncQueueWorker(new WriteI2cBlockDataWorker(
    callback,
    fd,
    cmd,
    length,
    bufferData,
    bufferHandle
  ));
  NanReturnUndefined();
}

NAN_METHOD(WriteI2cBlockDataSync) {
  NanScope();

  if (args.Length() < 4 ||
      !args[0]->IsInt32() ||
      !args[1]->IsInt32() ||
      !args[2]->IsUint32() ||
      !args[3]->IsObject()) {
    return NanThrowError("incorrect arguments passed to writeI2cBlockDataSync"
      "(int fd, int cmd, int length, Buffer buffer)");
  }

  int fd = args[0]->Int32Value();
  __u8 cmd = args[1]->Int32Value();
  __u32 length = args[2]->Uint32Value();
  v8::Local<v8::Object> bufferHandle = args[3].As<v8::Object>();

  const __u8* bufferData = (const __u8*) node::Buffer::Data(bufferHandle);
  size_t bufferLength = node::Buffer::Length(bufferHandle);

  if (length > I2C_SMBUS_I2C_BLOCK_MAX) {
    return NanThrowError("writeI2cBlockDataSync can't write blocks "
      "with more than 32 bytes");
  }

  if (length > bufferLength) {
    return NanThrowError("buffer passed to writeI2cBlockDataSync "
      "contains less data than expected");
  }

  __s32 ret = WriteI2cBlockData(fd, cmd, length, bufferData);
  if (ret == -1) {
    return NanThrowError(strerror(errno), errno);
  }

  NanReturnUndefined();
}

