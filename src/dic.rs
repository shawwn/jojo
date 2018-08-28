use std::collections::HashMap;

pub struct Entry <T> {
    name: String,
    value: Option <T>,
}

pub struct Dic <T> {
    index_map: HashMap <String, usize>,
    entry_vector: Vec <Entry <T>>,
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

    fn lack (&self) -> usize {
        let mut n = 0;
        for entry in &self.entry_vector {
            if let None = &entry.value {
                n += 1;
            }
        }
        n
    }

    fn nth (&self, index: usize) -> &Entry <T> {
        &self.entry_vector [index]
    }

    fn set_nth (&mut self, index: usize, value: Option <T>) {
        self.entry_vector [index] .value = value;
    }

    fn has_name (&self, name: &str) -> bool {
        self.index_map.contains_key (name)
    }

    fn ins (&mut self, name: &str, value: Option <T>) -> usize {
        assert! (! self.has_name (name));
        let index = self.len ();
        let entry = Entry { name: name.to_string (), value };
        self.entry_vector.push (entry);
        self.index_map.insert (name.to_string (), index);
        index
    }

    fn set (&mut self, name: &str, value: Option <T>) {
        if let Some (index) = self.index_map.get (name) {
            self.set_nth (*index, value);
        } else {
            eprintln! ("- Dic::set");
            eprintln! ("  dic does not have name : {}", name);
            panic! ("Dic::set fail!");
        }
    }

    fn get (&self, name: &str) -> Option <&T> {
        if let Some (index) = self.index_map.get (name) {
            let entry = self.nth (*index);
            if let Some (value) = &entry.value {
                Some (value)
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
    let mut dic: Dic <Vec <isize>> = Dic::new ();
    assert_eq! (0, dic.len ());

    let index = dic.ins ("key1", Some (vec! [ 0, 1, 2, 3 ]));
    assert_eq! (0, index);
    assert_eq! (1, dic.len ());
    assert! (dic.has_name ("key1"));
    assert! (! dic.has_name ("non-key"));
    let entry = dic.nth (0);
    assert_eq! (entry.name, "key1");
    assert_eq! (entry.value, Some (vec! [ 0, 1, 2, 3 ]));

    let index = dic.ins ("key2", Some (vec! [ 4, 5, 6, 7 ]));
    assert_eq! (1, index);
    assert_eq! (2, dic.len ());
    assert! (dic.has_name ("key2"));
    let entry = dic.nth (1);
    assert_eq! (entry.name, "key2");
    assert_eq! (entry.value, Some (vec! [ 4, 5, 6, 7 ]));

    let value = dic.get ("key1");
    assert_eq! (value, Some (&vec! [ 0, 1, 2, 3 ]));

    let value = dic.get ("key2");
    assert_eq! (value, Some (&vec! [ 4, 5, 6, 7 ]));

    dic.set ("key1", Some (vec! [ 4, 5, 6, 7 ]));
    let value = dic.get ("key1");
    assert_eq! (value, Some (&vec! [ 4, 5, 6, 7 ]));

    assert_eq! (2, dic.len ());
    assert_eq! (0, dic.lack ());

    dic.set ("key2", None);
    let value = dic.get ("key2");
    assert_eq! (value, None);

    assert_eq! (2, dic.len ());
    assert_eq! (1, dic.lack ());
}
