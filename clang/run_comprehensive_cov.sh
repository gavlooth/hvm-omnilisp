#!/bin/bash
# Comprehensive coverage test script

cd "$(dirname "$0")"
rm -f *.gcda

echo "Running comprehensive coverage tests..."

# ============================================================================
# BASIC OPERATIONS
# ============================================================================

# Arithmetic - all operators
./main-cov -e "(+ 1 2)" > /dev/null 2>&1
./main-cov -e "(- 10 3)" > /dev/null 2>&1
./main-cov -e "(* 4 5)" > /dev/null 2>&1
./main-cov -e "(/ 20 4)" > /dev/null 2>&1
./main-cov -e "(mod 10 3)" > /dev/null 2>&1
./main-cov -e "(+ 1.5 2.5)" > /dev/null 2>&1
./main-cov -e "(* 3.14 2)" > /dev/null 2>&1

# Comparison operators
./main-cov -e "(= 1 1)" > /dev/null 2>&1
./main-cov -e "(= 1 2)" > /dev/null 2>&1
./main-cov -e "(< 1 2)" > /dev/null 2>&1
./main-cov -e "(> 2 1)" > /dev/null 2>&1
./main-cov -e "(<= 1 1)" > /dev/null 2>&1
./main-cov -e "(>= 2 2)" > /dev/null 2>&1
./main-cov -e "(!= 1 2)" > /dev/null 2>&1

# Boolean operators
./main-cov -e "(and true false)" > /dev/null 2>&1
./main-cov -e "(and true true)" > /dev/null 2>&1
./main-cov -e "(or true false)" > /dev/null 2>&1
./main-cov -e "(or false false)" > /dev/null 2>&1
./main-cov -e "(not true)" > /dev/null 2>&1
./main-cov -e "(not false)" > /dev/null 2>&1

# Bitwise operators
./main-cov -e "(bit-and 5 3)" > /dev/null 2>&1
./main-cov -e "(bit-or 5 3)" > /dev/null 2>&1
./main-cov -e "(bit-xor 5 3)" > /dev/null 2>&1
./main-cov -e "(bit-not 5)" > /dev/null 2>&1
./main-cov -e "(bit-shift-left 1 4)" > /dev/null 2>&1
./main-cov -e "(bit-shift-right 16 2)" > /dev/null 2>&1

# ============================================================================
# CONDITIONALS
# ============================================================================

./main-cov -e "(if true 1 2)" > /dev/null 2>&1
./main-cov -e "(if false 1 2)" > /dev/null 2>&1
./main-cov -e "(cond (true 1) (false 2))" > /dev/null 2>&1
./main-cov -e "(cond (false 1) (true 2))" > /dev/null 2>&1
./main-cov -e "(when true 42)" > /dev/null 2>&1
./main-cov -e "(when false 42)" > /dev/null 2>&1
./main-cov -e "(unless false 42)" > /dev/null 2>&1
./main-cov -e "(unless true 42)" > /dev/null 2>&1
./main-cov -e "(case 1 1 :one 2 :two _ :other)" > /dev/null 2>&1
./main-cov -e "(case 3 1 :one 2 :two _ :other)" > /dev/null 2>&1

# ============================================================================
# BINDINGS
# ============================================================================

./main-cov -e "(let [x 1] x)" > /dev/null 2>&1
./main-cov -e "(let [x 1 y 2] (+ x y))" > /dev/null 2>&1
./main-cov -e "(let [x 1 y 2 z 3] (+ x (+ y z)))" > /dev/null 2>&1
./main-cov -e "(letrec [f (lambda [n] (if (= n 0) 1 (* n (f (- n 1)))))] (f 5))" > /dev/null 2>&1
./main-cov -e "(let* [x 1 y (+ x 1)] y)" > /dev/null 2>&1

# ============================================================================
# LAMBDAS AND FUNCTIONS
# ============================================================================

./main-cov -e "(lambda [x] x)" > /dev/null 2>&1
./main-cov -e "(lambda [x] (+ x 1))" > /dev/null 2>&1
./main-cov -e "(lambda [x y] (+ x y))" > /dev/null 2>&1
./main-cov -e "((lambda [x] x) 42)" > /dev/null 2>&1
./main-cov -e "((lambda [x] (* x x)) 5)" > /dev/null 2>&1
./main-cov -e "((lambda [x y] (+ x y)) 3 4)" > /dev/null 2>&1
./main-cov -e "((lambda [x y z] (+ x (+ y z))) 1 2 3)" > /dev/null 2>&1
./main-cov -e "((\\ [x] (+ x 1)) 5)" > /dev/null 2>&1

