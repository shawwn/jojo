extern crate jojo;

use jojo::*;

fn main () {
    println! ("JOJO's Bizarre Programming Adventure!");

    let mut env = Env::new ();

    let x: num::Num = 1.0;
    let num = Ptr::new (x);
    env.obj_stack.push (num);
}
