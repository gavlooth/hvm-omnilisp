;; test_file_write.lisp - Tests for file writing

;; Write-file writes string to file
;; TEST: write and read back
;; EXPECT: "hello world"
(with-temp-dir
  (write-file "test.txt" "hello world")
  (read-file "test.txt"))

;; TEST: write overwrites
;; EXPECT: "new content"
(with-temp-dir
  (write-file "test.txt" "old content")
  (write-file "test.txt" "new content")
  (read-file "test.txt"))

;; TEST: write empty
;; EXPECT: ""
(with-temp-dir
  (write-file "empty.txt" "")
  (read-file "empty.txt"))

;; Append to file
;; TEST: append-file
;; EXPECT: "line1\nline2"
(with-temp-dir
  (write-file "log.txt" "line1")
  (append-file "log.txt" "\nline2")
  (read-file "log.txt"))

;; Write lines
;; TEST: write-lines
;; EXPECT: "a\nb\nc"
(with-temp-dir
  (write-lines "lines.txt" '("a" "b" "c"))
  (read-file "lines.txt"))

;; Spit is alias for write-file
;; TEST: spit
;; EXPECT: "content"
(with-temp-dir
  (spit "file.txt" "content")
  (read-file "file.txt"))

;; Write with encoding
;; TEST: write utf8
;; EXPECT: "unicode: \u00e9"
(with-temp-dir
  (write-file "utf8.txt" "unicode: \u00e9" :encoding "utf-8")
  (read-file "utf8.txt"))

;; Write bytes
;; TEST: write-bytes
;; EXPECT: true
(with-temp-dir
  (write-bytes "bin.dat" (bytes-from-string "abc"))
  (file-exists? "bin.dat"))

;; TEST: write creates parent dirs
;; EXPECT-FINAL: "nested"
(with-temp-dir
  (write-file "a/b/c/file.txt" "nested" :create-parents true)
  (read-file "a/b/c/file.txt"))
