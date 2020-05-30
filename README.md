# web-crawler
A minimalistic web crawler written in C

To crawl any URL using this crawler
1. Clone the repository
2. Run the `Makefile`
3. Crawl a URL by running a.out as `./a.out <URL to crawl>`

See the list of URL(s) present on the site!

<h3>Crawler Log</h3>
<ul>
<li>All the log information regarding the crawler is stored in `.logfile` at your specified location.</li>
<li>Update the `log_file_path` in `logger.c` to point to the location(relative to home directory) where you wish to see the logs.</li>
<li>If you don't wish to see the logs, leave the `logger.c` file as it is and ignore the warning issued regarding creation of log file</li>
</ul>
