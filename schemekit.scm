(define load-event 0)
(define close-event 1)
(define empty-event 2)

(define (open-page url)
 (qu-push
  load-event
  url
  message-qu))

(call-with-new-thread
  (lambda ()
    (launch-webkit-blocking message-qu)))