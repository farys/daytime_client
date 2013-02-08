#ifndef DAYTIME_CLIENT_H
#define DAYTIME_CLIENT_H
#include <iostream>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <time.h>
#include <cstdio>

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
    DaytimeClient();

    /**
     * Destruktor klasy, zwalnia pamiec zajmowana przez atrybuty.
     */
    ~DaytimeClient();

    /**
     * Metoda sluzaca do przydzielenia bibliotek, uslug, badz zresetowania atrybutow klasy, dla prawidlowego rozpoczecia dzialania klienta.
     * Zwraca true jesli wszystko sie powiedzie
     */
    bool start();

    /**
     * Metoda sluzaca do oddelegowania wymaganej biblioteki winsock,
     * w przypadku kompilacji pod system Windows
     */
    void stop();

    /**
     *  Metoda odbiera pakiet UDP zawierajacy date i czas w formacie ISO8601
     *  @param timeout_sec okresla ilosc sekund oczekiwania na nadejscie pakietu
     *  @return data i czas w formacie ISO8601
     */
    std::string receiveAnswer(int timeout_sec);

    /**
    * Wysyła zadanie o czas do serwera Daytime
    * @param host adres serwera Daytime
    * @return funkcja zwraca wartosc true w przypadku pomyslnego wyslania pakietu, w przeciwnym razie false
    */
    bool sendRequest(char *host);

    /**
     * Metoda sprawdza czy podana data jest w formacie ISO8601
     * @param daytime String zawierajacy date i czas
     * @return Zwraca true jesli data jest w poprawnym formacie, w przeciwnym przypadku falsz
     */
    bool isDateCorrent(std::string daytime);
};

#endif // DAYTIME_CLIENT_H
