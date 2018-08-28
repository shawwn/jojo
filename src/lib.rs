#![feature(nll)]
#![allow(unused_parens)]
#![allow(dead_code)]

pub mod dic;

mod jojo;
pub use jojo::*;

pub mod closure;
pub mod prim;
pub mod num;

pub mod scan;
