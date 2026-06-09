//----------------------------------------------------------------------
// Titel	:	canbus_ringbuffer.h
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	10.09.2025
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	CAN-Bus
// Quelle	:	
//----------------------------------------------------------------------

// Saveguard symbol
//----------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------

// Dateiheader definieren
//----------------------------------------------------------------------
#ifndef INC_CANBUS_RINGBUFFER_H_
#define INC_CANBUS_RINGBUFFER_H_
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der eigenen Include Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Version definieren
//----------------------------------------------------------------------
#define CANBUS_RINGBUFFER_MAJOR					0
#define CANBUS_RINGBUFFER_MINOR					0
#define CANBUS_RINGBUFFER_PATCH					0
#define CANBUS_RINGBUFFER_DEV					0
//----------------------------------------------------------------------

// Typedefine fuer Rigbuffer
//----------------------------------------------------------------------
typedef struct
{
	volatile uint16_t head;													// Kopf des Ringbusses
	volatile uint16_t tail;													// Schwanz des Ringbusses
	uint16_t size;															// Groesse des Ringbusses
	volatile CAN_message_t *buffer;											// Nachrichtenbuffer
} RingbufferCANBusTypeDef;
//----------------------------------------------------------------------

// Funktionen definieren
//----------------------------------------------------------------------
void canbus_ringbuffer_init (RingbufferCANBusTypeDef *ring, volatile CAN_message_t *buffer, uint32_t size);	// Ringpuffer initialisieren
bool canbus_addToRingBuffer (RingbufferCANBusTypeDef *ring, CAN_message_t *msg);	// Nachricht zu Ringpuffer hinzufuegen
bool canbus_removeFromRingBuffer (RingbufferCANBusTypeDef *ring, CAN_message_t *msg);	// Nachricht von Ringpuffer loeschen
bool canbus_isRingBufferEmpty (RingbufferCANBusTypeDef *ring);				// Abfrage, ob Ringpuffer leer ist
//----------------------------------------------------------------------

#endif /* INC_CANBUS_RINGBUFFER_H_ */
//----------------------------------------------------------------------
