# EAuto_CAN
---
Die Funktion dieser Bibliothek ist es die CAN Kommunikation sicher zustellen und dafür zu sorgen,
dass alle konfigurierten CAN-Nachrichten rechtzeitig versendet werden. Alle benötigten CAN-Nachrichten
müssen zu beginn des Programmes in der CAN_Output_Paket_Liste definiert werden.

Jede Nachricht ist ein Array-Eintrag und hat das folgende Format.

CAN_Output_Paket_Liste[x] = CAN-Nachricht(ID, Länge, Sendeintervall, Initiale Sendezeit);

x ist hierbei der Listeneintrag. Die ID und Länge geben Aufschluss über die Nachricht und Länge
der Nachricht. Das Sendeintervall wird in Millisekunden angegeben und gibt an, mit welchem
Zeitintervall die Nachricht versendet wird. Die initiale Sendezeit gibt eine Zeitversatz zwischen
den Nachrichten an. Dies ist notwendig, um alle Nachrichten nacheinander zu versenden und Wartezeit
beim Verschicken der Nachrichten zu verhinden.