# Define
./main-cov -e "(define f [x] (+ x 1))" > /dev/null 2>&1
./main-cov -e "(do (define f [x] (+ x 1)) (f 5))" > /dev/null 2>&1
./main-cov -e "(do (define add [x] [y] (+ x y)) ((add 3) 4))" > /dev/null 2>&1
./main-cov -e "(do (define id [x] x) (id 42))" > /dev/null 2>&1

# ============================================================================
# LISTS
# ============================================================================

./main-cov -e "'()" > /dev/null 2>&1
./main-cov -e "'(1)" > /dev/null 2>&1
./main-cov -e "'(1 2 3)" > /dev/null 2>&1
./main-cov -e "'(1 2 3 4 5)" > /dev/null 2>&1
./main-cov -e "(head '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(tail '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(cons 0 '(1 2))" > /dev/null 2>&1
./main-cov -e "(cons 1 '())" > /dev/null 2>&1
./main-cov -e "(empty? '())" > /dev/null 2>&1
./main-cov -e "(empty? '(1))" > /dev/null 2>&1
./main-cov -e "(length '())" > /dev/null 2>&1
./main-cov -e "(length '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(append '(1 2) '(3 4))" > /dev/null 2>&1
./main-cov -e "(reverse '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(nth '(1 2 3) 0)" > /dev/null 2>&1
./main-cov -e "(nth '(1 2 3) 2)" > /dev/null 2>&1
./main-cov -e "(member 2 '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(member 5 '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(list 1 2 3)" > /dev/null 2>&1
./main-cov -e "(flatten '((1 2) (3 4)))" > /dev/null 2>&1

# ============================================================================
# ARRAYS
# ============================================================================

./main-cov -e "[]" > /dev/null 2>&1
./main-cov -e "[1]" > /dev/null 2>&1
./main-cov -e "[1 2 3]" > /dev/null 2>&1
./main-cov -e "[1 2 3 4 5]" > /dev/null 2>&1
./main-cov -e "(get [1 2 3] 0)" > /dev/null 2>&1
./main-cov -e "(get [1 2 3] 2)" > /dev/null 2>&1
./main-cov -e "(length [1 2 3])" > /dev/null 2>&1
./main-cov -e "(set [1 2 3] 1 42)" > /dev/null 2>&1
./main-cov -e "(slice [1 2 3 4 5] 1 3)" > /dev/null 2>&1
./main-cov -e "(slice [1 2 3 4 5] 0 5)" > /dev/null 2>&1
./main-cov -e "(array? [1 2 3])" > /dev/null 2>&1
./main-cov -e "(array? '(1 2 3))" > /dev/null 2>&1

# ============================================================================
# DICTS
# ============================================================================

./main-cov -e "#{}" > /dev/null 2>&1
./main-cov -e "#{\"a\" 1}" > /dev/null 2>&1
./main-cov -e "#{\"a\" 1 \"b\" 2}" > /dev/null 2>&1
./main-cov -e "(dict-get #{\"a\" 1} \"a\")" > /dev/null 2>&1
./main-cov -e "(dict-set #{\"a\" 1} \"b\" 2)" > /dev/null 2>&1
./main-cov -e "(dict-keys #{\"a\" 1 \"b\" 2})" > /dev/null 2>&1
./main-cov -e "(dict-vals #{\"a\" 1 \"b\" 2})" > /dev/null 2>&1
./main-cov -e "(dict-remove #{\"a\" 1 \"b\" 2} \"a\")" > /dev/null 2>&1
./main-cov -e "(dict-merge #{\"a\" 1} #{\"b\" 2})" > /dev/null 2>&1

# ============================================================================
# STRINGS
# ============================================================================

