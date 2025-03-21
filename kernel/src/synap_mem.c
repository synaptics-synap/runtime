#include <linux/pagemap.h>
#ifdef CONFIG_ION
#include <linux/ion.h>
#else
#include <linux/dma-heap.h>
#endif
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/swap.h>

#include "synap_kernel_log.h"
#include "synap_mem.h"

#include <synap_kernel_ca.h>

#if defined(CONFIG_ION)
static u32 ion_ns_cont_heap_id_mask;
static u32 ion_s_cont_heap_id_mask;
static u32 ion_scatter_heap_id_mask;

bool synap_mem_init(void)
{

    int heap_num, i;
    struct ion_heap_data *hdata;

    hdata = kmalloc(sizeof(*hdata) * ION_NUM_MAX_HEAPS, GFP_KERNEL);
    if (!hdata) {
        KLOGE("unable to look up heaps");
        return false;
    }

    heap_num = ion_query_heaps_kernel(hdata, ION_NUM_MAX_HEAPS);

    ion_ns_cont_heap_id_mask = 0;
    ion_s_cont_heap_id_mask = 0;
    ion_scatter_heap_id_mask = 0;

    for (i = 0; i < heap_num; i++) {
        if (hdata[i]. type == ION_HEAP_TYPE_DMA_CUST) {
            ion_ns_cont_heap_id_mask |= 1 << hdata[i].heap_id;
        }
        if (hdata[i]. type == ION_HEAP_TYPE_BERLIN_SECURE) {
            ion_s_cont_heap_id_mask |= 1 << hdata[i].heap_id;
        }
        if (hdata[i].type == ION_HEAP_TYPE_SYSTEM_CUST) {
            ion_scatter_heap_id_mask |= 1 << hdata[i].heap_id;
        }
    }

    if (ion_ns_cont_heap_id_mask == 0) {
        KLOGE("unable to find ION_HEAP_TYPE_DMA_CUST heap");
        return false;
    }

    if (ion_s_cont_heap_id_mask == 0) {
        KLOGE("unable to find ION_HEAP_TYPE_BERLIN_SECURE heap");
        return false;
    }

    if (ion_ns_cont_heap_id_mask == 0) {
        KLOGE("unable to find ION_HEAP_TYPE_DMA_CUST heap");
        return false;
    }

    kfree(hdata);

    KLOGI("heap mask scatter=0x%x s_cont=0x%x ns_cont=0x%x",
          ion_scatter_heap_id_mask, ion_s_cont_heap_id_mask, ion_ns_cont_heap_id_mask);

    return true;
}

#elif defined(CONFIG_DMABUF_HEAPS)
static struct dma_heap *ns_cont_heap;
static struct dma_heap *s_cont_heap;
static struct dma_heap *scatter_heap;

bool synap_mem_init(void)
{

    ns_cont_heap = dma_heap_find("CMA-CUST-reserved");

    if (!ns_cont_heap) {
        KLOGE("unable to find CMA-CUST-reserved heap");
        return false;
    }

    s_cont_heap = dma_heap_find("Secure");

    if (!s_cont_heap) {
        KLOGE("unable to find Secure heap");
        return false;
    }

    scatter_heap = dma_heap_find("system_cust");

    if (!scatter_heap) {
        KLOGE("unable to find system_cust heap");
        return false;
    }

    return true;
}
#else
#error Either CONFIG_ION or CONFIG_DMABUF_HEAPS must be enabled to use SyNAP
#endif

void synap_mem_free(struct synap_mem* mem)
{

    struct sg_page_iter sg_iter;

    KLOGI("freeing buffer");

    if (!IS_ERR_OR_NULL(mem->sg_table)) {

        if (!IS_ERR_OR_NULL(mem->dmabuf_attach)) {
            dma_buf_unmap_attachment(mem->dmabuf_attach, mem->sg_table, DMA_BIDIRECTIONAL);
        } else {
            for_each_sgtable_page(mem->sg_table, &sg_iter, 0) {
                struct page *page = sg_page_iter_page(&sg_iter);
                mark_page_accessed(page);
                put_page(page);
            }

            sg_free_table(mem->sg_table);
        }

    }

    if (!IS_ERR_OR_NULL(mem->dmabuf_attach)) {
        dma_buf_detach(mem->dmabuf, mem->dmabuf_attach);
    }

    if (!IS_ERR_OR_NULL(mem->dmabuf)) {

        KLOGI("freeing ref=%d", mem->dmabuf->file->f_count);

        dma_buf_put(mem->dmabuf);
    }

    kfree(mem);

}

