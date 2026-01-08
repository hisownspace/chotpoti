#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include "server.c"

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

typedef struct {
  char ** plaintext;
  int method;
  char * path;
  char * host;
  int content_length;
  int content_type;
} request_header;

// typedef struct {
//   request_header header;
//   request_body body;
// } request;

typedef struct {
  int type;
  char ** plaintext;
} request_body;

void rio_readinitb(rio_t *rp, int fd) {
  printf("In read init function\n");
  rp->rio_fd = fd;  
  rp->rio_cnt = 0;  
  rp->rio_bufptr = rp->rio_buf;
  printf("ending read init function\n");
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

void parse_header(request_header * header, int line_no) {
  for (int i = 0; i < line_no; i++) {
    printf("%s", header->plaintext[i]);
  }
  header->content_length = 37;
}

void parse_body(char ** rough_body, int line_no) {
  for (int i = 0; i < line_no; i++) {
    printf("%s", rough_body[i]);
  }
}

int read_header(rio_t * rio, request_header * header) {
  int line_no = 0;
  char buf[MAXLINE];
  header->plaintext = calloc(MAXLINE, MAXLINE);

  printf("In read header function\n");

  while (strcmp(buf, "\r\n")) {
  rio_readlineb(rio, buf, MAXLINE);
    header->plaintext[line_no]= strdup(buf);
    line_no++;
  }
  parse_header(header, line_no);
  return 1;
}

int read_body(rio_t * rio, request_header * header, request_body * body) {
  ssize_t body_size = 0;
  char buf[MAXLINE];
  int line_no = 0;
  body->plaintext = calloc(MAXLINE, MAXLINE);

  while (body_size < header->content_length) {
    rio_readlineb(rio, buf, MAXLINE);
    body->plaintext[line_no] = strdup(buf);
    line_no++;
  }

  return 1;
}

void handle_request(int connfd) {
  rio_t rio;
  request_header * header = malloc(sizeof(request_header));
  request_body * body = malloc(sizeof(request_body));

  rio_readinitb(&rio, connfd);
  read_header(&rio, header);
  printf("Before Body\n");
  read_body(&rio, header, body);
  printf("After Body\n");
    
  // parse_body(rough_body, line_no);
  // rio_writen(connfd, buf, n);
  free(header->plaintext);
  free(header);
  // free(rough_body);
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
    printf("After Request\n");
    close(connfd);
  }
  exit(0);
}
