use std::collections::HashMap;
use std::slice;
use std::vec;

#[derive(Clone)]
pub struct Entry <T> {
    pub name: String,
    pub value: Option <T>,
}

#[derive(Clone)]
pub struct Dic <T> {
    index_map: HashMap <String, usize>,
    entry_vector: Vec <Entry <T>>,
}

impl <T> Dic <T> {
    pub fn new () -> Dic <T> {
        Dic {
            index_map: HashMap::new (),
            entry_vector: Vec::new (),
        }
    }

    pub fn len (&self) -> usize {
        self.entry_vector.len ()
    }

    pub fn lack (&self) -> usize {
        let mut n = 0;
        for entry in &self.entry_vector {
            if let None = &entry.value {
                n += 1;
            }
        }
        n
    }

    pub fn idx (&self, index: usize) -> &Entry <T> {
        &self.entry_vector [index]
    }

    pub fn idx_set_value (&mut self, index: usize, value: Option <T>) {
        self.entry_vector [index] .value = value;
    }

    pub fn has_name (&self, name: &str) -> bool {
        self.index_map.contains_key (name)
    }

    pub fn ins (&mut self, name: &str, value: Option <T>) -> usize {
        assert! (! self.has_name (name));
        let index = self.len ();
        let entry = Entry { name: name.to_string (), value };
        self.entry_vector.push (entry);
        self.index_map.insert (name.to_string (), index);
        index
    }

    pub fn set (&mut self, name: &str, value: Option <T>) {
        if let Some (index) = self.index_map.get (name) {
            self.idx_set_value (*index, value);
        } else {
            eprintln! ("- Dic::set");
            eprintln! ("  dic does not have name : {}", name);
            panic! ("Dic::set fail!");
        }
    }

    pub fn get (&self, name: &str) -> Option <&T> {
        if let Some (index) = self.index_map.get (name) {
            let entry = self.idx (*index);
            if let Some (value) = &entry.value {
                Some (value)
            } else {
                None
            }
        } else {
            None
        }
    }

    pub fn eat (&mut self, x: T) {
        for entry in &mut self.entry_vector {
            if let None = entry.value {
                entry.value = Some (x);
                return;
            }
        }
        panic! ("Dic::eat fail!");
    }
}

#[test]
fn test_dic () {
    let mut dic: Dic <Vec <isize>> = Dic::new ();
    assert_eq! (0, dic.len ());

    let index = dic.ins ("key1", Some (vec! [1, 1, 1]));
    assert_eq! (0, index);
    assert_eq! (1, dic.len ());
    assert! (dic.has_name ("key1"));
    assert! (! dic.has_name ("non-key"));
    let entry = dic.idx (0);
    assert_eq! (entry.name, "key1");
    assert_eq! (entry.value, Some (vec! [1, 1, 1]));

    let index = dic.ins ("key2", Some (vec! [2, 2, 2]));
    assert_eq! (1, index);
    assert_eq! (2, dic.len ());
    assert! (dic.has_name ("key2"));
    let entry = dic.idx (1);
    assert_eq! (entry.name, "key2");
    assert_eq! (entry.value, Some (vec! [2, 2, 2]));

    assert_eq! (dic.get ("key1"), Some (&vec! [1, 1, 1]));
    assert_eq! (dic.get ("key2"), Some (&vec! [2, 2, 2]));

    dic.set ("key1", Some (vec! [2, 2, 2]));
    assert_eq! (dic.get ("key1"), Some (&vec! [2, 2, 2]));
    assert_eq! (2, dic.len ());
    assert_eq! (0, dic.lack ());

    dic.set ("key2", None);
    assert_eq! (dic.get ("key2"), None);
    assert_eq! (2, dic.len ());
    assert_eq! (1, dic.lack ());

    dic.eat (vec! [6, 6, 6]);
    assert_eq! (dic.get ("key2"), Some (&vec! [6, 6, 6]));

    dic.set ("key1", None);
    assert_eq! (dic.get ("key1"), None);

    dic.eat (vec! [7, 7, 7]);
    assert_eq! (dic.get ("key1"), Some (&vec! [7, 7, 7]));

    dic.set ("key1", None);
    dic.set ("key2", None);
    dic.eat (vec! [6, 6, 6]);
    dic.eat (vec! [7, 7, 7]);
    assert_eq! (dic.get ("key1"), Some (&vec! [6, 6, 6]));
    assert_eq! (dic.get ("key2"), Some (&vec! [7, 7, 7]));
}

