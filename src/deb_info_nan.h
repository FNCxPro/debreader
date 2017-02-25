#ifndef DEB_INFO_H
#define DEB_INFO_H
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <nan.h>
#include "mmap_file.h"

#define READ_INTO(STM, STRUCT, DST)  STM->read((char*)&STRUCT->DST, sizeof(STRUCT->DST));
typedef struct {
  char identifier[16];
  char timestamp[12];
  char owner[6];
  char gid[6];
  char perm[8];
  char filesize[10];
  char end[2]; // `\n
} deb_entry;

typedef struct {
  char magic[8];
  deb_entry header_entry;
  char version[4];
} deb_header;

class DebReader : public Nan::AsyncWorker {
private:
  const unsigned int deb_pkg_header_size = 71;
  int read_header(deb_header *header);
  int read_entry(deb_entry *deb_entry);
  deb_entry read_entry();
  void iterate_entries();
  void list_files(size_t len);
  void read_control(size_t len);
  void pack_vector();
  std::string control_buffer;
  std::string err;
  int read_proxy();
  Nan::Callback* callback;
  std::vector<std::string> pkg_content;
  fakeFile *debfile = nullptr;
  deb_header *header;

public:
  explicit DebReader (Nan::Callback *callback, const std::string filename);
  void Execute();
  void HandleOKCallback();
  void cleanup();
  virtual ~DebReader ();

};

// Exported to JS world
static void parse(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::String> opts = info[0].As<v8::String>();
  std::string fn(*v8::String::Utf8Value(opts->ToString()));
  Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());
  Nan::AsyncQueueWorker(new DebReader(callback, fn));
  return;
}
#endif