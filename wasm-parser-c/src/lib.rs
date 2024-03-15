use std::ffi::CStr;
use std::os::raw::c_char;

#[no_mangle]
pub unsafe extern "C" fn print_hello_from_rust(c_filename: *const c_char) {
    let c_str = CStr::from_ptr(c_filename);
    if let Ok(rust_filename) = c_str.to_str() {
        println!("{}", rust_filename);
    } else {
        println!("Error: Invalid UTF-8 sequence in filename.");
    }
}

#[no_mangle]
pub extern "C" fn get_int_from_rust() -> i32 {
    43
}

use pretty_hex::pretty_hex;
use std::fs;
use std::io;
use std::io::Read;
use std::str;

// fn get_custom_section(filename: &str, section_name: &str) -> Result<(), BoxErr> {
//     let bytes =
//         fs::read(filename).map_err(|err| format!("failed to read {}: {}", filename, err))?;

//     let parser = wasmparser::Parser::new(0);

//     for payload in parser.parse_all(&bytes) {
//         match payload? {
//             wasmparser::Payload::CustomSection { name, data, .. } => {
//                 if name == section_name {
//                     println!("Section `{}` ({} bytes):", name, data.len());
//                     println!("{}", pretty_hex(&data));
//                     return Ok(());
//                 }
//             }
//             _ => {}
//         }
//     }

//     Err(format!("Section `{}` not found", section_name).into())
// }

use std::os::raw::c_int;

use std::ptr;

fn string_c2rust(c_str: *const c_char) -> String {
    unsafe {
        if c_str.is_null() {
            return String::new();
        }
        match CStr::from_ptr(c_str).to_str() {
            Ok(s) => s.to_string(),
            Err(_) => String::new(),
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_wasm_section(
    c_file_name: *const c_char,
    c_sec_name: *const c_char,
    c_ret_buf: *mut std::ffi::c_void,
    c_sec_len: *mut c_int,
) {
    let file_name = string_c2rust(c_file_name);
    let sec_name = string_c2rust(c_sec_name);

    let bytes = fs::read(file_name).unwrap();

    let parser = wasmparser::Parser::new(0);

    for payload in parser.parse_all(&bytes) {
        match payload {
            Ok(wasmparser::Payload::CustomSection { name, data, .. }) => {
                if name == sec_name {
                    println!("Section `{}` ({} bytes):", name, data.len());
                    println!("{}", pretty_hex(&data));
                    std::ptr::copy_nonoverlapping(data.as_ptr(), c_ret_buf as *mut u8, data.len());
                    ptr::write(c_sec_len, data.len() as i32);
                }
            }
            _ => {}
        }
    }
}
