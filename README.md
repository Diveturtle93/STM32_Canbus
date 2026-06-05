# STM32 CAN-Bus
 
Eine in C implementierte CAN-Bus-Bibliothek für STM32-Mikrocontroller mit
interrupt-gesteuertem Senden und Empfangen über Ringpuffer. Die Bibliothek basiert
konzeptionell auf der FlexCAN-Bibliothek des Teensy 3.6 und ermöglicht die
zeitgesteuerte, automatische Übertragung einer konfigurierbaren Liste von CAN-Nachrichten.
 
## Beschreibung
 
Die Bibliothek stellt einen vollständigen CAN-Bus-Stack für den STM32 bereit.
Sende- und Empfangsnachrichten werden jeweils in einem Ringpuffer (FIFO)
zwischengespeichert, der per Interrupt befüllt und geleert wird. Dadurch gehen
keine Nachrichten verloren, wenn die Hardware-Mailboxen kurzzeitig belegt sind.
 
Das zentrale Konzept ist die `CAN_Output_PaketListe`: eine fest definierte Liste
aller CAN-Nachrichten, die das Gerät zyklisch auf den Bus sendet. Jeder Eintrag
enthält die Nachrichten-ID, die Datenlänge, das Sendeintervall und die Anzahl
der Sendungen. `CANwork()` wird in der Hauptschleife aufgerufen und sorgt dafür,
dass alle fälligen Nachrichten rechtzeitig versendet werden.
 
Die Bibliothek ist aktuell auf **CAN3** ausgelegt. Erweiterungen für CAN1 und CAN2
sind im Code als TODO-Kommentare vorgesehen.
 
## Dateien
 
| Datei       | Beschreibung                                                                    |
|-------------|---------------------------------------------------------------------------------|
| `canbus.h`  | Datentypen (`CAN_message_t`, `CAN_PaketTypeDef`, Ringpuffer), API-Definitionen  |
| `canbus.c`  | Implementierung inkl. Ringpuffer, Interrupt-Handler, Paketverwaltung            |
 
## Datentypen
 
### `CAN_message_t` – CAN-Nachricht
 
| Feld               | Typ        | Beschreibung                                            |
|--------------------|------------|---------------------------------------------------------|
| `id`               | `uint32_t` | CAN-Identifier (Standard 11-Bit oder Extended 29-Bit)   |
| `timestamp`        | `uint16_t` | Zeitstempel des Empfangs                                |
| `idhit`            | `uint8_t`  | Index des Filters, der die Nachricht erkannt hat        |
| `flags.extended`   | `uint8_t`  | 1 = Extended ID (29-Bit), 0 = Standard ID (11-Bit)     |
| `flags.remote`     | `uint8_t`  | 1 = Remote Transmit Request (RTR)                       |
| `flags.overrun`    | `uint8_t`  | 1 = Nachrichtenüberlauf aufgetreten                     |
| `len`              | `uint8_t`  | Datenlänge (0–8 Byte)                                   |
| `buf[8]`           | `uint8_t`  | Datenpuffer (max. 8 Byte)                               |
| `mb`               | `int8_t`   | Mailbox-Identifier                                      |
| `bus`              | `uint8_t`  | Bus-Identifier (1 = CAN3)                               |
| `seq`              | `bool`     | Sequentieller Rahmen                                    |
 
### `CAN_PaketTypeDef` – Sendepaket mit Zeitsteuerung
 
| Feld              | Typ              | Beschreibung                                                        |
|-------------------|------------------|---------------------------------------------------------------------|
| `msg`             | `CAN_message_t`  | Die eigentliche CAN-Nachricht mit ID, Länge und Datenpuffer         |
| `sendeintervall`  | `uint16_t`       | Sendeintervall in Millisekunden                                     |
| `sendetime`       | `uint32_t`       | Zeitstempel des letzten Sendevorgangs (für Intervallberechnung)     |
| `sendpossible`    | `uint8_t`        | Sendesteuerung: `0` = nicht senden, `255` = dauerhaft, sonst Anzahl |
 
