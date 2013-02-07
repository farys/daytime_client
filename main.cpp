#include <iostream>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <cstdio>
#include <unistd.h>

//naglowki do windowsa
#ifdef WIN32
    #include <winsock2.h>
#else
    //naglowki do systemu linux
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#endif

using namespace std;
/**
 * Główna klasa klienta
 */
class DaytimeClient{
private:
    /** atrybut klasy do identyfikacji utworzonego gniazda sieciowego */
    int sock;

    /** dlugosc tablicy znakow buf */
    int buflen;

    /** tablica znakow dla przechowywania zawartosci otrzymywanych paczek UDP */
    char *buf;
public:

    /**
     * Konstruktor klasy, ustawawia domyślne wartosci atrybutow klasy.
     */
    DaytimeClient(){
        buflen = 60;
        buf = new char[buflen];
    }

    /**
     * Destruktor klasy, zwalnia pamiec zajmowana przez atrybuty.
     */
    ~DaytimeClient(){
        delete [] buf;
    }

    /**
     * Metoda sluzaca do przydzielenia bibliotek, uslug, badz zresetowania atrybutow klasy, dla prawidlowego rozpoczecia dzialania klienta.
     * Zwraca true jesli wszystko sie powiedzie
     */
    bool start() {
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

    /**
     * Metoda sluzaca do oddelegowania wymaganej biblioteki winsock,
     * w przypadku kompilacji pod system Windows
     */
    void stop() {
        // w przypadku gdy kompilujemy plik binarny dla systemu Windows
        #ifdef WIN32
            //funkcja odlacza biblioteke Winsock od procesu
            WSACleanup();
        #endif
    }

    /**
     *  Metoda odbiera pakiet UDP zawierajacy date i czas w formacie ISO8601
     *  @param timeout_sec okresla ilosc sekund oczekiwania na nadejscie pakietu
     *  @return data i czas w formacie ISO8601
     */
    string receiveAnswer(int timeout_sec){

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
            std::cerr << "Wystapil blad funkcji select()" << endl;
        }
        return "Nie otrzymano poprawnego formatu daytime";
    }

    /**
    * Wysyła zadanie o czas do serwera Daytime
    * @param host adres serwera Daytime
    * @return funkcja zwraca wartosc true w przypadku pomyslnego wyslania pakietu, w przeciwnym razie false
    */
    bool sendRequest(char *host){
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
           cout << "UDP OK" << '\n';

        //ustawiamy rodzine adresu
        host_addr.sin_family=AF_INET;

        //szukamy uslugi po nazwie,
        //jesli jest, to ustawiamy jej port,
        //jesli nie ma to ustawiamy nasz port
        if((pse = getservbyname("13", "udp")) == NULL)
                host_addr.sin_port=htons(13);
        else host_addr.sin_port=pse->s_port;

        //wyswietlamy adres ip hostu oraz port do ktorego wyslemy pusty pakiet
        cout <<"Adres IP: " << host << endl;
        cout <<"Port: " << ntohs(host_addr.sin_port) << endl;

        //wpisujemy adres hostu do struktury
        //inet_addr zamienia nasz zapis adresu hosta na liczbe
        host_addr.sin_addr.s_addr=inet_addr(host);

        //jesli zamiana adresu funkcja inet_addr nie powiodla sie
        if (host_addr.sin_addr.s_addr == INADDR_NONE) {
            //jesli nie znaleziono hosta
            if ((phe=gethostbyname(host)) == NULL) {
                cout << "Nie znaleziono hosta" << endl;
                return false;
            }
            //kopiujemy caly adres z h_addr do s_addr
            memcpy(&host_addr.sin_addr.s_addr, &phe->h_addr[0], sizeof(host_addr.sin_addr.s_addr));
        }

        //wysylamy pusty pakiet pod adres serwera daytime
        sendto(sock, NULL, 0, 0, (sockaddr*) &host_addr, sizeof(host_addr));
        cout << "Wyslano pytanie o czas." << endl;

        //zwracamy true - wszystko sie powiodlo
        return true;
    }

    /**
     * Metoda sprawdza czy podana data jest w formacie ISO8601
     * @param daytime String zawierajacy date i czas
     * @return Zwraca true jesli data jest w poprawnym formacie, w przeciwnym przypadku falsz
     */
    bool isDateCorrent(string daytime){
        //wyrazenie regularne okreslajace prawidlowa strukture daty i czasu w standardzie ISO8601
        //regex e("\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}Z");

        //sprawdzanie otrzymanego daytime - czy poprawne
        return daytime.length() == 20;//regex_match(daytime, e);
    }
};

/**
 * Główna funkcja programu
 * @param argc Ilosc argumentow
 * @param argv Tablica argumentow
 */
int main(int argc, char *argv[])
{
    //deklarujemy obiekt client
    DaytimeClient client;

    //domyslny adres hosta
    char *host = (char*)"localhost";

    //jesli beda przynajmniej 2 parametry przy wywolaniu programu
    //to 2 parametr ustawia jako adres hosta
    if(argc >= 2)
        host = argv[1];

    //startujemy klienta
    client.start();

    //wysylamy zadanie o date i czas pod adres z zmiennej host
    client.sendRequest(host);

    //czekamy do 2s na odpowiedz, po czym ja wyswietlamy
    cout << "Otrzymano odpowiedz: " << client.receiveAnswer(2) << endl;

    //wylaczamy klienta
    client.stop();

    return 0;
}
