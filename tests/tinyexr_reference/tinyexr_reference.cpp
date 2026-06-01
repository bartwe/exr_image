#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#define EXR_IMAGE_IMPLEMENTATION
#include "../../exr_image.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

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

struct ChannelInfo
{
   bool has_rgb;
   bool has_alpha;
   std::string reason;
};

static LoadResult failed_result(char const *reason)
{
   LoadResult r;
   r.ok = false;
   r.reason = reason ? reason : "";
   r.image.w = 0;
   r.image.h = 0;
   r.image.comp = 0;
   return r;
}

static LoadResult load_tinyexr(char const *path)
{
   LoadResult result;
   float *pixels;
   char const *err;
   int w;
   int h;

   result.ok = false;
   pixels = NULL;
   err = NULL;
   w = 0;
   h = 0;

   if (LoadEXR(&pixels, &w, &h, path, &err) != TINYEXR_SUCCESS) {
      result.reason = err ? err : "TinyEXR load failed";
      if (err)
         FreeEXRErrorMessage(err);
      return result;
   }

   if (pixels == NULL || w <= 0 || h <= 0) {
      std::free(pixels);
      return failed_result("TinyEXR returned invalid image metadata");
   }
   if ((size_t) w > ((size_t) -1) / (size_t) h ||
       (size_t) w * (size_t) h > ((size_t) -1) / (4u * sizeof(float))) {
      std::free(pixels);
      return failed_result("TinyEXR image is too large for this reference harness");
   }

   result.image.w = w;
   result.image.h = h;
   result.image.comp = 4;
   result.image.pixels.assign(pixels, pixels + (size_t) w * (size_t) h * 4u);
   std::free(pixels);
   result.ok = true;
   return result;
}

static ChannelInfo inspect_channels(char const *path)
{
   ChannelInfo info;
   int num_channels;
   int c;

   info.has_rgb = false;
   info.has_alpha = false;

   if (!exri_part_channel_count(path, 0, &num_channels)) {
      info.reason = exri_failure_reason();
      return info;
   }

   {
      bool has_r = false;
      bool has_g = false;
      bool has_b = false;

      for (c = 0; c < num_channels; ++c) {
         char name[256];

         if (!exri_part_channel_name(path, 0, c, name, (int) sizeof(name))) {
            info.reason = exri_failure_reason();
            return info;
         }

         if (std::strcmp(name, "R") == 0)
            has_r = true;
         else if (std::strcmp(name, "G") == 0)
            has_g = true;
         else if (std::strcmp(name, "B") == 0)
            has_b = true;
         else if (std::strcmp(name, "A") == 0)
            info.has_alpha = true;
      }

      info.has_rgb = has_r && has_g && has_b;
   }

   if (!info.has_rgb)
      info.reason = "fixture has no unlayered R/G/B channel set";
   return info;
}

static LoadResult load_exri(char const *path)
{
   LoadResult result;
   float *pixels;
   int x;
   int y;
   int channels_in_file;

   result.ok = false;
   pixels = NULL;
   x = 0;
   y = 0;
   channels_in_file = 0;

   if (!exri_loadf(&pixels, path, &x, &y, &channels_in_file, 4, EXRI_LOAD_DEFAULT)) {
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

static int compare_images(Image const &a, Image const &b, double epsilon, std::string const &label, int compare_alpha)
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
      if (!compare_alpha && (i % 4u) == 3u)
         continue;
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

static int write_tinyexr_image(char const *path, Image const &image)
{
   char const *err;
   int ret;

   err = NULL;
   ret = SaveEXR(&image.pixels[0], image.w, image.h, image.comp, 0, path, &err);
   if (ret != TINYEXR_SUCCESS) {
      std::fprintf(stderr, "tinyexr-write-failed %s: %s\n", path, err ? err : "TinyEXR write failed");
      if (err)
         FreeEXRErrorMessage(err);
      return 0;
   }
   return 1;
}

static std::string temp_path(int index, char const *tag)
{
   std::string path("/tmp/exri_tinyexr_reference_");
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
   std::string tinyexr_path;
   LoadResult loaded;
   int ok;

   ok = 1;
   exri_path = temp_path(case_index, "exri");
   tinyexr_path = temp_path(case_index, "tinyexr");

   if (!write_exri_image(exri_path.c_str(), reference)) {
      std::fprintf(stderr, "exri-write-failed %s: %s\n", label.c_str(), exri_failure_reason());
      ok = 0;
   } else {
      loaded = load_tinyexr(exri_path.c_str());
      if (!loaded.ok) {
         std::fprintf(stderr, "redecode-failed %s via exri->tinyexr: %s\n", label.c_str(), loaded.reason.c_str());
         ok = 0;
      } else if (!compare_images(reference, loaded.image, epsilon, label + " exri->tinyexr", 1)) {
         ok = 0;
      } else {
         stats->reencoded += 1;
      }
   }

   if (!write_tinyexr_image(tinyexr_path.c_str(), reference)) {
      ok = 0;
   } else {
      loaded = load_exri(tinyexr_path.c_str());
      if (!loaded.ok) {
         std::fprintf(stderr, "redecode-failed %s via tinyexr->exri: %s\n", label.c_str(), loaded.reason.c_str());
         ok = 0;
      } else if (!compare_images(reference, loaded.image, epsilon, label + " tinyexr->exri", 1)) {
         ok = 0;
      } else {
         stats->reencoded += 1;
      }
   }

   std::remove(exri_path.c_str());
   std::remove(tinyexr_path.c_str());
   return ok;
}

static void check_case(char const *path, int case_index, double epsilon, Stats *stats)
{
   LoadResult reference;
   LoadResult ours;
   ChannelInfo channels;
   int ok;

   stats->cases_seen += 1;

   channels = inspect_channels(path);
   if (!channels.has_rgb) {
      std::printf("skip %s: %s\n", path, channels.reason.c_str());
      stats->skipped += 1;
      return;
   }

   reference = load_tinyexr(path);
   ours = load_exri(path);

   if (!reference.ok) {
      if (!ours.ok) {
         std::printf("not-loaded %s: tinyexr=%s exri=%s\n",
                     path, reference.reason.c_str(), ours.reason.c_str());
      } else {
         std::printf("skip %s: TinyEXR did not load reference pixels: %s\n",
                     path, reference.reason.c_str());
      }
      stats->skipped += 1;
      return;
   }

   if (!ours.ok) {
      std::fprintf(stderr, "not-loaded %s: %s\n", path, ours.reason.c_str());
      stats->failed += 1;
      return;
   }

   ok = compare_images(reference.image, ours.image, epsilon, std::string(path) + " tinyexr-vs-exri", channels.has_alpha ? 1 : 0);
   if (ok)
      ok = check_reencode(reference.image, case_index, epsilon, path, stats);

   if (ok) {
      std::printf("compare-ok %s: %dx%d comp=4\n", path, reference.image.w, reference.image.h);
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

   std::memset(&stats, 0, sizeof(stats));
   epsilon = 0.0001;

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

   for (i = 0; i < (int) files.size(); ++i)
      check_case(files[(size_t) i].c_str(), i, epsilon, &stats);

   std::printf("summary cases=%d compared=%d reencoded=%d skipped=%d failed=%d epsilon=%g\n",
               stats.cases_seen, stats.compared, stats.reencoded, stats.skipped, stats.failed, epsilon);
   return stats.failed == 0 ? 0 : 1;
}
