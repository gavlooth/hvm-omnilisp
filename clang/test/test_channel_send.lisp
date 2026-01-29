;; test_channel_send.lisp - Tests for channel send operations

;; Channel-send puts value into channel
;; TEST: send to unbuffered channel
;; EXPECT: 42
(let [ch (channel)]
  (fiber-spawn (lambda [] (channel-send ch 42)))
  (channel-recv ch))

;; TEST: send to buffered channel
;; EXPECT: true
(let [ch (channel 10)]
  (channel-send ch 1)
  (channel-send ch 2)
  (channel-send ch 3)
  (= 3 (channel-count ch)))

;; TEST: send returns true on success
;; EXPECT: true
(let [ch (channel 1)]
  (channel-send ch :value))

;; TEST: send multiple values
;; EXPECT: (1 2 3 4 5)
(let [ch (channel 5)]
  (for-each (lambda [x] (channel-send ch x)) '(1 2 3 4 5))
  (collect (take 5 (channel-iter ch))))

;; TEST: send blocks on full unbuffered
;; EXPECT: :sent
(let [ch (channel)]
(let [state :waiting]
  (fiber-spawn (lambda []
    (set! state :sending)
    (channel-send ch :value)
    (set! state :sent)))
  (sleep 10)
  (channel-recv ch)
  (sleep 10)
  state))

;; TEST: try-send non-blocking
;; EXPECT: false
(let [ch (channel)]
  (channel-try-send ch :value))

;; TEST: try-send succeeds on buffered
;; EXPECT: true
(let [ch (channel 1)]
  (channel-try-send ch :value))

;; TEST: send with timeout
;; EXPECT: :timeout
(let [ch (channel)]
  (channel-send-timeout ch :value 10))

;; TEST: send! variant
;; EXPECT-FINAL: :ok
(let [ch (channel 1)]
  (channel-send! ch :data)
  :ok)
