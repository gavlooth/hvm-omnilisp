;; test_channel.lisp - Tests for channels

;; Channel creation
;; TEST: create channel
;; EXPECT: true
(channel? (channel-new))

;; TEST: buffered channel
;; EXPECT: true
(channel? (channel-new :buffer 10))

;; TEST: channel is empty initially
;; EXPECT: true
(channel-empty? (channel-new))

;; Send and receive
;; TEST: send then receive
;; EXPECT: 42
(let [ch (channel-new :buffer 1)]
  (chan-send ch 42)
  (chan-recv ch))

;; TEST: FIFO order
;; EXPECT: 1
(let [ch (channel-new :buffer 3)]
  (chan-send ch 1)
  (chan-send ch 2)
  (chan-send ch 3)
  (chan-recv ch))

;; TEST: receive order
;; EXPECT: (1 2 3)
(let [ch (channel-new :buffer 3)]
  (chan-send ch 1)
  (chan-send ch 2)
  (chan-send ch 3)
  (list (chan-recv ch) (chan-recv ch) (chan-recv ch)))

;; Channel predicates
;; TEST: channel not empty after send
;; EXPECT: false
(let [ch (channel-new :buffer 1)]
  (chan-send ch 42)
  (channel-empty? ch))

;; TEST: channel full
;; EXPECT: true
(let [ch (channel-new :buffer 1)]
  (chan-send ch 42)
  (channel-full? ch))

;; Close channel
;; TEST: close channel
;; EXPECT: true
(let [ch (channel-new)]
  (channel-close ch)
  (channel-closed? ch))

;; TEST: receive from closed returns nil
;; EXPECT: nil
(let [ch (channel-new :buffer 1)]
  (chan-send ch 42)
  (chan-recv ch)
  (channel-close ch)
  (chan-recv ch))

;; TEST: send to closed fails
;; EXPECT-FINAL: false
(let [ch (channel-new)]
  (channel-close ch)
  (handle
    (chan-send ch 42)
    (ChannelClosed [_ resume] false)))
