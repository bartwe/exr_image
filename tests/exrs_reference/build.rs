use std::env;
use std::path::PathBuf;
use std::process::Command;

fn run(mut command: Command) {
    let status = command.status().expect("failed to run build command");
    if !status.success() {
        panic!("build command failed with status {}", status);
    }
}

fn main() {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let root_dir = env::var_os("EXRI_ROOT_DIR")
        .map(PathBuf::from)
        .unwrap_or_else(|| manifest_dir.join("../.."));
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let source = manifest_dir.join("exri_impl.c");
    let object = out_dir.join("exri_impl.o");
    let library = out_dir.join("libexri_reference.a");
    let cc = env::var("CC").unwrap_or_else(|_| "cc".to_string());
    let ar = env::var("AR").unwrap_or_else(|_| "ar".to_string());

    let mut compile = Command::new(cc);
    compile
        .arg("-std=c89")
        .arg("-O2")
        .arg("-I")
        .arg(&root_dir)
        .arg("-c")
        .arg(&source)
        .arg("-o")
        .arg(&object);
    run(compile);

    let mut archive = Command::new(ar);
    archive.arg("crs").arg(&library).arg(&object);
    run(archive);

    println!("cargo:rerun-if-changed={}", source.display());
    println!(
        "cargo:rerun-if-changed={}",
        root_dir.join("exr_image.h").display()
    );
    println!("cargo:rustc-link-search=native={}", out_dir.display());
    println!("cargo:rustc-link-lib=static=exri_reference");
}
