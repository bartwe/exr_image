#define EXR_IMAGE_IMPLEMENTATION
#include "../../exr_image.h"

#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfCompression.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfInputPart.h>
#include <OpenEXR/ImfMultiPartInputFile.h>
#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfPartType.h>
#include <OpenEXR/ImfStringAttribute.h>

#include <Imath/ImathBox.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <ctime>
#include <vector>

namespace IMF = OPENEXR_IMF_NAMESPACE;
namespace IMATH = IMATH_NAMESPACE;

struct Image
{
   int w;
   int h;
   int comp;
   std::vector<float> pixels;
};

struct LoadResult
{
   bool ok;
   bool skip;
   std::string reason;
   Image image;
};

struct Stats
{
   int cases_seen;
   int compared;
   int skipped;
   int failed;
   int reencoded;
};

static LoadResult failed_result(char const *reason)
{
   LoadResult r;
   r.ok = false;
   r.skip = false;
   r.reason = reason;
   r.image.w = 0;
   r.image.h = 0;
   r.image.comp = 0;
   return r;
}

static LoadResult skipped_result(char const *reason)
{
   LoadResult r;
   r.ok = false;
   r.skip = true;
   r.reason = reason;
   r.image.w = 0;
   r.image.h = 0;
   r.image.comp = 0;
   return r;
}

static int has_rgb_channels(IMF::Header const &header, std::string *reason)
{
   IMF::ChannelList const &channels = header.channels();
   IMF::Channel const *r = channels.findChannel("R");
   IMF::Channel const *g = channels.findChannel("G");
   IMF::Channel const *b = channels.findChannel("B");

   if (r == NULL || g == NULL || b == NULL) {
      *reason = "OpenEXR fixture has no R/G/B channel set";
      return 0;
   }
   if (r->xSampling != 1 || r->ySampling != 1 ||
       g->xSampling != 1 || g->ySampling != 1 ||
       b->xSampling != 1 || b->ySampling != 1) {
      *reason = "OpenEXR fixture has subsampled RGB channels";
      return 0;
   }
   return 1;
}

static int has_flat_image_type(IMF::Header const &header, std::string *reason)
{
   IMF::StringAttribute const *type;

   type = header.findTypedAttribute<IMF::StringAttribute>("type");
   if (type == NULL)
      return 1;
   if (IMF::isDeepData(type->value())) {
      *reason = "OpenEXR fixture is deep data; use the deep API";
      return 0;
   }
   if (!IMF::isImage(type->value())) {
      *reason = "OpenEXR fixture is not a flat image part";
      return 0;
   }
   return 1;
}

template <class Input>
static LoadResult read_openexr_input(Input &input)
{
   LoadResult result;
   IMATH::Box2i data_window;
   long long w64;
   long long h64;
   int w;
   int h;
   std::string reason;
   IMF::FrameBuffer fb;
   size_t x_stride;
   size_t y_stride;

   result.ok = false;
   result.skip = false;

   if (!has_rgb_channels(input.header(), &reason))
      return skipped_result(reason.c_str());
   if (!has_flat_image_type(input.header(), &reason))
      return skipped_result(reason.c_str());

   data_window = input.header().dataWindow();
   w64 = (long long) data_window.max.x - (long long) data_window.min.x + 1;
   h64 = (long long) data_window.max.y - (long long) data_window.min.y + 1;
   if (w64 <= 0 || h64 <= 0 ||
       w64 > (long long) std::numeric_limits<int>::max() ||
       h64 > (long long) std::numeric_limits<int>::max()) {
      return failed_result("OpenEXR fixture has invalid dimensions");
   }

   w = (int) w64;
   h = (int) h64;
   if ((size_t) w > ((size_t) -1) / (size_t) h ||
       (size_t) w * (size_t) h > ((size_t) -1) / (4u * sizeof(float))) {
      return failed_result("OpenEXR fixture is too large for this reference harness");
   }

   result.image.w = w;
   result.image.h = h;
   result.image.comp = 4;
   result.image.pixels.assign((size_t) w * (size_t) h * 4u, 0.0f);

   x_stride = 4u * sizeof(float);
   y_stride = (size_t) w * x_stride;

   fb.insert("R", IMF::Slice::Make(IMF::FLOAT, &result.image.pixels[0],     data_window, x_stride, y_stride, 1, 1, 0.0));
   fb.insert("G", IMF::Slice::Make(IMF::FLOAT, &result.image.pixels[1],     data_window, x_stride, y_stride, 1, 1, 0.0));
   fb.insert("B", IMF::Slice::Make(IMF::FLOAT, &result.image.pixels[2],     data_window, x_stride, y_stride, 1, 1, 0.0));
   fb.insert("A", IMF::Slice::Make(IMF::FLOAT, &result.image.pixels[3],     data_window, x_stride, y_stride, 1, 1, 1.0));

   input.setFrameBuffer(fb);
   input.readPixels(data_window.min.y, data_window.max.y);

   result.ok = true;
   return result;
}

