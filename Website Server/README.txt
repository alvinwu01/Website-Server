
 
The description should indicate whether your solution for the part is
working or not.  You may also want to include anything else you would
like to communicate to the grader such as extra functionalities you
implemented or how you tried to fix your non-working code.
==7471== Memcheck, a memory error detector
==7471== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==7471== Using Valgrind-3.13.0 and LibVEX; rerun with -h for copyright info
==7471== Command: ./http-server 8787 /mnt/disks/students209/aw3254/html localhost 9898
==7471== 
72.68.194.117 "GET /mdb-lookup?key=yes HTTP/1.1" 200 OK
72.68.194.117 "GET /favicon.ico HTTP/1.1" 404 Not Found
72.68.194.117 "GET /mdb-lookup?key= HTTP/1.1" 200 OK
72.68.194.117 "GET /favicon.ico HTTP/1.1" 404 Not Found
72.68.194.117 "GET /mdb-lookup?key=hello HTTP/1.1" 200 OK
72.68.194.117 "GET /favicon.ico HTTP/1.1" 404 Not Found
72.68.194.117 "GET /mdb-lookup?key= HTTP/1.1" 200 OK
72.68.194.117 "GET /favicon.ico HTTP/1.1" 404 Not Found
72.68.194.117 "" 400 Bad Request
72.68.194.117 "GET /cs3157/tng/ HTTP/1.1" 200 OK
72.68.194.117 "GET /cs3157/tng/images/crew.jpg HTTP/1.1" 200 OK
72.68.194.117 "GET /cs3157/tng/images/ship.jpg HTTP/1.1" 200 OK
72.68.194.117 "GET /favicon.ico HTTP/1.1" 404 Not Found
72.68.194.117 "GET /cs3157/tng HTTP/1.1" 404 Not Found
72.68.194.117 "GET /favicon.ico HTTP/1.1" 404 Not Found
72.68.194.117 "GET /cs3157/tng/images/ HTTP/1.1" 404 Not Found
72.68.194.117 "GET /favicon.ico HTTP/1.1" 404 Not Found
72.68.194.117 "GET /cs3157/tng/images/crew.jpg HTTP/1.1" 200 OK
72.68.194.117 "GET /favicon.ico HTTP/1.1" 404 Not Found
35.236.227.151 "GET /../html HTTP/1.0" 400 Bad Request
35.236.227.151 "PUT /cs3157/tng/ HTTP/1.0" 501 not implemented
35.236.227.151 "GET /cs3157/tng/.
" 400 Bad Request
35.236.227.151 "GET /cs3157/tng/ HTTP/2.0" 501 not implemented
==7471== 
==7471== Process terminating with default action of signal 2 (SIGINT)
==7471==    at 0x4F60674: accept (accept.c:26)
==7471==    by 0x109BD7: main (http-server.c:219)
==7471== 
==7471== HEAP SUMMARY:
==7471==     in use at exit: 552 bytes in 1 blocks
==7471==   total heap usage: 113 allocs, 112 frees, 162,985 bytes allocated
==7471== 
==7471== LEAK SUMMARY:
==7471==    definitely lost: 0 bytes in 0 blocks
==7471==    indirectly lost: 0 bytes in 0 blocks
==7471==      possibly lost: 0 bytes in 0 blocks
==7471==    still reachable: 552 bytes in 1 blocks
==7471==         suppressed: 0 bytes in 0 blocks
==7471== Reachable blocks (those to which a pointer was found) are not shown.
==7471== To see them, rerun with: --leak-check=full --show-leak-kinds=all
==7471== 
==7471== For counts of detected and suppressed errors, rerun with: -v
==7471== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
