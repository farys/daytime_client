#include <iostream>

#include "daytime_client.h"

using namespace std;

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
