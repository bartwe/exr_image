/* bench.cpp - in-memory read/downsample/write benchmark
 *
 * Methodology matches https://aras-p.info/blog/2025/11/22/OpenEXR-vs-tinyexr/
 * "Compression/decompression time measured without actual disk I/O — data
 * processed from memory."
 *
 * All input files are loaded into RAM before any timing starts.  Each library
 * is timed on decode-from-memory + 2x downsample + encode-to-memory only.
 */

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#define EXR_IMAGE_IMPLEMENTATION
#include "../../exr_image.h"

#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfCompression.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfIO.h>
#include <OpenEXR/ImfOutputFile.h>
#include <Imath/ImathBox.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

/* ---- raw file loading ---------------------------------------------------- */

static bool read_file(char const *path, std::vector<unsigned char> *out)
{
   std::ifstream f(path, std::ios::binary | std::ios::ate);
   if (!f) return false;
   std::streamsize sz = f.tellg();
   if (sz <= 0) return false;
   f.seekg(0);
   out->resize((size_t)sz);
   f.read(reinterpret_cast<char *>(out->data()), sz);
   return (bool)f;
}

/* ---- simple RGBA float image --------------------------------------------- */

struct Image { int w, h; std::vector<float> px; };

static Image downsample(Image const &src)
{
   Image dst;
   dst.w = src.w / 2;
   dst.h = src.h / 2;
   if (dst.w < 1) dst.w = 1;
   if (dst.h < 1) dst.h = 1;
   dst.px.resize((size_t)dst.w * (size_t)dst.h * 4u);
   for (int y = 0; y < dst.h; ++y)
      for (int x = 0; x < dst.w; ++x) {
         float const *s = &src.px[((size_t)y*2*src.w + (size_t)x*2) * 4];
         float       *d = &dst.px[((size_t)y*dst.w  + (size_t)x)   * 4];
         d[0]=s[0]; d[1]=s[1]; d[2]=s[2]; d[3]=s[3];
      }
   return dst;
}

/* ---- exr_image.h --------------------------------------------------------- */

static bool exri_load(std::vector<unsigned char> const &buf, Image *out)
{
   float *px = nullptr; int w=0, h=0, ch=0;
   if (!exri_loadf_from_memory(&px, buf.data(), buf.size(), &w, &h, &ch, 4, EXRI_LOAD_DEFAULT))
      return false;
   out->w=w; out->h=h;
   out->px.assign(px, px + (size_t)w*(size_t)h*4u);
   exri_image_free(px);
   return true;
}

static bool exri_save(Image const &img, std::vector<unsigned char> *out)
{
   exri_write_options opts; std::memset(&opts, 0, sizeof(opts));
   opts.compression = EXRI_WRITE_COMPRESSION_ZIP;
   opts.pixel_type  = EXRI_WRITE_PIXEL_FLOAT;
   unsigned char *data = nullptr; size_t len = 0;
   if (!exri_writef_to_memory(&data, &len, img.w, img.h, 4, img.px.data(), &opts))
      return false;
   out->assign(data, data + len);
   exri_image_free(data);
   return true;
}

/* ---- tinyexr ------------------------------------------------------------- */

static bool tiny_load(std::vector<unsigned char> const &buf, Image *out)
{
   float *px = nullptr; int w=0, h=0;
   char const *err = nullptr;
   if (LoadEXRFromMemory(&px, &w, &h, buf.data(), buf.size(), &err) != TINYEXR_SUCCESS) {
      if (err) FreeEXRErrorMessage(err);
      return false;
   }
   out->w=w; out->h=h;
   out->px.assign(px, px + (size_t)w*(size_t)h*4u);
   std::free(px);
   return true;
}

static bool tiny_save(Image const &img, std::vector<unsigned char> *out)
{
   unsigned char *data = nullptr;
   char const *err = nullptr;
   int len = SaveEXRToMemory(img.px.data(), img.w, img.h, 4, 0, &data, &err);
   if (len < 0) {
      if (err) FreeEXRErrorMessage(err);
      return false;
   }
   out->assign(data, data + len);
   std::free(data);
   return true;
}

