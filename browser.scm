
(define atomic-webview (make-atomic-box #f))

(define (open-page url)
 (cond
  ((atomic-box-ref atomic-webview)
   (open-page-with-webview (atomic-box-ref atomic-webview) url))
  (else #f)))

(call-with-new-thread
  (lambda ()
  (define webview (make-webview))

  (atomic-box-set! atomic-webview webview)
  (launch-webkit-blocking webview)))
