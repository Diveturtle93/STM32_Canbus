# STM32 CAN-Bus

Eine in C implementierte CAN-Bus-Bibliothek für STM32-Mikrocontroller mit
interrupt-gesteuertem Senden und Empfangen über Ringpuffer. Die Bibliothek basiert
konzeptionell auf der FlexCAN-Bibliothek des Teensy 3.6 und ermöglicht die
zeitgesteuerte, automatische Übertragung einer konfigurierbaren Liste von CAN-Nachrichten.

## Beschreibung

Die Bibliothek setzt auf dem STM32 HAL (bxCAN) auf. Sende- und Empfangsnachrichten
werden jeweils in einem Ringpuffer (FIFO) zwischengespeichert, der per Interrupt
befüllt und geleert wird. Sind alle drei Hardware-Mailboxen belegt, landet eine
Sendenachricht im Sendepuffer und wird verschickt, sobald eine Mailbox frei wird.

Das zentrale Konzept ist die `CAN_Output_PaketListe`: eine fest definierte Liste
aller CAN-Nachrichten, die das Gerät zyklisch auf den Bus sendet. Jeder Eintrag
enthält die Nachrichten-ID, die Datenlänge, das Sendeintervall und die Anzahl
der Sendungen. Die Liste wird in `CAN_config()` im eigenen Projekt befüllt.
`CANwork()` wird in der Hauptschleife aufgerufen und versendet alle fälligen Nachrichten.

Besonderheiten:

- Ringpuffer für Senden und Empfangen, Größe jeweils von 2 bis 1024 Nachrichten wählbar
- zyklisches Senden mit Intervall, Startversatz und begrenzter oder unbegrenzter Sendeanzahl
- Standard-IDs (11 Bit) und Extended-IDs (29 Bit)
- aktuell nur **CAN3**, Erweiterungen für CAN1 und CAN2 sind im Code als TODO vorgesehen

Wie die Module zusammenspielen, welche Teile im Interrupt laufen und wie die Puffer
geschützt sind, beschreibt [ARCHITECTURE.md](ARCHITECTURE.md).

## Dateien

| Datei                                                                | Beschreibung                                                                      |
|----------------------------------------------------------------------|-----------------------------------------------------------------------------------|
| `canbus.h`                                                           | Datentypen (`CAN_message_t`, `CAN_PaketTypeDef`), Puffergrößen, Version, API      |
| `canbus.c`                                                           | Implementierung inkl. HAL-Callbacks, IRQ-Handler und Paketverwaltung              |
| `canbus_ringbuffer.h`                                                | Ringpuffer-Typ `RingbufferCANBusTypeDef` und Funktionen                           |
| `canbus_ringbuffer.c`                                                | Ringpuffer für Sende- und Empfangsnachrichten (FIFO)                              |
| [`examples/can_config.c.template`](examples/can_config.c.template)   | Vorlage für `CAN_config()`, wird als `can_config.c` in das eigene Projekt kopiert |

## Hardware-Konfiguration

Einstellungen in STM32CubeMX:

| Einstellung                          | Wert                                                                   |
|--------------------------------------|------------------------------------------------------------------------|
| Connectivity → CAN3                  | aktiviert, Bitrate nach Bedarf                                         |
| NVIC → CAN3 TX interrupts            | aktiviert                                                              |
| NVIC → CAN3 RX0 interrupts           | aktiviert                                                              |
| NVIC → Code generation               | „Generate IRQ handler“ für CAN3 TX und CAN3 RX0 **deaktivieren**       |

Die IRQ-Handler `CAN3_TX_IRQHandler` und `CAN3_RX0_IRQHandler` sind in der Bibliothek
implementiert. Erzeugt CubeMX sie zusätzlich in `stm32xxxx_it.c`, sind sie doppelt definiert.

`CANinit()` ruft nur `MX_CAN3_Init()` auf. Filter, Start des Busses und
Empfangsinterrupt werden im Bereich `USER CODE BEGIN CAN3_Init 2` in `can.c` eingerichtet:

