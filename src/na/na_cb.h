/*
 * Copyright (C) 2013 Argonne National Laboratory, Department of Energy,
 *                    UChicago Argonne, LLC and The HDF Group.
 * All rights reserved.
 *
 * The full copyright notice, including terms governing use, modification,
 * and redistribution, is contained in the COPYING file that can be
 * found at the root of the source code distribution tree.
 */

#ifndef NA_H
#define NA_H

#include "na_config.h"

#include <limits.h>

typedef struct na_class na_class_t; /* Abstract network class */
typedef void *      na_addr_t;      /* Abstract network address */
typedef na_uint64_t na_size_t;      /* Size */
typedef na_uint32_t na_tag_t;       /* Tag */
typedef void *      na_request_t;   /* Abstract request */
typedef struct      na_status {     /* Operation status */
    na_bool_t completed;            /* - true if operation has completed */
    na_size_t count;                /* - number of bytes transmitted */
} na_status_t;

typedef void *      na_mem_handle_t; /* Absract memory handle */
typedef na_uint64_t na_offset_t;     /* Offset */
typedef struct      na_segment {     /* Segment */
    na_ptr_t   address;              /* - address of the segment */
    na_size_t  size;                 /* - size of the segment in bytes */
} na_segment_t;

/* Constant values */
#define NA_ADDR_NULL    ((na_addr_t)0)
#define NA_REQUEST_NULL ((na_request_t)0)
#define NA_STATUS_IGNORE ((na_status_t *)1)
#define NA_MEM_HANDLE_NULL ((na_mem_handle_t)0)

/* Max timeout */
#define NA_MAX_IDLE_TIME (3600*1000)

/* Tag upper bound
 * NB. This is not the user tag limit but only the limit imposed by the type */
#define NA_TAG_UB UINT_MAX

/* The memory attributes associated with the memory handle
 * can be defined as read/write or read only */
#define NA_MEM_READWRITE  0x00
#define NA_MEM_READ_ONLY  0x01

/* Error return codes */
#define NA_SUCCESS  1
#define NA_FAIL    -1
#define NA_TRUE     1
#define NA_FALSE    0


//typedef struct na_op_id {
//    void * ;
//    uint64_t cookie;
//    na_callback_info_t
//} na_op_id_t;

typedef void * na_op_id_t;

enum cb_type {
    CB_LOOKUP,
    CB_SEND_UNEXPECTED,
    CB_RECV_UNEXPECTED,
    CB_SEND_EXPECTED,
    CB_RECV_EXPECTED,
    CB_PUT,
    CB_GET
};

struct na_callback_addr_info {
    na_addr_t addr;
};

struct na_callback_addr_info {
    na_addr_t addr;
};

struct na_cb_send_unexpected_info {

};

struct na_cb_recv_unexpected_info {
    na_addr_t;
};


struct na_cb_get_info {

};

struct na_cb_put_info {

};

typedef struct na_callback_info {
    void *user_data;
    enum cb_type type;
    int error;
//    na_op_id_t operation_id;

    union info {
      struct na_cb_lookup_info lookup;
      struct na_cb_send_unexpected_info send_unexpected;
      struct na_cb_recv_unexpected_info recv_unexpected;
      struct na_cb_put_info put;
      struct na_cb_get_info get;
    };
} na_callback_info_t;

typedef int (*na_callback_t)(const na_callback_info_t *info);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the network abstraction layer.
 *
 * \param method [IN]           method name, available methods depend on
 *                              configuration (e.g., "bmi", "mpi")
 * \param port_name [IN]        port name string that may be used by underlying
 *                              plugin
 * \param listen [IN]           listen for incoming connections
 *
 * \return Pointer to network class
 */
NA_EXPORT na_class_t *
NA_Initialize(const char *method, const char *port_name, na_bool_t listen);