static LoadResult load_openexr(char const *path, int part_index)
{
   try {
      if (part_index >= 0) {
         IMF::MultiPartInputFile multi(path, 1);
         if (part_index >= multi.parts())
            return failed_result("OpenEXR part index out of range");
         IMF::InputPart part(multi, part_index);
         return read_openexr_input(part);
      } else {
         IMF::InputFile input(path, 1);
         return read_openexr_input(input);
      }
   } catch (std::exception const &e) {
      return failed_result(e.what());
   } catch (...) {
      return failed_result("OpenEXR threw a non-standard exception");
   }
}

static LoadResult load_exri(char const *path, int part_index)
{
   LoadResult result;
   float *pixels;
   int x;
   int y;
   int channels_in_file;
   int ok;

   result.ok = false;
   result.skip = false;
   pixels = NULL;
   x = y = channels_in_file = 0;

   if (part_index >= 0)
      ok = exri_loadf_part(&pixels, path, part_index, &x, &y, &channels_in_file, 4, EXRI_LOAD_DEFAULT);
   else
      ok = exri_loadf(&pixels, path, &x, &y, &channels_in_file, 4, EXRI_LOAD_DEFAULT);

   if (!ok) {
      result.reason = exri_failure_reason();
      return result;
   }
   if (pixels == NULL || x <= 0 || y <= 0) {
      exri_image_free(pixels);
      return failed_result("exr_image returned invalid image metadata");
   }
   if ((size_t) x > ((size_t) -1) / (size_t) y ||
       (size_t) x * (size_t) y > ((size_t) -1) / (4u * sizeof(float))) {
      exri_image_free(pixels);
      return failed_result("exr_image image is too large for this reference harness");
   }

   result.image.w = x;
   result.image.h = y;
   result.image.comp = 4;
   result.image.pixels.assign(pixels, pixels + (size_t) x * (size_t) y * 4u);
   exri_image_free(pixels);
   result.ok = true;
   return result;
}

static int nearly_equal(float a, float b, double epsilon)
{
   double da;
   double db;
   double diff;
   double scale;

   if (std::isnan(a) || std::isnan(b))
      return std::isnan(a) && std::isnan(b);
   if (std::isinf(a) || std::isinf(b))
      return a == b;

   da = (double) a;
   db = (double) b;
   diff = std::fabs(da - db);
   scale = std::fabs(da);
   if (std::fabs(db) > scale)
      scale = std::fabs(db);
   if (scale < 1.0)
      scale = 1.0;
   return diff <= epsilon * scale;
}

static int compare_images(Image const &a, Image const &b, double epsilon, std::string const &label)
{
   size_t count;
   size_t i;

   if (a.w != b.w || a.h != b.h || a.comp != b.comp) {
      std::fprintf(stderr, "mismatch %s: dimensions %dx%d/%d != %dx%d/%d\n",
                   label.c_str(), a.w, a.h, a.comp, b.w, b.h, b.comp);
      return 0;
   }

   count = (size_t) a.w * (size_t) a.h * (size_t) a.comp;
   for (i = 0; i < count; ++i) {
      if (!nearly_equal(a.pixels[i], b.pixels[i], epsilon)) {
         std::fprintf(stderr, "mismatch %s: pixel[%lu] %.9g != %.9g\n",
                      label.c_str(), (unsigned long) i, (double) a.pixels[i], (double) b.pixels[i]);
         return 0;
      }
   }

   return 1;
}

