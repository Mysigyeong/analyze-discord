#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
void die(char *s)
{
	perror(s);
	exit(1);
}

int main(int argc, char** argv)
{
	struct sockaddr_in si_other;
	int s, i, sz, slen=sizeof(si_other);
	char* message = NULL;

	int port = 0;

	FILE* fp = fopen(argv[1], "r");

	FILE* port_pointer = popen("netstat -nulp 2> /dev/null | awk \'/Discord/ { print $4 }\' | awk -F \':\' \'{ print $2 }\'", "r");
	if (port_pointer == NULL) {
		puts("popen failed");
		exit(1);
	}
	char net_port_str[100] = { 0 }; 
	fgets(net_port_str, sizeof(net_port_str), port_pointer);
	puts(net_port_str);
	port = atoi(net_port_str);

	pclose(port_pointer);
	if (port == 0) {
		puts("port is 0");
		exit(1);
	}

	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(port);
	
	if (inet_aton("127.0.0.1", &si_other.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	fseek(fp, 0L, SEEK_END);
	sz = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	message = (char*)calloc(sz+10, sizeof(char));

	fgets(message, sz, fp);

	fclose(fp);
	
	//send the message
	if (sendto(s, message, sz , 0 , (struct sockaddr *) &si_other, slen)==-1)
	{
			die("sendto()");
	}

	close(s);
	return 0;
}
