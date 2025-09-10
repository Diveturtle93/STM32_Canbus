//----------------------------------------------------------------------
// Titel	:	canbus_ringbuffer.c
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	10.09.2025
// Version	:	1.0
// Autor	:	Diveturtle93
// Projekt	:	CAN-Bus
// Quelle	:	
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------
#include <string.h>
//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------
#include "main.h"
//----------------------------------------------------------------------

// Einfuegen der eigenen Include Dateien
//----------------------------------------------------------------------
#include "canbus_ringbuffer.h"
//----------------------------------------------------------------------

// Initialisiere Ringbuffer
//----------------------------------------------------------------------
void canbus_ringbuffer_init (RingbufferCANBusTypeDef *ring, volatile CAN_message_t *buffer, uint32_t size)
{
	ring->buffer = buffer;
	ring->size = size;
	ring->head = 0;
	ring->tail = 0;
}
//----------------------------------------------------------------------

// Nachricht zum Ring hinzufuegen
//----------------------------------------------------------------------
bool canbus_addToRingBuffer (RingbufferCANBusTypeDef *ring, CAN_message_t *msg)
{
	// Variable definieren
	uint16_t nextEntry;
	nextEntry = (ring->head + 1) % ring->size;

	// Pruefe, ob Ringbuffer gefuellt ist
	if (nextEntry == ring->tail)
		return false;

	// Fuege Element zum Ring hinzu
	memcpy((void *)&ring->buffer[ring->head], (void *)msg, sizeof(CAN_message_t));

	// Ringbuffer Kopf hochzaehlen
	ring->head = nextEntry;

	return true;
}
//----------------------------------------------------------------------

// Nachricht von Ring entfernen
//----------------------------------------------------------------------
bool canbus_removeFromRingBuffer (RingbufferCANBusTypeDef *ring, CAN_message_t *msg)
{
	// Pruefen, ob Nachrichten im Ring sind
	if (canbus_isRingBufferEmpty(ring) == true)
		return false;

	// Kopiere Nachricht
	memcpy((void *)msg, (void *)&ring->buffer[ring->tail], sizeof(CAN_message_t));

	// Ringbuffer Schwanz hochzaehlen
	ring->tail = (ring->tail + 1) % ring->size;

	return true;
}
//----------------------------------------------------------------------

// Abfrage, ob Ringbuffer Nachrichten hat
//----------------------------------------------------------------------
bool canbus_isRingBufferEmpty (RingbufferCANBusTypeDef *ring)
{
	// Wenn Ringpuffer leer
	if (ring->head == ring->tail)
		return true;

	return false;
}
//----------------------------------------------------------------------
