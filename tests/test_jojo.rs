extern crate jojo;

use std::path::Path;

#[test]
fn test_jojo () {
    let module_path = Path::new ("tests/jojo/test-simple.jo");
    let _env = jojo::Env::from_module_path (module_path);

    let module_path = Path::new ("tests/jojo/test-str.jo");
    let _env = jojo::Env::from_module_path (module_path);
}
