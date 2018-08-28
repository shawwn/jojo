extern crate jojo;

fn main () {
    println! ("JOJO's Bizarre Programming Adventure!");

    let mut env = jojo::Env::new ();

    let x: jojo::num::Num = 1.0;
    let num = jojo::Ptr::new (x);
    env.obj_stack.push (num);
}
