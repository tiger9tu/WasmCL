use pretty_hex::pretty_hex;
use std::ffi::CStr;
use std::fs;
use std::os::raw::c_char;
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
