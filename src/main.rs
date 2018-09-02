extern crate jojo;

use jojo::ObjFrom;

fn main () {
    println! ("JOJO's Bizarre Programming Adventure!");
    let mut env = jojo::Env::new ();
    let num = jojo::Num::obj (1.0);
    env.obj_stack.push (num);
}
