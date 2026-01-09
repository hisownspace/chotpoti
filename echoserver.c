#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include "server.c"
#include "./hash-table/hash-table.h"

#define MAXLINE 8192
#define RIO_BUFSIZE 8192
#define HEADER 1
#define BODY 2

typedef struct {
  int rio_fd;                /* Descriptor for this internal buf */
  int rio_cnt;               /* Unread bytes in internal buf */
  char *rio_bufptr;          /* Next unread byte in internal buf */
  char rio_buf[RIO_BUFSIZE]; /* Internal buffer */
} rio_t;

struct request_line {
  int method;
  char * target;
  int version;
};

struct request_header {
  int no_lines;
  char ** plaintext;
  ht * content;
};

struct request_body {
  int type;
  int no_lines;
  char ** plaintext;
  ht * content;
};

typedef struct {
  struct request_line * start_line;
  struct request_header * headers;
  struct request_body * body;
} request;

void rio_readinitb(rio_t *rp, int fd) {
  rp->rio_fd = fd;  
  rp->rio_cnt = 0;  
  rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n) {
  size_t nleft = n;
  ssize_t nwritten;
  char *bufp = usrbuf;

  while (nleft > 0) {
    if ((nwritten = write(fd, bufp, nleft)) <= 0) {
      if (errno == EINTR)  /* Interrupted by sig handler return */
        nwritten = 0;    /* and call write() again */
      else
        return -1;       /* errno set by write() */
    }
    nleft -= nwritten;
    bufp += nwritten;
  }
  return n;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
    int cnt;

    while (rp->rio_cnt <= 0) {  /* Refill if buf is empty */
	rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, 
			   sizeof(rp->rio_buf));
	if (rp->rio_cnt < 0) {
	    if (errno != EINTR) /* Interrupted by sig handler return */
		return -1;
	}
	else if (rp->rio_cnt == 0)  /* EOF */
	    return 0;
	else 
	    rp->rio_bufptr = rp->rio_buf; /* Reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;          
    if (rp->rio_cnt < n)   
	cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) {
        if ((rc = rio_read(rp, &c, 1)) == 1) {
	        *bufp++ = c;
	        if (c == '\n') {
            n++;
     		    break;
          }
	      } else if (rc == 0) {
	        if (n == 1)
		        return 0; /* EOF, no data read */
	        else
		        break;    /* EOF, some data was read */
	      } else
	        return -1;	  /* Error */
    }
    *bufp = 0;
    return n-1;
}

void parse_header(request * req) {

  for (int i = 1; i < req->headers->no_lines; i++) {
    char * header = req->headers->plaintext[i];
    if (strcmp(header, "\r\n") == 0)
      break;
    int ws_idx = (int) strcspn(header, ":");
    char * key = strndup(header, ws_idx);
    char * value = strdup(&header[ws_idx + 2]);
    key[ws_idx] = 0;
    set_entry(req->headers->content, key, value, 0);
    // free(key); free(value);
  }
  printf("%s\n", get_str_entry(req->headers->content, "Content-Length"));
  if (get_str_entry(req->headers->content, "Host") != NULL)
    printf("%s\n", get_str_entry(req->headers->content, "Host"));
  printf("Content length: %d\n", get_ht_length(req->headers->content));
}

void parse_body(request * req) {
  for (int i = 0; i < req->body->no_lines; i++) {
    printf("%s", req->body->plaintext[i]);
  }
}

int read_header(rio_t * rio, request* req) {
  int line_no = 0;
  char buf[MAXLINE];

  printf("In read header function\n");

  while (strcmp(buf, "\r\n")) {
  rio_readlineb(rio, buf, MAXLINE);
    req->headers->plaintext[line_no]= strdup(buf);
    line_no++;
  }
  req->headers->no_lines = line_no;
  parse_header(req);
  return 1;
}

// Reads body from socket and populates the body struct. At present the
// function is only designed to handle text-based bodies. The function 
// does not read the final line of the body if it does not end in a new
// line character. A new read function must be made to handle this case.
int read_body(rio_t * rio, request * req) {
  ssize_t body_size = 0;
  char buf[MAXLINE];
  int no_lines = 0;

  printf("In read body function\n");
  while (body_size < get_int_entry(req->headers->content, "Content-Length")) {
    body_size += rio_readlineb(rio, buf, MAXLINE);
    req->body->plaintext[no_lines] = strdup(buf);
    no_lines++;
  }
  req->body->no_lines = no_lines;

  return 1;
}

void handle_request(int connfd) {
  rio_t rio;
  // This should be refactored to create request function
  request * req = malloc(sizeof(request));
  req->headers = calloc(MAXLINE, sizeof(struct request_header));
  req->headers->plaintext = calloc(MAXLINE, MAXLINE);
  req->headers->content = create_ht();
  req->body = malloc(sizeof(struct request_body));
  req->body->plaintext = calloc(MAXLINE, MAXLINE);

  rio_readinitb(&rio, connfd);
  read_header(&rio, req);
  printf("After read header\n");
  if (req->body) {
    printf("Before read body\n");
    read_body(&rio, req);
    parse_body(req);
  }
  printf("after read body\n");
  char * response = "HTTP/1.1 200 OK\nContent-Length: 19\nContent-Type: text/html\n\n<p>Hello World!</p>";
  rio_writen(connfd, response, strlen(response));
  free(req->headers->plaintext);
  free(req->headers);
  free(req->body->plaintext);
  free(req);
}

int main(int argc, char **argv) {
  int listenfd, connfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  char client_hostname[MAXLINE], client_port[MAXLINE];

  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(0);
  }

  listenfd = open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(struct sockaddr_storage);
    connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
    getnameinfo((struct sockaddr *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
    printf("Connected to (%s %s)\n", client_hostname, client_port);
    handle_request(connfd);
    close(connfd);
  }
  exit(0);
}
