extern crate jojo;

use std::fs;
use std::io;
use std::fs::DirEntry;
use std::path::Path;

#[test]
fn test_jojo () -> io::Result <()> {
    visit_dirs (Path::new ("tests/jojo"), &|entry| {
        let module_path = entry.path ();
        let _env = jojo::Env::from_module_path (&module_path);
    })
}

fn visit_dirs (
    dir: &Path,
    cb: &Fn (&DirEntry),
) -> io::Result <()> {
    if dir.is_dir () {
        for entry in fs::read_dir (dir)? {
            let entry = entry?;
            let path = entry.path ();
            if path.is_file () {
                cb (&entry);
            }
        }
    }
    Ok (())
}
