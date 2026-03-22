#include "iqueue.h"

/******************************************************************************
 * Enumerations, structures & Variables
 ******************************************************************************/

/******************************************************************************
 * Declaration | Static Functions
 ******************************************************************************/

/******************************************************************************
 * Definition  | Static Functions
 ******************************************************************************/

/******************************************************************************
 * Definition  | Public Functions
 ******************************************************************************/

i_status iqueue_init(iqueue_t* _queue, uint32_t _max_elements, size_t _element_size, void* _storage) {
    if ((_queue != NULL) && (_storage != NULL)) {
        (void) memset(_storage, 0, _element_size * _max_elements);
        _queue->element_size = _element_size;
        _queue->max_elements = _max_elements;
        _queue->first = (uintptr_t) NULL;
        _queue->next = (uintptr_t) _storage;
        _queue->storage = _storage;
        return I_OK;
    } else {
        if ((_queue != NULL))printf("null 1\r\n");
        if ((_storage != NULL))printf("null 2\r\n");
        return I_ERROR;
    }
}

void* iqueue_get_next_enqueue(iqueue_t* _queue) {
    return (_queue != NULL) ? (void *) (_queue->next) : NULL;
}

i_status iqueue_advance_next(iqueue_t* _queue) {
    if (_queue->first != _queue->next) {
        uintptr_t storage_end = (uintptr_t) (_queue->storage) + (_queue->element_size * _queue->max_elements);
        _queue->first = (_queue->first == 0U) ? _queue->next : _queue->first;
        _queue->next = ((_queue->next + _queue->element_size) == storage_end) ? (uintptr_t) _queue->storage : (_queue->next + _queue->element_size);
        return I_OK;
    } else {
        return I_FULL;
    }
}

i_status iqueue_enqueue(iqueue_t* _queue, void* _element) {
    if (_queue->first != _queue->next) {
        (void) memmove((void*) _queue->next, (void*) _element, _queue->element_size);
        return iqueue_advance_next(_queue);
    }
    if (_queue->first == _queue->next) printf("null \r\n");

    return I_FULL;
}

i_status iqueue_dequeue(iqueue_t* _queue, void* _element) {
    void* x = iqueue_dequeue_fast(_queue);
    if (x != NULL) {
        (void) memmove((void*) _element, x, _queue->element_size);
        return I_OK;
    } else {
        return I_EMPTY;
    }
}

void* iqueue_dequeue_fast(iqueue_t* _queue) {
    if (_queue == NULL) {
        return NULL;
    }

    if (_queue->first != 0U) {
        void* ret_ptr = (void*) _queue->first;

        uintptr_t storage_end = (uintptr_t) _queue->storage + (_queue->element_size * _queue->max_elements);
        uintptr_t next_first = _queue->first + _queue->element_size;

        _queue->first = (next_first == storage_end) ? (uintptr_t) _queue->storage : next_first;
        _queue->first = (_queue->first == _queue->next) ? 0U : _queue->first;
        return ret_ptr;
    }

    return NULL;
}

i_status iqueue_size(iqueue_t* _queue, size_t* _size) {
    *_size = _queue->first == 0
            ? 0
            : _queue->first < _queue->next
            ? (_queue->next - _queue->first) / _queue->element_size
            : _queue->max_elements - ((_queue->first - _queue->next) / _queue->element_size);

    return I_OK;
}
