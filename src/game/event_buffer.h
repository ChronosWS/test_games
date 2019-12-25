#pragma once

#include "event.h"

namespace game
{
struct EventBuffer {
  uint8_t* buffer = nullptr;
  int buffer_size = 0;
  int idx = 0;
  int poll_idx = 0;
};

// Initialize an event buffer of size_bytes
void AllocateEventBuffer(int size_bytes);

// Free memory in event buffer.
void DeallocateEventBuffer();

// Reserve memory in the buffer, the return type should be
// casted to the desired type.
void* EnqueueEvent(uint16_t size, uint16_t metadata);

// Enqueue an event by copying from the given buffer. This
// function is thread safe.
void EnqueueEvent(uint8_t* event, int size);

// Poll the event buffer until it's empty.
bool PollEvent(Event* event);

// Memset all memory in the buffer to 0.
void ResetEventBuffer();

// When a custom event buffer is set running PollEvent
// will poll from this buffer instead. ResentEventBuffer
// will resent events in both this event buffer and
// the default one.
void SetCustomEventBuffer(EventBuffer* event_buffer);

// Given a type and ID create an event. Typically ID is
// some sort of enum that will later be used to determine type.
template <class T, class ID>
T*
EnqueueEvent(ID event_id)
{
  T* e = (T*)EnqueueEvent(sizeof(T), (uint16_t)event_id);
  return e;
}

}  // namespace game
