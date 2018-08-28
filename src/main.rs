extern crate jojo;

fn main () {
    println! ("JOJO's Bizarre Programming Adventure!");
    let mut env = jojo::Env::new ();
    let num = jojo::Ptr::new (jojo::Num (1.0));
    env.obj_stack.push (num);
}
