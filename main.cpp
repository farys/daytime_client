#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <cstdio>
#include <unistd.h>

using namespace std;

class DaytimeClient{
private:

    int sock;
    int buflen;
    char *buf;
public:
    DaytimeClient(){
        buflen = 168;
        buf = new char[buflen];
    }

    ~DaytimeClient(){
        delete [] buf;
    }

    int start() {
        #ifdef WIN32
          WORD wVersionRequested;
          WSADATA wsaData;
          wVersionRequested = MAKEWORD(1,1);
          int ver = WSAStartup(wVersionRequested, &wsaData);
          int hib = HIBYTE(wsaData.wHighVersion);
          int lob = LOBYTE(wsaData.wHighVersion);
          cout << "Wersja: " << hib << "." << lob << endl;
          cout << "maks. liczba socketow: " << wsaData.iMaxSockets << endl;
          return ver;
        #else
          return 0;
        #endif
    }

    void stop() {
         #ifdef WIN32
           WSACleanup();
           return;
         #else
           return;
         #endif
    }

    string receiveAnswer(int timeout_sec){

        char *pbuf = buf;
        char * pbufend;
        char endline[3] = {13, 10, 0};

        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(sock, &rset);
        int ret;
        timeval tout = {timeout_sec,0};

        if ((ret = select(sock+1, &rset, NULL, NULL, &tout)) == -1)
        {
            cout<<"Wystapil blad select"<<endl;
            return "";
        }

        if (ret > 0 && FD_ISSET(sock, &rset)){

            int n = recv(sock, buf, buflen, 0);

            if (n==0) {return "";}
            pbuf += n;
            buflen -= n;
            *pbuf=0;

            if ((pbufend = strstr(buf, endline)) != NULL) {
                *pbufend=0;
                return buf;
            }
        }

        return "";
    }

   bool sendRequest(char *host, char *port){
        struct sockaddr_in host_addr;
        struct hostent *phe;
        struct servent *pse;

        sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock>=0)
           cout << "UDP OK" << '\n';

        host_addr.sin_family=AF_INET;
        if((pse = getservbyname(port, "udp")) == NULL)
                host_addr.sin_port=htons(atoi(port));
        else host_addr.sin_port=pse->s_port;

        cout <<"Port: " << ntohs(host_addr.sin_port) << endl;

        host_addr.sin_addr.s_addr=inet_addr(host);

        if (host_addr.sin_addr.s_addr == INADDR_NONE) {
            if ((phe=gethostbyname(host)) == NULL) {
                cout << "Nie znaleziono hosta" << endl;
                return false;
            }
            memcpy(&host_addr.sin_addr.s_addr, &phe->h_addr[0], sizeof(host_addr.sin_addr.s_addr));
        }

        char tekst[2] = {'0', 0};
        sendto(sock, tekst, strlen(tekst), 0, (sockaddr*) &host_addr, sizeof(host_addr));
        cout << "Wyslano zadanio o czas." << endl;
        return true;
    }
};

int main()
{
    DaytimeClient client;

    client.start();
    client.sendRequest("localhost", "3000");
    cout << "Otrzymano odpowiedz: " << client.receiveAnswer(2) << endl;
    client.stop();

    return 0;
}