./main-cov -e "\"\"" > /dev/null 2>&1
./main-cov -e "\"hello\"" > /dev/null 2>&1
./main-cov -e "\"hello world\"" > /dev/null 2>&1
./main-cov -e "(str-concat \"hello\" \" \" \"world\")" > /dev/null 2>&1
./main-cov -e "(str-length \"hello\")" > /dev/null 2>&1
./main-cov -e "(str-slice \"hello\" 1 3)" > /dev/null 2>&1
./main-cov -e "(str-get \"hello\" 0)" > /dev/null 2>&1
./main-cov -e "(str-get \"hello\" 4)" > /dev/null 2>&1
./main-cov -e "(str-split \"a,b,c\" \",\")" > /dev/null 2>&1
./main-cov -e "(str-join \",\" '(\"a\" \"b\" \"c\"))" > /dev/null 2>&1
./main-cov -e "(str-replace \"hello\" \"l\" \"x\")" > /dev/null 2>&1
./main-cov -e "(str-upper \"hello\")" > /dev/null 2>&1
./main-cov -e "(str-lower \"HELLO\")" > /dev/null 2>&1
./main-cov -e "(str-trim \"  hello  \")" > /dev/null 2>&1
./main-cov -e "(str-starts-with \"hello\" \"he\")" > /dev/null 2>&1
./main-cov -e "(str-ends-with \"hello\" \"lo\")" > /dev/null 2>&1
./main-cov -e "(str-contains \"hello\" \"ll\")" > /dev/null 2>&1
./main-cov -e "(str-reverse \"hello\")" > /dev/null 2>&1
./main-cov -e "(string? \"hello\")" > /dev/null 2>&1

# Escape sequences
./main-cov -e "\"hello\\nworld\"" > /dev/null 2>&1
./main-cov -e "\"tab\\there\"" > /dev/null 2>&1
./main-cov -e "\"quote\\\"test\"" > /dev/null 2>&1

# ============================================================================
# CHARACTERS
# ============================================================================

./main-cov -e "\\a" > /dev/null 2>&1
./main-cov -e "\\z" > /dev/null 2>&1
./main-cov -e "\\0" > /dev/null 2>&1
./main-cov -e "\\newline" > /dev/null 2>&1
./main-cov -e "\\space" > /dev/null 2>&1
./main-cov -e "\\tab" > /dev/null 2>&1
./main-cov -e "(char? \\a)" > /dev/null 2>&1
./main-cov -e "(char->int \\a)" > /dev/null 2>&1
./main-cov -e "(int->char 97)" > /dev/null 2>&1

# ============================================================================
# SYMBOLS AND QUOTING
# ============================================================================

./main-cov -e "'foo" > /dev/null 2>&1
./main-cov -e "'bar" > /dev/null 2>&1
./main-cov -e "':keyword" > /dev/null 2>&1
./main-cov -e ":keyword" > /dev/null 2>&1
./main-cov -e "(symbol? 'foo)" > /dev/null 2>&1
./main-cov -e "(quote (1 2 3))" > /dev/null 2>&1
./main-cov -e "\`(1 2 3)" > /dev/null 2>&1
./main-cov -e "(let [x 5] \`(1 ,x 3))" > /dev/null 2>&1
./main-cov -e "(let [x '(2 3)] \`(1 ,@x 4))" > /dev/null 2>&1

# ============================================================================
# HIGHER-ORDER FUNCTIONS
# ============================================================================

./main-cov -e "(map (lambda [x] (* x 2)) '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(map (lambda [x] (+ x 1)) '())" > /dev/null 2>&1
./main-cov -e "(filter (lambda [x] (> x 2)) '(1 2 3 4))" > /dev/null 2>&1
./main-cov -e "(filter (lambda [x] (< x 0)) '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(foldl + 0 '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(foldl + 0 '())" > /dev/null 2>&1
./main-cov -e "(foldr cons '() '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(reduce + '(1 2 3 4))" > /dev/null 2>&1
./main-cov -e "(for-each (lambda [x] x) '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(every (lambda [x] (> x 0)) '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(every (lambda [x] (> x 0)) '(1 -2 3))" > /dev/null 2>&1
./main-cov -e "(some (lambda [x] (= x 2)) '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(some (lambda [x] (= x 5)) '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(take 2 '(1 2 3 4 5))" > /dev/null 2>&1
./main-cov -e "(drop 2 '(1 2 3 4 5))" > /dev/null 2>&1
./main-cov -e "(take-while (lambda [x] (< x 3)) '(1 2 3 4 5))" > /dev/null 2>&1
./main-cov -e "(drop-while (lambda [x] (< x 3)) '(1 2 3 4 5))" > /dev/null 2>&1
./main-cov -e "(zip '(1 2 3) '(a b c))" > /dev/null 2>&1
./main-cov -e "(partition (lambda [x] (> x 2)) '(1 2 3 4))" > /dev/null 2>&1
./main-cov -e "(group-by (lambda [x] (mod x 2)) '(1 2 3 4 5))" > /dev/null 2>&1
./main-cov -e "(find (lambda [x] (> x 2)) '(1 2 3 4))" > /dev/null 2>&1
./main-cov -e "(count (lambda [x] (> x 2)) '(1 2 3 4 5))" > /dev/null 2>&1
./main-cov -e "(sort '(3 1 4 1 5))" > /dev/null 2>&1
./main-cov -e "(sort-by (lambda [x] (- 0 x)) '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(unique '(1 2 2 3 3 3))" > /dev/null 2>&1
./main-cov -e "(concat '(1 2) '(3 4) '(5 6))" > /dev/null 2>&1

