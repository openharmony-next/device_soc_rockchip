/*
 *
 * (C) COPYRIGHT 2010-2020 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 */

/* THIS FILE IS AUTOGENERATED BY mali_trace_generator.py.
 * DO NOT EDIT.
 */

/* clang-format off */

#include "mali_kbase_mipe_proto.h"

/**
 * This header generates MIPE tracepoint declaration BLOB at
 * compile time.
 *
 * It is intentional that there is no header guard.
 * The header could be included multiple times for
 * different blobs compilation.
 *
 * Before including this header MIPE_HEADER_* parameters must be
 * defined. See documentation below:
 */

/**
 * The name of the variable where the result BLOB will be stored.
 */
#if !defined(MIPE_HEADER_BLOB_VAR_NAME)
#error "MIPE_HEADER_BLOB_VAR_NAME must be defined!"
#endif

/**
 * A compiler attribute for the BLOB variable.
 *
 * e.g. __attribute__((section("my_section")))
 *
 * Default value is no attribute.
 */
#if !defined(MIPE_HEADER_BLOB_VAR_ATTRIBUTE)
#define MIPE_HEADER_BLOB_VAR_ATTRIBUTE
#endif

/**
 * MIPE stream id.
 *
 * See enum tl_stream_id.
 */
#if !defined(MIPE_HEADER_STREAM_ID)
#error "MIPE_HEADER_STREAM_ID must be defined!"
#endif

/**
 * MIPE packet class.
 *
 * See enum tl_packet_class.
 */
#if !defined(MIPE_HEADER_PKT_CLASS)
#error "MIPE_HEADER_PKT_CLASS must be defined!"
#endif

/**
 * The list of tracepoints to process.
 *
 * It should be defined as follows:
 *
 * #define MIPE_HEADER_TRACEPOINT_LIST \
 *     TRACEPOINT_DESC(FIRST_TRACEPOINT, "Some description", "@II", "first_arg,second_arg") \
 *     TRACEPOINT_DESC(SECOND_TRACEPOINT, "Some description", "@II", "first_arg,second_arg") \
 *     etc.
 *
 * Where the first argument is tracepoints name, the second
 * argument is a short tracepoint description, the third argument
 * argument types (see MIPE documentation), and the fourth argument
 * is comma separated argument names.
 */
#if !defined(MIPE_HEADER_TRACEPOINT_LIST)
#error "MIPE_HEADER_TRACEPOINT_LIST must be defined!"
#endif

/**
 * The number of entries in MIPE_HEADER_TRACEPOINT_LIST.
 */
#if !defined(MIPE_HEADER_TRACEPOINT_LIST_SIZE)
#error "MIPE_HEADER_TRACEPOINT_LIST_SIZE must be defined!"
#endif

/**
 * The list of enums to process.
 *
 * It should be defined as follows:
 *
 * #define MIPE_HEADER_ENUM_LIST \
 *     ENUM_DESC(enum_arg_name, enum_value) \
 *     ENUM_DESC(enum_arg_name, enum_value) \
 *     etc.
 *
 * Where enum_arg_name is the name of a tracepoint argument being used with
 * this enum. enum_value is a valid C enum value.
 *
 * Default value is an empty list.
 */
#if defined(MIPE_HEADER_ENUM_LIST)

/**
 * Tracepoint message ID used for enums declaration.
 */
#if !defined(MIPE_HEADER_ENUM_MSG_ID)
#error "MIPE_HEADER_ENUM_MSG_ID must be defined!"
#endif

#else
#define MIPE_HEADER_ENUM_LIST
#endif

/**
 * The MIPE tracepoint declaration BLOB.
 */
const struct {
    u32 _mipe_w0;
    u32 _mipe_w1;
    u8  _protocol_version;
    u8  _pointer_size;
    u32 _tp_count;
#define TRACEPOINT_DESC(name, desc, arg_types, arg_names)    \
    struct {                    \
        u32  _name;                \
        u32  _size_string_name;        \
        char _string_name[sizeof(#name)];    \
        u32  _size_desc;            \
        char _desc[sizeof(desc)];        \
        u32  _size_arg_types;        \
        char _arg_types[sizeof(arg_types)];    \
        u32  _size_arg_names;        \
        char _arg_names[sizeof(arg_names)];    \
    } __attribute__ ((__packed__)) __ ## name;

#define ENUM_DESC(arg_name, value)                    \
    struct {                            \
        u32 _msg_id;                    \
        u32 _arg_name_len;                    \
        char _arg_name[sizeof(#arg_name)];            \
        u32 _value;                        \
        u32 _value_str_len;                    \
        char _value_str[sizeof(#value)];            \
    } __attribute__ ((__packed__)) __ ## arg_name ## _ ## value;

    MIPE_HEADER_TRACEPOINT_LIST
    MIPE_HEADER_ENUM_LIST
#undef TRACEPOINT_DESC
#undef ENUM_DESC
} __attribute__((packed)) MIPE_HEADER_BLOB_VAR_NAME MIPE_HEADER_BLOB_VAR_ATTRIBUTE = {
    ._mipe_w0 = MIPE_PACKET_HEADER_W0(
        TL_PACKET_FAMILY_TL,
        MIPE_HEADER_PKT_CLASS,
        TL_PACKET_TYPE_HEADER,
        MIPE_HEADER_STREAM_ID),
    ._mipe_w1 = MIPE_PACKET_HEADER_W1(
        sizeof(MIPE_HEADER_BLOB_VAR_NAME) - PACKET_HEADER_SIZE,
        0),
    ._protocol_version = SWTRACE_VERSION,
    ._pointer_size = sizeof(void *),
    ._tp_count = MIPE_HEADER_TRACEPOINT_LIST_SIZE,
#define TRACEPOINT_DESC(name, desc, arg_types, arg_names)    \
    .__ ## name = {                    \
        ._name = (name),                \
        ._size_string_name = sizeof(#name),    \
        ._string_name = #name,            \
        ._size_desc = sizeof(desc),        \
        ._desc = (desc),                \
        ._size_arg_types = sizeof(arg_types),    \
        ._arg_types = (arg_types),        \
        ._size_arg_names = sizeof(arg_names),    \
        ._arg_names = arg_names            \
    },
#define ENUM_DESC(arg_name, value)                \
    .__ ## arg_name ## _ ## value = {            \
        ._msg_id = MIPE_HEADER_ENUM_MSG_ID,        \
        ._arg_name_len = sizeof(#arg_name),        \
        ._arg_name = #arg_name,                \
        ._value = (value),                \
        ._value_str_len = sizeof(#value),        \
        ._value_str = #value                \
    },

    MIPE_HEADER_TRACEPOINT_LIST
    MIPE_HEADER_ENUM_LIST
#undef TRACEPOINT_DESC
#undef ENUM_DESC
};

#undef MIPE_HEADER_BLOB_VAR_NAME
#undef MIPE_HEADER_BLOB_VAR_ATTRIBUTE
#undef MIPE_HEADER_STREAM_ID
#undef MIPE_HEADER_PKT_CLASS
#undef MIPE_HEADER_TRACEPOINT_LIST
#undef MIPE_HEADER_TRACEPOINT_LIST_SIZE
#undef MIPE_HEADER_ENUM_LIST
#undef MIPE_HEADER_ENUM_MSG_ID

/* clang-format on */
