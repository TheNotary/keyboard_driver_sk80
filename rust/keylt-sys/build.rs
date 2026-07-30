//! Locates the native `keylt` library, emits the link directives for it, and
//! optionally regenerates the FFI bindings from `keylt.h`.
//!
//! Discovery is tried in this order, and the first mode that succeeds wins:
//!
//! 1. `KEYLT_LIB_DIR` + `KEYLT_INCLUDE_DIR` -- an explicit install prefix.
//! 2. `pkg-config` -- the `keylt.pc` installed by the CMake project.
//! 3. The `vendored` feature -- build the CMake project at the repository root.
//!
//! Putting the environment variables first means a CI job that has already
//! staged an install can reuse it without paying for a second CMake build, even
//! though the `vendored` feature is on by default.

use std::env;
use std::path::PathBuf;

/// How the native library was located, and where its headers ended up.
struct Native {
    include_dirs: Vec<PathBuf>,
    /// Static linkage drags in keylt's own dependencies, because
    /// `keylt_static` links hidapi as a PUBLIC usage requirement. The shared
    /// library hides them behind `--exclude-libs,ALL` and needs none of this.
    static_link: bool,
}

fn main() {
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-changed=wrapper.h");
    for var in [
        "KEYLT_LIB_DIR",
        "KEYLT_INCLUDE_DIR",
        "KEYLT_LIB_NAME",
        "DOCS_RS",
    ] {
        println!("cargo:rerun-if-env-changed={var}");
    }

    // docs.rs builds in a sandbox with no CMake toolchain, no vcpkg checkout and
    // no installed keylt. Rustdoc never links, so skip discovery entirely.
    if env::var_os("DOCS_RS").is_some() {
        generate_bindings(&[]);
        return;
    }

    let native = discover();

    // Downstream crates with a build script of their own can read these back as
    // DEP_KEYLT_INCLUDE / DEP_KEYLT_STATIC, courtesy of the `links` key.
    println!(
        "cargo:include={}",
        native
            .include_dirs
            .iter()
            .map(|p| p.display().to_string())
            .collect::<Vec<_>>()
            .join(";")
    );
    println!("cargo:static={}", native.static_link);

    generate_bindings(&native.include_dirs);
}

fn discover() -> Native {
    if let Some(native) = discover_from_env() {
        return native;
    }

    if let Some(native) = discover_from_pkg_config() {
        return native;
    }

    discover_vendored()
}

#[cfg(feature = "vendored")]
fn discover_vendored() -> Native {
    build_vendored()
}

#[cfg(not(feature = "vendored"))]
fn discover_vendored() -> Native {
    panic!(
        "could not locate the native `keylt` library.\n\
         \n\
         Choose one of:\n\
         \x20 * set KEYLT_LIB_DIR and KEYLT_INCLUDE_DIR to an install prefix, e.g.\n\
         \x20     KEYLT_LIB_DIR=/usr/local/lib KEYLT_INCLUDE_DIR=/usr/local/include\n\
         \x20 * install keylt so that pkg-config can find `keylt.pc`\n\
         \x20 * enable the `vendored` feature to build it from source with CMake"
    );
}

/// An explicit install prefix supplied by the caller.
fn discover_from_env() -> Option<Native> {
    let lib_dir = env::var_os("KEYLT_LIB_DIR")?;
    let include_dir = env::var_os("KEYLT_INCLUDE_DIR")?;
    let lib_dir = PathBuf::from(lib_dir);
    let include_dir = PathBuf::from(include_dir);

    // Failing loudly here beats emitting a link search path that silently
    // resolves to some other copy of the library.
    assert!(
        lib_dir.is_dir(),
        "KEYLT_LIB_DIR is not a directory: {}",
        lib_dir.display()
    );
    assert!(
        include_dir.join("keylt").join("keylt.h").is_file(),
        "KEYLT_INCLUDE_DIR does not contain keylt/keylt.h: {}",
        include_dir.display()
    );

    let static_link = cfg!(feature = "static");
    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    emit_link_lib(static_link);
    if static_link {
        emit_transitive_deps();
    }
    track_native_inputs(
        std::slice::from_ref(&lib_dir),
        std::slice::from_ref(&include_dir),
    );

    Some(Native {
        include_dirs: vec![include_dir],
        static_link,
    })
}

