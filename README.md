# EAuto_CAN

Die Funktion dieser Bibliothek ist es die CAN Kommunikation sicher zustellen und dafür zu sorgen,
dass alle konfigurierten CAN-Nachrichten rechtzeitig versendet werden. Alle benötigten CAN-Nachrichten
müssen zu beginn des Programmes in der CAN_Output_Paket_Liste definiert werden.

Jeder Nachricht ist ein Array-Eintrag in dieser Liste zugewiesen und hat das folgende Format.

CAN_Output_Paket_Liste[x] = CAN-Nachricht(ID, Länge, Sendeintervall, Initiale Sendezeit, Sendeanzahl);

x ist hierbei der Listeneintrag. Die ID und Länge geben Aufschluss über Priorisierung und Länge
der Nachricht. Das Sendeintervall wird in Millisekunden angegeben und gibt an, mit welchem
Zeitintervall die Nachricht versendet wird. Die initiale Sendezeit gibt eine Zeitversatz zwischen
den Nachrichten an. Dies ist notwendig, um alle Nachrichten nacheinander zu versenden und Wartezeit
beim Verschicken der Nachrichten zu verhinden. Durch die Sendeanzahl kann noch ausgewählt werden,
ob eine Nachricht dauerhaft oder nicht versendet werden soll, bzw ob die Nachricht mit einer bestimmt
Anzahl versendet wird.

Bei 0 wird die Nachricht nicht gesendet. Ist der Eintrag 255 wir die Nachricht dauerhaft im angegebenen
Zeitintervall versendet. Liegt die Zahl dazwischen, ist dies die Anzahl der zu sendenden Nachrichten.
Z.B. 10, dann wird die CAN-Nachricht 10x auf dem CAN-Bus ausgegeben.