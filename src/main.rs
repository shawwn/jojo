extern crate jojo;

fn main () {
    println! ("JOJO's Bizarre Programming Adventure!");

    let mut env = jojo::new_env ();

    let num = jojo::Ptr::new (1);
    env.obj_stack.push (num);
}
