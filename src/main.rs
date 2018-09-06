extern crate jojo;

use std::env;
use std::path::Path;

fn main () {
    for arg in env::args () .skip (1) {
        let module_path = Path::new (&arg);
        let _env = jojo::Env::from_module_path (module_path);
    }
}
