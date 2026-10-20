# Changelog

Alle nennenswerten Änderungen an diesem Projekt werden hier dokumentiert.
Format nach [Keep a Changelog](https://keepachangelog.com/de/1.1.0/),
Versionierung nach [Semantic Versioning](https://semver.org/lang/de/).

## [Unveröffentlicht]

### Hinzugefügt
- Erste versionierte Veröffentlichung der CAN-Bus-Bibliothek für CAN3.
- `CANinit()` legt Empfangs- und Sendepuffer in wählbarer Größe (2 bis 1024 Nachrichten) an und initialisiert CAN3.
- `CANwrite()` sendet eine Nachricht über eine freie Mailbox oder legt sie im Sendepuffer ab, der per Interrupt geleert wird.
- `CANread()` und `CAN_available()` lesen empfangene Nachrichten aus dem Empfangspuffer, der per Interrupt befüllt wird.
- Paketliste `CAN_Output_PaketListe` mit `CAN_Nachricht()` zum Anlegen von Sendepaketen mit Intervall, Startversatz und Sendeanzahl.
- `CANwork()` versendet in der Hauptschleife alle fälligen Pakete der Paketliste.
- `clearCAN()` setzt die Datenpuffer aller Sendepakete auf 0.
- `CAN_config()` als `__weak`-Funktion zum Befüllen der Paketliste im eigenen Projekt, dazu die Vorlage `examples/can_config.c.template`.
- Ringpuffer als eigenes Modul `canbus_ringbuffer.c` / `canbus_ringbuffer.h`.
- Versions-Defines `CANBUS_MAJOR`, `CANBUS_MINOR`, `CANBUS_PATCH` und `CANBUS_DEV` in `canbus.h`.
- `ARCHITECTURE.md` beschreibt Module, Datenfluss beim Senden und Empfangen, Speicherbedarf, Nebenläufigkeit und die Erweiterung um weitere Pakete und CAN-Busse.