## API
 
### Initialisierung und Betrieb
 
```c
void CANinit(RXQUEUE_TABLE rxSize, TXQUEUE_TABLE txSize);  // CAN-Bus und Ringpuffer initialisieren
void CANwork(void);                                        // Fällige Pakete aus der Paketliste senden
void CAN_config(void);                                     // CAN-Bus konfigurieren (Filter etc.)
void clearCAN(void);                                       // Datenpuffer aller Sendenachrichten auf 0 setzen
```
 
### Senden und Empfangen
 
```c
bool CANwrite(CAN_message_t *CAN_tx_msg, bool MB);         // Nachricht senden oder in Sendepuffer schreiben
uint8_t CAN_available(void);                               // Anzahl empfangener Nachrichten im Ringpuffer
bool CANread(CAN_message_t *CAN_rx_msg);                   // Nachricht aus Empfangspuffer lesen
```
 
### Paketverwaltung
 
```c
CAN_PaketTypeDef CAN_Nachricht(uint16_t id, uint8_t length,
    uint16_t sendeintervall, uint32_t sendetime,
    uint8_t sendpossible);                                 // Sendepaket erstellen
```
 
### Ringpuffer (intern)
 
```c
bool isInitialized(void);
void initializeBuffer(void);
void initRingBuffer(RingbufferTypeDef *ring, volatile CAN_message_t *buffer, uint32_t size);
bool addToRingBuffer(RingbufferTypeDef *ring, CAN_message_t *msg);
bool removeFromRingBuffer(RingbufferTypeDef *ring, CAN_message_t *msg);
bool isRingBufferEmpty(RingbufferTypeDef *ring);
```
 
## Ringpuffergröße
 
Empfangs- und Sendepuffer werden bei der Initialisierung mit `CANinit()` in einer der
folgenden Größen angelegt:
 
| Enum-Wert     | Anzahl Nachrichten |
|---------------|--------------------|
| `RX/TX_SIZE_2`   | 2                |
| `RX/TX_SIZE_4`   | 4                |
| `RX/TX_SIZE_8`   | 8                |
| `RX/TX_SIZE_16`  | 16               |
| `RX/TX_SIZE_32`  | 32               |
| `RX/TX_SIZE_64`  | 64               |
| `RX/TX_SIZE_128` | 128              |
| `RX/TX_SIZE_256` | 256              |
| `RX/TX_SIZE_512` | 512              |
| `RX/TX_SIZE_1024`| 1024             |
 
## Verwendung
 
### 1. Dateien einbinden
 
`canbus.h` und `canbus.c` in das STM32-Projekt kopieren und den Header einbinden:
 
```c
#include "canbus.h"
```
 
### 2. Anzahl der Sendepakete definieren
 
In `main.h` muss die Gesamtanzahl der konfigurierten Sendepakete festgelegt werden:
 
```c
#define ANZAHL_OUTPUT_PAKETE    5
```
 
### 3. Paketliste befüllen
 
Zu Beginn des Programms (vor der Hauptschleife) alle Sendepakete in die `CAN_Output_PaketListe` eintragen:
 
```c
// Format: CAN_Nachricht(ID, Länge, Intervall [ms], Startverzögerung [ms], Sendeanzahl)
 
CAN_Output_PaketListe[0] = CAN_Nachricht(0x100, 8, 10,  0, 255);  // alle 10 ms, dauerhaft
CAN_Output_PaketListe[1] = CAN_Nachricht(0x200, 8, 20,  2, 255);  // alle 20 ms, Versatz 2 ms
CAN_Output_PaketListe[2] = CAN_Nachricht(0x300, 8, 100, 5, 255);  // alle 100 ms, Versatz 5 ms
CAN_Output_PaketListe[3] = CAN_Nachricht(0x400, 2, 50,  0, 10);   // 10x senden, dann stop
CAN_Output_PaketListe[4] = CAN_Nachricht(0x500, 8, 50,  0, 0);    // nicht senden
```
 
