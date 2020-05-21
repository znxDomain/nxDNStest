#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <switch.h>

void libnx_getaddrinfo(const char *hostname, const char *port)
{
    socketInitializeDefault();

    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    printf("\nCalling getaddrinfo hostname: \"%s\" port \"%s\":\n", hostname, port);

    if ((status = getaddrinfo(hostname, port, &hints, &res)) != 0) {
        printf("ERR getaddrinfo: %s, error nr: %d\n", gai_strerror(status), status);
    }

    printf("IP addresses for %s:\n", hostname);

    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(res); // free the linked list
}

void libnx_gethostbyname(const char *hostname){
    socketInitializeDefault();

    int i;
    struct hostent *he;
    struct in_addr **addr_list;

    printf("\nCalling gethostbyname hostname: \"%s\":\n", hostname);

    if ((he = gethostbyname(hostname)) == NULL) {
        herror("gethostbyname");
    }

    printf("IP addresses for %s:\n", hostname);

    printf("Official name is: %s\n", he->h_name);
    printf("    IP addresses: ");
    addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++) {
        printf("%s ", inet_ntoa(*addr_list[i]));
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    char hostname[] = "www.google.com";
    char port[] = "80";

    consoleInit(NULL);
    printf("0 - Press PLUS to exit\n");

    // cmd 6    sfdnsresGetAddrInfoRequest
    libnx_getaddrinfo(hostname, port);
    
    // cmd 2    sfdnsresGetHostByNameRequest
    libnx_gethostbyname(hostname);

    // Main loop
    while(appletMainLoop())
    {
        //Scan all the inputs. This should be done once for each frame
        hidScanInput();
        //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_PLUS) break; // break in order to return to hbmenu
        consoleUpdate(NULL);
    }

    socketExit();

    consoleExit(NULL);
    return 0;
}
