/* bench.cpp - read/downsample/write benchmark for exr_image.h, tinyexr, OpenEXR
 *
 * Models the methodology from:
 *   https://aras-p.info/blog/2025/11/22/OpenEXR-vs-tinyexr/
 *
 * For each input file, each library: reads the image, downsamples 2x
 * (nearest-neighbor, same "really_bad_downsample" approach), writes it back
 * as ZIP-compressed EXR. Elapsed wall time is reported.
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
#include <OpenEXR/ImfOutputFile.h>
#include <Imath/ImathBox.h>

#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

/* ---- simple RGBA float image -------------------------------------------- */

struct Image
{
   int w, h;
   std::vector<float> px; /* RGBA interleaved */
};

static Image downsample(Image const &src)
{
   Image dst;
   dst.w = src.w / 2;
   dst.h = src.h / 2;
   if (dst.w < 1) dst.w = 1;
   if (dst.h < 1) dst.h = 1;
   dst.px.resize((size_t)dst.w * (size_t)dst.h * 4u);
   for (int y = 0; y < dst.h; ++y) {
      for (int x = 0; x < dst.w; ++x) {
         float const *s = &src.px[((size_t)y * 2 * src.w + (size_t)x * 2) * 4];
         float       *d = &dst.px[((size_t)y * dst.w + (size_t)x) * 4];
         d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
      }
   }
   return dst;
}

/* ---- exr_image.h (exri) ------------------------------------------------- */

static bool exri_load(char const *path, Image *out)
{
   float *px = nullptr;
   int w = 0, h = 0, ch = 0;
   if (!exri_loadf(&px, path, &w, &h, &ch, 4, EXRI_LOAD_DEFAULT))
      return false;
   out->w = w; out->h = h;
   out->px.assign(px, px + (size_t)w * (size_t)h * 4u);
   exri_image_free(px);
   return true;
}

static bool exri_save(char const *path, Image const &img)
{
   exri_write_options opts;
   std::memset(&opts, 0, sizeof(opts));
   opts.compression = EXRI_WRITE_COMPRESSION_ZIP;
   opts.pixel_type  = EXRI_WRITE_PIXEL_FLOAT;
   return exri_writef(path, img.w, img.h, 4, img.px.data(), &opts) != 0;
}

/* ---- tinyexr ------------------------------------------------------------ */

static bool tiny_load(char const *path, Image *out)
{
   float *px = nullptr;
   int w = 0, h = 0;
   char const *err = nullptr;
   if (LoadEXR(&px, &w, &h, path, &err) != TINYEXR_SUCCESS) {
      if (err) { std::fprintf(stderr, "tinyexr load: %s\n", err); FreeEXRErrorMessage(err); }
      return false;
   }
   out->w = w; out->h = h;
   out->px.assign(px, px + (size_t)w * (size_t)h * 4u);
   std::free(px);
   return true;
}

static bool tiny_save(char const *path, Image const &img)
{
   char const *err = nullptr;
   int ret = SaveEXR(img.px.data(), img.w, img.h, 4, 0, path, &err);
   if (ret != TINYEXR_SUCCESS) {
      if (err) { std::fprintf(stderr, "tinyexr save: %s\n", err); FreeEXRErrorMessage(err); }
      return false;
   }
   return true;
}

/* ---- OpenEXR ------------------------------------------------------------- */

namespace IMF   = OPENEXR_IMF_NAMESPACE;
namespace IMATH = IMATH_NAMESPACE;

static bool oexr_load(char const *path, Image *out)
{
   try {
      IMF::InputFile input(path, 1 /* single thread */);
      IMF::Header const &hdr = input.header();

      /* must have at least R G B */
      IMF::ChannelList const &clist = hdr.channels();
      if (!clist.findChannel("R") || !clist.findChannel("G") || !clist.findChannel("B"))
         return false;

      IMATH::Box2i dw = hdr.dataWindow();
      int w = dw.max.x - dw.min.x + 1;
      int h = dw.max.y - dw.min.y + 1;
      out->w = w; out->h = h;
      out->px.assign((size_t)w * (size_t)h * 4u, 0.0f);

      size_t xs = 4u * sizeof(float);
      size_t ys = (size_t)w * xs;
      IMF::FrameBuffer fb;
      fb.insert("R", IMF::Slice::Make(IMF::FLOAT, &out->px[0], dw, xs, ys, 1, 1, 0.0));
      fb.insert("G", IMF::Slice::Make(IMF::FLOAT, &out->px[1], dw, xs, ys, 1, 1, 0.0));
      fb.insert("B", IMF::Slice::Make(IMF::FLOAT, &out->px[2], dw, xs, ys, 1, 1, 0.0));
      fb.insert("A", IMF::Slice::Make(IMF::FLOAT, &out->px[3], dw, xs, ys, 1, 1, 1.0));
      input.setFrameBuffer(fb);
      input.readPixels(dw.min.y, dw.max.y);
      return true;
   } catch (std::exception const &e) {
      std::fprintf(stderr, "openexr load: %s\n", e.what());
      return false;
   }
}