```c
/* USER CODE BEGIN CAN3_Init 2 */
	CAN_FilterTypeDef sFilterConfig;

	// Starte CAN-Bus
	if (HAL_CAN_Start(&hcan3) != HAL_OK)
	{
		Error_Handler();
	}

	// Aktiviere Empfangsinterrupt
	if (HAL_CAN_ActivateNotification(&hcan3, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		Error_Handler();
	}

	// Akzeptiere alle CAN-Pakete
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0;
	sFilterConfig.FilterIdLow = 0x0;
	sFilterConfig.FilterMaskIdHigh = 0x0;
	sFilterConfig.FilterMaskIdLow = 0x0;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;

	if (HAL_CAN_ConfigFilter(&hcan3, &sFilterConfig) != HAL_OK)
	{
		Error_Handler();
	}
/* USER CODE END CAN3_Init 2 */
```

## Konfigurationsparameter

| Parameter              | Ort      | Beschreibung                                                      |
|------------------------|----------|-------------------------------------------------------------------|
| `ANZAHL_OUTPUT_PAKETE` | `main.h` | Anzahl der Einträge in `CAN_Output_PaketListe`, muss gesetzt sein |

## Datentypen

### `CAN_message_t` – CAN-Nachricht

| Feld               | Typ        | Beschreibung                                            |
|--------------------|------------|---------------------------------------------------------|
| `id`               | `uint32_t` | CAN-Identifier (Standard 11 Bit oder Extended 29 Bit)   |
| `timestamp`        | `uint16_t` | Zeitstempel des Empfangs                                |
| `idhit`            | `uint8_t`  | Filter, der die Nachricht erkannt hat (derzeit ungenutzt) |
| `flags.extended`   | `uint8_t`  | 1 = Extended ID (29 Bit), 0 = Standard ID (11 Bit)      |
| `flags.remote`     | `uint8_t`  | 1 = Remote Transmit Request (RTR)                       |
| `flags.overrun`    | `uint8_t`  | 1 = Nachrichtenüberlauf aufgetreten                     |
| `len`              | `uint8_t`  | Datenlänge (0–8 Byte)                                   |
| `buf[8]`           | `uint8_t`  | Datenpuffer (max. 8 Byte)                               |
| `mb`               | `int8_t`   | Beim Empfang: Index des Filters, der getroffen hat      |
| `bus`              | `uint8_t`  | Bus-Identifier (1 = CAN3)                               |
| `seq`              | `bool`     | Sequentieller Rahmen                                    |

### `CAN_PaketTypeDef` – Sendepaket mit Zeitsteuerung

| Feld              | Typ              | Beschreibung                                                          |
|-------------------|------------------|-----------------------------------------------------------------------|
| `msg`             | `CAN_message_t`  | Die eigentliche CAN-Nachricht mit ID, Länge und Datenpuffer           |
| `sendeintervall`  | `uint16_t`       | Sendeintervall in Millisekunden                                       |
| `sendetime`       | `uint32_t`       | Zeitpunkt des letzten Sendevorgangs, beim Anlegen der Startversatz    |
| `sendpossible`    | `uint8_t`        | Sendesteuerung: `0` = nicht senden, `255` = dauerhaft, sonst Anzahl   |

### Ringpuffergröße

Empfangs- und Sendepuffer werden bei `CANinit()` mit `malloc` in einer der folgenden
Größen angelegt (`RXQUEUE_TABLE` bzw. `TXQUEUE_TABLE`). Ein Puffer der Größe N fasst
N − 1 Nachrichten.

| Enum-Wert                         | Größe |
|-----------------------------------|-------|
| `RX_SIZE_2` / `TX_SIZE_2`         | 2     |
| `RX_SIZE_4` / `TX_SIZE_4`         | 4     |
| `RX_SIZE_8` / `TX_SIZE_8`         | 8     |
| `RX_SIZE_16` / `TX_SIZE_16`       | 16    |
| `RX_SIZE_32` / `TX_SIZE_32`       | 32    |
| `RX_SIZE_64` / `TX_SIZE_64`       | 64    |
| `RX_SIZE_128` / `TX_SIZE_128`     | 128   |
| `RX_SIZE_256` / `TX_SIZE_256`     | 256   |
| `RX_SIZE_512` / `TX_SIZE_512`     | 512   |
| `RX_SIZE_1024` / `TX_SIZE_1024`   | 1024  |

