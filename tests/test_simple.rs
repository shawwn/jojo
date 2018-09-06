extern crate jojo;

use std::path::Path;

#[test]
fn simple () {
    let module_path = Path::new ("tests/jojo/test-simple.jo");
    let _env = jojo::Env::from_module_path (module_path);
}
