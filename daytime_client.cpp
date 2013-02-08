#include "daytime_client.h"

DaytimeClient::DaytimeClient(){
    buflen = 60;
    buf = new char[buflen];
}

DaytimeClient::~DaytimeClient(){
    delete [] buf;
}


bool DaytimeClient::start() {
    // dyrektywa preprocesora sprawdzajaca czy istnieje zdefiniowana zmienna globalna WIN32,
    // co sugeruje iż kompilowany plik binarny ma byc dla systemu z rodziny Windows
    #ifdef WIN32

        //zmienna przechowuje wersje biblioteki, ktora chcielibysmy wykorzystac
        WORD wVersionRequested;

        //struktura zawierajaca informacje o implementacji Windows Socket w systemie,
        //wypelniana przy pomocy funkcji WSAStartup
        WSADATA wsaData;

        //wykorzystujemy makro zdefiniowane w Windef.h
        //przy pomocy makra deklarujemy wersje biblioteki, ktora chcemy wykorzystac
        wVersionRequested = MAKEWORD(1,1);

        //funkcja podpina pod proces klienta windowsowe biblioteki Winsock
        //gdy biblioteki zostana pomyslnie dolaczone do procesu, funkcja zwroci 0
        if(WSAStartup(wVersionRequested, &wsaData) != 0) return false;
    #endif
        return true;
}

void DaytimeClient::stop() {
    // w przypadku gdy kompilujemy plik binarny dla systemu Windows
    #ifdef WIN32
        //zamykamy gniazdo sieciowe
        closesocket(sock);
        //funkcja odlacza biblioteke Winsock od procesu
        WSACleanup();
    #elif __linux
        //zamykamy gniazdo sieciowe
        close(sock);
    #endif
}

std::string DaytimeClient::receiveAnswer(int timeout_sec){

    //wskaznik na bufor
    char *pbuf = buf;
    //obiekt okreslajacy zbior gniazd na ktorych bedziemy wyczekiwac na pakiety
    fd_set rset;
    //zerujemy zbior rset
    FD_ZERO(&rset);
    //dodajemy nasz socket do zbioru rset
    FD_SET(sock, &rset);
    //zmienna do przechowywania ilosci gniazd na ktorych wystapilo zdarzenie
    int ret;

    //obiekt, ktory sluzy za okreslenie czasu oczekiwania na zdarzenie
    timeval tout = {timeout_sec, 0};

    //sprawdzamy na ilu gniazdach mozna czytac
    while((ret = select(sock+1, &rset, NULL, NULL, &tout)) > 0){

        //jesli otrzymano zdarzenie odczytu
        if (FD_ISSET(sock, &rset)){

            //odbieramy dane ze stosu przypisane do naszego socketu
            //zmienna n = ilosc otrzymanych pakietow
            int n = recv(sock, buf, buflen, 0);

            //jesli otrzymano pusty pakiet
            if (n==0) { continue; }

            //przesuwamy wskaznik o n znakow
            pbuf += n;
            //skracamy dlugosc wolnego miejsca w buforze
            buflen -= n;
            //ustawiamy na koniec nowego lancucha znakow,
            //znak o wartosci 0 - oznacza koniec lancucha znakow
            *pbuf=0;

            //jesli otrzymano poprawna date oraz czas
            if (this->isDateCorrent(buf)){
                //zwracamy wskaznik na otrzymana date
                return buf;
            }
        }
    }

    //jesli funkcja select() zwrocila blad
    if(ret == -1){
        std::cerr << "Wystapil blad funkcji select()" << std::endl;
    }
    return "Nie otrzymano poprawnego formatu daytime";
}

bool DaytimeClient::sendRequest(char *host){
    //struktura adresowa przystosowana dla protokołu IP w wersji 4,
    //odpowiednio spreparowana wersja sockaddr, ktora ulatwia dostep do czesci struktury
    //zwiazanej z rodzina protokolu, adresu oraz portu
    struct sockaddr_in host_addr;

    //struktura sluzaca do przechowywania informacji o danym hoscie
    //przechowuje takie informacje jak typ adresu, dlugosc adresu oraz sam adres hosta
    struct hostent *phe;

    //struktura do przechowywania informacji o usludze,
    //zawiera oficjalna nazwe uslugi, liste innych nazw uslugi, port uslugi, protokol na ktorym usluga dziala
    struct servent *pse;

    //tworzymy gniazdo dla UDP
    //funkcja zwraca identyfikator utworzonego gniazda sieciowego
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // jesli socket zostal utworzony
    if (sock>=0)
       std::cout << "UDP OK" << '\n';

    //ustawiamy rodzine adresu
    host_addr.sin_family=AF_INET;

    //szukamy uslugi po nazwie,
    //jesli jest, to ustawiamy jej port,
    //jesli nie ma to ustawiamy nasz port
    if((pse = getservbyname("daytime", "udp")) == NULL)
            host_addr.sin_port=htons(13);
    else host_addr.sin_port=pse->s_port;

    //wyswietlamy adres ip hostu oraz port do ktorego wyslemy pusty pakiet
    std::cout <<"Adres IP: " << host << std::endl;
    std::cout <<"Port: " << ntohs(host_addr.sin_port) << std::endl;

    //wpisujemy adres hostu do struktury
    //inet_addr zamienia nasz zapis adresu hosta na liczbe
    host_addr.sin_addr.s_addr=inet_addr(host);

    //jesli zamiana adresu funkcja inet_addr nie powiodla sie
    if (host_addr.sin_addr.s_addr == INADDR_NONE) {
        //jesli nie znaleziono hosta
        if ((phe=gethostbyname(host)) == NULL) {
            std::cout << "Nie znaleziono hosta" << std::endl;
            return false;
        }
        //kopiujemy caly adres z h_addr do s_addr
        memcpy(&host_addr.sin_addr.s_addr, &phe->h_addr[0], sizeof(host_addr.sin_addr.s_addr));
    }

    //wysylamy pusty pakiet pod adres serwera daytime
    sendto(sock, NULL, 0, 0, (sockaddr*) &host_addr, sizeof(host_addr));
    std::cout << "Wyslano pytanie o czas." << std::endl;

    //zwracamy true - wszystko sie powiodlo
    return true;
}

bool DaytimeClient::isDateCorrent(std::string daytime){
    //wyrazenie regularne okreslajace prawidlowa strukture daty i czasu w standardzie ISO8601
    //regex e("\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}Z");

    //sprawdzanie otrzymanego daytime - czy poprawne
    return daytime.length() == 20;//regex_match(daytime, e);
}