## API

### Initialisierung und Betrieb

```c
void CANinit (RXQUEUE_TABLE rxSize, TXQUEUE_TABLE txSize);   // Ringpuffer anlegen und CAN3 initialisieren
void CAN_config (void);                                      // Paketliste befuellen, im eigenen Projekt implementieren
void CANwork (void);                                         // Faellige Pakete aus der Paketliste senden
void clearCAN (void);                                        // Datenpuffer aller Sendepakete auf 0 setzen
```

### Senden und Empfangen

```c
bool CANwrite (CAN_message_t *CAN_tx_msg, bool MB);          // Nachricht senden, MB = true: nicht puffern
uint8_t CAN_available (void);                                // Anzahl empfangener Nachrichten im Ringpuffer
bool CANread (CAN_message_t *CAN_rx_msg);                    // Nachricht aus Empfangspuffer lesen
```

`CANwrite()` legt die Nachricht in eine freie Mailbox. Ist keine frei, wird sie bei
`MB = false` in den Sendepuffer geschrieben, bei `MB = true` verworfen. Rückgabe `false`,
wenn die Nachricht weder gesendet noch gepuffert werden konnte.

### Paketverwaltung

```c
CAN_PaketTypeDef CAN_Nachricht (uint16_t id, uint8_t length,
	uint16_t sendeintervall, uint32_t sendetime,
	uint8_t sendpossible);                                   // Sendepaket fuer die Paketliste erzeugen
```

### Intern

Diese Funktionen nutzt die Bibliothek selbst, im eigenen Programm werden sie nicht benötigt:

```c
bool isInitialized (void);                                   // Abfrage, ob Ringpuffer angelegt
void initializeBuffer (void);                                // Empfangs- und Sendepuffer anlegen
void canbus_ringbuffer_init (RingbufferCANBusTypeDef *ring, volatile CAN_message_t *buffer, uint32_t size);
bool canbus_addToRingBuffer (RingbufferCANBusTypeDef *ring, CAN_message_t *msg);
bool canbus_removeFromRingBuffer (RingbufferCANBusTypeDef *ring, CAN_message_t *msg);
bool canbus_isRingBufferEmpty (RingbufferCANBusTypeDef *ring);
```

## Verwendung

### 1. Dateien einbinden

`canbus.h`, `canbus.c`, `canbus_ringbuffer.h` und `canbus_ringbuffer.c` in das
STM32-Projekt kopieren (oder das Repository als Submodul einbinden) und den Header einbinden:

```c
#include "canbus.h"
```

### 2. Anzahl der Sendepakete festlegen

In `main.h`:

```c
#define ANZAHL_OUTPUT_PAKETE					5
```

### 3. `CAN_config()` implementieren

`examples/can_config.c.template` als `can_config.c` in das eigene Projekt kopieren und
dort die `CAN_Output_PaketListe` befüllen. Die Endung `.template` sorgt dafür, dass die
Vorlage beim Einbinden als Submodul nicht mitkompiliert wird.

```c
void CAN_config (void)
{
	// Format: CAN_Nachricht(ID, Laenge, Intervall [ms], Startversatz [ms], Sendeanzahl)
	CAN_Output_PaketListe[0] = CAN_Nachricht(0x100, 8, 10,  0, 255);	// alle 10 ms, dauerhaft
	CAN_Output_PaketListe[1] = CAN_Nachricht(0x200, 8, 20,  2, 255);	// alle 20 ms, Versatz 2 ms
	CAN_Output_PaketListe[2] = CAN_Nachricht(0x300, 8, 100, 5, 255);	// alle 100 ms, Versatz 5 ms
	CAN_Output_PaketListe[3] = CAN_Nachricht(0x400, 2, 50,  0, 10);	// 10x senden, dann stopp
	CAN_Output_PaketListe[4] = CAN_Nachricht(0x500, 8, 50,  0, 0);		// nicht senden
}
```

**Sendeanzahl `sendpossible`:**

- `0` → Nachricht wird nicht gesendet
- `1–254` → Nachricht wird genau so oft gesendet, danach gestoppt
- `255` → Nachricht wird dauerhaft im angegebenen Intervall gesendet

