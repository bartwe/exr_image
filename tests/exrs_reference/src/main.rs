use exr::prelude::*;
use std::env;
use std::ffi::{CStr, CString};
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::os::raw::{c_char, c_int, c_void};
use std::path::{Path, PathBuf};

const EXRI_LOAD_DEFAULT: c_int = 0;
const EXRI_WRITE_COMPRESSION_ZIP: c_int = 3;
const EXRI_WRITE_PIXEL_FLOAT: c_int = 0;

#[repr(C)]
struct ExriWriteOptions {
    compression: c_int,
    pixel_type: c_int,
    tiled: c_int,
    tile_size: c_int,
    level_mode: c_int,
    level_rounding: c_int,
}

extern "C" {
    fn exri_loadf(
        out_pixels: *mut *mut f32,
        filename: *const c_char,
        x: *mut c_int,
        y: *mut c_int,
        channels_in_file: *mut c_int,
        desired_channels: c_int,
        load_flags: c_int,
    ) -> c_int;
    fn exri_loadf_part(
        out_pixels: *mut *mut f32,
        filename: *const c_char,
        part_index: c_int,
        x: *mut c_int,
        y: *mut c_int,
        channels_in_file: *mut c_int,
        desired_channels: c_int,
        load_flags: c_int,
    ) -> c_int;
    fn exri_part_count(filename: *const c_char, parts: *mut c_int) -> c_int;
    fn exri_writef(
        filename: *const c_char,
        w: c_int,
        h: c_int,
        comp: c_int,
        data: *const f32,
        options: *const ExriWriteOptions,
    ) -> c_int;
    fn exri_image_free(ptr: *mut c_void);
    fn exri_failure_reason() -> *const c_char;
}

#[derive(Clone)]
struct Image {
    w: usize,
    h: usize,
    pixels: Vec<f32>,
}

struct LoadResult {
    ok: bool,
    reason: String,
    image: Option<Image>,
}

#[derive(Default)]
struct Stats {
    cases_seen: usize,
    compared: usize,
    skipped: usize,
    failed: usize,
    reencoded: usize,
}

fn failed(reason: impl Into<String>) -> LoadResult {
    LoadResult {
        ok: false,
        reason: reason.into(),
        image: None,
    }
}

fn ok(image: Image) -> LoadResult {
    LoadResult {
        ok: true,
        reason: String::new(),
        image: Some(image),
    }
}

fn path_to_cstring(path: &Path) -> std::result::Result<CString, String> {
    CString::new(path.to_string_lossy().as_bytes())
        .map_err(|_| format!("path contains embedded NUL: {}", path.display()))
}

fn exri_reason() -> String {
    unsafe {
        let reason = exri_failure_reason();
        if reason.is_null() {
            String::from("unknown exr_image failure")
        } else {
            CStr::from_ptr(reason).to_string_lossy().into_owned()
        }
    }
}

fn take_exri_image(pixels: *mut f32, x: c_int, y: c_int) -> LoadResult {
    if pixels.is_null() || x <= 0 || y <= 0 {
        unsafe {
            exri_image_free(pixels.cast::<c_void>());
        }
        return failed("exr_image returned invalid image metadata");
    }

    let count = match (x as usize)
        .checked_mul(y as usize)
        .and_then(|v| v.checked_mul(4))
    {
        Some(count) => count,
        None => {
            unsafe {
                exri_image_free(pixels.cast::<c_void>());
            }
            return failed("exr_image image is too large for this reference harness");
        }
    };

    let image = unsafe {
        let slice = std::slice::from_raw_parts(pixels, count);
        let image = Image {
            w: x as usize,
            h: y as usize,
            pixels: slice.to_vec(),
        };
        exri_image_free(pixels.cast::<c_void>());
        image
    };

    ok(image)
}

fn load_exrs(path: &Path) -> LoadResult {
    let read = read_first_rgba_layer_from_file(
        path,
        |resolution, _channels: &RgbaChannels| Image {
            w: resolution.width(),
            h: resolution.height(),
            pixels: vec![0.0; resolution.width() * resolution.height() * 4],
        },
        |image: &mut Image, position: Vec2<usize>, (r, g, b, a): (f32, f32, f32, f32)| {
            let index = (position.y() * image.w + position.x()) * 4;
            image.pixels[index] = r;
            image.pixels[index + 1] = g;
            image.pixels[index + 2] = b;
            image.pixels[index + 3] = a;
        },
    );

    match read {
        Ok(image) => ok(image.layer_data.channel_data.pixels),
        Err(error) => failed(error.to_string()),
    }
}