# ============================================================================
# MATH FUNCTIONS
# ============================================================================

./main-cov -e "(abs -5)" > /dev/null 2>&1
./main-cov -e "(abs 5)" > /dev/null 2>&1
./main-cov -e "(min 3 7)" > /dev/null 2>&1
./main-cov -e "(max 3 7)" > /dev/null 2>&1
./main-cov -e "(floor 3.7)" > /dev/null 2>&1
./main-cov -e "(ceil 3.2)" > /dev/null 2>&1
./main-cov -e "(round 3.5)" > /dev/null 2>&1
./main-cov -e "(sqrt 16)" > /dev/null 2>&1
./main-cov -e "(pow 2 10)" > /dev/null 2>&1
./main-cov -e "(gcd 12 18)" > /dev/null 2>&1
./main-cov -e "(lcm 4 6)" > /dev/null 2>&1
./main-cov -e "(sum '(1 2 3 4 5))" > /dev/null 2>&1
./main-cov -e "(product '(1 2 3 4))" > /dev/null 2>&1
./main-cov -e "(range 5)" > /dev/null 2>&1
./main-cov -e "(range 1 5)" > /dev/null 2>&1
./main-cov -e "(range 0 10 2)" > /dev/null 2>&1

# ============================================================================
# TYPE PREDICATES
# ============================================================================

./main-cov -e "(int? 5)" > /dev/null 2>&1
./main-cov -e "(int? 3.14)" > /dev/null 2>&1
./main-cov -e "(float? 3.14)" > /dev/null 2>&1
./main-cov -e "(float? 5)" > /dev/null 2>&1
./main-cov -e "(number? 5)" > /dev/null 2>&1
./main-cov -e "(number? 3.14)" > /dev/null 2>&1
./main-cov -e "(string? \"hello\")" > /dev/null 2>&1
./main-cov -e "(string? 5)" > /dev/null 2>&1
./main-cov -e "(list? '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(list? [1 2 3])" > /dev/null 2>&1
./main-cov -e "(fn? (lambda [x] x))" > /dev/null 2>&1
./main-cov -e "(fn? 5)" > /dev/null 2>&1
./main-cov -e "(bool? true)" > /dev/null 2>&1
./main-cov -e "(bool? 1)" > /dev/null 2>&1
./main-cov -e "(nothing? nothing)" > /dev/null 2>&1
./main-cov -e "(nothing? 1)" > /dev/null 2>&1

# ============================================================================
# PATTERN MATCHING
# ============================================================================

./main-cov -e "(match 1 1 :one 2 :two _ :other)" > /dev/null 2>&1
./main-cov -e "(match 2 1 :one 2 :two _ :other)" > /dev/null 2>&1
./main-cov -e "(match 3 1 :one 2 :two _ :other)" > /dev/null 2>&1
./main-cov -e "(match '(1 2 3) (h .. t) h _ 0)" > /dev/null 2>&1
./main-cov -e "(match '() (h .. t) h _ :empty)" > /dev/null 2>&1
./main-cov -e "(match [1 2 3] [a b c] (+ a (+ b c)) _ 0)" > /dev/null 2>&1
./main-cov -e "(match '(1 2) (a b) (+ a b) _ 0)" > /dev/null 2>&1
./main-cov -e "(match 5 x & (> x 3) :big _ :small)" > /dev/null 2>&1
./main-cov -e "(match 2 x & (> x 3) :big _ :small)" > /dev/null 2>&1