impl <'a, T> From <Vec <(&'a str, T)>> for Dic <T> {
    fn from (vec: Vec <(&'a str, T)>) -> Dic <T> {
        let mut dic = Dic::new ();
        for x in vec {
            let name = x.0;
            let value = x.1;
            dic.ins (name, Some (value));
        }
        dic
    }
}

impl <'a, T> From <Vec <&'a str>> for Dic <T> {
    fn from (vec: Vec <&'a str>) -> Dic <T> {
        let mut dic = Dic::new ();
        for name in vec {
            dic.ins (name, None);
        }
        dic
    }
}

#[test]
fn test_from () {
    let dic = Dic::from (vec! [
        ("x", 0),
        ("y", 1),
        ("z", 2),
    ]);
    assert_eq! (dic.get ("x"), Some (&0));
    assert_eq! (dic.get ("y"), Some (&1));
    assert_eq! (dic.get ("z"), Some (&2));
    assert_eq! (dic.get ("_"), None);
}

pub struct Iter <'a, T: 'a> {
    slice_iter: slice::Iter <'a, Entry <T>>,
}

impl <'a, T: 'a> Iterator for Iter <'a, T> {
    type Item = (&'a str, &'a T);

    fn next (&mut self) -> Option <Self::Item> {
        while let Some (entry) = self.slice_iter.next () {
            if let Some (value) = &entry.value {
                return Some((&entry.name, value));
            }
        }
        None
    }
}

impl <T> Dic <T> {
    pub fn iter (&self) -> Iter <T> {
        Iter {
            slice_iter: self.entry_vector[..].iter ()
        }
    }
}

#[test]
fn test_iter () {
    let dic = Dic::from (vec! [
        ("x", 0),
        ("y", 1),
        ("z", 2),
    ]);
    let mut iter = dic.iter ();
    assert_eq! (Some (("x", &0)), iter.next ());
    assert_eq! (Some (("y", &1)), iter.next ());
    assert_eq! (Some (("z", &2)), iter.next ());
    assert_eq! (None, iter.next ());

    // skip None
    let mut dic = Dic::from (vec! [
        ("x", 0),
        ("y", 1),
        ("z", 2),
    ]);
    dic.set ("y", None);
    let mut iter = dic.iter ();
    assert_eq! (Some (("x", &0)), iter.next ());
    assert_eq! (Some (("z", &2)), iter.next ());
    assert_eq! (None, iter.next ());
}

pub struct IntoIter <T> {
    vec_into_iter: vec::IntoIter <Entry <T>>,
}

impl <T> Iterator for IntoIter <T> {
    type Item = (String, T);

    fn next (&mut self) -> Option <Self::Item> {
        while let Some (entry) = self.vec_into_iter.next () {
            if let Some (value) = entry.value {
                return Some((entry.name, value));
            }
        }
        None
    }
}

impl <T> Dic <T> {
    pub fn into_iter (self) -> IntoIter <T> {
        IntoIter {
            vec_into_iter: self.entry_vector.into_iter ()
        }
    }
}

#[test]
fn test_into_iter () {
    let dic = Dic::from (vec! [
        ("x", 0),
        ("y", 1),
        ("z", 2),
    ]);
    let mut into_iter = dic.into_iter ();
    assert_eq! (Some ((String::from ("x"), 0)), into_iter.next ());
    assert_eq! (Some ((String::from ("y"), 1)), into_iter.next ());
    assert_eq! (Some ((String::from ("z"), 2)), into_iter.next ());
    assert_eq! (None, into_iter.next ());

    // skip None
    let mut dic = Dic::from (vec! [
        ("x", 0),
        ("y", 1),
        ("z", 2),
    ]);
    dic.set ("y", None);
    let mut into_iter = dic.into_iter ();
    assert_eq! (Some ((String::from ("x"), 0)), into_iter.next ());
    assert_eq! (Some ((String::from ("z"), 2)), into_iter.next ());
    assert_eq! (None, into_iter.next ());
}
