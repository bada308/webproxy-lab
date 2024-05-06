####################################################################
# CS:APP Proxy Lab
#
# Student Source Files
####################################################################

This directory contains the files you will need for the CS:APP Proxy
Lab.

proxy.c
csapp.h
csapp.c

    These are starter files.  csapp.c and csapp.h are described in
    your textbook. 

    You may make any changes you like to these files.  And you may
    create and handin any additional files you like.

    Please use `port-for-user.pl' or 'free-port.sh' to generate
    unique ports for your proxy or tiny server.


Makefile

    This is the makefile that builds the proxy program.  Type "make"
    to build your solution, or "make clean" followed by "make" for a
    fresh build. 

    Type "make handin" to create the tarfile that you will be handing
    in. You can modify it any way you like. Your instructor will use your
    Makefile to build your proxy from source.


port-for-user.pl

    Generates a random port for a particular user
    usage: ./port-for-user.pl <userID>

free-port.sh

    Handy script that identifies an unused TCP port that you can use
    for your proxy or tiny. 
    usage: ./free-port.sh

driver.sh

    The autograder for Basic, Concurrency, and Cache.        
    usage: ./driver.sh

nop-server.py

     helper for the autograder.         

tiny

    Tiny Web server from the CS:APP text


<br />
<br />

# 번역

### proxy.c / csapp.h / csapp.c

    csapp.c와 csapp.h는 교재에 설명되어 있다.
    이 파일들은 원하는 대로 수정 가능하다.
    추가 파일을 만들어 제출할 수 있다.
    프록시나 tiny 서버에 고유한 포트를 생성하기 위해 `port-for-user.pl`이나 `free-port.sh`를 사용해라.


### Makefile

    프록시 프로그램을 빌드하는데 사용된다.
    make : 솔루션 빌드
    make clean : 기존 빌드 파일 제거
    make handin : 제출할 tar 파일 생성 / make handin 명령어는 수정 가능

### port-for-user.pl

    특정 사용자를 위한 무작위 포트를 생성하는 Perl 스크립트
    사용법: `./port-for-user.pl <userID>`

### free-port.sh

    사용 가능한 미사용 TCP 포트를 식별하는 스크립트
    프록시나 tiny 웹 서버에 사용할 수 있음
    사용법: `./free-port.sh`

### driver.sh

    Basic, Concurrency, Cache의 자동 채점 스크립트
    사용법: `./drvier.sh`

### nop-server.py

    오토크레이더를 위한 도움미 스크립트


### tiny

    CS:APP 교재에서 제공되는 간단한 웹 서버인 tiny 웹 서버