static int write_exri_image(char const *path, Image const &image)
{
   exri_write_options options;

   std::memset(&options, 0, sizeof(options));
   options.compression = EXRI_WRITE_COMPRESSION_ZIP;
   options.pixel_type = EXRI_WRITE_PIXEL_FLOAT;
   return exri_writef(path, image.w, image.h, image.comp, &image.pixels[0], &options);
}

static int write_openexr_image(char const *path, Image const &image)
{
   try {
      IMF::Header header(image.w, image.h);
      IMF::FrameBuffer fb;
      IMATH::Box2i data_window;
      size_t x_stride;
      size_t y_stride;

      header.compression() = IMF::ZIP_COMPRESSION;
      header.channels().insert("R", IMF::Channel(IMF::FLOAT));
      header.channels().insert("G", IMF::Channel(IMF::FLOAT));
      header.channels().insert("B", IMF::Channel(IMF::FLOAT));
      header.channels().insert("A", IMF::Channel(IMF::FLOAT));

      data_window = header.dataWindow();
      x_stride = 4u * sizeof(float);
      y_stride = (size_t) image.w * x_stride;
      fb.insert("R", IMF::Slice::Make(IMF::FLOAT, &image.pixels[0], data_window, x_stride, y_stride));
      fb.insert("G", IMF::Slice::Make(IMF::FLOAT, &image.pixels[1], data_window, x_stride, y_stride));
      fb.insert("B", IMF::Slice::Make(IMF::FLOAT, &image.pixels[2], data_window, x_stride, y_stride));
      fb.insert("A", IMF::Slice::Make(IMF::FLOAT, &image.pixels[3], data_window, x_stride, y_stride));

      IMF::OutputFile output(path, header, 1);
      output.setFrameBuffer(fb);
      output.writePixels(image.h);
      return 1;
   } catch (std::exception const &e) {
      std::fprintf(stderr, "openexr-write-failed %s: %s\n", path, e.what());
      return 0;
   } catch (...) {
      std::fprintf(stderr, "openexr-write-failed %s: non-standard exception\n", path);
      return 0;
   }
}

static std::string case_label(char const *path, int part_index)
{
   std::string label(path);
   if (part_index >= 0) {
      label += " part=";
      label += std::to_string(part_index);
   }
   return label;
}

static std::string temp_path(int index, char const *tag)
{
   std::string path("/tmp/exri_openexr_reference_");
   path += std::to_string((long long) std::time(NULL));
   path += "_";
   path += std::to_string(index);
   path += "_";
   path += tag;
   path += ".exr";
   return path;
}

static int check_reencode(Image const &reference, int case_index, double epsilon, std::string const &label, Stats *stats)
{
   std::string exri_path;
   std::string openexr_path;
   LoadResult loaded;
   int ok;

   ok = 1;
   exri_path = temp_path(case_index, "exri");
   openexr_path = temp_path(case_index, "openexr");

   if (!write_exri_image(exri_path.c_str(), reference)) {
      std::fprintf(stderr, "exri-write-failed %s: %s\n", label.c_str(), exri_failure_reason());
      ok = 0;
   } else {
      loaded = load_openexr(exri_path.c_str(), -1);
      if (!loaded.ok) {
         std::fprintf(stderr, "redecode-failed %s via exri->openexr: %s\n", label.c_str(), loaded.reason.c_str());
         ok = 0;
      } else if (!compare_images(reference, loaded.image, epsilon, label + " exri->openexr")) {
         ok = 0;
      } else {
         stats->reencoded += 1;
      }
   }

   if (!write_openexr_image(openexr_path.c_str(), reference)) {
      ok = 0;
   } else {
      loaded = load_exri(openexr_path.c_str(), -1);
      if (!loaded.ok) {
         std::fprintf(stderr, "redecode-failed %s via openexr->exri: %s\n", label.c_str(), loaded.reason.c_str());
         ok = 0;
      } else if (!compare_images(reference, loaded.image, epsilon, label + " openexr->exri")) {
         ok = 0;
      } else {
         stats->reencoded += 1;
      }
   }

   std::remove(exri_path.c_str());
   std::remove(openexr_path.c_str());
   return ok;
}