/**
 * Finalize the network abstraction layer.
 *
 * \param network_class [IN]    pointer to network class
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Finalize(na_class_t *network_class);

/**
 * Lookup an addr from a peer address/name. Addresses need to be
 * freed by calling NA_Addr_free.
 *
 * \param network_class [IN]    pointer to network class
 * \param name [IN]             lookup name
 * \param addr [OUT]            pointer to returned abstract address
 *
 * callback will be called with pointer to na_addr_t that contains addr ::na_callback_addr_info
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Addr_lookup(na_class_t *network_class, na_callback_t cb, void *user_data, const char *name);

/**
 * Free the addr from the list of peers.
 *
 * \param network_class [IN]    pointer to network class
 * \param addr [IN]             abstract address
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Addr_free(na_class_t *network_class, na_addr_t addr);

/**
 * Get the maximum size of messages supported by expected send/recv.
 * Small message size that may differ from the unexpected message size.
 *
 * \param network_class [IN]    pointer to network class
 *
 * \return Non-negative value
 */
NA_EXPORT na_size_t
NA_Msg_get_max_expected_size(na_class_t *network_class);

/**
 * Get the maximum size of messages supported by unexpected send/recv.
 * Small message size.
 *
 * \param network_class [IN]    pointer to network class
 *
 * \return Non-negative value
 */
NA_EXPORT na_size_t
NA_Msg_get_max_unexpected_size(na_class_t *network_class);

/**
 * Get the maximum tag value that can be used by send/recv.
 * (both expected and unexpected)
 *
 * \param network_class [IN]    pointer to network class
 *
 * \return Non-negative value
 */
NA_EXPORT na_tag_t
NA_Msg_get_max_tag(na_class_t *network_class);

