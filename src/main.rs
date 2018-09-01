extern crate jojo;

use jojo::Make;

fn main () {
    println! ("JOJO's Bizarre Programming Adventure!");
    let mut env = jojo::Env::new ();
    let num = jojo::Num::make (1.0);
    env.obj_stack.push (num);
}
