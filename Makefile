a.out: main.o web_crawler.o
	gcc main.o web_crawler.o -ltidy -lcurl
web_crawler.o: web_crawler.c web_crawler.h
	gcc -c web_crawler.c
main.o: main.c web_crawler.h
	gcc -c main.c
