#include "utils.h"

/**
 * @brief HTTP 요청을 처리하는 함수
 *
 * @param fd 클라이언트와의 연결을 나타내는 파일 디스크립터
 */
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);           /* rio_t 구조체 초기화 */
    Rio_readlineb(&rio, buf, MAXLINE); /* buf에 request line 저장 */
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version); /* request line에서 method, uri, version 추출 */

    /* GET 이외의 method로 요청 시 예외처리 */
    if (strcasecmp(method, "GET"))
    {
        clienterror(fd, filename, "501", "Not implemented", "Tiny does not implement this method");
        return;
    }
    read_requesthdrs(&rio);

    /* URI 구문 분석 */
    is_static = parse_uri(uri, filename, cgiargs);
    /* 존재하지 않는 파일에 접근 시 예외처리 */
    if (stat(filename, &sbuf) < 0)
    {
        clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
        return;
    }

    if (is_static) /* Serve static content */
    {
        /* 접근하는 파일의 유형이 일반 파일이 아니거나, Read 권한이 없을 시 예외처리  */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size, version);
    }
    else /* Serve dynamic content */
    {
        /* 접근하는 파일의 유형이 일반 파일이 아니거나, Exec 권한이 없을 시 예외처리 */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}

/**
 * @brief HTTP 요청의 헤더를 읽고 출력하는 함수
 *
 * @param rp 요청을 읽기 위한 rio_t 구조체 포인터
 */
void read_requesthdrs(rio_t *rp)
{
    /* tiny 서버는 request header을 사용하지 않음 */
    char buf[MAXLINE];

    /* request header 출력 */
    Rio_readlineb(rp, buf, MAXLINE); /* header의 첫 번째 라인 */
    while (strcmp(buf, "\r\n"))
    {
        Rio_readlineb(rp, buf, MAXLINE); /* header의 나머지 라인 */
        printf("%s", buf);
    }

    return;
}

/**
 * @brief 주어진 URI를 분석하여 요청된 파일의 이름과 CGI 인수를 추출하는 함수
 *
 * @param uri 분석할 URI
 * @param filename 추출된 파일 이름을 저장할 버퍼
 * @param cgiargs 추출된 CGI 인수를 저장할 버퍼
 * @return int 정적 컨텐츠인 경우 1, 동적 컨텐츠인 경우 0을 반환
 */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "/cgi-bin")) /* static content */
    {
        /* CGI 매개변수 설정 - blank */
        strcpy(cgiargs, "");

        /* filename 설정 */
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == '/')
            strcat(filename, "home.html");

        return 1;
    }
    else /* dynamic content */
    {
        /* CGI 매개변수 설정 */
        ptr = index(uri, '?');
        if (ptr) /* Query parameter 존재할 경우 - parameter */
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else /* Query parameter 존재하지 않을 경우 - blank */
        {
            strcpy(cgiargs, "");
        }

        /* filename 설정 */
        strcpy(filename, ".");
        strcat(filename, uri);

        return 0;
    }
}

/**
 * @brief 정적 컨텐츠를 클라이언트에게 제공하는 함수
 *
 * @param fd 클라이언트와의 연결을 나타내는 파일 디스크립터
 * @param filename 클라이언트에게 제공할 파일의 이름
 * @param filesize 제공할 파일의 크기
 */
void serve_static(int fd, char *filename, int filesize, char *version) // TODO: 11.6 C - DONE
{
    int srcfd; /* 읽은 파일의 디스크립터 */
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);

    sprintf(buf, "%s 200 OK\r\n", version);
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);                        /* 파일을 읽기 전용으로 열기 */
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); /* 파일을 메모리에 매핑하여 읽기 전용으로 할당 */
    Close(srcfd);                                               /* 파일 디스크립터 닫기 */
    Rio_writen(fd, srcp, filesize);                             /* 클라이언트 소켓으로 메모리에 매핑된 파일 데이터 전송 */
    Munmap(srcp, filesize);                                     /* 메모리 매핑 해제 */
}

/**
 * @brief 주어진 파일 이름의 확장자를 기반으로 해당 파일의 MIME 타입을 결정하는 함수
 *
 * @param filename 확장자를 확인할 파일의 이름
 * @param filetype 결정된 MIME 타입을 저장할 버퍼
 */
void get_filetype(char *filename, char *filetype)
{
    /* filename의 확장자명을 기반으로 filetype 지정 */
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

/**
 * @brief 동적 컨텐츠를 클라이언트에게 제공하는 함수
 *
 * @param fd 클라이언트와의 연결을 나타내는 파일 디스크립터
 * @param filename 실행할 CGI 프로그램의 파일 이름
 * @param cgiargs CGI 프로그램에 전달할 인수
 */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL};

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* 자식 프로세스 생성 */
    if (Fork() == 0)
    {
        setenv("QUERY_STRING", cgiargs, 1);   /* CGI 프로그램에 전달할 환경 변수 설정 */
        Dup2(fd, STDOUT_FILENO);              /* 표준 출력을 클라이언트 소켓으로 리다이렉션 - CGI 프로그램의 출력이 클라이언트에게 전송됨 */
        Execve(filename, emptylist, environ); /* CGI 프로그램 실행 */
    }
    Wait(NULL); /* Parent waits for and reaps child */
}

/**
 * @brief 클라이언트에게 오류 메세지를 전송하는 함수
 *
 * @param fd 클라이언트와의 연결을 나타내는 파일 디스크립터
 * @param cause 오류의 원인
 * @param errnum 오류 번호
 * @param shortmsg 짧은 오류 메세지
 * @param longmsg 긴 오류 메세지
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html>\r\n<title>Tiny Error</title>\r\n");
    sprintf(body, "%s<body bgcolor="
                  "ffffff"
                  ">\r\n",
            body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em></p>\r\n", body);

    /* Print the HTTP response */
    /* 1. response line */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    /* 2. response header */
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    /* 3. response body */
    Rio_writen(fd, body, strlen(body));
}