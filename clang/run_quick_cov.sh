#!/bin/bash
# Quick coverage run script

cd "$(dirname "$0")"
rm -f *.gcda

echo "Running coverage tests..."

# Arithmetic
./main-cov -e "(+ 1 2)" > /dev/null 2>&1
./main-cov -e "(- 10 3)" > /dev/null 2>&1
./main-cov -e "(* 4 5)" > /dev/null 2>&1
./main-cov -e "(/ 20 4)" > /dev/null 2>&1
./main-cov -e "(mod 10 3)" > /dev/null 2>&1

# Comparison
./main-cov -e "(= 1 1)" > /dev/null 2>&1
./main-cov -e "(< 1 2)" > /dev/null 2>&1
./main-cov -e "(> 2 1)" > /dev/null 2>&1
./main-cov -e "(<= 1 1)" > /dev/null 2>&1
./main-cov -e "(>= 2 2)" > /dev/null 2>&1
./main-cov -e "(!= 1 2)" > /dev/null 2>&1

# Boolean
./main-cov -e "(and true false)" > /dev/null 2>&1
./main-cov -e "(or true false)" > /dev/null 2>&1
./main-cov -e "(not true)" > /dev/null 2>&1

# Conditionals
./main-cov -e "(if true 1 2)" > /dev/null 2>&1
./main-cov -e "(if false 1 2)" > /dev/null 2>&1
./main-cov -e "(cond (true 1) (false 2))" > /dev/null 2>&1
./main-cov -e "(when true 42)" > /dev/null 2>&1
./main-cov -e "(unless false 42)" > /dev/null 2>&1

# Let and lambda
./main-cov -e "(let [x 1] (+ x 2))" > /dev/null 2>&1
./main-cov -e "(let [x 1 y 2] (+ x y))" > /dev/null 2>&1
./main-cov -e "(lambda [x] (+ x 1))" > /dev/null 2>&1
./main-cov -e "((lambda [x] (* x x)) 5)" > /dev/null 2>&1
./main-cov -e "((lambda [x y] (+ x y)) 3 4)" > /dev/null 2>&1

# Lists
./main-cov -e "'(1 2 3)" > /dev/null 2>&1
./main-cov -e "(head '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(tail '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(cons 0 '(1 2))" > /dev/null 2>&1
./main-cov -e "(empty? '())" > /dev/null 2>&1
./main-cov -e "(length '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(append '(1 2) '(3 4))" > /dev/null 2>&1
./main-cov -e "(reverse '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(nth '(1 2 3) 1)" > /dev/null 2>&1

# Arrays
./main-cov -e "[1 2 3]" > /dev/null 2>&1
./main-cov -e "(get [1 2 3] 1)" > /dev/null 2>&1
./main-cov -e "(length [1 2 3])" > /dev/null 2>&1
./main-cov -e "(set [1 2 3] 1 42)" > /dev/null 2>&1
./main-cov -e "(slice [1 2 3 4 5] 1 3)" > /dev/null 2>&1

# Define and functions
./main-cov -e "(define f [x] (+ x 1))" > /dev/null 2>&1
./main-cov -e "(do (define f [x] (+ x 1)) (f 5))" > /dev/null 2>&1
./main-cov -e "(do (define add [x] [y] (+ x y)) ((add 3) 4))" > /dev/null 2>&1

# Match
./main-cov -e "(match 1 1 :one 2 :two _ :other)" > /dev/null 2>&1
./main-cov -e "(match '(1 2 3) (h .. t) h _ 0)" > /dev/null 2>&1

# Do/sequence
./main-cov -e "(do 1 2 3)" > /dev/null 2>&1

# Strings
./main-cov -e "\"hello world\"" > /dev/null 2>&1
./main-cov -e "(str-concat \"hello\" \" \" \"world\")" > /dev/null 2>&1
./main-cov -e "(str-length \"hello\")" > /dev/null 2>&1
./main-cov -e "(str-slice \"hello\" 1 3)" > /dev/null 2>&1
./main-cov -e "(str-get \"hello\" 0)" > /dev/null 2>&1

# Higher-order functions
./main-cov -e "(map (lambda [x] (* x 2)) '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(filter (lambda [x] (> x 2)) '(1 2 3 4))" > /dev/null 2>&1
./main-cov -e "(foldl + 0 '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(foldr cons '() '(1 2 3))" > /dev/null 2>&1

# Bitwise
./main-cov -e "(bit-and 5 3)" > /dev/null 2>&1
./main-cov -e "(bit-or 5 3)" > /dev/null 2>&1
./main-cov -e "(bit-xor 5 3)" > /dev/null 2>&1
./main-cov -e "(bit-not 5)" > /dev/null 2>&1
./main-cov -e "(bit-shift-left 1 4)" > /dev/null 2>&1
./main-cov -e "(bit-shift-right 16 2)" > /dev/null 2>&1

# Math
./main-cov -e "(abs -5)" > /dev/null 2>&1
./main-cov -e "(min 3 7)" > /dev/null 2>&1
./main-cov -e "(max 3 7)" > /dev/null 2>&1

# Type predicates
./main-cov -e "(int? 5)" > /dev/null 2>&1
./main-cov -e "(float? 3.14)" > /dev/null 2>&1
./main-cov -e "(string? \"hello\")" > /dev/null 2>&1
./main-cov -e "(list? '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(array? [1 2 3])" > /dev/null 2>&1
./main-cov -e "(fn? (lambda [x] x))" > /dev/null 2>&1
./main-cov -e "(symbol? 'foo)" > /dev/null 2>&1
./main-cov -e "(bool? true)" > /dev/null 2>&1

# Quote and quasiquote
./main-cov -e "'foo" > /dev/null 2>&1
./main-cov -e "\`(1 2 3)" > /dev/null 2>&1
./main-cov -e "(let [x 5] \`(1 ,x 3))" > /dev/null 2>&1

# Characters
./main-cov -e "\\a" > /dev/null 2>&1
./main-cov -e "(char? \\a)" > /dev/null 2>&1

# Dicts
./main-cov -e "#{\"a\" 1 \"b\" 2}" > /dev/null 2>&1
./main-cov -e "(dict-get #{\"a\" 1} \"a\")" > /dev/null 2>&1
./main-cov -e "(dict-set #{\"a\" 1} \"b\" 2)" > /dev/null 2>&1
./main-cov -e "(dict-keys #{\"a\" 1 \"b\" 2})" > /dev/null 2>&1
./main-cov -e "(dict-vals #{\"a\" 1 \"b\" 2})" > /dev/null 2>&1

# Parse mode
./main-cov -p -e "(+ 1 2)" > /dev/null 2>&1
./main-cov -p -e "(lambda [x y] (+ x y))" > /dev/null 2>&1
./main-cov -p -e "(if true 1 2)" > /dev/null 2>&1

# Range
./main-cov -e "(range 5)" > /dev/null 2>&1
./main-cov -e "(range 1 5)" > /dev/null 2>&1

# Every/some
./main-cov -e "(every (lambda [x] (> x 0)) '(1 2 3))" > /dev/null 2>&1
./main-cov -e "(some (lambda [x] (= x 2)) '(1 2 3))" > /dev/null 2>&1

# Case
./main-cov -e "(case 2 1 :one 2 :two _ :other)" > /dev/null 2>&1

# Letrec
./main-cov -e "(letrec [f (lambda [n] (if (= n 0) 1 (* n (f (- n 1)))))] (f 5))" > /dev/null 2>&1

echo "Tests completed. Generating coverage report..."

# Generate coverage report
gcov -o . main.c 2>/dev/null | grep -E "^File|^Lines"
