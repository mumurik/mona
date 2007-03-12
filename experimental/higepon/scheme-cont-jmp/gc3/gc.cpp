#include "GCRecord.h"

GCRecord root;
//static GCRecord root;
static uint32_t gc_heap_min;
static uint32_t gc_heap_max;
static char* gc_stack_bottom;
static char* gc_data_start;
static char* gc_data_end;
static bool gc_initialized = false;
static int gc_count = 0;
static uint32_t gc_total_allocated_size;
static uint32_t gc_total_allocated_count;
static uint32_t gc_total_sweeped_count;
static uint32_t gc_total_sweeped_size;

static bool gc_check_valid_record(GCRecord* r);
inline static GCRecord* gc_is_memory_block(uint32_t address);
static bool gc_check_valid_record(GCRecord* r);
static void gc_mark_stack();
static void gc_mark_global();
static void gc_mark_heap(GCRecord* r);
static void gc_mark_registers();
static void gc_mark();
static void gc_sweep();

#pragma weak __libc_stack_end
extern void* __libc_stack_end;
void* get_stack_bottom()
{
//     long pagesize = sysconf(_SC_PAGESIZE);
//     return (void*)(((uintptr_t)__libc_stack_end + pagesize) & ~(pagesize - 1));
    GC_TRACE_OUT("__libc_stack_end = %x\n", __libc_stack_end);
    return __libc_stack_end;
}


void* operator new(unsigned int size)
{
    return gc_malloc(size);
}

void* operator new(unsigned int size, bool haspointer)
{
    return gc_malloc(size, haspointer);
}

void* operator new[](size_t size)
{
    return gc_malloc(size);
}

void* operator new[](unsigned int size, bool haspointer)
{
    return gc_malloc(size, haspointer);
}

void gc()
{
    gc_mark();
    gc_sweep();
}

void gc_init_internal(char* stack_bottom, char* data_start, char* data_end)
{
    GC_TRACE_OUT("    ==== %s ==== \n", __func__);
    if (gc_initialized)
    {
        return;
    }
    gc_record_initialize(&root);
    gc_heap_max = GC_SAFE_POINTER(0);
    gc_heap_min = GC_SAFE_POINTER(0xfffffff);
    gc_stack_bottom = (char*)get_stack_bottom();
    gc_total_allocated_size = 0;
    gc_total_allocated_count = 0;
    gc_total_sweeped_size = 0;
    gc_total_sweeped_count = 0;
    gc_data_start   = data_start;
    gc_data_end     = data_end;
    gc_initialized  = true;
}

void* gc_malloc(uint32_t size, bool haspointer)
{
    if (!gc_initialized)
    {
        gc_init();
    }

    // gc の呼出しは割り当て前に行わないとdouble free の可能性が
    gc_count++;
    if (gc_count % 5000 == 0)
    {
        gc();
    }

    uint32_t alloc_size = sizeof(GCRecord) + size;
    gc_total_allocated_size += size;
    gc_total_allocated_count++;
    GCRecord* r = (GCRecord*)malloc(alloc_size);
    GC_ASSERT_NOT_NULL(r);
    if ((uint32_t)r < GC_SAFE_POINTER(gc_heap_min)) gc_heap_min = GC_SAFE_POINTER(r);
    if ((uint32_t)r + alloc_size > GC_SAFE_POINTER(gc_heap_max)) gc_heap_max = GC_SAFE_POINTER(r + alloc_size);
    gc_record_initialize(r);
    gc_record_add_to_next(&root, r);
    r->size = size;
    r->haspointer = haspointer;
    GC_TRACE_OUT("    ==== %x(%d) new ====\n", r->data, size);

    return r->data;
}

void gc_fini()
{
    printf("gc:gc_total_allocated_count = %d\n", gc_total_allocated_count);
    printf("gc:gc_total_allocated_size = %d\n", gc_total_allocated_size / 1024);
    printf("gc:gc_total_sweeped_count = %d\n", gc_total_sweeped_count);
    printf("gc:gc_total_sweeped_size = %d\n", gc_total_sweeped_size / 1024);

}

void gc_free(GCRecord* r)
{
    r->magic = GC_MAGIC_NOT_USED;
    free(r);
}


bool gc_check_valid_record(GCRecord* r)
{
    return r->magic == GC_MAGIC;
}

void gc_mark_block(GCRecord* r)
{
    if (r->reachable) return;
    r->reachable = 1;
    return;
}

#define GC_IS_MEMORY_BLOCK(R, A) ((A & 0x3) == 0 && \
                                             A <= GC_SAFE_POINTER(gc_heap_max) && \
                                             A >= GC_SAFE_POINTER(gc_heap_min) && \
                                             (R = (GCRecord*)(A - sizeof(GCRecord)))->magic == GC_MAGIC)

