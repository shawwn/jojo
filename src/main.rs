    #![feature(nll)]
    use std::collections::HashMap;
    use std::rc::Rc;
  type Ptr <T> = Rc <T>;

  type Name = String;

  type ObjRecord = Vec <(Name, Ptr <Obj>)>;
  type ObjRef = usize; // index in to ObjRecord

  type TypeRecord = Vec <(Name, Ptr <Type>)>;
  type Tag = usize; // index in to TypeRecord
  type TagMap = HashMap <Tag, Name>;

  type JoVec = Vec <Ptr <Jo>>;

  type ObjStack = Vec <Ptr <Obj>>;

  type Bind = (Name, Ptr <Obj>);
  type BindVec = Vec <Bind>; // index from end
  type LocalScope = Vec <BindVec>; // index from end

  type FrameStack = Vec <Box <Frame>>;

  type StringVec = Vec <String>;
    trait Obj {
        fn tag (&self) -> Tag;
        fn obj_map (&self) -> HashMap <Name, Ptr <Obj>>;
        fn repr (&self, env: &Env) -> String;
        fn print (&self, env: &Env);
        fn eq (&self, env: &Env, obj: Ptr <Obj>) -> bool;
        fn apply (&self, env: &Env, arity: usize);
        fn apply_to_arg_dict (&self, env: &Env);
    }
    trait Jo {
        fn exe (&self, env: &mut Env, local_scope: Ptr <LocalScope>);
        fn repr (&self, env: &Env) -> String;
    }
    struct Frame {
        index: usize,
        jojo: Ptr <JoVec>,
        local_scope: Ptr <LocalScope>,
    }
    struct Env {
        obj_record: ObjRecord,
        obj_stack: ObjStack,
        frame_stack: FrameStack,
        type_record: TypeRecord,
        tag_map: TagMap,
    }
    fn env_step (env: &mut Env) {
        if let Some (mut frame) = env.frame_stack.pop () {
            let jo = frame.jojo [frame.index] .clone ();
            frame.index += 1;
            if frame.index < frame.jojo.len () {
                let local_scope = frame.local_scope.clone ();
                env.frame_stack.push (frame);
                jo.exe (env, local_scope);
            }
            else {
                jo.exe (env, frame.local_scope);
            }
        }
    }
    fn env_run (env: &mut Env) {
        while ! env.frame_stack.is_empty () {
            env_step (env);
        }
    }
    fn env_run_with_base (env: &mut Env, base: usize) {
        while env.frame_stack.len () > base {
            env_step (env);
        }
    }
    struct Type {

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
    fn scan_word_vector (code: &str) -> StringVec {
        let mut word_vector = StringVec::new ();
        let mut iter = code.chars () .peekable ();
        while iter.peek () != None {
            if let Some (c) = iter.next () {
                if space_char_p (c) {
                }
                else if delimiter_char_p (c) {
                    let mut word = String::from ("");
                    word.push (c);
                    word_vector.push (word);
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
                            // error
                        }
                    }
                    word_vector.push (word);
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
                    word_vector.push (word);
                }
            }
        }
        word_vector
    }
    fn main() {
        println! ("{:#?}", scan_word_vector ("(a (a b c) b c)"));
    }
