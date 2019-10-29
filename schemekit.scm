(define load-event 0)
(define close-event 1)
(define empty-event 2)

; Used to configure webkit at runtime
(define config
 (alist->hash-table
  '(
    ("console-log" . 0)
    ("compositing-indicators" . 0)
    ("hw-acceleration" . 0)
   )))

(define (open-page url)
 (qu-push
  load-event
  url
  message-qu))

(call-with-new-thread
  (lambda ()
    (launch-webkit-blocking message-qu)))