/* ---- OpenEXR ------------------------------------------------------------- */

namespace IMF   = OPENEXR_IMF_NAMESPACE;
namespace IMATH = IMATH_NAMESPACE;

class MemIStream : public IMF::IStream
{
   unsigned char const *_data;
   size_t               _size;
   size_t               _pos;
public:
   MemIStream(unsigned char const *data, size_t size)
      : IMF::IStream("mem"), _data(data), _size(size), _pos(0) {}
   bool     read(char c[], int n) override {
      if (_pos + (size_t)n > _size) throw std::runtime_error("oexr read past end");
      std::memcpy(c, _data + _pos, (size_t)n);
      _pos += (size_t)n;
      return _pos < _size;
   }
   uint64_t tellg()           override { return (uint64_t)_pos; }
   void     seekg(uint64_t p) override { _pos = (size_t)p; }
   void     clear()           override {}
};

class MemOStream : public IMF::OStream
{
   std::vector<unsigned char> _buf;
   size_t                     _pos;
public:
   MemOStream() : IMF::OStream("mem"), _pos(0) {}
   void     write(char const c[], int n) override {
      size_t end = _pos + (size_t)n;
      if (end > _buf.size()) _buf.resize(end);
      std::memcpy(_buf.data() + _pos, c, (size_t)n);
      _pos = end;
   }
   uint64_t tellp()           override { return (uint64_t)_pos; }
   void     seekp(uint64_t p) override { _pos = (size_t)p; }
   std::vector<unsigned char> take() { return std::move(_buf); }
};

static bool oexr_load(std::vector<unsigned char> const &buf, Image *out)
{
   try {
      MemIStream is(buf.data(), buf.size());
      IMF::InputFile input(is, 1);
      IMF::ChannelList const &cl = input.header().channels();
      if (!cl.findChannel("R") || !cl.findChannel("G") || !cl.findChannel("B"))
         return false;
      IMATH::Box2i dw = input.header().dataWindow();
      int w = dw.max.x - dw.min.x + 1;
      int h = dw.max.y - dw.min.y + 1;
      out->w=w; out->h=h;
      out->px.assign((size_t)w*(size_t)h*4u, 0.0f);
      size_t xs = 4u*sizeof(float), ys = (size_t)w*xs;
      IMF::FrameBuffer fb;
      fb.insert("R", IMF::Slice::Make(IMF::FLOAT, &out->px[0], dw, xs, ys, 1,1,0.0));
      fb.insert("G", IMF::Slice::Make(IMF::FLOAT, &out->px[1], dw, xs, ys, 1,1,0.0));
      fb.insert("B", IMF::Slice::Make(IMF::FLOAT, &out->px[2], dw, xs, ys, 1,1,0.0));
      fb.insert("A", IMF::Slice::Make(IMF::FLOAT, &out->px[3], dw, xs, ys, 1,1,1.0));
      input.setFrameBuffer(fb); input.readPixels(dw.min.y, dw.max.y);
      return true;
   } catch (std::exception const &e) {
      std::fprintf(stderr, "openexr load: %s\n", e.what()); return false;
   }
}

