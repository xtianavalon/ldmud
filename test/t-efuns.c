#include "/inc/base.inc"
#include "/inc/testarray.inc"
#include "/inc/gc.inc"
#include "/inc/deep_eq.inc"

#define TESTFILE "/log/testfile"

mapping json_testdata = ([ "test 1": 42, "test 2": 42.0,
                          "test 3": "hello world\n",
                          "test 4": ({1,2,3,4,5,42.0,"teststring"}),
                          "test 5": (["a": 26, "b": 27.0, "c": "letter",
                                      "d": (["x":1,"y":2,"z":3]),
                                      "e": ({10,11,12,13}) ])
                          ]);
string json_teststring = "{ \"test 5\": { \"d\": { \"z\": 3, \"y\": 2, \"x\": 1 }, \"e\": [ 10, 11, 12, 13 ], \"b\": 27.000000, \"a\": 26, \"c\": \"letter\" }, \"test 1\": 42, \"test 3\": \"hello world\\n\", \"test 4\": [ 1, 2, 3, 4, 5, 42.000000, \"teststring\" ], \"test 2\": 42.000000 }";

int f(int arg);

mixed *tests = ({
    // TODO: Add cases for indexing at string end ("abc"[3])
    ({ "[ 1", 0,        (: ({1,2,3,4,5})[4] == 5 :) }),
    ({ "[ 2", TF_ERROR, (: ({1,2,3,4,5})[5] :) }),
    ({ "[ 3", TF_ERROR, (: ({1,2,3,4,5})[-1] :) }),
    ({ "[ 4", TF_ERROR, (: ({})[0] :) }),
    ({ "[ 5", 0,        (: "Bla, bla, blup."[0] == 'B' :) }),
    ({ "[ 6", TF_ERROR, (: "Bla."[5] :) }),
    ({ "[ 7", TF_ERROR, (: "Bla."[-1] :) }),
    ({ "[< 1", 0,        (: ({1,2,3,4,5})[<4] == 2 :) }),
    ({ "[< 2", TF_ERROR, (: ({1,2,3,4,5})[<6] :) }),
    ({ "[< 3", TF_ERROR, (: ({1,2,3,4,5})[<-1] :) }),
    ({ "[< 4", TF_ERROR, (: ({1,2,3,4,5})[<0] :) }),
    ({ "[< 5", TF_ERROR, (: ({})[<1] :) }),
    ({ "[< 6", 0,        (: "Bla, bla, blup."[<1] == '.' :) }),
    ({ "[< 7", TF_ERROR, (: "Bla."[<-1] :) }),
    // TODO: #'[..], #'[<..], #'[..<], #'[<..<]
    ({ "abs 1", 0,        (: abs(20) == 20 :) }),
    ({ "abs 2", 0,        (: abs(-20) == 20 :) }),
    ({ "abs 3", 0,        (: abs(__INT_MAX__) == __INT_MAX__ :) }),
    ({ "abs 4", 0,        (: abs(-__INT_MAX__) == __INT_MAX__ :) }),
    ({ "abs 5", TF_ERROR, (: funcall(#'abs,"20") :) }),
    ({ "abs 6", TF_ERROR, (: funcall(#'abs, 1, 2) :) }),
    ({ "abs 7", TF_ERROR, (: funcall(#'abs) :) }),
    ({ "abs 8", 0,        (: abs(20.3) == 20.3 :) }),
    ({ "abs 9", 0,        (: abs(-20.3) == 20.3 :) }),
    ({ "abs 10", 0,        (: abs(__FLOAT_MAX__) == __FLOAT_MAX__ :) }),
    ({ "abs 11", 0,        (: abs(-__FLOAT_MAX__) == __FLOAT_MAX__ :) }),
    ({ "abs 12", TF_ERROR, (: funcall(#'abs, 1.0, 2) :) }),
    ({ "acos 1", 0,        (: acos(1.0) == 0 :) }),
    ({ "acos 2", TF_ERROR, (: funcall(#'acos, "1.0") :) }),
    ({ "acos 3", TF_ERROR, (: acos(1.1) :) }),
    ({ "acos 4", TF_ERROR, (: acos(-1.1) :) }),
    ({ "all_environment 1", 0,
	(:
	    object o = clone_object(this_object());
	    mixed res;
            set_environment(o, this_object());
	    res = all_environment(o);
	    destruct(o);
	    return sizeof(res) == 1 && res[0] == this_object();
	:)
    }),
/* Doku - Korrektur: all_environment liefert bei zerstoerten Objekten einen Fehler und keine 0
    ({ "all_environment", 0, 
	(:
	    object o = clone_object(this_object());
            mixed arr = ({o});
            set_environment(o, this_object());
	    destruct(o);
	    return !all_environment(arr[0]);
	:)
    }),
*/
    ({ "all_environment 2", 0, (: all_environment(this_object()) == 0 :) }),
    ({ "all_inventory 1", 0,
	(:
	    mixed res = all_inventory(this_object());
	    return pointerp(res) && !sizeof(res);
	:)
    }),
    ({ "all_inventory 2", 0,
	(:
	    object o = clone_object(this_object());
	    mixed inv, res;

            set_environment(o, this_object());
	    inv = all_inventory(this_object());
	    res = sizeof(inv)==1 && clonep(inv[0]);
	    if(res)
		destruct(inv[0]);
	    return res;
	:)
    }),
    ({ "all_inventory 3", 0, 
	(:
	    object o = clone_object(this_object());
	    set_environment(o, this_object());
	    destruct(o);
	    return sizeof(all_inventory(this_object())) == 0;
	:)
    }),
    ({ "asin 1", 0,        (: asin(0.0) == 0 :) }),
    ({ "asin 2", TF_ERROR, (: funcall(#'asin,"1.0") :) }),
    ({ "asin 3", TF_ERROR, (: asin(1.1) :) }),
    ({ "asin 4", TF_ERROR, (: asin(-1.1) :) }),
    ({ "save_object 1", 0, (: stringp(save_object()) :) }), /* Bug #594 */
    ({ "strstr 01", 0, (: strstr("","") == 0 :) }), /* Bug #536 */
    ({ "strstr 02", 0, (: strstr("","", 1) == -1 :) }),
    ({ "strstr 03", 0, (: strstr("abc","") == 0 :) }),
    ({ "strstr 04", 0, (: strstr("abc","", 3) == 3 :) }),
    ({ "strstr 05", 0, (: strstr("abc","", 4) == -1 :) }),
    ({ "strstr 06", 0, (: strstr("abcdefa","a") == 0 :) }),
    ({ "strstr 07", 0, (: strstr("abcdefa","a", 1) == 6 :) }),
    ({ "strstr 08", 0, (: strstr("abcdefa","a", 6) == 6 :) }),
    ({ "strstr 09", 0, (: strstr("abcdefa","a", 7) == -1 :) }),
    ({ "strstr 10", 0, (: strstr("abcdefabc","a", 7) == -1 :) }),
    ({ "strstr 11", 0, (: strstr("abcdefabc","c") == 2 :) }),
    ({ "write_file 1", 0, 
        (:
            int res;
            rm(TESTFILE);
            write_file(TESTFILE, "abc");
            res = read_file(TESTFILE)=="abc";
            rm(TESTFILE);
            return res;
        :)
    }),
    ({ "write_file 2", 0,
        (:
            int res;
            rm(TESTFILE);
            write_file(TESTFILE, "abc");
            write_file(TESTFILE, "abc");
            res = read_file(TESTFILE)=="abcabc";
            rm(TESTFILE);
            return res;
        :)
    }),
    ({ "write_file 3", 0,
        (:
            int res;
            write_file(TESTFILE, "abc");
            write_file(TESTFILE, "abc", 1);
            res = read_file(TESTFILE)=="abc";
            rm(TESTFILE);
            return res;
        :)
    }),
    ({ "write_file 4", 0, /* Bug #512 */
        (:
            int res;
            rm(TESTFILE);
            write_file(TESTFILE, "abc", 1);
            res = read_file(TESTFILE)=="abc";
            rm(TESTFILE);
            return res;
        :)
    }),
    ({ "map string 1", 0, (: map("abc", (['a':'x'])) == "xbc" :) }),
    ({ "map string 2", 0, (: map("abc", (['a':'x';'y']), 1) == "ybc" :) }),
    ({ "map string 3", TF_ERROR, (: map("abc", (['a']), 0) == "abc" :) }),
    ({ "map string 4", 0, (: map("abc", #'+, 1) == "bcd" :) }),
    ({ "map string 5", 0, (: map("abc", "f") == "bcd" :) }),
    ({ "map string x", TF_ERROR, (: map("abc", (['a':'x']), 2) :) }),
    ({ "map array 1", 0, (: deep_eq(map(({1,2,3}), ([1:5])), ({5,2,3})) :) }),
    ({ "map array 2", 0, (: deep_eq(map(({1,2,3}), ([1:5;6]), 1),({6,2,3})) :) }),
    ({ "map array 3", TF_ERROR, (: map(({1,2,3}), ([1]), 0) :) }),
    ({ "map array 4", 0, (: deep_eq(map(({1,2,3}), #'+, 1), ({2,3,4})) :) }),
    ({ "map array 5", 0, (: deep_eq(map(({1,2,3}), "f"), ({2,3,4})) :) }),
    ({ "map mapping 1", TF_ERROR, (: deep_eq(map(([1,2,3]), ([1:5])), ({5,2,3})) :) }),
    ({ "map mapping 2", TF_ERROR, (: deep_eq(map(([1,2,3]), ([1:5;6]), 1),({6,2,3})) :) }),
    ({ "map mapping 3", TF_ERROR, (: map(([1,2,3]), ([1]), 0) :) }),
    ({ "map mapping 4", 0, (: deep_eq(map(([1,2,3]), (: $1 + $3 :), 1), ([1:2,2:3,3:4])) :) }),
    ({ "map mapping 5", 0, (: deep_eq(map(([1,2,3]), "f"), ([1:2,2:3,3:4])) :) }),
    ({ "explode 1", 0, (: deep_eq(explode("",""), ({""})) :) }),
    ({ "explode 2", 0, (: deep_eq(explode("","anything"), ({""})) :) }),
    ({ "explode 3", 0, (: deep_eq(explode("abc",""), ({"a","b","c"})) :) }),
    ({ "explode 4", 0, (: deep_eq(explode("abc","xyz"), ({"abc"})) :) }),
    ({ "explode 5", 0, (: deep_eq(explode("abc","abc"), ({"",""})) :) }),
    ({ "explode 6", 0, (: deep_eq(explode(" ab cd ef ", " "), 
                                  ({ "", "ab", "cd", "ef", "" })) :) }),
    ({ "implode 1", 0, (: implode(({ "foo", "bar", "" }), "*") == "foo*bar*":) }),
    ({ "implode 2", 0, (: implode(({ "a", 2, this_object(), "c" }), "b") == "abc" :) }),
    ({ "implode 3", 0, (: implode(({ "", "" }), "") == "":) }),
    ({ "save_/restore_value 1", 0, 
        (: restore_value(save_value(__FLOAT_MAX__)) == __FLOAT_MAX__ :) }),
    ({ "save_/restore_value 2", 0, 
        (: restore_value(save_value(__FLOAT_MIN__)) == __FLOAT_MIN__ :) }),
    ({ "sort_array 1", 0, (: deep_eq(sort_array(({4,5,2,6,1,3,0}),#'>),
                                     ({0,1,2,3,4,5,6})) :) }),
    ({ "sort_array 2", 0, // sort in-place
        (:
           int *a = ({4,5,2,6,1,3,0});
           int *b = a;
           sort_array(&a, #'>);
           return deep_eq(a, b) && deep_eq(a, ({0,1,2,3,4,5,6}) );
        :)
    }),
#ifdef __JSON__
    ({ "json_parse/_serialize 1", 0,
        (: json_parse(json_serialize(1) ) == 1:) }),
    ({ "json_parse/_serialize 2", 0,
        (: json_parse(json_serialize(42.0) ) == 42.0:) }),
    ({ "json_parse/_serialize 3", 0,
        (: json_parse(json_serialize("hello world\n") ) == "hello world\n":) }),
    ({ "json_parse/_serialize 4", 0,
        (: json_parse(json_serialize( ({}) ) ) == ({}) :) }),
    ({ "json_parse/_serialize 5", 0,
        (: deep_eq(json_parse(json_serialize( ({1,2,3,4,5,6}) ) ), ({1,2,3,4,5,6}) ) :) }),
    ({ "json_parse/_serialize 6", 0,
        (: deep_eq(json_parse(json_serialize( ([]) ) ), ([]) ) :) }),
    ({ "json_parse/_serialize 7", 0,
        (: deep_eq(json_parse(json_serialize( json_testdata ) ), json_testdata) :) }),
    ({ "json_parse/_serialize 8", 0,
        (: json_parse(json_serialize(__FLOAT_MAX__) ) == __FLOAT_MAX__:) }),
//    ({ "json_parse/_serialize 9", 0,
//        (: json_parse(json_serialize(__FLOAT_MIN__) ) == __FLOAT_MIN__ :) }),
#if __INT_MAX__ <= 2147483647
    // 64 bit values work on only few systems with a very new json-c library.
    ({ "json_parse/_serialize 10", 0,
        (: json_parse(json_serialize(__INT_MAX__) ) == __INT_MAX__:) }),
    ({ "json_parse/_serialize 11", 0,
        (: json_parse(json_serialize(__INT_MIN__) ) == __INT_MIN__ :) }),
#endif
    ({ "json_parse 1", 0,
        (: json_parse("true") == 1 :) }),
    ({ "json_parse 2", 0,
        (: json_parse("false") == 0 :) }),
    ({ "json_parse 3", 0,
        (: deep_eq(json_parse(json_teststring), json_testdata) :) }),
    ({ "json_serialize + json_parse 1", 0,
        (: deep_eq(json_parse(json_serialize(json_testdata)), json_testdata) 
         :) }),
#endif // __JSON__
});

void run_test()
{
    msg("\nRunning test suite for efuns:\n"
          "-----------------------------\n");

    run_array(tests,
        (:
            if($1)
                shutdown(1);
            else
                start_gc(#'shutdown);

            return 0;
        :));
}

string *epilog(int eflag)
{
    run_test();
    return 0;
}

int f(int arg)
{
    return arg + 1;
}