/**
 * Send an unexpected message to dest.
 * Unexpected sends do not require a matching receive to complete.
 * Note also that unexpected messages do not require an unexpected receive to
 * be posted at the destination before sending the message and the destination
 * is allowed to drop the message without notification.
 *
 * \param network_class [IN]    pointer to network class
 * \param buf [IN]              pointer to send buffer
 * \param buf_size [IN]         buffer size
 * \param dest [IN]             abstract address of destination
 * \param tag [IN]              tag attached to message
 * \param request [OUT]         pointer to returned abstract request
 * \param op_arg [IN/OUT]       optional arguments
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Msg_send_unexpected(na_class_t *network_class, na_callback_t cb, void *user_data,
        const void *buf, na_size_t buf_size, na_addr_t dest,
        na_tag_t tag, na_op_id_t *op_id);

/**
 * Receive an unexpected message.
 * Unexpected receives may wait on ANY_TAG and ANY_SOURCE depending on the
 * implementation.
 *
 * \param network_class [IN]    pointer to network class
 * \param buf [IN]              pointer to buffer used to receive message
 * \param buf_size [IN]         buffer size
 * \param actual_buf_size [OUT] actual size of the received buffer
 * \param source [OUT]          pointer to abstract address of source
 * \param tag [OUT]             pointer to returned tag attached to message
 * \param request [OUT]         pointer to returned abstract request
 * \param op_arg [IN/OUT]       optional arguments
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Msg_recv_unexpected(na_class_t *network_class, na_callback_t cb, void *user_data,
        void *buf, na_size_t buf_size);

/**
 * Send an expected message to dest. Note that expected messages require
 * an expected receive to be posted at the destination before sending the
 * message, otherwise the destination is allowed to drop the message without
 * notification.
 *
 * \param network_class [IN]    pointer to network class
 * \param buf [IN]              pointer to send buffer
 * \param buf_size [IN]         buffer size
 * \param dest [IN]             abstract address of destination
 * \param tag [IN]              tag attached to message
 * \param request [OUT]         pointer to returned abstract request
 * \param op_arg [IN/OUT]       optional arguments
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Msg_send_expected(na_class_t *network_class, na_callback_t cb, void *user_data,
        const void *buf, na_size_t buf_size, na_addr_t dest,
        na_tag_t tag);

/**
 * Receive an expected message from source.
 *
 * \param network_class [IN]    pointer to network class
 * \param buf [IN]              pointer to receive buffer
 * \param buf_size [IN]         buffer size
 * \param source [IN]           abstract address of source
 * \param tag [IN]              matching tag used to receive message
 * \param request [OUT]         pointer to returned abstract request
 * \param op_arg [IN/OUT]       optional arguments
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Msg_recv_expected(na_class_t *network_class, na_callback_t cb, void *user_data,
        void *buf, na_size_t buf_size, na_addr_t source,
        na_tag_t tag);

/**
 * Register memory for RMA operations.
 * Memory pieces must be registered before one-sided transfers can be
 * initiated.
 * Register can be used to register a contiguous piece of memory.
 *
 * \param network_class [IN]    pointer to network class
 * \param buf [IN]              pointer to buffer that needs to be registered
 * \param buf_size [IN]         buffer size
 * \param flags [IN]            permission flag:
 *                                - NA_MEM_READWRITE
 *                                - NA_MEM_READ_ONLY
 * \param mem_handle [OUT]      pointer to returned abstract memory handle
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Mem_register(na_class_t *network_class,
        void *buf, na_size_t buf_size, unsigned long flags,
        na_mem_handle_t *mem_handle);

/**
 * Register segmented memory for RMA operations.
 * Register_segments can be used to register fragmented pieces and get
 * a single memory handle, this is implemented only if the network
 * transport supports it.
 *
 * \param network_class [IN]    pointer to network class
 * \param segments [IN]         pointer to array of na_segment_t composed of:
 *                                - address of the segment that needs to be
 *                                  registered
 *                                - size of the segment in bytes
 * \param segment_count [IN]    segment count
 * \param flags [IN]            permission flag:
 *                                - NA_MEM_READWRITE
 *                                - NA_MEM_READ_ONLY
 * \param mem_handle [OUT]      pointer to returned abstract memory handle
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Mem_register_segments(na_class_t *network_class,
        na_segment_t *segments, na_size_t segment_count, unsigned long flags,
        na_mem_handle_t *mem_handle);

/**
 * Unregister memory.
 *
 * \param network_class [IN]    pointer to network class
 * \param mem_handle [IN]       abstract memory handle
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Mem_deregister(na_class_t *network_class, na_mem_handle_t mem_handle);

/**
 * Get size required to serialize handle.
 *
 * \param network_class [IN]    pointer to network class
 * \param mem_handle [IN]       abstract memory handle
 *
 * \return Non-negative value
 */
NA_EXPORT na_size_t
NA_Mem_handle_get_serialize_size(na_class_t *network_class,
        na_mem_handle_t mem_handle);