**Sendeanzahl `sendpossible`:**
- `0` → Nachricht wird nicht gesendet
- `1–254` → Nachricht wird genau diese Anzahl gesendet, dann gestoppt
- `255` → Nachricht wird dauerhaft im angegebenen Intervall gesendet
**Startverzögerung `sendetime`:** Gibt einen zeitlichen Versatz in Millisekunden an,
mit dem die ersten Sendungen der einzelnen Pakete gestaffelt werden. Dies verhindert,
dass alle Nachrichten gleichzeitig auf den Bus gelangen.
 
### 4. `CAN_config` im eigenen Programm implementieren
 
Die Funktion `CAN_config()` ist in der Bibliothek als `__weak` definiert. Das bedeutet,
dass das Projekt auch ohne eigene Implementierung kompiliert wird – die leere
Standardimplementierung wird dann automatisch verwendet.
 
Im eigenen Programm **muss** `CAN_config()` jedoch definiert werden, um die
`CAN_Output_PaketListe` mit den gewünschten Sendepaketen zu befüllen:
 
```c
void CAN_config(void)
{
    CAN_Output_PaketListe[0] = CAN_Nachricht(0x100, 8, 10,  0, 255);
    CAN_Output_PaketListe[1] = CAN_Nachricht(0x200, 8, 20,  2, 255);
    // weitere Einträge ...
}
```
 
Diese Funktion wird einmalig von `CANinit()` aufgerufen, nachdem der Ringpuffer und der
CAN-Bus initialisiert wurden.
 
### 5. CAN-Bus initialisieren
 
```c
CANinit(RX_SIZE_64, TX_SIZE_32);
```
 
### 6. Hauptschleife
 
```c
while (1)
{
    // Fällige Pakete aus der Paketliste senden
    CANwork();
 
    // Empfangene Nachrichten verarbeiten
    if (CAN_available())
    {
        CAN_message_t msg;
        CANread(&msg);
        // msg.id, msg.buf, msg.len auswerten
    }
}
```
 
### 7. Nutzdaten aktualisieren
 
Der Datenpuffer einer Nachricht kann jederzeit direkt beschrieben werden:
 
```c
CAN_Output_PaketListe[0].msg.buf[0] = sensor_value_high;
CAN_Output_PaketListe[0].msg.buf[1] = sensor_value_low;
```
 
## Interrupt-Handler
 
Die Bibliothek überschreibt die HAL-Callback-Funktionen für CAN3. Diese müssen **nicht**
manuell aufgerufen werden – der STM32 HAL ruft sie automatisch aus dem Interrupt auf:
 
| Callback                            | Beschreibung                                          |
|-------------------------------------|-------------------------------------------------------|
| `HAL_CAN_TxMailbox0CompleteCallback` | Mailbox 0 frei → nächste Nachricht aus Sendepuffer    |
| `HAL_CAN_TxMailbox1CompleteCallback` | Mailbox 1 frei → nächste Nachricht aus Sendepuffer    |
| `HAL_CAN_TxMailbox2CompleteCallback` | Mailbox 2 frei → nächste Nachricht aus Sendepuffer    |
| `HAL_CAN_RxFifo0MsgPendingCallback`  | Empfangene Nachricht in Empfangspuffer schreiben      |
 
Die IRQ-Handler `CAN3_RX0_IRQHandler` und `CAN3_TX_IRQHandler` sind in der Bibliothek
implementiert und leiten an `HAL_CAN_IRQHandler` weiter. Der in `stm32xxxx_it.c`
generierte Handler für CAN3 muss daher **entfernt oder auskommentiert** werden.
 
## Quellen
 
Diese Bibliothek ist konzeptionell angelehnt an:
- [FlexCAN_T4 (Teensy)](https://github.com/tonton81/FlexCAN_T4)

## Abhängigkeiten
 
- `main.h` – STM32 HAL
- `can.h` – CAN-Handle (`hcan3`) und HAL-CAN-Definitionen
- `millis.h` – Zeitbasis für Intervallberechnung

## Lizenz
 
Dieses Projekt steht unter der [GPL-3.0 Lizenz](LICENSE).