/// The `keylt.pc` installed alongside the CMake package.
fn discover_from_pkg_config() -> Option<Native> {
    let static_link = cfg!(feature = "static");
    let library = pkg_config::Config::new()
        .statik(static_link)
        .cargo_metadata(true)
        .probe("keylt")
        .ok()?;

    track_native_inputs(&library.link_paths, &library.include_paths);

    // pkg-config already emitted the link directives, including
    // `Requires.private` when solving statically.
    Some(Native {
        include_dirs: library.include_paths,
        static_link,
    })
}

/// Make cargo re-run this script, and therefore relink, when the native library
/// or its header changes underneath us.
///
/// Cargo tracks a build script's declared inputs, not the files the link line
/// happens to name. Without this, reinstalling a different build of keylt into
/// the same prefix leaves the path identical, the script is considered fresh,
/// and the previously linked archive is silently reused.
fn track_native_inputs(lib_dirs: &[PathBuf], include_dirs: &[PathBuf]) {
    for dir in lib_dirs {
        // Cover every linkage and platform naming convention; only the files
        // that actually exist are worth declaring.
        for name in [
            "libkeylt.a",
            "libkeylt.so",
            "libkeylt.dylib",
            "keylt.lib",
            "keylt.dll",
            "keylt_static.lib",
        ] {
            let candidate = dir.join(name);
            if candidate.exists() {
                println!("cargo:rerun-if-changed={}", candidate.display());
            }
        }
    }

    for dir in include_dirs {
        let header = dir.join("keylt").join("keylt.h");
        if header.is_file() {
            println!("cargo:rerun-if-changed={}", header.display());
        }
    }
}

/// Build the CMake project at the repository root and install it into `OUT_DIR`.
///
/// This always produces a static archive: a vendored shared library would have
/// to be found again at run time, and a build script cannot inject an rpath into
/// the crates that depend on it.
#[cfg(feature = "vendored")]
fn build_vendored() -> Native {
    let repo = repository_root();
    assert!(
        repo.join("CMakeLists.txt").is_file(),
        "the `vendored` feature needs the CMake project at {}, but no CMakeLists.txt is there.\n\
         Set KEYLT_LIB_DIR and KEYLT_INCLUDE_DIR instead when building against a published crate.",
        repo.display()
    );

    let mut cfg = cmake::Config::new(&repo);
    cfg.define("KEYLT_BUILD_TESTS", "OFF")
        .define("KEYLT_BUILD_BENCHMARKS", "OFF")
        .define("KEYLT_BUILD_DEMO", "OFF")
        .define("KEYLT_BUILD_SHARED", "OFF")
        .define("KEYLT_BUILD_STATIC", "ON")
        // Only the C ABI is bound, so the internal C++ headers are dead weight.
        .define("KEYLT_INSTALL_CXX_HEADERS", "OFF")
        // The root vcpkg.json enables the `benchmarks` and `tests` features by
        // default; without this, every build of this crate would also compile
        // gtest and google-benchmark.
        .define("VCPKG_MANIFEST_NO_DEFAULT_FEATURES", "ON");

    // The repository deliberately points at its own vcpkg checkout rather than
    // $VCPKG_ROOT, which may hold a stale value. Match that.
    let toolchain = repo.join("vcpkg/scripts/buildsystems/vcpkg.cmake");
    if toolchain.is_file() {
        cfg.define("CMAKE_TOOLCHAIN_FILE", &toolchain);
    }

    // Editing the C++ sources should rebuild the library. Cargo scans a
    // directory given to rerun-if-changed recursively, so these three cover it
    // without naming every file.
    for path in ["CMakeLists.txt", "include", "src"] {
        println!("cargo:rerun-if-changed={}", repo.join(path).display());
    }

    let prefix = cfg.build();

    println!(
        "cargo:rustc-link-search=native={}",
        prefix.join("lib").display()
    );
    // Multilib layouts install to lib64; harmless when it does not exist.
    println!(
        "cargo:rustc-link-search=native={}",
        prefix.join("lib64").display()
    );
    emit_link_lib(true);
    emit_transitive_deps();

    Native {
        include_dirs: vec![prefix.join("include")],
        static_link: true,
    }
}