fn load_exri(path: &Path) -> LoadResult {
    let filename = match path_to_cstring(path) {
        Ok(filename) => filename,
        Err(reason) => return failed(reason),
    };
    let mut pixels: *mut f32 = std::ptr::null_mut();
    let mut x: c_int = 0;
    let mut y: c_int = 0;
    let mut channels: c_int = 0;

    let result = unsafe {
        exri_loadf(
            &mut pixels,
            filename.as_ptr(),
            &mut x,
            &mut y,
            &mut channels,
            4,
            EXRI_LOAD_DEFAULT,
        )
    };

    if result == 0 {
        let first_reason = exri_reason();
        let mut parts: c_int = 0;
        let part_count_result = unsafe { exri_part_count(filename.as_ptr(), &mut parts) };
        if part_count_result == 0 || parts <= 1 {
            return failed(first_reason);
        }

        for part in 0..parts {
            pixels = std::ptr::null_mut();
            x = 0;
            y = 0;
            channels = 0;
            let part_result = unsafe {
                exri_loadf_part(
                    &mut pixels,
                    filename.as_ptr(),
                    part,
                    &mut x,
                    &mut y,
                    &mut channels,
                    4,
                    EXRI_LOAD_DEFAULT,
                )
            };
            if part_result != 0 {
                return take_exri_image(pixels, x, y);
            }
        }

        return failed(first_reason);
    }

    take_exri_image(pixels, x, y)
}

fn nearly_equal(a: f32, b: f32, epsilon: f64) -> bool {
    if a.is_nan() || b.is_nan() {
        return a.is_nan() && b.is_nan();
    }
    if a.is_infinite() || b.is_infinite() {
        return a == b;
    }

    let da = a as f64;
    let db = b as f64;
    let scale = da.abs().max(db.abs()).max(1.0);
    (da - db).abs() <= epsilon * scale
}

fn compare_images(a: &Image, b: &Image, epsilon: f64, label: &str) -> bool {
    if a.w != b.w || a.h != b.h {
        eprintln!(
            "mismatch {label}: dimensions {}x{} != {}x{}",
            a.w, a.h, b.w, b.h
        );
        return false;
    }

    for index in 0..a.pixels.len() {
        if !nearly_equal(a.pixels[index], b.pixels[index], epsilon) {
            eprintln!(
                "mismatch {label}: pixel[{index}] {:.9} != {:.9}",
                a.pixels[index], b.pixels[index]
            );
            return false;
        }
    }

    true
}

fn write_exri(path: &Path, image: &Image) -> std::result::Result<(), String> {
    let filename = path_to_cstring(path)?;
    let options = ExriWriteOptions {
        compression: EXRI_WRITE_COMPRESSION_ZIP,
        pixel_type: EXRI_WRITE_PIXEL_FLOAT,
        tiled: 0,
        tile_size: 0,
        level_mode: 0,
        level_rounding: 0,
    };

    let result = unsafe {
        exri_writef(
            filename.as_ptr(),
            image.w as c_int,
            image.h as c_int,
            4,
            image.pixels.as_ptr(),
            &options,
        )
    };

    if result == 0 {
        Err(exri_reason())
    } else {
        Ok(())
    }
}

fn write_exrs(path: &Path, image: &Image) -> std::result::Result<(), String> {
    write_rgba_file(path, image.w, image.h, |x, y| {
        let index = (y * image.w + x) * 4;
        (
            image.pixels[index],
            image.pixels[index + 1],
            image.pixels[index + 2],
            image.pixels[index + 3],
        )
    })
    .map_err(|error| error.to_string())
}

fn temp_path(index: usize, tag: &str) -> PathBuf {
    let mut path = env::temp_dir();
    let now = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_secs())
        .unwrap_or(0);
    path.push(format!("exri_exrs_reference_{now}_{index}_{tag}.exr"));
    path
}

