;; test_file_walk.lisp - Tests for directory traversal

;; File-walk traverses directory recursively
;; TEST: walk collects all files
;; EXPECT: 3
(with-temp-dir "walktest"
  (lambda [dir]
    (file-write (str dir "/a.txt") "a")
    (file-write (str dir "/b.txt") "b")
    (file-mkdir (str dir "/sub"))
    (file-write (str dir "/sub/c.txt") "c")
    (length (file-walk dir))))

;; TEST: walk with filter
;; EXPECT: 2
(with-temp-dir "walkfilter"
  (lambda [dir]
    (file-write (str dir "/a.txt") "a")
    (file-write (str dir "/b.md") "b")
    (file-write (str dir "/c.txt") "c")
    (length (file-walk dir (lambda [f] (str-ends-with? f ".txt"))))))

;; TEST: walk returns full paths
;; EXPECT: true
(with-temp-dir "walkpaths"
  (lambda [dir]
    (file-write (str dir "/test.txt") "test")
    (let [files (file-walk dir)]
      (every? (lambda [f] (str-starts-with? f dir)) files))))

;; TEST: walk empty directory
;; EXPECT: ()
(with-temp-dir "empty"
  (lambda [dir]
    (file-walk dir)))

;; TEST: walk with depth limit
;; EXPECT: 2
(with-temp-dir "depth"
  (lambda [dir]
    (file-mkdir (str dir "/a"))
    (file-mkdir (str dir "/a/b"))
    (file-write (str dir "/1.txt") "1")
    (file-write (str dir "/a/2.txt") "2")
    (file-write (str dir "/a/b/3.txt") "3")
    (length (file-walk dir :max-depth 1))))

;; Walk with visitor function
;; TEST: walk-each applies function
;; EXPECT: 3
(with-temp-dir "walkeach"
  (lambda [dir]
    (file-write (str dir "/a.txt") "a")
    (file-write (str dir "/b.txt") "b")
    (file-write (str dir "/c.txt") "c")
    (let [count 0]
      (file-walk-each dir (lambda [f] (set! count (+ count 1))))
      count)))

;; TEST: walk directories only
;; EXPECT-FINAL: 2
(with-temp-dir "walkdirs"
  (lambda [dir]
    (file-mkdir (str dir "/sub1"))
    (file-mkdir (str dir "/sub2"))
    (file-write (str dir "/file.txt") "f")
    (length (file-walk-dirs dir))))
