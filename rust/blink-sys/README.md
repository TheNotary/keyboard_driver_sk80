# blink-sys

Raw FFI bindings to the `blink` C ABI for RGB mechanical keyboard LED and LCD
control. This is a mechanical translation of `blink.h` and applies no safety
rules of its own — prefer the [`blink`](https://crates.io/crates/blink) crate.

The build script finds the native library in this order:

1. `BLINK_LIB_DIR` and `BLINK_INCLUDE_DIR`, pointing at an install prefix.
2. `pkg-config`, using the `blink.pc` the CMake project installs.
3. The `vendored` feature (on by default), which builds the CMake project from
   source and links the resulting static archive.

| Feature | Default | Effect |
| --- | --- | --- |
| `vendored` | yes | Build the C library from source with CMake. Implies static linkage. |
| `static` | no | Link the static archive when using modes 1 or 2. |
| `bindgen` | no | Regenerate the declarations from `blink.h`. Needs libclang. |

The checked-in declarations in `src/bindings.rs` are maintained by hand so that
the common case needs no libclang; CI regenerates them with bindgen and runs the
test suite against both.
