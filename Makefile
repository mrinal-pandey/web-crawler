a.out: main.o web_crawler.o logger.o
	gcc main.o web_crawler.o logger.o -ltidy -lcurl

web_crawler.o: web_crawler.c web_crawler.h logger.h
	gcc -c web_crawler.c

logger.o: logger.c logger.h
	gcc -c logger.c

main.o: main.c web_crawler.h
	gcc -c main.c