fn check_reencode(
    reference: &Image,
    case_index: usize,
    epsilon: f64,
    label: &str,
    stats: &mut Stats,
) -> bool {
    let exri_path = temp_path(case_index, "exri");
    let exrs_path = temp_path(case_index, "exrs");
    let mut all_ok = true;

    match write_exri(&exri_path, reference) {
        Ok(()) => {
            let loaded = load_exrs(&exri_path);
            if !loaded.ok {
                eprintln!("redecode-failed {label} via exri->exrs: {}", loaded.reason);
                all_ok = false;
            } else if !compare_images(
                reference,
                loaded.image.as_ref().unwrap(),
                epsilon,
                &format!("{label} exri->exrs"),
            ) {
                all_ok = false;
            } else {
                stats.reencoded += 1;
            }
        }
        Err(reason) => {
            eprintln!("exri-write-failed {label}: {reason}");
            all_ok = false;
        }
    }

    match write_exrs(&exrs_path, reference) {
        Ok(()) => {
            let loaded = load_exri(&exrs_path);
            if !loaded.ok {
                eprintln!("redecode-failed {label} via exrs->exri: {}", loaded.reason);
                all_ok = false;
            } else if !compare_images(
                reference,
                loaded.image.as_ref().unwrap(),
                epsilon,
                &format!("{label} exrs->exri"),
            ) {
                all_ok = false;
            } else {
                stats.reencoded += 1;
            }
        }
        Err(reason) => {
            eprintln!("exrs-write-failed {label}: {reason}");
            all_ok = false;
        }
    }

    let _ = std::fs::remove_file(exri_path);
    let _ = std::fs::remove_file(exrs_path);
    all_ok
}

fn check_case(path: &Path, case_index: usize, epsilon: f64, stats: &mut Stats) {
    stats.cases_seen += 1;
    let label = path.display().to_string();
    let reference = load_exrs(path);
    let ours = load_exri(path);

    if !reference.ok {
        if !ours.ok {
            println!(
                "not-loaded {}: exrs={} exri={}",
                path.display(),
                reference.reason,
                ours.reason
            );
        } else {
            println!(
                "skip {}: exrs did not load reference pixels: {}",
                path.display(),
                reference.reason
            );
        }
        stats.skipped += 1;
        return;
    }

    if !ours.ok {
        eprintln!("not-loaded {}: {}", path.display(), ours.reason);
        stats.failed += 1;
        return;
    }

    let reference_image = reference.image.as_ref().unwrap();
    let ours_image = ours.image.as_ref().unwrap();
    let mut passed = compare_images(
        reference_image,
        ours_image,
        epsilon,
        &format!("{label} exrs-vs-exri"),
    );
    if passed {
        passed = check_reencode(reference_image, case_index, epsilon, &label, stats);
    }

    if passed {
        println!(
            "compare-ok {}: {}x{} comp=4",
            path.display(),
            reference_image.w,
            reference_image.h
        );
        stats.compared += 1;
    } else {
        stats.failed += 1;
    }
}

fn add_list_file(path: &Path, files: &mut Vec<PathBuf>) -> std::result::Result<(), String> {
    let file = File::open(path)
        .map_err(|error| format!("could not open list file {}: {error}", path.display()))?;
    for line in BufReader::new(file).lines() {
        let line =
            line.map_err(|error| format!("could not read list file {}: {error}", path.display()))?;
        if !line.is_empty() {
            files.push(PathBuf::from(line));
        }
    }
    Ok(())
}

fn usage(program: &str) {
    eprintln!("usage: {program} [--epsilon value] [--list file] file.exr ...");
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let mut files = Vec::new();
    let mut epsilon = 0.0001_f64;
    let mut index = 1;

    while index < args.len() {
        if args[index] == "--epsilon" {
            index += 1;
            if index >= args.len() {
                usage(&args[0]);
                std::process::exit(2);
            }
            epsilon = args[index].parse::<f64>().unwrap_or(0.0);
            if epsilon <= 0.0 {
                eprintln!("invalid epsilon");
                std::process::exit(2);
            }
        } else if args[index] == "--list" {
            index += 1;
            if index >= args.len() {
                usage(&args[0]);
                std::process::exit(2);
            }
            if let Err(reason) = add_list_file(Path::new(&args[index]), &mut files) {
                eprintln!("{reason}");
                std::process::exit(2);
            }
        } else if args[index].starts_with("--") {
            usage(&args[0]);
            std::process::exit(2);
        } else {
            files.push(PathBuf::from(&args[index]));
        }
        index += 1;
    }

    if files.is_empty() {
        usage(&args[0]);
        std::process::exit(2);
    }

    let mut stats = Stats::default();
    for (case_index, file) in files.iter().enumerate() {
        check_case(file, case_index, epsilon, &mut stats);
    }

    println!(
        "summary cases={} compared={} reencoded={} skipped={} failed={} epsilon={}",
        stats.cases_seen, stats.compared, stats.reencoded, stats.skipped, stats.failed, epsilon
    );
    std::process::exit(if stats.failed == 0 { 0 } else { 1 });
}
