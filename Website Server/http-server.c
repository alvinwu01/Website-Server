#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>


static void die(const char *str){
	perror(str);
	exit(1);
}

static void printErr(const char *str,const char *clntip,int clntsock,char *buf){
	fprintf(stderr, "%s \"%s\" %s\n", clntip,buf,str);
	
	char sendbuf[4096];

	snprintf(sendbuf,sizeof(sendbuf),
		       	"HTTP/1.0 %s\r\n"
			"\r\n"
			"<html><body>\n"
			"<h1>%s</h1>\n"
			"</body></html>\n",str,str);

	if(send(clntsock,sendbuf,strlen(sendbuf),0) != strlen(sendbuf)) {
			perror("send failed\n");
	}
}

static int sendOk(const char *clntip,int clntsock,char *buf){
	
	fprintf(stderr,"%s \"%s\" 200 OK\n",clntip,buf);
	
	char *sendbuf=	"HTTP/1.0 200 OK\r\n"
			"\r\n";
	if(send(clntsock,sendbuf,strlen(sendbuf),0) !=strlen(sendbuf)) {
		perror("send failed\n");
		close(clntsock);
		return 1;
	}
	return 0;
}

char *formatRequest(char *buf,const char *method, const char *request, const char *version){
	strcpy(buf,method);
	strcat(buf," ");
	strcat(buf,request);
	strcat(buf," ");
	strcat(buf,version);
	return buf;
}

void sendMDB(int mdbsock,FILE *mdbf, int clntsock, const char* request){
	
	const char *form =
		"<html><body>\n"	
		"<h1>mdb-lookup</h1>\n"
		"<p>\n"
		"<form method=GET action=/mdb-lookup>\n"
		"lookup: <input type=text name=key>\n"
		"<input type=submit>\n"
		"</form>\n"
		"<p>\n";
	if(send(clntsock,form,strlen(form),0) != strlen(form)) {
		perror("send mdb form  failed (client could have disconnected)\n");
		return;
	}

	if(strncmp("/mdb-lookup?key=",request,16) == 0){
		const char *key = strchr(request,'=')+1;

		char sendbuf[4096];
		strcpy(sendbuf,key);
		strcat(sendbuf,"\n");
	
		if(send(mdbsock,sendbuf,strlen(sendbuf),0) != strlen(sendbuf)) {
			perror("MDB send failed(MDB socket may be closed) \n");
			return;
		}
		char *table =
		       	"<p><table border>";
		if (send(clntsock,table,strlen(table),0) != strlen(table)) {
			perror("table format send failed (client could have disonnected)\n");
			return;
		}

		int row = 0;
	
		while(1) {
			if(fgets(sendbuf,sizeof(sendbuf),mdbf) == NULL) {
				if(ferror(mdbf)){
					perror("MDB socket connection error\n");
				}
				fprintf(stderr, "MDB connection ended prematurely\n");
				return;
			}

			if(strcmp("\n",sendbuf) == 0) {
				break;
			}
		       	char *tablerow;
			if((row % 2)== 0){ 
				tablerow = "\n<tr><td>";
			}
			else {
				tablerow = "\n<tr><td bgcolor=yellow>";
			}

			if(send(clntsock,tablerow,strlen(tablerow),0) != strlen(tablerow)){
				perror("tablerow send failed(client may have disconnected)\n");
				return;
			}
			if(send(clntsock,sendbuf,strlen(sendbuf),0) != strlen(sendbuf)) {
				perror("sending MDB struct row failed(client may have disonnected)\n");
				return;
			}
			row++;
		}

		char *tableend = "\n</table>";

		if(send(clntsock,tableend,strlen(tableend),0) != strlen(tableend)){
			perror("end of table send failed(client may have disonnected) \n");
			return;
		}
	}

	char *html = "\n</body></html>\n";

	if(send(clntsock, html,strlen(html),0) != strlen(html)) {
		perror("html end send failed(client may have disonnected)\n");
		return;
	}
}