# ============================================================================
# DO BLOCKS
# ============================================================================

./main-cov -e "(do)" > /dev/null 2>&1
./main-cov -e "(do 1)" > /dev/null 2>&1
./main-cov -e "(do 1 2)" > /dev/null 2>&1
./main-cov -e "(do 1 2 3)" > /dev/null 2>&1
./main-cov -e "(do (+ 1 2) (* 3 4))" > /dev/null 2>&1

# ============================================================================
# EFFECTS
# ============================================================================

./main-cov -e "(handle 42 (fail [x resume] 0))" > /dev/null 2>&1
./main-cov -e "(handle (+ 1 2) (fail [x resume] 0))" > /dev/null 2>&1
./main-cov -e "(handle (perform fail :error) (fail [x resume] :caught))" > /dev/null 2>&1
./main-cov -e "(handle (+ 1 (perform get)) (get [x resume] (resume 10)))" > /dev/null 2>&1

# ============================================================================
# CONVERSIONS
# ============================================================================

./main-cov -e "(str->int \"42\")" > /dev/null 2>&1
./main-cov -e "(str->float \"3.14\")" > /dev/null 2>&1
./main-cov -e "(int->str 42)" > /dev/null 2>&1
./main-cov -e "(float->str 3.14)" > /dev/null 2>&1
./main-cov -e "(list->array '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(array->list [1 2 3])" > /dev/null 2>&1

# ============================================================================
# COMMAND LINE OPTIONS
# ============================================================================

# Help
./main-cov -h > /dev/null 2>&1 || true

# Version
./main-cov -v > /dev/null 2>&1 || true

# Parse only mode
./main-cov -p -e "(+ 1 2)" > /dev/null 2>&1
./main-cov -p -e "(lambda [x y] (+ x y))" > /dev/null 2>&1
./main-cov -p -e "(if true 1 2)" > /dev/null 2>&1
./main-cov -p -e "(match x 1 :one _ :other)" > /dev/null 2>&1
./main-cov -p -e "(define f [x] (+ x 1))" > /dev/null 2>&1

# Compile mode
./main-cov -c -e "(+ 1 2)" > /dev/null 2>&1 || true
./main-cov -c -e "(lambda [x] (+ x 1))" > /dev/null 2>&1 || true
./main-cov -c -e "(let [x 1] x)" > /dev/null 2>&1 || true
./main-cov -c -e "(if true 1 2)" > /dev/null 2>&1 || true
./main-cov -c -e "(match x 1 :one _ :other)" > /dev/null 2>&1 || true
./main-cov -c -e "(define f [x] (+ x 1))" > /dev/null 2>&1 || true
./main-cov -c -e "(do (define f [x] (+ x 1)) (f 5))" > /dev/null 2>&1 || true

# Compile to file
./main-cov -c -o /tmp/test_output.hvm4 -e "(+ 1 2)" > /dev/null 2>&1 || true
rm -f /tmp/test_output.hvm4

# Debug mode
./main-cov -d -e "(+ 1 2)" > /dev/null 2>&1

# Stats mode
./main-cov -s -e "(+ 1 2)" > /dev/null 2>&1

# Collapse mode
./main-cov -C 5 -e "(+ 1 2)" > /dev/null 2>&1
./main-cov -C 20 -e "(+ 1 2)" > /dev/null 2>&1

# ============================================================================
# FILE INPUT
# ============================================================================

# Create test file
cat > /tmp/test_file.omni << 'EOF'
(+ 1 2)
EOF
./main-cov /tmp/test_file.omni > /dev/null 2>&1
./main-cov -p /tmp/test_file.omni > /dev/null 2>&1
./main-cov -c /tmp/test_file.omni > /dev/null 2>&1 || true
rm -f /tmp/test_file.omni

# Test multi-expression file
cat > /tmp/test_multi.omni << 'EOF'
(define f [x] (+ x 1))
(define g [y] (* y 2))
(+ (f 5) (g 3))
EOF
./main-cov /tmp/test_multi.omni > /dev/null 2>&1
rm -f /tmp/test_multi.omni

# ============================================================================
# ERROR CASES (should not crash)
# ============================================================================

