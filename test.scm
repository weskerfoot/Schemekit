(use-modules (ice-9 threads))

(call-with-new-thread (lambda () (start-browser)))
