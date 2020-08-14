(use-modules
  (srfi srfi-98)
  (ice-9 format))

(define load-event 0)
(define close-event 1)
(define empty-event 2)

; Used to configure webkit at runtime
(define config
 (alist->hash-table
  '(
    ("console-log" . 0)
    ("compositing-indicators" . 0)
    ("hw-acceleration" . 1)
    ("smooth-scrolling" . 0)
   )))

(define (open-page url)
 (qu-push
  load-event
  url
  gtk-qu))


(define (make-opener domain-name)
  (lambda ()
    (open-page
      (format "https://~a" domain-name))))

(call-with-new-thread
  (lambda ()
    (launch-webkit-blocking gtk-qu guile-qu)))

(define (handle-events)
  (begin
   (sleep 1)
   (let ((msg (qu-pop guile-qu)))
    (if msg
     (display (format "~a loaded\n" msg))
     '())
    (handle-events))))


(define home-dir
  (getenv "HOME"))

(display home-dir)

(cond
  ((eq? #f home-dir)
   (display "Could not load custom schemekit.scm"))
  (else
    (load
      (format "~a/.config/schemekit.scm"
              home-dir))))


(call-with-new-thread handle-events)