static bool oexr_save(Image const &img, std::vector<unsigned char> *out)
{
   try {
      MemOStream os;
      IMF::Header hdr(img.w, img.h);
      hdr.compression() = IMF::ZIP_COMPRESSION;
      hdr.channels().insert("R", IMF::Channel(IMF::FLOAT));
      hdr.channels().insert("G", IMF::Channel(IMF::FLOAT));
      hdr.channels().insert("B", IMF::Channel(IMF::FLOAT));
      hdr.channels().insert("A", IMF::Channel(IMF::FLOAT));
      IMATH::Box2i dw = hdr.dataWindow();
      size_t xs=4u*sizeof(float), ys=(size_t)img.w*xs;
      IMF::FrameBuffer fb;
      fb.insert("R", IMF::Slice::Make(IMF::FLOAT, const_cast<float*>(&img.px[0]), dw,xs,ys));
      fb.insert("G", IMF::Slice::Make(IMF::FLOAT, const_cast<float*>(&img.px[1]), dw,xs,ys));
      fb.insert("B", IMF::Slice::Make(IMF::FLOAT, const_cast<float*>(&img.px[2]), dw,xs,ys));
      fb.insert("A", IMF::Slice::Make(IMF::FLOAT, const_cast<float*>(&img.px[3]), dw,xs,ys));
      IMF::OutputFile output(os, hdr, 1);
      output.setFrameBuffer(fb); output.writePixels(img.h);
      *out = os.take();
      return true;
   } catch (std::exception const &e) {
      std::fprintf(stderr, "openexr save: %s\n", e.what()); return false;
   }
}

/* ---- timing -------------------------------------------------------------- */

using Clock   = std::chrono::steady_clock;
using Seconds = std::chrono::duration<double>;
static double now_s() {
   return std::chrono::duration_cast<Seconds>(Clock::now().time_since_epoch()).count();
}

/* ---- library table ------------------------------------------------------- */

struct LibOps {
   char const *name;
   bool (*load)(std::vector<unsigned char> const &, Image *);
   bool (*save)(Image const &, std::vector<unsigned char> *);
};

static LibOps const LIBS[] = {
   { "exr_image.h", exri_load, exri_save },
   { "tinyexr",     tiny_load, tiny_save },
   { "openexr",     oexr_load, oexr_save },
};
static int const NUM_LIBS = (int)(sizeof(LIBS)/sizeof(LIBS[0]));

/* ---- main ---------------------------------------------------------------- */

static void usage(char const *argv0)
{
   std::fprintf(stderr, "usage: %s file.exr ...\n", argv0);
}

int main(int argc, char **argv)
{
   if (argc < 2) { usage(argv[0]); return 2; }

   /* --- pre-load all files into RAM --- */
   std::vector<std::string>              paths;
   std::vector<std::vector<unsigned char>> blobs;
   for (int i = 1; i < argc; ++i) {
      std::vector<unsigned char> b;
      if (!read_file(argv[i], &b)) {
         std::fprintf(stderr, "cannot read: %s\n", argv[i]); return 1;
      }
      paths.push_back(argv[i]);
      blobs.push_back(std::move(b));
   }

   std::printf("benchmark: decode-from-memory + downsample-2x + encode-to-memory\n");
   std::printf("%d file(s) pre-loaded — no disk I/O during timing\n\n",
               (int)paths.size());

   double totals[NUM_LIBS] = {};

   for (int li = 0; li < NUM_LIBS; ++li) {
      LibOps const &lib = LIBS[li];
      std::printf("[%s]\n", lib.name);

      for (int i = 0; i < (int)paths.size(); ++i) {
         std::vector<unsigned char> out;

         double t0 = now_s();

         Image img;
         if (!lib.load(blobs[(size_t)i], &img)) {
            std::fprintf(stderr, "  [%s] load failed: %s\n", lib.name, paths[(size_t)i].c_str());
            continue;
         }
         Image small = downsample(img);
         if (!lib.save(small, &out))
            std::fprintf(stderr, "  [%s] save failed: %s\n", lib.name, paths[(size_t)i].c_str());

         double elapsed = now_s() - t0;
         totals[li] += elapsed;
         std::printf("  %-14s  %s  %.3fs\n", lib.name, paths[(size_t)i].c_str(), elapsed);
         std::fflush(stdout);
      }
      std::printf("  %-14s  TOTAL  %.3fs\n\n", lib.name, totals[li]);
   }

   std::printf("%-14s  %s\n", "library", "total (s)");
   std::printf("%-14s  %s\n", "-------", "---------");
   for (int li = 0; li < NUM_LIBS; ++li)
      std::printf("%-14s  %.3f\n", LIBS[li].name, totals[li]);

   return 0;
}