/**
 * Serialize memory handle into a buffer.
 * One-sided transfers require prior exchange of memory handles between
 * peers, serialization callbacks can be used to "pack" a memory handle and
 * send it across the network.
 * NB. Memory handles can be variable size, therefore the space required
 * to serialize a handle into a buffer can be obtained using
 * NA_Mem_handle_get_serialize_size.
 *
 * \param network_class [IN]    pointer to network class
 * \param buf [IN/OUT]          pointer to buffer used for serialization
 * \param buf_size [IN]         buffer size
 * \param mem_handle [IN]       abstract memory handle
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Mem_handle_serialize(na_class_t *network_class,
        void *buf, na_size_t buf_size, na_mem_handle_t mem_handle);

/**
 * Deserialize memory handle from buffer.
 *
 * \param network_class [IN]    pointer to network class
 * \param mem_handle [OUT]      pointer to abstract memory handle
 * \param buf [IN]              pointer to buffer used for deserialization
 * \param buf_size [IN]         buffer size
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Mem_handle_deserialize(na_class_t *network_class,
        na_mem_handle_t *mem_handle, const void *buf, na_size_t buf_size);

/**
 * Free memory handle.
 *
 * \param network_class [IN]    pointer to network class
 * \param mem_handle [IN]       abstract memory handle
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Mem_handle_free(na_class_t *network_class, na_mem_handle_t mem_handle);

/**
 * Put data to remote target.
 * Initiate a put or get to/from the registered memory regions with the
 * given offset/size.
 * NB. Memory must be registered and handles exchanged between peers.
 *
 * \param network_class [IN]     pointer to network class
 * \param local_mem_handle [IN]  abstract local memory handle
 * \param local_offset [IN]      local offset
 * \param remote_mem_handle [IN] abstract remote memory handle
 * \param remote_offset [IN]     remote offset
 * \param data_size [IN]         size of data that needs to be transferred
 * \param remote_addr [IN]       abstract address of remote destination
 * \param request [OUT]          pointer to returned abstract request
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Put(na_class_t *network_class, na_callback_t cb, void *user_data,
        na_mem_handle_t local_mem_handle, na_offset_t local_offset,
        na_mem_handle_t remote_mem_handle, na_offset_t remote_offset,
        na_size_t data_size, na_addr_t remote_addr);

/**
 * Get data from remote target.
 *
 * \param network_class [IN]     pointer to network class
 * \param local_mem_handle [IN]  abstract local memory handle
 * \param local_offset [IN]      local offset
 * \param remote_mem_handle [IN] abstract remote memory handle
 * \param remote_offset [IN]     remote offset
 * \param data_size [IN]         size of data that needs to be transferred
 * \param remote_addr [IN]       abstract address of remote source
 * \param request [OUT]          pointer to returned abstract request
 *
 * \return Non-negative on success or negative on failure
 */
NA_EXPORT int
NA_Get(na_class_t *network_class, na_callback_t cb, void *user_data,
        na_mem_handle_t local_mem_handle, na_offset_t local_offset,
        na_mem_handle_t remote_mem_handle, na_offset_t remote_offset,
        na_size_t data_size, na_addr_t remote_addr);

/**
 * Wait for a request to complete or until timeout is reached.
 *
 * \param network_class [IN]    pointer to network class
 * \param request [IN]          abstract request
 * \param timeout [IN]          timeout (in milliseconds)
 * \param status [OUT]          pointer to returned status
 *
 * \return Non-negative on success or negative on failure
 */
//NA_EXPORT int
//NA_Wait(na_class_t *network_class,
//        na_request_t request, unsigned int timeout, na_status_t *status);

/**
 * Try to progress communication for at most timeout until imeout reached or
 * any completion has occurred.
 *
 * \param network_class [IN]    pointer to network class
 * \param timeout [IN]          timeout (in milliseconds)
 * \param status [OUT]          pointer to returned status
 *
 * \return Non-negative on success or negative on failure
 * per plugin push callback to the queue when something completes
 * wake up everyone in the trigger in main na
 */
NA_EXPORT int
NA_Progress(na_class_t *network_class, unsigned int timeout, na_status_t *status);

/**
 * Execute at most max_count callbacks. If timeout is non-zero, wait up to timeout
 * before returning. Function can return when at least one or more callbacks are triggered
 * (at most max_count).
 *
 * \return Number of callbacks triggered
 * in main na layer (locking)
 */
NA_EXPORT int
NA_Trigger(unsigned int timeout, unsigned int max_count, int *actual_count);


NA_EXPORT int
NA_Cancel(na_op_id_t op_id);

/**
 * Free a request without waiting for it to complete (request may be active
 * or inactive).
 *
 * \param network_class [IN]    pointer to network class
 * \param request [IN]          abstract request
 *
 * \return Non-negative on success or negative on failure
 */
//NA_EXPORT int
//NA_Request_free(na_class_t *network_class, na_request_t request);

#ifdef __cplusplus
}
#endif

#endif /* NA_H */