#![feature(nll)]
#![allow(unused_parens)]
#![allow(dead_code)]

pub mod dic;

mod core;
pub use core::*;

mod data;
mod closure;
mod prim;
mod bool;
mod str;
mod sym;
mod num;
mod list;
mod vect;
mod dict;

pub mod scan;
pub mod sexp;
