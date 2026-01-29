;; test_file_exists.lisp - Tests for file existence checks

;; File-exists? checks if file exists
;; TEST: file exists true
;; EXPECT: true
(with-temp-file "test.txt" "content"
  (file-exists? "test.txt"))

;; TEST: file exists false
;; EXPECT: false
(file-exists? "nonexistent.txt")

;; TEST: directory exists
;; EXPECT: true
(with-temp-dir
  (make-dir "mydir")
  (file-exists? "mydir"))

;; Directory? predicate
;; TEST: directory? true
;; EXPECT: true
(with-temp-dir
  (make-dir "mydir")
  (directory? "mydir"))

;; TEST: directory? false for file
;; EXPECT: false
(with-temp-file "file.txt" "x"
  (directory? "file.txt"))

;; File? predicate (regular file)
;; TEST: file? true
;; EXPECT: true
(with-temp-file "test.txt" "x"
  (file? "test.txt"))

;; TEST: file? false for directory
;; EXPECT: false
(with-temp-dir
  (make-dir "mydir")
  (file? "mydir"))

;; Readable/writable checks
;; TEST: readable?
;; EXPECT: true
(with-temp-file "test.txt" "x"
  (readable? "test.txt"))

;; TEST: writable?
;; EXPECT: true
(with-temp-file "test.txt" "x"
  (writable? "test.txt"))

;; File size
;; TEST: file-size
;; EXPECT: 5
(with-temp-file "test.txt" "hello"
  (file-size "test.txt"))

;; TEST: file-size empty
;; EXPECT-FINAL: 0
(with-temp-file "empty.txt" ""
  (file-size "empty.txt"))