static struct synap_mem *synap_mem_wrap_pages(struct page **pages, int num_pages, size_t offset,
                                              size_t size) {

    struct synap_mem *mem;

    if ((mem = kzalloc(sizeof(struct synap_mem), GFP_KERNEL)) == NULL) {
        KLOGE("kzalloc synap_mem failed");
        return NULL;
    }

    mem->size = size;
    mem->offset = offset;

    mem->sg_table = kzalloc(sizeof(struct sg_table), GFP_KERNEL);

    if (!mem->sg_table) {
        KLOGE("unable to allocate sgtable");
        synap_mem_free(mem);
        return NULL;
    }

    if (sg_alloc_table_from_pages(mem->sg_table, pages, num_pages, 0,
                                  (u64)num_pages << PAGE_SHIFT, GFP_KERNEL)) {
        synap_mem_free(mem);
        return NULL;
    }

    KLOGI("wrapped %d pages for buffer at offset %d and size %d to sg_table with %d nents",
          num_pages, offset, size, mem->sg_table->nents);

    return mem;

}

static void *synap_mem_alloc_pages(u64 addr, size_t size, u64 *start_addr,
                                   unsigned int *num_pages) {

    if (size == 0) {
        KLOGE("zero size buffer not supported");
        return NULL;
    }

    *start_addr = addr & ~(PAGE_SIZE - 1);
    *num_pages = ((addr - *start_addr) + size + PAGE_SIZE - 1) / PAGE_SIZE;

    KLOGI("allocating pages for buffer at 0x%08x size %d (start address 0x%08x num_pages %d)",
          addr, size, *start_addr, *num_pages);

    return vzalloc(sizeof(struct page*) * (*num_pages));
}


struct synap_mem *synap_mem_wrap_userbuffer(u64 addr, size_t size) {
    unsigned int pinned = 0;
    int r;
    unsigned int num_pages;
    u64 start_addr;

    struct page **pages;
    struct synap_mem *mem;

    pages = synap_mem_alloc_pages(addr, size, &start_addr, &num_pages);

    if (!pages) {
        KLOGE("unable to allocate pages array");
        return NULL;
    }

    do {
        r = get_user_pages(start_addr + pinned * PAGE_SIZE,
                           num_pages - pinned,
                           0, pages + pinned, NULL);
        if (r < 0) {
            release_pages(pages, pinned);
            vfree(pages);
            return NULL;
        }

        pinned += r;

    } while (pinned < num_pages);

    mem = synap_mem_wrap_pages(pages, num_pages, addr - start_addr, size);

    if (!mem) {
        release_pages(pages, pinned);
        vfree(pages);
        KLOGE("unable to allocate mem struct");
        return NULL;
    }

    vfree(pages);

    return mem;
}


static struct synap_mem *synap_mem_wrap_dmabuf(struct synap_device* dev,
                                           struct dma_buf *dmabuf, size_t size, u64 offset) {

    struct synap_mem *mem = NULL;
    u32 dma_buf_size = 0;
    struct scatterlist *sg = NULL;
    size_t i;
    struct device *r_dev = &dev->pdev->dev;

    // we use the MMU of the NPU to map the buffer with the offset so we can only map at
    // page boundries
    if (offset % SYNAP_MEM_NPU_PAGE_SIZE != 0) {
        KLOGE("offset must be multiple of npu page size");
        return NULL;
    }

    if ((mem = kzalloc(sizeof(struct synap_mem), GFP_KERNEL)) == NULL) {
        KLOGE("kzalloc synap_mem failed");
        return NULL;
    }

    KLOGI("wrapping buffer %p", dmabuf);

    mem->size = size;
    mem->offset = offset;
    mem->dmabuf = dmabuf;

    mem->dmabuf_attach = dma_buf_attach(mem->dmabuf, r_dev);

    if (IS_ERR(mem->dmabuf_attach)) {
        KLOGE("attach dmabuf failed when register memory");
        synap_mem_free(mem);
        return NULL;
    }

    // FIXME: we could be more efficient if we knew the direction
    mem->sg_table = dma_buf_map_attachment(mem->dmabuf_attach, DMA_BIDIRECTIONAL);
    if (IS_ERR(mem->sg_table)) {
        KLOGE("get areas from sgtable failed");
        synap_mem_free(mem);
        return NULL;
    }

    for_each_sg(mem->sg_table->sgl, sg, mem->sg_table->nents, i) {
        dma_buf_size += sg->length;
    }

    // the backing dma buffer must have enough space to fit the whole buffer
    // including the padding till the end of the page after the end of the
    // buffer otherwise we may map unexpected things in the NPU page table
    if (SYNAP_MEM_ALIGN_SIZE(size) + offset > dma_buf_size) {
        KLOGE("specified range doesn't fit within the dmabuf");
        synap_mem_free(mem);
        return NULL;
    }

    if (size == 0) {
        KLOGE("zero size buffer not supported");
        synap_mem_free(mem);
        return NULL;
    }

    return mem;
}

