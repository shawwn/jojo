#![feature(nll)]
#![allow(unused_parens)]
#![allow(dead_code)]

pub mod dic;

mod core;
pub use core::*;

pub mod data;
pub mod closure;
pub mod prim;
pub mod bool;
pub mod str;
pub mod sym;
pub mod num;
pub mod list;
pub mod vect;
pub mod dict;

pub mod scan;
pub mod sexp;
