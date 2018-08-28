#![feature(nll)]
#![allow(unused_parens)]
#![allow(dead_code)]

pub mod dic;

mod core;
pub use core::*;

pub mod data;
pub mod closure;
pub mod prim;
pub mod str;
pub mod num;

pub mod scan;