./main-cov -e "(+ 1)" > /dev/null 2>&1 || true
./main-cov -e "(if true)" > /dev/null 2>&1 || true
./main-cov -e "(" > /dev/null 2>&1 || true
./main-cov -e ")" > /dev/null 2>&1 || true
./main-cov -e "(unknown-function 1 2)" > /dev/null 2>&1 || true
./main-cov nonexistent_file.omni > /dev/null 2>&1 || true

# ============================================================================
# NESTED EXPRESSIONS
# ============================================================================

./main-cov -e "(+ 1 (+ 2 (+ 3 4)))" > /dev/null 2>&1
./main-cov -e "(* (+ 1 2) (- 5 3))" > /dev/null 2>&1
./main-cov -e "(if (> 3 2) (+ 1 1) (- 1 1))" > /dev/null 2>&1
./main-cov -e "(let [x (+ 1 2)] (let [y (* x 2)] y))" > /dev/null 2>&1
./main-cov -e "((lambda [f] (f 5)) (lambda [x] (* x x)))" > /dev/null 2>&1

# ============================================================================
# RECURSION
# ============================================================================

./main-cov -e "(letrec [fact (lambda [n] (if (= n 0) 1 (* n (fact (- n 1)))))] (fact 5))" > /dev/null 2>&1
./main-cov -e "(letrec [fib (lambda [n] (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))] (fib 10))" > /dev/null 2>&1

# ============================================================================
# ASSOC / DISSOC / GET-IN / UPDATE-IN
# ============================================================================

./main-cov -e "(assoc #{\"a\" 1} \"b\" 2)" > /dev/null 2>&1
./main-cov -e "(dissoc #{\"a\" 1 \"b\" 2} \"a\")" > /dev/null 2>&1
./main-cov -e "(get-in #{\"a\" #{\"b\" 1}} '(\"a\" \"b\"))" > /dev/null 2>&1
./main-cov -e "(update #{\"a\" 1} \"a\" (lambda [x] (+ x 1)))" > /dev/null 2>&1
./main-cov -e "(assoc-in #{\"a\" #{\"b\" 1}} '(\"a\" \"c\") 2)" > /dev/null 2>&1
./main-cov -e "(update-in #{\"a\" #{\"b\" 1}} '(\"a\" \"b\") (lambda [x] (+ x 1)))" > /dev/null 2>&1

# ============================================================================
# SETS
# ============================================================================

./main-cov -e "(set-add #(1 2 3) 4)" > /dev/null 2>&1
./main-cov -e "(set-remove #(1 2 3) 2)" > /dev/null 2>&1
./main-cov -e "(set-member #(1 2 3) 2)" > /dev/null 2>&1
./main-cov -e "(set-union #(1 2) #(2 3))" > /dev/null 2>&1
./main-cov -e "(set-intersection #(1 2 3) #(2 3 4))" > /dev/null 2>&1
./main-cov -e "(set-difference #(1 2 3) #(2 3))" > /dev/null 2>&1

# ============================================================================
# COMPOSITION
# ============================================================================

./main-cov -e "(comp (lambda [x] (+ x 1)) (lambda [x] (* x 2)))" > /dev/null 2>&1
./main-cov -e "((comp (lambda [x] (+ x 1)) (lambda [x] (* x 2))) 5)" > /dev/null 2>&1
./main-cov -e "(identity 42)" > /dev/null 2>&1
./main-cov -e "(constantly 42)" > /dev/null 2>&1
./main-cov -e "((constantly 42) 1 2 3)" > /dev/null 2>&1
./main-cov -e "(partial + 1)" > /dev/null 2>&1
./main-cov -e "((partial + 1) 2)" > /dev/null 2>&1
./main-cov -e "(flip -)" > /dev/null 2>&1
./main-cov -e "((flip -) 2 10)" > /dev/null 2>&1

# ============================================================================
# THREADING
# ============================================================================

./main-cov -e "(-> 1 (+ 2) (* 3))" > /dev/null 2>&1
./main-cov -e "(->> 1 (+ 2) (* 3))" > /dev/null 2>&1
./main-cov -e "(as-> 1 x (+ x 2) (* x 3))" > /dev/null 2>&1

echo ""
echo "Comprehensive tests completed. Generating coverage report..."

# Generate coverage report
gcov main.c --object-file main-cov-main.gcno 2>/dev/null | grep -E "^File|^Lines executed"
