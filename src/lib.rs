#![feature(nll)]

#![allow(unused_parens)]
// #![allow(dead_code)]
#![allow(unused_macros)]

extern crate dic;

pub mod token;

mod core;
pub use core::*;
