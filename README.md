# Dokumentacja projektu: Symulator bankomatu - "The Limitless Bank"

Autor: Szymon Król

## 1. Wstęp

Projekt stanowi symulację systemu bankowego opartego na architekturze klient-serwer. Aplikacja umożliwia użytkownikom
zdalne zarządzanie kontem bankowym (logowanie, sprawdzanie salda, wpłaty, wypłaty oraz przelewy). System został
zaprojektowany z myślą o bezpiecznej obsłudze wielu klientów jednocześnie, dbając o spójność bazy danych zapisanej w
plikach binarnych.

## 2. Architektura i wykorzystane konstrukcje

### Komunikacja sieciowa (Sokety)

Podstawą komunikacji między klientem a serwerem są sokety komunikacji sieciowej.

* **W pliku `server.h`/`server.c`**: Serwer tworzy gniazdo, wiąże je z portem i przechodzi w stan
  nasłuchiwania.
* **W pliku `client.c`**: Klient nawiązuje połączenie z serwerem za pomocą funkcji `connect`.
* **Protokół**: Implementacja w `communication.c` definiuje własny sposób przesyłania danych. Każda wiadomość
  poprzedzona jest rozmiarem (`ssize_t`), co pozwala na przesyłanie dynamicznych struktur danych i ciągów znaków o
  zmiennej długości.

### Wielowątkowość (Pthreads)

Serwer jest wielowątkowy, co pozwala na równoległą obsługę wielu klientów.

* **Tworzenie wątków**: Każde nowe połączenie skutkuje utworzeniem nowego wątku, który
  deleguje obsługę sesji do funkcji `connection_handler`.

### Synchronizacja i bezpieczeństwo danych

Ze względu na współbieżny dostęp do plików bazy danych, zaimplementowano mechanizmy blokowania:

* **Mutexy**: Globalny licznik aktywnych połączeń (`active_threads`) jest chroniony za pomocą `pthread_mutex_t`, co
  zapobiega wyścigom podczas monitorowania stanu serwera.
* **Blokowanie rekordów**: W module `db_manager.c` wykorzystano funkcję `fcntl`.
  System blokuje konkretne zakresy bajtów (rekordy), co pozwala wielu
  użytkownikom pracować na bazie danych jednocześnie, o ile nie modyfikują tego samego konta.
* **Unikanie zakleszczeń**: Przy operacjach przelewu, gdzie wymagane jest zablokowanie dwóch kont
  jednocześnie, system zawsze zakłada blokady w kolejności rosnących identyfikatorów kont.

### Zarządzanie plikami i trwałość danych

System przechowuje dane w plikach binarnych (`.db`).

* **Operacje wejścia/wyjścia**: Wykorzystano niskopoziomowe funkcje systemowe takie jak `open`, `close`, `pread` oraz
  `pwrite`.
* **Zarządzanie strukturą**: Pliki posiadają nagłówek przechowujący liczbę rekordów, co pozwala na szybkie obliczanie
  offsetów dla konkretnych ID użytkowników.

### Obsługa sygnałów

Aplikacja została wyposażona w mechanizm „bezpiecznego wyłączania” (Graceful Shutdown).

* **SIGINT**: Serwer przechwytuje sygnał przerwania (Ctrl+C) za pomocą `sigaction`.
* Wątki robocze mają zablokowaną obsługę sygnału `SIGINT` (`pthread_sigmask`), dzięki czemu tylko główny
  wątek steruje procesem zamykania, czekając na zakończenie wszystkich aktywnych sesji przed ostatecznym wyjściem z
  programu.

### Dynamiczne zarządzanie pamięcią

W całym projekcie (szczególnie w logice komunikacji i dostępu do bazy danych) stosowana jest dynamiczna alokacja
pamięci (`malloc`, `free`). Pozwala to na elastyczne wczytywanie rekordów użytkowników oraz odbieranie komunikatów
sieciowych o dowolnym rozmiarze bez ryzyka przepełnienia bufora.

## 3. Jak korzystać

### Budowanie projektu

```bash
make
```

### Uruchomienie serwera

```bash
./TheLimitlessBankSimulator server
```

Serwer uruchamia się na porcie `8080`. Aby go zatrzymać, naciśnij `Ctrl+C` — serwer poczeka na zakończenie wszystkich aktywnych sesji przed wyłączeniem.

### Uruchomienie klienta

W osobnym terminalu:

```bash
./TheLimitlessBankSimulator client
```

### Dostępne operacje

Po zalogowaniu dostępne są następujące opcje:

| Opcja | Opis            |
|-------|-----------------|
| `1`   | Sprawdź saldo   |
| `2`   | Wpłać środki    |
| `3`   | Wypłać środki   |
| `4`   | Wykonaj przelew |
| `0`   | Wyjdź           |