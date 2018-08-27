/// todo-list:
///   use str as key

use std::collections::HashMap;

pub type Name = String;

pub struct Entry <T> {
    name: Name,
    value: T,
}

pub struct Dic <T> {
    index_map: HashMap <Name, usize>,
    entry_vector: Vec <Option <Entry <T>>>,
}

impl <T> Dic <T> {
    fn new () -> Dic <T> {
        Dic {
            index_map: HashMap::new (),
            entry_vector: Vec::new (),
        }
    }

    fn len (&self) -> usize {
        self.entry_vector.len ()
    }

    fn nth (&self, index: usize) -> &Option <Entry <T>> {
        if index < self.len () {
            &self.entry_vector [index]
        } else {
            &None
        }
    }

    fn set_nth (&mut self, index: usize, value: T) {
        if let Some (entry) = &mut self.entry_vector [index] {
            entry.value = value;
        }
    }

    fn has (&self, name: &Name) -> bool {
        self.index_map.contains_key (name)
    }

    fn ins_none (&mut self, name: &Name) -> usize {
        assert! (! self.has (name));
        let index = self.len ();
        self.entry_vector.push (None);
        self.index_map.insert (name.clone (), index);
        index
    }

    fn ins (&mut self, name: &Name, value: T) -> usize {
        assert! (! self.has (name));
        let index = self.len ();
        let entry = Entry { name: name.clone (), value };
        self.entry_vector.push (Some (entry));
        self.index_map.insert (name.clone (), index);
        index
    }

    fn set (&mut self, name: &Name, value: T) {
        if let Some (index) = self.index_map.get (name) {
            self.set_nth (*index, value);
        } else {
            eprintln! ("- Dic::set");
            eprintln! ("  dic does not have name : {}", name);
            panic! ("Dic::set fail!");
        }
    }

    fn get (&self, name: &Name) -> Option <&T> {
        if let Some (index) = self.index_map.get (name) {
            if let Some (entry) = &self.entry_vector [*index] {
                Some (&entry.value)
            } else {
                None
            }
        } else {
            None
        }
    }
}

#[test]
fn test_dic () {
    let mut dic: Dic <Vec <String>> = Dic::new ();
    assert_eq! (0, dic.len ());
    let index = dic.ins (&"k1".to_string (),
                         vec! [
                             "s1".to_string (),
                             "s2".to_string (),
                         ]);
    assert_eq! (1, dic.len ());
    assert! (dic.has (&"k1".to_string ()));
    if let Some (entry) = dic.nth (0) {
        assert_eq! (entry.name, "k1".to_string ());
        assert_eq! (entry.value,
                    vec! [ "s1".to_string (), "s2".to_string () ]);
    } else {
        assert! (false);
    }
}