static void check_case(char const *path, int part_index, int case_index, double epsilon, Stats *stats)
{
   LoadResult reference;
   LoadResult ours;
   std::string label;
   int ok;

   stats->cases_seen += 1;
   label = case_label(path, part_index);

   reference = load_openexr(path, part_index);
   if (reference.skip) {
      std::printf("skip %s: %s\n", label.c_str(), reference.reason.c_str());
      stats->skipped += 1;
      return;
   }

   ours = load_exri(path, part_index);
   if (!reference.ok) {
      if (!ours.ok) {
         std::printf("not-loaded %s: openexr=%s exri=%s\n",
                     label.c_str(), reference.reason.c_str(), ours.reason.c_str());
         stats->skipped += 1;
         return;
      }
      std::fprintf(stderr, "unexpected-load %s: OpenEXR failed with %s but exri loaded\n",
                   label.c_str(), reference.reason.c_str());
      stats->failed += 1;
      return;
   }

   if (!ours.ok) {
      std::fprintf(stderr, "not-loaded %s: %s\n", label.c_str(), ours.reason.c_str());
      stats->failed += 1;
      return;
   }

   ok = compare_images(reference.image, ours.image, epsilon, label + " openexr-vs-exri");
   if (ok)
      ok = check_reencode(reference.image, case_index, epsilon, label, stats);

   if (ok) {
      std::printf("compare-ok %s: %dx%d comp=4\n", label.c_str(), reference.image.w, reference.image.h);
      stats->compared += 1;
   } else {
      stats->failed += 1;
   }
}

static void add_list_file(char const *path, std::vector<std::string> *files)
{
   std::ifstream input(path);
   std::string line;

   if (!input)
      throw std::runtime_error(std::string("could not open list file: ") + path);

   while (std::getline(input, line)) {
      if (!line.empty())
         files->push_back(line);
   }
}

static void usage(char const *argv0)
{
   std::fprintf(stderr, "usage: %s [--epsilon value] [--list file] file.exr ...\n", argv0);
}

int main(int argc, char **argv)
{
   std::vector<std::string> files;
   Stats stats;
   double epsilon;
   int i;
   int part_count;
   int case_index;

   std::memset(&stats, 0, sizeof(stats));
   epsilon = 0.0001;
   case_index = 0;

   for (i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--epsilon") == 0) {
         if (i + 1 >= argc) {
            usage(argv[0]);
            return 2;
         }
         epsilon = std::strtod(argv[++i], NULL);
         if (!(epsilon > 0.0)) {
            std::fprintf(stderr, "invalid epsilon\n");
            return 2;
         }
      } else if (std::strcmp(argv[i], "--list") == 0) {
         if (i + 1 >= argc) {
            usage(argv[0]);
            return 2;
         }
         try {
            add_list_file(argv[++i], &files);
         } catch (std::exception const &e) {
            std::fprintf(stderr, "%s\n", e.what());
            return 2;
         }
      } else if (argv[i][0] == '-' && argv[i][1] == '-') {
         usage(argv[0]);
         return 2;
      } else {
         files.push_back(argv[i]);
      }
   }

   if (files.empty()) {
      usage(argv[0]);
      return 2;
   }

   for (i = 0; i < (int) files.size(); ++i) {
      part_count = 0;
      if (exri_part_count(files[(size_t) i].c_str(), &part_count) && part_count > 1) {
         int part_index;
         for (part_index = 0; part_index < part_count; ++part_index)
            check_case(files[(size_t) i].c_str(), part_index, case_index++, epsilon, &stats);
      } else {
         check_case(files[(size_t) i].c_str(), -1, case_index++, epsilon, &stats);
      }
   }

   std::printf("summary cases=%d compared=%d reencoded=%d skipped=%d failed=%d epsilon=%g\n",
               stats.cases_seen, stats.compared, stats.reencoded, stats.skipped, stats.failed, epsilon);

   return stats.failed == 0 ? 0 : 1;
}
