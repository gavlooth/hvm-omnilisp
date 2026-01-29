;; test_channel_recv.lisp - Tests for channel receive operations

;; Channel-recv takes value from channel
;; TEST: recv from channel
;; EXPECT: 42
(let [ch (channel 1)]
  (channel-send ch 42)
  (channel-recv ch))

;; TEST: recv blocks until value
;; EXPECT: :received
(let [ch (channel)]
(let [result nil]
  (fiber-spawn (lambda []
    (sleep 10)
    (channel-send ch :value)))
  (set! result (channel-recv ch))
  (if (= result :value) :received :failed)))

;; TEST: recv multiple values
;; EXPECT: (a b c)
(let [ch (channel 3)]
  (channel-send ch 'a)
  (channel-send ch 'b)
  (channel-send ch 'c)
  (list (channel-recv ch) (channel-recv ch) (channel-recv ch)))

;; TEST: try-recv non-blocking
;; EXPECT: nil
(let [ch (channel)]
  (channel-try-recv ch))

;; TEST: try-recv returns value
;; EXPECT: 42
(let [ch (channel 1)]
  (channel-send ch 42)
  (channel-try-recv ch))

;; TEST: recv with timeout
;; EXPECT: :timeout
(let [ch (channel)]
  (channel-recv-timeout ch 10))

;; TEST: recv with default
;; EXPECT: :default
(let [ch (channel)]
  (channel-recv-or ch :default))

;; TEST: recv preserves order
;; EXPECT: (1 2 3 4 5)
(let [ch (channel 5)]
  (for-each (lambda [x] (channel-send ch x)) '(1 2 3 4 5))
  (map (lambda [_] (channel-recv ch)) '(1 2 3 4 5)))

;; TEST: recv from closed channel
;; EXPECT-FINAL: nil
(let [ch (channel 1)]
  (channel-send ch :last)
  (channel-recv ch)
  (channel-close ch)
  (channel-recv ch))