**Startversatz `sendetime`:** Zeitlicher Versatz in Millisekunden, mit dem die ersten
Sendungen der einzelnen Pakete gestaffelt werden. So gelangen nicht alle Nachrichten
gleichzeitig auf den Bus.

`CAN_config()` ist in der Bibliothek als `__weak` mit leerem Rumpf definiert, damit das
Projekt auch ohne eigene Implementierung kompiliert. Ohne eigene Implementierung bleibt
die Paketliste leer.

### 4. CAN-Bus initialisieren

`CANinit()` legt die Ringpuffer an und initialisiert CAN3. `CAN_config()` wird **nicht**
automatisch aufgerufen und folgt direkt danach:

```c
CANinit(RX_SIZE_64, TX_SIZE_32);
CAN_config();
```

### 5. Hauptschleife

```c
while (1)
{
	// Faellige Pakete aus der Paketliste senden
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

### 6. Nutzdaten aktualisieren

Der Datenpuffer eines Sendepakets kann jederzeit direkt beschrieben werden, `CANwork()`
sendet beim nächsten Intervall den aktuellen Inhalt:

```c
CAN_Output_PaketListe[0].msg.buf[0] = sensorHigh;
CAN_Output_PaketListe[0].msg.buf[1] = sensorLow;
```

## Hinweise

### Interrupt-Callbacks

Die Bibliothek implementiert die folgenden HAL-Callbacks. Sie werden vom HAL aus dem
Interrupt aufgerufen und dürfen im eigenen Projekt nicht noch einmal definiert werden:

| Callback                             | Beschreibung                                          |
|--------------------------------------|-------------------------------------------------------|
| `HAL_CAN_TxMailbox0CompleteCallback` | Mailbox 0 frei → nächste Nachricht aus Sendepuffer    |
| `HAL_CAN_TxMailbox1CompleteCallback` | Mailbox 1 frei → nächste Nachricht aus Sendepuffer    |
| `HAL_CAN_TxMailbox2CompleteCallback` | Mailbox 2 frei → nächste Nachricht aus Sendepuffer    |
| `HAL_CAN_RxFifo0MsgPendingCallback`  | Empfangene Nachricht in Empfangspuffer schreiben      |

### Einschränkungen

- Nur CAN3 wird unterstützt.
- `CAN_available()` liefert `uint8_t`. Bei Empfangspuffern über 256 Einträgen kann der Wert überlaufen.
- `CAN_Nachricht()` setzt nur ID, Länge und Zeitsteuerung. `msg.flags` und `msg.buf` sind
  danach nicht initialisiert. `buf` lässt sich mit `clearCAN()` nullen, `flags.extended`
  muss vor dem ersten Senden gesetzt werden (`0` für Standard-IDs).
- Die Ringpuffer liegen auf dem Heap. Der Heap im Linker-Skript muss beide Puffer fassen,
  der Rückgabewert von `malloc` wird nicht geprüft.
- Ist der Empfangspuffer voll, werden neue Nachrichten verworfen. `flags.overrun` wird
  dabei nicht gesetzt.
- `CANwork()` vergleicht `millis()` mit `sendetime + sendeintervall`. Nach dem Überlauf von
  `millis()` (etwa 49 Tage Laufzeit) stimmen die Sendeintervalle nicht mehr.

### Eingesetzt in

- [EAuto_BMS](https://github.com/Diveturtle93/EAuto_BMS)
- [EAuto_Motor](https://github.com/Diveturtle93/EAuto_Motor)

## Quellen

Diese Bibliothek ist konzeptionell angelehnt an:

- [FlexCAN_T4 (Teensy)](https://github.com/tonton81/FlexCAN_T4)

## Abhängigkeiten

- `main.h` – STM32 HAL, Definition von `ANZAHL_OUTPUT_PAKETE`
- `can.h` – CAN-Handle `hcan3` und `MX_CAN3_Init()`, von CubeMX erzeugt
- `millis.h` – Zeitbasis für die Sendeintervalle, [STM32_Millis](https://github.com/Diveturtle93/STM32_Millis)

## Lizenz

Dieses Projekt steht unter der [GPL-3.0 Lizenz](LICENSE).