/// `rust/keylt-sys` -> `rust` -> repository root.
#[cfg(feature = "vendored")]
fn repository_root() -> PathBuf {
    let manifest = PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR"));
    manifest
        .parent()
        .and_then(|rust_dir| rust_dir.parent())
        .expect("keylt-sys should live two directories below the repository root")
        .to_path_buf()
}

fn emit_link_lib(static_link: bool) {
    let kind = if static_link { "static" } else { "dylib" };
    println!("cargo:rustc-link-lib={kind}={}", library_name(static_link));
}

/// On MSVC the shared library's import library is already called `keylt.lib`, so
/// the static archive is renamed to avoid the collision. Everywhere else both
/// linkages share the `keylt` stem and are told apart by the file extension.
fn library_name(static_link: bool) -> String {
    if let Ok(name) = env::var("KEYLT_LIB_NAME") {
        return name;
    }
    let msvc = env::var("CARGO_CFG_TARGET_ENV").as_deref() == Ok("msvc");
    if static_link && msvc {
        "keylt_static".to_string()
    } else {
        "keylt".to_string()
    }
}

/// Emit the libraries `keylt_static` exposes as PUBLIC usage requirements.
///
/// A static archive carries no record of its own dependencies, so they have to
/// be restated on the final link line.
fn emit_transitive_deps() {
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();

    if target_os == "windows" {
        // Windows talks to HID through the platform SDK, not hidapi.
        println!("cargo:rustc-link-lib=dylib=setupapi");
        println!("cargo:rustc-link-lib=dylib=hid");
        // MSVC links its C++ runtime automatically; MinGW does not.
        if env::var("CARGO_CFG_TARGET_ENV").as_deref() != Ok("msvc") {
            println!("cargo:rustc-link-lib=dylib=stdc++");
        }
        return;
    }

    // hidapi-libusb pulls in libusb-1.0 and libudev through its own .pc file.
    let hidapi = pkg_config::Config::new()
        .statik(true)
        .cargo_metadata(true)
        .probe("hidapi-libusb")
        .or_else(|_| {
            pkg_config::Config::new()
                .statik(true)
                .cargo_metadata(true)
                .probe("hidapi")
        });
    if let Err(err) = hidapi {
        println!("cargo:warning=pkg-config could not resolve hidapi ({err}); linking -lhidapi-libusb directly");
        println!("cargo:rustc-link-lib=dylib=hidapi-libusb");
    }

    // keylt is C++ even though its interface is C.
    println!("cargo:rustc-link-lib=dylib=stdc++");
}

/// Regenerate `src/bindings.rs` into `OUT_DIR`, or do nothing when the
/// checked-in copy is being used.
#[allow(unused_variables)]
fn generate_bindings(include_dirs: &[PathBuf]) {
    #[cfg(feature = "bindgen")]
    {
        let out = PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR"));
        let mut builder = bindgen::Builder::default()
            .header("wrapper.h")
            .allowlist_function("keylt_.*")
            .allowlist_type("keylt_.*")
            .allowlist_var("KEYLT_.*")
            // Plain constants rather than Rust enums: a `#[repr(C)] enum` with
            // an out-of-range discriminant is undefined behaviour, and a future
            // version of the library is free to add status codes.
            .default_enum_style(bindgen::EnumVariation::Consts)
            .derive_default(true)
            .derive_debug(true)
            .derive_copy(true)
            .derive_partialeq(true)
            .derive_eq(true)
            .derive_hash(true)
            // Comparing or hashing function pointers is meaningless and rustc
            // warns about it, so the vtable opts out. `Default` is provided by
            // hand in lib.rs, so that both binding sources expose the same
            // all-NULL constructor and neither collides with the other.
            .no_partialeq("keylt_usb_io")
            .no_hash("keylt_usb_io")
            .no_default("keylt_usb_io")
            .prepend_enum_name(false)
            .generate_comments(true)
            .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()));

        for dir in include_dirs {
            builder = builder.clang_arg(format!("-I{}", dir.display()));
        }
        // Suppress __declspec(dllimport) so the layout tests compile when the
        // static archive is in use.
        if cfg!(feature = "static") || cfg!(feature = "vendored") {
            builder = builder.clang_arg("-DKEYLT_STATIC");
        }

        builder
            .generate()
            .expect("failed to generate bindings from keylt.h")
            .write_to_file(out.join("bindings.rs"))
            .expect("failed to write bindings.rs");
    }
}