int main(int argc, char **argv){

	// ignore SIGPIPE so that we don’t terminate when we call
	// send() on a disconnected socket.
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		die("signal() failed");

	if(argc != 5){
		fprintf(stderr, "usage: %s <server_port> <web_root> <mdb-lookup-host> <mdb-lookup-port>\n", argv[0]);
		exit(1);
	}

	const char *serverport;
	const char *webroot;
	const char *mdbhost;
	unsigned short mdbport;
	struct hostent *he;

	char buf[4096];
	int sock;
	int mdbsock;

	serverport = argv[1];
	webroot = argv[2];
	mdbhost = argv[3];
	mdbport = atoi(argv[4]);
       
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		die("socket failed");
	}
	if((mdbsock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0) {
		die("socket failed");
	}


	 struct sockaddr_in servaddr;
      	 memset(&servaddr, 0, sizeof(servaddr));
       	 servaddr.sin_family = AF_INET;
	 servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // any network interface
	 unsigned short port = atoi(serverport);
       	 servaddr.sin_port = htons(port);

    // Bind to the local address
    	
	 if (bind(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		 die("bind failed");

    // Start listening for incoming connections
    
	 if (listen(sock, 5 /* queue size for connection requests */ ) < 0)
		 die("listen failed");

	  if((he = gethostbyname(mdbhost)) == NULL) {
		  die("gethostbyname failed");
	  }

	  char *mdbhostip = inet_ntoa(*(struct in_addr *) he->h_addr);

	  struct sockaddr_in mdbaddr;
	  memset(&mdbaddr, 0, sizeof(mdbaddr)); 
	  mdbaddr.sin_family      = AF_INET;
	  mdbaddr.sin_addr.s_addr = inet_addr(mdbhostip);
	  mdbaddr.sin_port        = htons(mdbport);

	  if (connect(mdbsock,(struct sockaddr *)&mdbaddr,sizeof(mdbaddr)) < 0)
		  die("connect failed");
	  

	 FILE *mdbf;
		if((mdbf = fdopen(mdbsock,"r")) == NULL)
		       	die("fdopen failed");	       
	 int clntsock;
	 socklen_t clntlen;
	 struct sockaddr_in clntaddr;

	 while(1){
		 buf[0] = '\0';
		 clntlen = sizeof(clntaddr);
		
		 if ((clntsock = accept(sock,(struct sockaddr *) &clntaddr, &clntlen)) < 0)
		 	 die("accept failed");

		 FILE *fd;
		 const char* clntip = inet_ntoa(clntaddr.sin_addr);

		 if ((fd= fdopen(clntsock,"r"))==NULL)
			 die("fdopen failed");

		 if (fgets(buf, sizeof(buf), fd) == NULL) {
			 if (ferror(fd)){
				 perror("IO error fgets\n");
			 }
				 printErr("400 Bad Request",clntip,clntsock,buf);
				 goto OuterLoop;
		 }


		 char orig[4096];
		 strcpy(orig,buf);

		 char *token_separators = "\t \r\n"; // tab, space, new line
		 char *method = strtok(orig, token_separators);
		 char *request = strtok(NULL, token_separators);
		 char *httpVersion = strtok(NULL, token_separators);



		 if(method == NULL || request == NULL || httpVersion == NULL){
			 printErr("400 Bad Request",clntip,clntsock,buf);
			 goto OuterLoop;
		 }

		 formatRequest(buf,method,request,httpVersion);

		 if (strncmp("HTTP/1.0",httpVersion, 8) != 0 && strncmp("HTTP/1.1",httpVersion, 8) != 0) {
			 printErr("501 not implemented",clntip,clntsock,buf);
			 goto OuterLoop;
		 }
		 if (strncmp("GET",method,3) != 0) {
			 printErr("501 not implemented",clntip,clntsock,buf);
			 goto OuterLoop;
		 }
		 char *p =strrchr(request, '/');

		 if (strncmp("/", request, 1) != 0 || strstr(request,"/../") != NULL || strncmp("/..",p,3) == 0){
			 printErr("400 Bad Request",clntip,clntsock,buf);
			 goto OuterLoop;
		 }

		 char parse[1000];

		 while(1){
			 if (fgets(parse,sizeof(parse),fd) == NULL) {
				 if(ferror(fd)){
					 perror("IO Error in fgets\n");
				 }
				 printErr("400 Bad Request", clntip,clntsock,buf);
				 goto OuterLoop;

			 }
			 if ( strcmp("\r\n", parse) == 0 || strcmp("\n", parse) == 0 ){
				 break;
			 }
		 }

		 if(strncmp("/mdb-lookup?key=",request,16) == 0 || strcmp("/mdb-lookup",request) == 0){
			 sendOk(clntip,clntsock,buf);
			 sendMDB(mdbsock,mdbf,clntsock,request);
			 goto OuterLoop;
		 }



		 char filepath[1000];
		 strcpy(filepath,webroot);
		 strcat(filepath,request);
		 struct stat sb;
		 FILE *file;

		 if(stat(filepath, &sb) == -1){
			 printErr("404 Not Found",clntip,clntsock,buf);
			 goto OuterLoop;
		 }

		 if ((sb.st_mode & S_IFMT) == S_IFREG) {
			if((file = fopen(filepath, "r")) == NULL){
				printErr("404 Not Found",clntip,clntsock,buf);
				goto OuterLoop;
			}
		 }

		 if ((sb.st_mode & S_IFMT) == S_IFDIR) {
			 strcat(filepath,"index.html");
			 if((file = fopen(filepath, "r")) == NULL){
				 printErr("404 Not Found",clntip,clntsock,buf);
				 goto OuterLoop;
			 }
		 }

		   size_t n;
		   if( sendOk(clntip,clntsock,buf) ){
			   goto OuterLoop;
		   }

	       	   while ((n = fread(buf, 1, sizeof(buf),file)) > 0) {
			   if (send(clntsock,buf, n,0) != n){
				   perror("send static file to client failed\n");
				   fclose(file);
				   goto OuterLoop;
			   }
		   }

		   if(ferror(file)){
			   perror("IO Error Reading Server Files\n");
		   }	   
		   fclose(file);
OuterLoop:
		   fclose(fd);

	 }

	 return 0;
}































