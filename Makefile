CFLAGS=-Wall -Wextra 

build:
	gcc $(CFLAGS) -o spam_detector spam_detector.c -lm

pack:
	zip -FSr 311CA_DinutaEduardStefan_SpamDetector.zip README Makefile *.c *.h

clean:
	rm -f spam_detector statistics.out prediction.out

.PHONY: pack clean
