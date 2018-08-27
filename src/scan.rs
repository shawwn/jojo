type StringVec = Vec <String>;

fn space_char_p (c: char) -> bool {
    (c == ' ' ||
     c == '\n' ||
     c == '\t')
}

fn delimiter_char_p (c: char) -> bool {
    (c == '(' ||
     c == ')' ||
     c == '[' ||
     c == ']' ||
     c == '{' ||
     c == '}' ||
     c == ',' ||
     c == '`' ||
     c == '\'')
}

fn semicolon_char_p (c: char) -> bool {
    (c == ';')
}

fn newline_char_p (c: char) -> bool {
    (c == '\n')
}

fn doublequote_char_p (c: char) -> bool {
    (c == '"')
}

pub fn scan_word_vec (code: &str) -> StringVec {
    let mut word_vec = StringVec::new ();
    let mut iter = code.chars () .peekable ();
    while iter.peek () != None {
        if let Some (c) = iter.next () {
            if space_char_p (c) {
            }
            else if delimiter_char_p (c) {
                let mut word = String::from ("");
                word.push (c);
                word_vec.push (word);
            }
            else if semicolon_char_p (c) {
                loop {
                    if let Some (c) = iter.next () {
                        if newline_char_p (c) {
                            break;
                        }
                    }
                    else {
                        break;
                    }
                }
            }
            else if doublequote_char_p (c) {
                let mut word = String::from ("");
                word.push (c);
                loop {
                    if let Some (c) = iter.next () {
                        word.push (c);
                        if doublequote_char_p (c) {
                            break;
                        }
                    }
                    else {
                        panic! ("doublequote mismatch!");
                    }
                }
                word_vec.push (word);
            }
            else {
                let mut word = String::from ("");
                word.push (c);
                loop {
                    if let Some (c) = iter.peek () {
                        let c = *c;
                        if (space_char_p (c) ||
                            doublequote_char_p (c) ||
                            semicolon_char_p (c) ||
                            delimiter_char_p (c))
                        {
                            break;
                        }
                        word.push (c);
                        iter.next ();
                    }
                    else {
                        iter.next ();
                        break;
                    }
                }
                word_vec.push (word);
            }
        }
    }
    word_vec
}

#[test]
fn test_scan () {
    assert_eq! (scan_word_vec (""),
                StringVec::new ());
    assert_eq! (scan_word_vec ("a b c"),
                ["a", "b", "c"]);
    assert_eq! (scan_word_vec ("(a b c)"),
                ["(", "a", "b", "c", ")"]);
    assert_eq! (scan_word_vec ("(a (b) c)"),
                ["(", "a", "(", "b", ")", "c", ")"]);
}
