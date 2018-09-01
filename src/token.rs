  use std::str::Chars;
  use std::iter::Peekable;
    pub enum Token {
        List (Vec <Token>),
        Vect (Vec <Token>),
        Dict (Vec <Token>),
        QuotationMark (String, String, Box <Token>),
        Num (f64),
        Str (String),
    }

    pub type TokenVec = Vec <Token>;
    type Word = String;
    type WordVec = Vec <Word>;
    pub fn scan (code: &str) -> TokenVec {
        let word_vec = split_to_word_vec (code);
        let mut token_vec = TokenVec::new ();
        let mut iter = word_vec .iter () .peekable ();
        while iter.peek () != None {
        }
        token_vec
    }
    fn split_to_word_vec (code: &str) -> WordVec {
        let mut word_vec = WordVec::new ();
        let mut iter = code.chars () .peekable ();
        while iter.peek () != None {
            if let Some (c) = iter.peek () {
                let c = *c;
                if space_char_p (c) {
                    iter.next ();
                }
                else if delimiter_char_p (c) {
                    word_vec.push (collect_delimiter (&mut iter));
                }
                else if semicolon_char_p (c) {
                    ignore_comment (&mut iter);
                }
                else if doublequote_char_p (c) {
                    word_vec.push (collect_doublequote (&mut iter));
                }
                else {
                    word_vec.push (collect_word (&mut iter));
                }
            }
        }
        word_vec
    }
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
    fn collect_delimiter (iter: &mut Peekable <Chars>) -> Word {
        let mut word = String::from ("");
        let c = iter.next () .unwrap ();
        word.push (c);
        word
    }
    fn ignore_comment (iter: &mut Peekable <Chars>) {
        iter.next ();
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
    fn collect_doublequote (iter: &mut Peekable <Chars>) -> Word {
        let mut word = String::from ("");
        let c = iter.next () .unwrap ();
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
        word
    }
    fn collect_word (iter: &mut Peekable <Chars>) -> Word {
        let mut word = String::from ("");
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
        word
    }
    #[test]
    fn test_split_to_word_vec () {
        assert_eq! (split_to_word_vec (""),
                    WordVec::new ());
        assert_eq! (split_to_word_vec ("a b c"),
                    ["a", "b", "c"]);
        assert_eq! (split_to_word_vec ("(a b c)"),
                    ["(", "a", "b", "c", ")"]);
        assert_eq! (split_to_word_vec ("(a (b) c)"),
                    ["(", "a", "(", "b", ")", "c", ")"]);
        assert_eq! (split_to_word_vec ("(\"a\" (b) c)"),
                    ["(", "\"a\"", "(", "b", ")", "c", ")"]);
        assert_eq! (split_to_word_vec ("(\"a\" (b) c) ;;;; 123"),
                    ["(", "\"a\"", "(", "b", ")", "c", ")"]);
    }