#if (DEBUG_LEVEL >= 2)
static const char* synap_mem_type_to_str(synap_mem_type type)
{
#define TO_STR(x) case x : return #x;
    switch (type) {
        TO_STR(SYNAP_MEM_POOL);
        TO_STR(SYNAP_MEM_CODE);
        TO_STR(SYNAP_MEM_PAGE_TABLE);
        TO_STR(SYNAP_MEM_IO_BUFFER);
        TO_STR(SYNAP_MEM_DRIVER_BUFFER);
        default: return "UNKNOWN TYPE";
    }
}
#endif

struct synap_mem *synap_mem_alloc(struct synap_device* dev, synap_mem_type mem_type,
                                  bool secure, size_t size)
{
    struct synap_mem *mem = NULL;
    struct dma_buf *dmabuf;

#if defined(CONFIG_ION)
    u32 heap_mask = ion_scatter_heap_id_mask;

    /* page table for the moment requires contiguous,
       driver buffers will always require contiguous memory*/
    if (mem_type == SYNAP_MEM_PAGE_TABLE || mem_type == SYNAP_MEM_DRIVER_BUFFER) {
        if (secure) {
            heap_mask = ion_s_cont_heap_id_mask;
        } else {
            heap_mask = ion_ns_cont_heap_id_mask;
        }
    }

    dmabuf = ion_alloc(size, heap_mask, ION_CACHED);

#elif defined(CONFIG_DMABUF_HEAPS)
    struct dma_heap *heap = scatter_heap;

    /* page table for the moment requires contiguous,
       driver buffers will always require contiguous memory*/
    if (mem_type == SYNAP_MEM_PAGE_TABLE || mem_type == SYNAP_MEM_DRIVER_BUFFER) {
        if (secure) {
            heap = s_cont_heap;
        } else {
            heap = ns_cont_heap;
        }
    }

    dmabuf = dma_heap_buffer_alloc(heap, size, O_RDWR | O_CLOEXEC, 0);
#else
#error Either CONFIG_ION or CONFIG_DMABUF_HEAPS must be enabled to use SyNAP
#endif

    if (IS_ERR_OR_NULL(dmabuf)) {
        KLOGE("dmabuf alloc failed, size=%d", size);
        return NULL;
    }

    mem = synap_mem_wrap_dmabuf(dev, dmabuf, size, 0);

    if (!mem) {
        dma_buf_put(dmabuf);
        return NULL;
    }

    KLOGI("allocate mem type=%s size=%d %ssecure",
          synap_mem_type_to_str(mem_type), size, secure ? "" : "non");

    return mem;
}

struct synap_mem *synap_mem_wrap_fd(struct synap_device* dev, int fd, size_t size, u64 offset) {
    struct synap_mem *mem = NULL;
    struct dma_buf *dmabuf;

    dmabuf = dma_buf_get(fd);

    if (IS_ERR_OR_NULL(dmabuf)) {
        KLOGE("fd get failed for dmabuf, fd=%d size=%d", fd, size);
        return NULL;
    }

    mem = synap_mem_wrap_dmabuf(dev, dmabuf, size, offset);

    if (!mem) {
        KLOGE("failed to wrap dmabuf")
        dma_buf_put(dmabuf);
        return NULL;
    }

    return mem;

}

void* synap_mem_map_dmabuf_vaddr(struct synap_mem* mem)
{
    if (dma_buf_begin_cpu_access(mem->dmabuf, DMA_BIDIRECTIONAL) != 0) {
        return NULL;
    }

    mem->dmabuf_vaddr = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
    mem->dmabuf_vaddr = dma_buf_vmap(mem->dmabuf);
#else
    if (dma_buf_vmap(mem->dmabuf, &mem->dmabuf_map) >= 0) {
        mem->dmabuf_vaddr = mem->dmabuf_map.vaddr;
    }
#endif

    if (mem->dmabuf_vaddr == NULL) {
        KLOGE("dmabuf kmap failed.");
    }

    return mem->dmabuf_vaddr;
}

void synap_mem_unmap_dmabuf_vaddr(struct synap_mem* mem)
{
    if (!mem || !mem->dmabuf_vaddr) {
        return;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0)
    dma_buf_vunmap(mem->dmabuf, mem->dmabuf_vaddr);
#else
    dma_buf_vunmap(mem->dmabuf, &mem->dmabuf_map);
#endif

    dma_buf_end_cpu_access(mem->dmabuf, DMA_BIDIRECTIONAL);
}