void gc_mark_range(char* from, char* to)
{
    GC_ASSERT((uint32_t)to > (uint32_t)from);
    uint32_t size = ((uint32_t)to - (uint32_t)from) / 4;
#if 0
    for (uint32_t i = 0; i < size; i++)
    {
        uint32_t value = *((uint32_t *)&from[i * 4]);
#else
    uint32_t* address = (uint32_t*)from;
    for (uint32_t i = 0; i < size; i++)
    {
        uint32_t value = address[i];
#endif
#if 1
        GCRecord* r = gc_is_memory_block(value);
        if (r != NULL)
#else
        GCRecord* r;
        if (GC_IS_MEMORY_BLOCK(r, value))
#endif
        {
            GC_TRACE_OUT("      mark=%x\n", value);
            gc_mark_block(r);
        }
    }
}

void gc_mark_stack()
{
    GC_TRACE_OUT("    ==== %s start ====\n", __func__);
    register char* stack_top asm ("%esp");
    gc_mark_range(stack_top, gc_stack_bottom);
    GC_TRACE_OUT("    ==== %s end ====\n", __func__);
}

void gc_mark_global()
{
    GC_TRACE_OUT("    ==== %s start ====\n", __func__);
    gc_mark_range(gc_data_start, gc_data_end);
    GC_TRACE_OUT("    ==== %s end ====\n", __func__);
}

void gc_mark_registers()
{
    myjmp_buf registers;
    mysetjmp(registers);
    // 4byte単位なのに注意
    for (int i = 0; i < _JBLEN; i++)
    {
        uint32_t valueOnRegister = registers[i];
#if 1
        GCRecord* r = gc_is_memory_block(valueOnRegister);
        if (NULL != r)
#else
        GCRecord* r;
        if (GC_IS_MEMORY_BLOCK(r, (uint32_t)registers[i]))
#endif
        {
            GC_TRACE_OUT("      mark=%x\n", valueOnRegister);
            gc_mark_block(r);
        }
    }
}


// not used.
GCRecord* gc_is_memory_block(uint32_t address)
{
    GCRecord* r;
    if (GC_IS_MEMORY_BLOCK(r, address)) return r;
    return NULL;

//     GCRecord* r;
//     if ((address & 0x3) == 0 &&
//         address <= GC_SAFE_POINTER(gc_heap_max) &&
//         address >= GC_SAFE_POINTER(gc_heap_min) &&
//         (r = (GCRecord*)(address - sizeof(GCRecord)))->magic == GC_MAGIC) return r;
//     return NULL;

//     GCRecord* r;
//     if (address & 0x3 ||
//         address > GC_SAFE_POINTER(gc_heap_max) ||
//         address < GC_SAFE_POINTER(gc_heap_min) ||
//         (r = (GCRecord*)(address - sizeof(GCRecord)))->magic != GC_MAGIC) return NULL;
//     return r;

//     // 4byte align
//     if (address & 0x3) return NULL;
//     if (address > GC_SAFE_POINTER(gc_heap_max) || address < GC_SAFE_POINTER(gc_heap_min)) return NULL;
//     GCRecord* r = (GCRecord*)(address - sizeof(GCRecord));
//     if (!gc_check_valid_record(r)) return NULL;
//     return r;
}

void gc_mark_heap(GCRecord* r)
{
    if (!r->reachable || !r->haspointer || r->checkdone) return;
    r->checkdone = 1;
    uint32_t size = r->size / 4;
#if 0
    char* address = r->data;
    for (uint32_t i = 0; i < size; i++)
    {
        uint32_t valueOnHeap = *((uint32_t *) &address[i * 4]);
#else
    uint32_t* address = (uint32_t*)r->data;
    for (uint32_t i = 0; i < size; i++)
    {
        uint32_t valueOnHeap = address[i];
#endif
#if 0
        if (valueOnHeap & 0x3) continue;
        GCRecord* target = gc_is_memory_block(valueOnHeap);
        if (target != NULL)
#else
        GCRecord* target;
        if (GC_IS_MEMORY_BLOCK(target, valueOnHeap))
#endif
        {
            GC_TRACE_OUT("      heap mark=%x\n", valueOnHeap);
            gc_mark_block(target);
            gc_mark_heap(target);
        }
    }
}

void gc_mark()
{
    GC_TRACE_OUT("    ==== %s start ====\n", __func__);
    FOREACH_GC_RECORD(&root, r)
    {
        r->reachable = 0;
        r->checkdone = 0;
    }

    gc_mark_stack();
    gc_mark_global();
    gc_mark_registers();

    FOREACH_GC_RECORD(&root, r)
    {
        gc_mark_heap(r);
    }

    GC_TRACE_OUT("    ==== %s end ====\n", __func__);
}

void gc_sweep()
{
    GC_TRACE_OUT("    ==== %s start ====\n", __func__);
    FOREACH_GC_RECORD(&root, r)
    {
         if (!r->reachable)
         {
            GC_TRACE_OUT("      sweep=%x\n", r->data);
            GCRecord* prev = r->prev;
            gc_record_remove(r);
            gc_total_sweeped_count++;
            gc_total_sweeped_size += r->size;
            gc_free(r);
            r = prev;

        }
    }

    GC_TRACE_OUT("    ==== %s end ====\n", __func__);
}