static bool oexr_save(char const *path, Image const &img)
{
   try {
      IMF::Header hdr(img.w, img.h);
      hdr.compression() = IMF::ZIP_COMPRESSION;
      hdr.channels().insert("R", IMF::Channel(IMF::FLOAT));
      hdr.channels().insert("G", IMF::Channel(IMF::FLOAT));
      hdr.channels().insert("B", IMF::Channel(IMF::FLOAT));
      hdr.channels().insert("A", IMF::Channel(IMF::FLOAT));

      IMATH::Box2i dw = hdr.dataWindow();
      size_t xs = 4u * sizeof(float);
      size_t ys = (size_t)img.w * xs;
      IMF::FrameBuffer fb;
      fb.insert("R", IMF::Slice::Make(IMF::FLOAT, const_cast<float *>(&img.px[0]), dw, xs, ys));
      fb.insert("G", IMF::Slice::Make(IMF::FLOAT, const_cast<float *>(&img.px[1]), dw, xs, ys));
      fb.insert("B", IMF::Slice::Make(IMF::FLOAT, const_cast<float *>(&img.px[2]), dw, xs, ys));
      fb.insert("A", IMF::Slice::Make(IMF::FLOAT, const_cast<float *>(&img.px[3]), dw, xs, ys));
      IMF::OutputFile output(path, hdr, 1 /* single thread */);
      output.setFrameBuffer(fb);
      output.writePixels(img.h);
      return true;
   } catch (std::exception const &e) {
      std::fprintf(stderr, "openexr save: %s\n", e.what());
      return false;
   }
}

/* ---- timing ------------------------------------------------------------- */

using Clock = std::chrono::steady_clock;
using Seconds = std::chrono::duration<double>;

static double now_s()
{
   return std::chrono::duration_cast<Seconds>(Clock::now().time_since_epoch()).count();
}

/* ---- per-library run ----------------------------------------------------- */

struct LibOps
{
   char const *name;
   bool (*load)(char const *, Image *);
   bool (*save)(char const *, Image *);
};

static bool lib_save_wrap_exri(char const *path, Image *img)  { return exri_save(path, *img); }
static bool lib_save_wrap_tiny(char const *path, Image *img)  { return tiny_save(path, *img); }
static bool lib_save_wrap_oexr(char const *path, Image *img)  { return oexr_save(path, *img); }

static bool lib_load_wrap_exri(char const *path, Image *img)  { return exri_load(path, img); }
static bool lib_load_wrap_tiny(char const *path, Image *img)  { return tiny_load(path, img); }
static bool lib_load_wrap_oexr(char const *path, Image *img)  { return oexr_load(path, img); }

static LibOps const LIBS[] = {
   { "exr_image.h", lib_load_wrap_exri, lib_save_wrap_exri },
   { "tinyexr",     lib_load_wrap_tiny, lib_save_wrap_tiny },
   { "openexr",     lib_load_wrap_oexr, lib_save_wrap_oexr },
};
static int const NUM_LIBS = (int)(sizeof(LIBS) / sizeof(LIBS[0]));

static double run_lib(LibOps const &lib, std::vector<std::string> const &files)
{
   double total = 0.0;
   for (int i = 0; i < (int)files.size(); ++i) {
      std::string out_path = "/tmp/bench_out_";
      out_path += lib.name;
      out_path += "_";
      out_path += std::to_string(i);
      out_path += ".exr";

      double t0 = now_s();

      Image img;
      if (!lib.load(files[(size_t)i].c_str(), &img)) {
         std::fprintf(stderr, "[%s] load failed: %s\n", lib.name, files[(size_t)i].c_str());
         continue;
      }
      Image small = downsample(img);
      if (!lib.save(out_path.c_str(), &small)) {
         std::fprintf(stderr, "[%s] save failed: %s\n", lib.name, out_path.c_str());
      }

      double elapsed = now_s() - t0;
      total += elapsed;
      std::printf("  %-16s  %s  %.3fs\n", lib.name, files[(size_t)i].c_str(), elapsed);
      std::fflush(stdout);

      std::remove(out_path.c_str());
   }
   return total;
}

/* ---- main ---------------------------------------------------------------- */

static void usage(char const *argv0)
{
   std::fprintf(stderr, "usage: %s file.exr ...\n", argv0);
}

int main(int argc, char **argv)
{
   if (argc < 2) { usage(argv[0]); return 2; }

   std::vector<std::string> files;
   for (int i = 1; i < argc; ++i)
      files.push_back(argv[i]);

   std::printf("benchmark: read + downsample-2x + write  (%d files)\n\n", (int)files.size());

   double totals[NUM_LIBS];
   for (int li = 0; li < NUM_LIBS; ++li) {
      std::printf("[%s]\n", LIBS[li].name);
      totals[li] = run_lib(LIBS[li], files);
      std::printf("  %-16s  TOTAL  %.3fs\n\n", LIBS[li].name, totals[li]);
   }

   std::printf("%-16s  %s\n", "library", "total (s)");
   std::printf("%-16s  %s\n", "-------", "---------");
   for (int li = 0; li < NUM_LIBS; ++li)
      std::printf("%-16s  %.3f\n", LIBS[li].name, totals[li]);

   return 0;
}
