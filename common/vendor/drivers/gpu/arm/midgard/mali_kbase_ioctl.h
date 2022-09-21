/*
 *
 * (C) COPYRIGHT 2017 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */



#ifndef _KBASE_IOCTL_H_
#define _KBASE_IOCTL_H_

#ifdef __cpluscplus
extern "C" {
#endif

#include <linux/types.h>

#define KBASE_IOCTL_TYPE 0x80

#ifdef ANDROID
/* Android's definition of ioctl is incorrect, specifying the type argument as
 * 'int'. This creates a warning when using _IOWR (as the top bit is set). Work
 * round this by redefining _IOC to include a case to 'int'.
 */
#undef _IOC
#define _IOC(dir, type, nr, size) \
    ((int)(((dir) << _IOC_DIRSHIFT) | ((type) << _IOC_TYPESHIFT) | \
    ((nr) << _IOC_NRSHIFT) | ((size) << _IOC_SIZESHIFT)))
#endif

/**
 * struct kbase_ioctl_version_check - Check version compatibility with kernel
 *
 * @major: Major version number
 * @minor: Minor version number
 */
struct kbase_ioctl_version_check {
    __u16 major;
    __u16 minor;
};

#define KBASE_IOCTL_VERSION_CHECK \
    _IOWR(KBASE_IOCTL_TYPE, 0, struct kbase_ioctl_version_check)

/**
 * struct kbase_ioctl_set_flags - Set kernel context creation flags
 *
 * @create_flags: Flags - see base_context_create_flags
 */
struct kbase_ioctl_set_flags {
    __u32 create_flags;
};

#define KBASE_IOCTL_SET_FLAGS \
    _IOW(KBASE_IOCTL_TYPE, 1, struct kbase_ioctl_set_flags)

/**
 * struct kbase_ioctl_job_submit - Submit jobs/atoms to the kernel
 *
 * @addr: Memory address of an array of struct base_jd_atom_v2
 * @nr_atoms: Number of entries in the array
 * @stride: sizeof(struct base_jd_atom_v2)
 */
struct kbase_ioctl_job_submit {
    union kbase_pointer addr;
    __u32 nr_atoms;
    __u32 stride;
};

#define KBASE_IOCTL_JOB_SUBMIT \
    _IOW(KBASE_IOCTL_TYPE, 2, struct kbase_ioctl_job_submit)

/**
 * struct kbase_ioctl_get_gpuprops - Read GPU properties from the kernel
 *
 * @buffer: Pointer to the buffer to store properties into
 * @size: Size of the buffer
 * @flags: Flags - must be zero for now
 *
 * The ioctl will return the number of bytes stored into @buffer or an error
 * on failure (e.g. @size is too small). If @size is specified as 0 then no
 * data will be written but the return value will be the number of bytes needed
 * for all the properties.
 *
 * @flags may be used in the future to request a different format for the
 * buffer. With @flags == 0 the following format is used.
 *
 * The buffer will be filled with pairs of values, a u32 key identifying the
 * property followed by the value. The size of the value is identified using
 * the bottom bits of the key. The value then immediately followed the key and
 * is tightly packed (there is no padding). All keys and values are
 * little-endian.
 *
 * 00 = u8
 * 01 = u16
 * 10 = u32
 * 11 = u64
 */
struct kbase_ioctl_get_gpuprops {
    union kbase_pointer buffer;
    __u32 size;
    __u32 flags;
};

#define KBASE_IOCTL_GET_GPUPROPS \
    _IOW(KBASE_IOCTL_TYPE, 3, struct kbase_ioctl_get_gpuprops)

#define KBASE_IOCTL_POST_TERM \
    _IO(KBASE_IOCTL_TYPE, 4)

/**
 * union kbase_ioctl_mem_alloc - Allocate memory on the GPU
 *
 * @va_pages: The number of pages of virtual address space to reserve
 * @commit_pages: The number of physical pages to allocate
 * @extent: The number of extra pages to allocate on each GPU fault which grows
 *          the region
 * @flags: Flags
 * @gpu_va: The GPU virtual address which is allocated
 *
 * @in: Input parameters
 * @out: Output parameters
 */
union kbase_ioctl_mem_alloc {
    struct {
        __u64 va_pages;
        __u64 commit_pages;
        __u64 extent;
        __u64 flags;
    } in;
    struct {
        __u64 flags;
        __u64 gpu_va;
    } out;
};

#define KBASE_IOCTL_MEM_ALLOC \
    _IOWR(KBASE_IOCTL_TYPE, 5, union kbase_ioctl_mem_alloc)

/**
 * struct kbase_ioctl_mem_query - Query properties of a GPU memory region
 * @gpu_addr: A GPU address contained within the region
 * @query: The type of query
 * @value: The result of the query
 *
 * Use a %KBASE_MEM_QUERY_xxx flag as input for @query.
 *
 * @in: Input parameters
 * @out: Output parameters
 */
union kbase_ioctl_mem_query {
    struct {
        __u64 gpu_addr;
        __u64 query;
    } in;
    struct {
        __u64 value;
    } out;
};

#define KBASE_IOCTL_MEM_QUERY \
    _IOWR(KBASE_IOCTL_TYPE, 6, union kbase_ioctl_mem_query)

#define KBASE_MEM_QUERY_COMMIT_SIZE    1
#define KBASE_MEM_QUERY_VA_SIZE        2
#define KBASE_MEM_QUERY_FLAGS        3

/**
 * struct kbase_ioctl_mem_free - Free a memory region
 * @gpu_addr: Handle to the region to free
 */
struct kbase_ioctl_mem_free {
    __u64 gpu_addr;
};

#define KBASE_IOCTL_MEM_FREE \
    _IOW(KBASE_IOCTL_TYPE, 7, struct kbase_ioctl_mem_free)

/**
 * struct kbase_ioctl_hwcnt_reader_setup - Setup HWC dumper/reader
 * @buffer_count: requested number of dumping buffers
 * @jm_bm:        counters selection bitmask (JM)
 * @shader_bm:    counters selection bitmask (Shader)
 * @tiler_bm:     counters selection bitmask (Tiler)
 * @mmu_l2_bm:    counters selection bitmask (MMU_L2)
 *
 * A fd is returned from the ioctl if successful, or a negative value on error
 */
struct kbase_ioctl_hwcnt_reader_setup {
    __u32 buffer_count;
    __u32 jm_bm;
    __u32 shader_bm;
    __u32 tiler_bm;
    __u32 mmu_l2_bm;
};

#define KBASE_IOCTL_HWCNT_READER_SETUP \
    _IOW(KBASE_IOCTL_TYPE, 8, struct kbase_ioctl_hwcnt_reader_setup)

/**
 * struct kbase_ioctl_hwcnt_enable - Enable hardware counter collection
 * @dump_buffer:  GPU address to write counters to
 * @jm_bm:        counters selection bitmask (JM)
 * @shader_bm:    counters selection bitmask (Shader)
 * @tiler_bm:     counters selection bitmask (Tiler)
 * @mmu_l2_bm:    counters selection bitmask (MMU_L2)
 */
struct kbase_ioctl_hwcnt_enable {
    __u64 dump_buffer;
    __u32 jm_bm;
    __u32 shader_bm;
    __u32 tiler_bm;
    __u32 mmu_l2_bm;
};

#define KBASE_IOCTL_HWCNT_ENABLE \
    _IOW(KBASE_IOCTL_TYPE, 9, struct kbase_ioctl_hwcnt_enable)

#define KBASE_IOCTL_HWCNT_DUMP \
    _IO(KBASE_IOCTL_TYPE, 10)

#define KBASE_IOCTL_HWCNT_CLEAR \
    _IO(KBASE_IOCTL_TYPE, 11)

/**
 * struct kbase_ioctl_disjoint_query - Query the disjoint counter
 * @counter:   A counter of disjoint events in the kernel
 */
struct kbase_ioctl_disjoint_query {
    __u32 counter;
};

#define KBASE_IOCTL_DISJOINT_QUERY \
    _IOR(KBASE_IOCTL_TYPE, 12, struct kbase_ioctl_disjoint_query)

/**
 * struct kbase_ioctl_get_ddk_version - Query the kernel version
 * @version_buffer: Buffer to receive the kernel version string
 * @size: Size of the buffer
 *
 * The ioctl will return the number of bytes written into version_buffer
 * (which includes a NULL byte) or a negative error code
 */
struct kbase_ioctl_get_ddk_version {
    union kbase_pointer version_buffer;
    __u32 size;
};

#define KBASE_IOCTL_GET_DDK_VERSION \
    _IOW(KBASE_IOCTL_TYPE, 13, struct kbase_ioctl_get_ddk_version)

/**
 * struct kbase_ioctl_mem_jit_init - Initialise the JIT memory allocator
 *
 * @va_pages: Number of VA pages to reserve for JIT
 *
 * Note that depending on the VA size of the application and GPU, the value
 * specified in @va_pages may be ignored.
 */
struct kbase_ioctl_mem_jit_init {
    __u64 va_pages;
};

#define KBASE_IOCTL_MEM_JIT_INIT \
    _IOW(KBASE_IOCTL_TYPE, 14, struct kbase_ioctl_mem_jit_init)

/**
 * struct kbase_ioctl_mem_sync - Perform cache maintenance on memory
 *
 * @handle: GPU memory handle (GPU VA)
 * @user_addr: The address where it is mapped in user space
 * @size: The number of bytes to synchronise
 * @type: The direction to synchronise: 0 is sync to memory (clean),
 * 1 is sync from memory (invalidate). Use the BASE_SYNCSET_OP_xxx constants.
 * @padding: Padding to round up to a multiple of 8 bytes, must be zero
 */
struct kbase_ioctl_mem_sync {
    __u64 handle;
    __u64 user_addr;
    __u64 size;
    __u8 type;
    __u8 padding[7];
};

#define KBASE_IOCTL_MEM_SYNC \
    _IOW(KBASE_IOCTL_TYPE, 15, struct kbase_ioctl_mem_sync)

/**
 * union kbase_ioctl_mem_find_cpu_offset - Find the offset of a CPU pointer
 *
 * @gpu_addr: The GPU address of the memory region
 * @cpu_addr: The CPU address to locate
 * @size: A size in bytes to validate is contained within the region
 * @offset: The offset from the start of the memory region to @cpu_addr
 *
 * @in: Input parameters
 * @out: Output parameters
 */
union kbase_ioctl_mem_find_cpu_offset {
    struct {
        __u64 gpu_addr;
        __u64 cpu_addr;
        __u64 size;
    } in;
    struct {
        __u64 offset;
    } out;
};

#define KBASE_IOCTL_MEM_FIND_CPU_OFFSET \
    _IOWR(KBASE_IOCTL_TYPE, 16, union kbase_ioctl_mem_find_cpu_offset)

/**
 * struct kbase_ioctl_get_context_id - Get the kernel context ID
 *
 * @id: The kernel context ID
 */
struct kbase_ioctl_get_context_id {
    int id; /* This should really be __u32, but see GPUCORE-10048 */
};

#define KBASE_IOCTL_GET_CONTEXT_ID \
    _IOR(KBASE_IOCTL_TYPE, 17, struct kbase_ioctl_get_context_id)

/**
 * struct kbase_ioctl_tlstream_acquire - Acquire a tlstream fd
 *
 * @flags: Flags
 *
 * The ioctl returns a file descriptor when successful
 */
struct kbase_ioctl_tlstream_acquire {
    __u32 flags;
};

#define KBASE_IOCTL_TLSTREAM_ACQUIRE \
    _IOW(KBASE_IOCTL_TYPE, 18, struct kbase_ioctl_tlstream_acquire)

#define KBASE_IOCTL_TLSTREAM_FLUSH \
    _IO(KBASE_IOCTL_TYPE, 19)

/**
 * struct kbase_ioctl_mem_commit - Change the amount of memory backing a region
 *
 * @gpu_addr: The memory region to modify
 * @pages:    The number of physical pages that should be present
 *
 * The ioctl may return on the following error codes or 0 for success:
 *   -ENOMEM: Out of memory
 *   -EINVAL: Invalid arguments
 */
struct kbase_ioctl_mem_commit {
    __u64 gpu_addr;
    __u64 pages;
};

#define KBASE_IOCTL_MEM_COMMIT \
    _IOW(KBASE_IOCTL_TYPE, 20, struct kbase_ioctl_mem_commit)

/**
 * union kbase_ioctl_mem_alias - Create an alias of memory regions
 * @flags: Flags, see BASE_MEM_xxx
 * @stride: Bytes between start of each memory region
 * @nents: The number of regions to pack together into the alias
 * @aliasing_info: Pointer to an array of struct base_mem_aliasing_info
 * @gpu_va: Address of the new alias
 * @va_pages: Size of the new alias
 *
 * @in: Input parameters
 * @out: Output parameters
 */
union kbase_ioctl_mem_alias {
    struct {
        __u64 flags;
        __u64 stride;
        __u64 nents;
        union kbase_pointer aliasing_info;
    } in;
    struct {
        __u64 flags;
        __u64 gpu_va;
        __u64 va_pages;
    } out;
};

#define KBASE_IOCTL_MEM_ALIAS \
    _IOWR(KBASE_IOCTL_TYPE, 21, union kbase_ioctl_mem_alias)

/**
 * union kbase_ioctl_mem_import - Import memory for use by the GPU
 * @flags: Flags, see BASE_MEM_xxx
 * @phandle: Handle to the external memory
 * @type: Type of external memory, see base_mem_import_type
 * @padding: Amount of extra VA pages to append to the imported buffer
 * @gpu_va: Address of the new alias
 * @va_pages: Size of the new alias
 *
 * @in: Input parameters
 * @out: Output parameters
 */
union kbase_ioctl_mem_import {
    struct {
        __u64 flags;
        union kbase_pointer phandle;
        __u32 type;
        __u32 padding;
    } in;
    struct {
        __u64 flags;
        __u64 gpu_va;
        __u64 va_pages;
    } out;
};

#define KBASE_IOCTL_MEM_IMPORT \
    _IOWR(KBASE_IOCTL_TYPE, 22, union kbase_ioctl_mem_import)

/**
 * struct kbase_ioctl_mem_flags_change - Change the flags for a memory region
 * @gpu_va: The GPU region to modify
 * @flags: The new flags to set
 * @mask: Mask of the flags to modify
 */
struct kbase_ioctl_mem_flags_change {
    __u64 gpu_va;
    __u64 flags;
    __u64 mask;
};

#define KBASE_IOCTL_MEM_FLAGS_CHANGE \
    _IOW(KBASE_IOCTL_TYPE, 23, struct kbase_ioctl_mem_flags_change)

/**
 * struct kbase_ioctl_stream_create - Create a synchronisation stream
 * @name: A name to identify this stream. Must be NULL-terminated.
 *
 * Note that this is also called a "timeline", but is named stream to avoid
 * confusion with other uses of the word.
 *
 * Unused bytes in @name (after the first NULL byte) must be also be NULL bytes.
 *
 * The ioctl returns a file descriptor.
 */
struct kbase_ioctl_stream_create {
    char name[32];
};

#define KBASE_IOCTL_STREAM_CREATE \
    _IOW(KBASE_IOCTL_TYPE, 24, struct kbase_ioctl_stream_create)

/**
 * struct kbase_ioctl_fence_validate - Validate a fd refers to a fence
 * @fd: The file descriptor to validate
 */
struct kbase_ioctl_fence_validate {
    int fd;
};

#define KBASE_IOCTL_FENCE_VALIDATE \
    _IOW(KBASE_IOCTL_TYPE, 25, struct kbase_ioctl_fence_validate)

/**
 * struct kbase_ioctl_get_profiling_controls - Get the profiling controls
 * @count: The size of @buffer in u32 words
 * @buffer: The buffer to receive the profiling controls
 */
struct kbase_ioctl_get_profiling_controls {
    union kbase_pointer buffer;
    __u32 count;
};

#define KBASE_IOCTL_GET_PROFILING_CONTROLS \
    _IOW(KBASE_IOCTL_TYPE, 26, struct kbase_ioctl_get_profiling_controls)

/**
 * struct kbase_ioctl_mem_profile_add - Provide profiling information to kernel
 * @buffer: Pointer to the information
 * @len: Length
 * @padding: Padding
 *
 * The data provided is accessible through a debugfs file
 */
struct kbase_ioctl_mem_profile_add {
    union kbase_pointer buffer;
    __u32 len;
    __u32 padding;
};

#define KBASE_IOCTL_MEM_PROFILE_ADD \
    _IOW(KBASE_IOCTL_TYPE, 27, struct kbase_ioctl_mem_profile_add)

/**
 * struct kbase_ioctl_soft_event_update - Update the status of a soft-event
 * @event: GPU address of the event which has been updated
 * @new_status: The new status to set
 * @flags: Flags for future expansion
 */
struct kbase_ioctl_soft_event_update {
    __u64 event;
    __u32 new_status;
    __u32 flags;
};

#define KBASE_IOCTL_SOFT_EVENT_UPDATE \
    _IOW(KBASE_IOCTL_TYPE, 28, struct kbase_ioctl_soft_event_update)

/***************
 * test ioctls *
 ***************/
#if MALI_UNIT_TEST
/* These ioctls are purely for test purposes and are not used in the production
 * driver, they therefore may change without notice
 */

#define KBASE_IOCTL_TEST_TYPE (KBASE_IOCTL_TYPE + 1)

/**
 * struct kbase_ioctl_tlstream_test - Start a timeline stream test
 *
 * @tpw_count: number of trace point writers in each context
 * @msg_delay: time delay between tracepoints from one writer in milliseconds
 * @msg_count: number of trace points written by one writer
 * @aux_msg:   if non-zero aux messages will be included
 */
struct kbase_ioctl_tlstream_test {
    __u32 tpw_count;
    __u32 msg_delay;
    __u32 msg_count;
    __u32 aux_msg;
};

#define KBASE_IOCTL_TLSTREAM_TEST \
    _IOW(KBASE_IOCTL_TEST_TYPE, 1, struct kbase_ioctl_tlstream_test)

/**
 * struct kbase_ioctl_tlstream_stats - Read tlstream stats for test purposes
 * @bytes_collected: number of bytes read by user
 * @bytes_generated: number of bytes generated by tracepoints
 */
struct kbase_ioctl_tlstream_stats {
    __u32 bytes_collected;
    __u32 bytes_generated;
};

#define KBASE_IOCTL_TLSTREAM_STATS \
    _IOR(KBASE_IOCTL_TEST_TYPE, 2, struct kbase_ioctl_tlstream_stats)

#endif

/**********************************
 * Definitions for GPU properties *
 **********************************/
#define KBASE_GPUPROP_VALUE_SIZE_U8    (0x0)
#define KBASE_GPUPROP_VALUE_SIZE_U16    (0x1)
#define KBASE_GPUPROP_VALUE_SIZE_U32    (0x2)
#define KBASE_GPUPROP_VALUE_SIZE_U64    (0x3)

#define KBASE_GPUPROP_VALUE_U8            1
#define KBASE_GPUPROP_VALUE_U16            2
#define KBASE_GPUPROP_VALUE_U32            4
#define KBASE_GPUPROP_VALUE_U64            8

#define KBASE_GPUPROP_PRODUCT_ID            1
#define KBASE_GPUPROP_VERSION_STATUS            2
#define KBASE_GPUPROP_MINOR_REVISION            3
#define KBASE_GPUPROP_MAJOR_REVISION            4
#define KBASE_GPUPROP_GPU_SPEED_MHZ            5
#define KBASE_GPUPROP_GPU_FREQ_KHZ_MAX            6
#define KBASE_GPUPROP_GPU_FREQ_KHZ_MIN            7
#define KBASE_GPUPROP_LOG2_PROGRAM_COUNTER_SIZE        8
#define KBASE_GPUPROP_TEXTURE_FEATURES_0        9
#define KBASE_GPUPROP_TEXTURE_FEATURES_1        10
#define KBASE_GPUPROP_TEXTURE_FEATURES_2        11
#define KBASE_GPUPROP_GPU_AVAILABLE_MEMORY_SIZE        12

#define KBASE_GPUPROP_L2_LOG2_LINE_SIZE            13
#define KBASE_GPUPROP_L2_LOG2_CACHE_SIZE        14
#define KBASE_GPUPROP_L2_NUM_L2_SLICES            15

#define KBASE_GPUPROP_TILER_BIN_SIZE_BYTES        16
#define KBASE_GPUPROP_TILER_MAX_ACTIVE_LEVELS        17

#define KBASE_GPUPROP_MAX_THREADS            18
#define KBASE_GPUPROP_MAX_WORKGROUP_SIZE        19
#define KBASE_GPUPROP_MAX_BARRIER_SIZE            20
#define KBASE_GPUPROP_MAX_REGISTERS            21
#define KBASE_GPUPROP_MAX_TASK_QUEUE            22
#define KBASE_GPUPROP_MAX_THREAD_GROUP_SPLIT        23
#define KBASE_GPUPROP_IMPL_TECH                24

#define KBASE_GPUPROP_RAW_SHADER_PRESENT        25
#define KBASE_GPUPROP_RAW_TILER_PRESENT            26
#define KBASE_GPUPROP_RAW_L2_PRESENT            27
#define KBASE_GPUPROP_RAW_STACK_PRESENT            28
#define KBASE_GPUPROP_RAW_L2_FEATURES            29
#define KBASE_GPUPROP_RAW_SUSPEND_SIZE            30
#define KBASE_GPUPROP_RAW_MEM_FEATURES            31
#define KBASE_GPUPROP_RAW_MMU_FEATURES            32
#define KBASE_GPUPROP_RAW_AS_PRESENT            33
#define KBASE_GPUPROP_RAW_JS_PRESENT            34
#define KBASE_GPUPROP_RAW_JS_FEATURES_0            35
#define KBASE_GPUPROP_RAW_JS_FEATURES_1            36
#define KBASE_GPUPROP_RAW_JS_FEATURES_2            37
#define KBASE_GPUPROP_RAW_JS_FEATURES_3            38
#define KBASE_GPUPROP_RAW_JS_FEATURES_4            39
#define KBASE_GPUPROP_RAW_JS_FEATURES_5            40
#define KBASE_GPUPROP_RAW_JS_FEATURES_6            41
#define KBASE_GPUPROP_RAW_JS_FEATURES_7            42
#define KBASE_GPUPROP_RAW_JS_FEATURES_8            43
#define KBASE_GPUPROP_RAW_JS_FEATURES_9            44
#define KBASE_GPUPROP_RAW_JS_FEATURES_10        45
#define KBASE_GPUPROP_RAW_JS_FEATURES_11        46
#define KBASE_GPUPROP_RAW_JS_FEATURES_12        47
#define KBASE_GPUPROP_RAW_JS_FEATURES_13        48
#define KBASE_GPUPROP_RAW_JS_FEATURES_14        49
#define KBASE_GPUPROP_RAW_JS_FEATURES_15        50
#define KBASE_GPUPROP_RAW_TILER_FEATURES        51
#define KBASE_GPUPROP_RAW_TEXTURE_FEATURES_0        52
#define KBASE_GPUPROP_RAW_TEXTURE_FEATURES_1        53
#define KBASE_GPUPROP_RAW_TEXTURE_FEATURES_2        54
#define KBASE_GPUPROP_RAW_GPU_ID            55
#define KBASE_GPUPROP_RAW_THREAD_MAX_THREADS        56
#define KBASE_GPUPROP_RAW_THREAD_MAX_WORKGROUP_SIZE    57
#define KBASE_GPUPROP_RAW_THREAD_MAX_BARRIER_SIZE    58
#define KBASE_GPUPROP_RAW_THREAD_FEATURES        59
#define KBASE_GPUPROP_RAW_COHERENCY_MODE        60

#define KBASE_GPUPROP_COHERENCY_NUM_GROUPS        61
#define KBASE_GPUPROP_COHERENCY_NUM_CORE_GROUPS        62
#define KBASE_GPUPROP_COHERENCY_COHERENCY        63
#define KBASE_GPUPROP_COHERENCY_GROUP_0            64
#define KBASE_GPUPROP_COHERENCY_GROUP_1            65
#define KBASE_GPUPROP_COHERENCY_GROUP_2            66
#define KBASE_GPUPROP_COHERENCY_GROUP_3            67
#define KBASE_GPUPROP_COHERENCY_GROUP_4            68
#define KBASE_GPUPROP_COHERENCY_GROUP_5            69
#define KBASE_GPUPROP_COHERENCY_GROUP_6            70
#define KBASE_GPUPROP_COHERENCY_GROUP_7            71
#define KBASE_GPUPROP_COHERENCY_GROUP_8            72
#define KBASE_GPUPROP_COHERENCY_GROUP_9            73
#define KBASE_GPUPROP_COHERENCY_GROUP_10        74
#define KBASE_GPUPROP_COHERENCY_GROUP_11        75
#define KBASE_GPUPROP_COHERENCY_GROUP_12        76
#define KBASE_GPUPROP_COHERENCY_GROUP_13        77
#define KBASE_GPUPROP_COHERENCY_GROUP_14        78
#define KBASE_GPUPROP_COHERENCY_GROUP_15        79

#ifdef __cpluscplus
}
#endif

#endif
