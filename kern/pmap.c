#include <drivers/dev_mp.h>
#include <env.h>
#include <mmu.h>
#include <pmap.h>
#include <printk.h>
#include <swap.h>
#include <string.h>


/* These variables are set by mips_detect_memory() */
static u_long memsize; /* Maximum physical address */
u_long npage;	       /* Amount of memory(in pages) */

Pde* cur_pgdir;

struct Page* pages;
static u_long freemem;

struct Page_list page_free_list; /* Free list of physical pages */

/* Overview:
 *   Read memory size from DEV_MP to initialize 'memsize' and calculate the corresponding 'npage'
 *   value.
 */
void mips_detect_memory()
{
	/* Step 1: Initialize memsize. */
	memsize = *(volatile u_int*)(KSEG1 | DEV_MP_ADDRESS | DEV_MP_MEMORY);

	/* Step 2: Calculate the corresponding 'npage' value. */
	/* Exercise 2.1: Your code here. */
	npage = memsize / BY2PG;

	printk("Memory size: %lu KiB, number of pages: %lu\n", memsize / 1024, npage);
	/*
	 * Memory size: 65536 KiB, number of pages: 16384
	 */
}

/* Overview:
 *   Allocate `n` bytes physical memory with alignment `align`, if `clear` is set,
 * clear the allocated memory.
 *   This allocator is used only while setting up virtual memory system.
 * Post-Condition:
 *   If we're out of memory, should panic, else return this address of memory we
 * have allocated.
 */
void* alloc(u_int n, u_int align, int clear)
{
	extern char end[];
	u_long alloced_mem;

	/* Initialize `freemem` if this is the first time. The first virtual
	 * address that the linker did *not* assign to any kernel code or global
	 * variables.
	 */
	if (freemem == 0)
	{
		freemem = (u_long)end; // end
	}

	/* Step 1: Round up `freemem` up to be aligned properly */
	freemem = ROUND(freemem, align);

	/* Step 2: Save current value of `freemem` as allocated chunk. */
	alloced_mem = freemem;

	/* Step 3: Increase `freemem` to record allocation. */
	freemem = freemem + n;

	// Panic if we're out of memory.
	panic_on(PADDR(freemem) >= memsize);

	/* Step 4: Clear allocated chunk if parameter `clear` is set. */
	if (clear)
	{
		memset((void*)alloced_mem, 0, n);
	}

	/* Step 5: return allocated chunk. */
	return (void*)alloced_mem;
}

/* Overview:
 *   Set up two-level page table.
 * Hint:
 *   You can get more details about `UPAGES` and `UENVS` in include/mmu.h.
 */
void mips_vm_init()
{
	/* Allocate proper size of physical memory for global array `pages`,
	 * for physical memory management. Then, map virtual address `UPAGES` to
	 * physical address `pages` allocated before. For consideration of alignment,
	 * you should round up the memory size before map. */
	pages = (struct Page*)alloc(npage * sizeof(struct Page), BY2PG, 1);
	printk("to memory %x for struct Pages.\n", freemem);
	printk("pmap.c:\t mips vm init success\n");
}

/* Overview:
 *   Initialize page structure and memory free list. The 'pages' array has one 'struct Page' entry
 * per physical page. Pages are reference counted, and free pages are kept on a linked list.
 *
 * Hint: Use 'LIST_INSERT_HEAD' to insert free pages to 'page_free_list'.
 */
void page_init(void)
{
	/* Step 1: Initialize page_free_list. */
	/* Hint: Use macro `LIST_INIT` defined in include/queue.h. */
	/* Exercise 2.3: Your code here. (1/4) */
	LIST_INIT(&page_free_list);

	/* Step 2: Align `freemem` up to multiple of BY2PG. */
	/* Exercise 2.3: Your code here. (2/4) */
	freemem = ROUND(freemem, BY2PG);

	/* Step 3: Mark all memory below `freemem` as used (set `pp_ref` to 1) */
	/* Exercise 2.3: Your code here. (3/4) */
	u_long allocated_pages = page2ppn(pa2page(PADDR(freemem)));
	for (u_long i = 0; i < allocated_pages; i++)
	{
		pages[i].pp_ref = 1;
	}

	/* Step 4: Mark the other memory as free. */
	/* Exercise 2.3: Your code here. (4/4) */

	for (u_long i = allocated_pages; i < npage; i++)
	{
		LIST_INSERT_HEAD(&page_free_list, &pages[i], pp_link);
		pages[i].pp_ref = 0;
	}
}

/* Overview:
 *   Allocate a physical page from free memory, and fill this page with zero.
 *
 * Post-Condition:
 *   If failed to allocate a new page (out of memory, there's no free page), return -E_NO_MEM.
 *   Otherwise, set the address of the allocated 'Page' to *pp, and return 0.
 *
 * Note:
 *   This does NOT increase the reference count 'pp_ref' of the page - the caller must do these if
 *   necessary (either explicitly or via page_insert).
 *
 * Hint: Use LIST_FIRST and LIST_REMOVE defined in include/queue.h.
 */
int page_alloc(struct Page** new)
{
	/* Step 1: Get a page from free memory. If fails, return the error code.*/
	struct Page* pp;

	/* Exercise 2.4: Your code here. (1/2) */
	if (LIST_EMPTY(&page_free_list))
		return -E_NO_MEM;
	pp = page_free_list.lh_first;

	LIST_REMOVE(pp, pp_link);

	/* Step 2: Initialize this page with zero.
	 * Hint: use `memset`. */
	 /* Exercise 2.4: Your code here. (2/2) */
	memset((void*)page2kva(pp), 0, BY2PG);

	*new = pp;

	return 0;
}

/* Overview:
 *   Release a page 'pp', mark it as free.
 *
 * Pre-Condition:
 *   'pp->pp_ref' is '0'.
 */
void page_free(struct Page* pp)
{
	assert(pp->pp_ref == 0);
	/* Just insert it into 'page_free_list'. */
	/* Exercise 2.5: Your code here. */
	LIST_INSERT_HEAD(&page_free_list, pp, pp_link);
}

/* Overview:
 *   Given 'pgdir', a pointer to a page directory, 'pgdir_walk' returns a pointer to the page table
 *   entry (with permission PTE_D|PTE_V) for virtual address 'va'.
 *
 * Pre-Condition:
 *   'pgdir' is a two-level page table structure.
 *
 * Post-Condition:
 *   If we're out of memory, return -E_NO_MEM.
 *   Otherwise, we get the page table entry, store
 *   the value of page table entry to *ppte, and return 0, indicating success.
 *
 * Hint:
 *   We use a two-level pointer to store page table entry and return a state code to indicate
 *   whether this function succeeds or not.
 */
static int pgdir_walk(Pde* pgdir, u_long va, int create, Pte** ppte)
{
	Pde* pgdir_entryp;

	struct Page* pp;

	u_long pdx = PDX(va);
	u_long ptx = PTX(va);
	
	/* Step 1: Get the corresponding page directory entry. */
	/* Exercise 2.6: Your code here. (1/3) */

	// The address of the page directory entry.
	// The value of the entry points to the page table, and contains flags
	pgdir_entryp = pgdir + pdx;

	/* Step 2: If the corresponding page table is not existent (valid) and
	 * parameter `create` is set, create one. Set the permission bits
	 * 'PTE_D | PTE_V' for this new page in the page directory.
	 * If failed to allocate a new page (out of memory), return the error. */
	 /* Exercise 2.6: Your code here. (2/3) */
	if (!(*pgdir_entryp & PTE_V))
	{
		if (create)
		{
			/*
			 * Here, we want to construct a page for page table.
			 * The page allocated will be used to store page table.
			 */
			int ret = page_alloc(&pp);
			if (ret != 0)
			{
				ppte = NULL;
				return ret;	// allocation failed
			}

			pp->pp_ref++;
			// see page_insert(), which uses page2pa
			/*
			 * The content of pgdir_entryp (*pgdir_entryp) points to the base
			 * physical address of page table, and since it only takes 20 bits,
			 * the remaining 12 bits are used for permissions.
			 */
			*pgdir_entryp = page2pa(pp);
			*pgdir_entryp |= (PTE_D | PTE_V);
		}
		else
		{
			*ppte = NULL;
			return 1;
		}
	}

	/* Step 3: Assign the kernel virtual address of the page table entry to '*ppte'. */
	/* Exercise 2.6: Your code here. (3/3) */
	
	// KADDR is for the access of kernel.
	*ppte = (Pte*)KADDR(PTE_ADDR(*pgdir_entryp)) + ptx;
	/*
	 * Must not be this! If so, KADDR will add offset to the address of
	 * type Pte*, which will cause 4 times of offset!
	 *
	ppte = (Pte*)KADDR((Pte*)PTE_ADDR(*pgdir_entryp) + ptx);
	 *
	 * An alternate way is to add pointer offset manually.
	 *
	ppte = (Pte*)KADDR(PTE_ADDR(*pgdir_entryp) + (ptx << 2));
	 *
	 */
	
	return 0;
}

/* Overview:
 *   Map the physical page 'pp' at virtual address 'va'. The permission (the low 12 bits) of the
 *   page table entry should be set to 'perm|PTE_V'.
 *
 * Post-Condition:
 *   Return 0 on success
 *   Return -E_NO_MEM, if page table couldn't be allocated
 *
 * Hint:
 *   If there is already a page mapped at `va`, call page_remove() to release this mapping.
 *   The `pp_ref` should be incremented if the insertion succeeds.
 */
int page_insert(Pde* pgdir, u_int asid, struct Page* pp, u_long va, u_int perm)
{
	Pte* pte;

	/* Step 1: Get corresponding page table entry. */
	pgdir_walk(pgdir, va, 0, &pte);

	/*
	 * pte is the address of page table entry, and just like *pgdir_entryp, the
	 * content of pte (*pte) contains the physical address of a page (4K, lowest
	 * 12 bits are 0) and permission bits. Then, after we get the physical
	 * address, we can combine that with inner page offset (lowest 12 bits in va)
	 * to access the memory.
	 */

	if (pte && (*pte & PTE_V))
	{
		// page table exists
		if (pa2page(*pte) != pp)
		{
			// another page mapped to va
			page_remove(pgdir, asid, va);
		}
		else
		{
			// current page already mapped to va
			tlb_invalidate(asid, va);
			*pte = page2pa(pp) | perm | PTE_V;
			// *pte |= (perm | PTE_V);
			return 0;
		}
	}

	/* Step 2: Flush TLB with 'tlb_invalidate'. */
	/* Exercise 2.7: Your code here. (1/3) */
	tlb_invalidate(asid, va);

	/* Step 3: Re-get or create the page table entry. */
	/* If failed to create, return the error. */
	/* Exercise 2.7: Your code here. (2/3) */
	if (pgdir_walk(pgdir, va, 1, &pte) != 0)
		return -E_NO_MEM;

	/* Step 4: Insert the page to the page table entry with 'perm | PTE_V' and increase its
	 * 'pp_ref'. */
	 /* Exercise 2.7: Your code here. (3/3) */
	*pte = page2pa(pp) | perm | PTE_V;
	pp->pp_ref++;

	return 0;
}

/*Overview:
	Look up the Page that virtual address `va` map to.
  Post-Condition:
	Return a pointer to corresponding Page, and store it's page table entry to *ppte.
	If `va` doesn't mapped to any Page, return NULL.*/
struct Page* page_lookup(Pde* pgdir, u_long va, Pte** ppte)
{
	struct Page* pp;
	Pte* pte;

	/* Step 1: Get the page table entry. */
	pgdir_walk(pgdir, va, 0, &pte);

	/* Hint: Check if the page table entry doesn't exist or is not valid. */
	if (pte == NULL || (*pte & PTE_V) == 0)
	{
		return NULL;
	}

	/* Step 2: Get the corresponding Page struct. */
	/* Hint: Use function `pa2page`, defined in include/pmap.h . */
	pp = pa2page(*pte);
	if (ppte)
	{
		*ppte = pte;
	}

	return pp;
}

/* Overview:
 *   Decrease the 'pp_ref' value of Page 'pp'.
 *   When there's no references (mapped virtual address) to this page, release it.
 */
void page_decref(struct Page* pp)
{
	assert(pp->pp_ref > 0);

	/* If 'pp_ref' reaches to 0, free this page. */
	if (--pp->pp_ref == 0)
	{
		page_free(pp);
	}
}

// Overview:
//   Unmap the physical page at virtual address 'va'.
void page_remove(Pde* pgdir, u_int asid, u_long va)
{
	Pte* pte;

	/* Step 1: Get the page table entry, and check if the page table entry is valid. */
	struct Page* pp = page_lookup(pgdir, va, &pte);
	if (pp == NULL)
	{
		return;
	}

	/* Step 2: Decrease reference count on 'pp'. */
	page_decref(pp);

	/* Step 3: Flush TLB. */
	*pte = 0;
	tlb_invalidate(asid, va);

	return;
}

/* Overview:
 *   Invalidate the TLB entry with specified 'asid' and virtual address 'va'.
 *
 * Hint:
 *   Construct a new Entry HI and call 'tlb_out' to flush TLB.
 *   'tlb_out' is defined in mm/tlb_asm.S
 */
void tlb_invalidate(u_int asid, u_long va)
{
	tlb_out(PTE_ADDR(va) | (asid << 6));
}

void physical_memory_manage_check(void)
{
	struct Page* pp, * pp0, * pp1, * pp2;
	struct Page_list fl;
	int* temp;

	// should be able to allocate three pages
	pp0 = pp1 = pp2 = 0;
	assert(page_alloc(&pp0) == 0);
	assert(page_alloc(&pp1) == 0);
	assert(page_alloc(&pp2) == 0);

	assert(pp0);
	assert(pp1 && pp1 != pp0);
	assert(pp2 && pp2 != pp1 && pp2 != pp0);

	// temporarily steal the rest of the free pages
	fl = page_free_list;
	// now this page_free list must be empty!!!!
	LIST_INIT(&page_free_list);
	// should be no free memory
	assert(page_alloc(&pp) == -E_NO_MEM);

	temp = (int*)page2kva(pp0);
	// write 1000 to pp0
	*temp = 1000;
	// free pp0
	page_free(pp0);
	printk("The number in address temp is %d\n", *temp);

	// alloc again
	assert(page_alloc(&pp0) == 0);
	assert(pp0);

	// pp0 should not change
	assert(temp == (int*)page2kva(pp0));
	// pp0 should be zero
	assert(*temp == 0);

	page_free_list = fl;
	page_free(pp0);
	page_free(pp1);
	page_free(pp2);
	struct Page_list test_free;
	struct Page* test_pages;
	test_pages = (struct Page*)alloc(10 * sizeof(struct Page), BY2PG, 1);
	LIST_INIT(&test_free);
	// LIST_FIRST(&test_free) = &test_pages[0];
	int i, j = 0;
	struct Page* p, * q;
	for (i = 9; i >= 0; i--)
	{
		test_pages[i].pp_ref = i;
		// test_pages[i].pp_link=NULL;
		// printk("0x%x  0x%x\n",&test_pages[i], test_pages[i].pp_link.le_next);
		LIST_INSERT_HEAD(&test_free, &test_pages[i], pp_link);
		// printk("0x%x  0x%x\n",&test_pages[i], test_pages[i].pp_link.le_next);
	}
	p = LIST_FIRST(&test_free);
	int answer1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	assert(p != NULL);
	while (p != NULL)
	{
		// printk("%d %d\n",p->pp_ref,answer1[j]);
		assert(p->pp_ref == answer1[j++]);
		// printk("ptr: 0x%x v: %d\n",(p->pp_link).le_next,((p->pp_link).le_next)->pp_ref);
		p = LIST_NEXT(p, pp_link);
	}
	// insert_after test
	int answer2[] = { 0, 1, 2, 3, 4, 20, 5, 6, 7, 8, 9 };
	q = (struct Page*)alloc(sizeof(struct Page), BY2PG, 1);
	q->pp_ref = 20;

	// printk("---%d\n",test_pages[4].pp_ref);
	LIST_INSERT_AFTER(&test_pages[4], q, pp_link);
	// printk("---%d\n",LIST_NEXT(&test_pages[4],pp_link)->pp_ref);
	p = LIST_FIRST(&test_free);
	j = 0;
	// printk("into test\n");
	while (p != NULL)
	{
		//      printk("%d %d\n",p->pp_ref,answer2[j]);
		assert(p->pp_ref == answer2[j++]);
		p = LIST_NEXT(p, pp_link);
	}

	printk("physical_memory_manage_check() succeeded\n");
}

void page_check(void)
{
	Pde* boot_pgdir = alloc(BY2PG, BY2PG, 1);
	struct Page* pp, * pp0, * pp1, * pp2;
	struct Page_list fl;

	// should be able to allocate three pages
	pp0 = pp1 = pp2 = 0;
	assert(page_alloc(&pp0) == 0);
	assert(page_alloc(&pp1) == 0);
	assert(page_alloc(&pp2) == 0);

	assert(pp0);
	assert(pp1 && pp1 != pp0);
	assert(pp2 && pp2 != pp1 && pp2 != pp0);

	// temporarily steal the rest of the free pages
	fl = page_free_list;
	// now this page_free list must be empty!!!!
	LIST_INIT(&page_free_list);

	// should be no free memory
	assert(page_alloc(&pp) == -E_NO_MEM);

	// there is no free memory, so we can't allocate a page table
	assert(page_insert(boot_pgdir, 0, pp1, 0x0, 0) < 0);

	// free pp0 and try again: pp0 should be used for page table
	page_free(pp0);
	assert(page_insert(boot_pgdir, 0, pp1, 0x0, 0) == 0);
	assert(PTE_ADDR(boot_pgdir[0]) == page2pa(pp0));

	printk("va2pa(boot_pgdir, 0x0) is %x\n", va2pa(boot_pgdir, 0x0));
	printk("page2pa(pp1) is %x\n", page2pa(pp1));
	//	printk("pp1->pp_ref is %d\n",pp1->pp_ref);
	assert(va2pa(boot_pgdir, 0x0) == page2pa(pp1));
	assert(pp1->pp_ref == 1);

	// should be able to map pp2 at BY2PG because pp0 is already allocated for page table
	assert(page_insert(boot_pgdir, 0, pp2, BY2PG, 0) == 0);
	assert(va2pa(boot_pgdir, BY2PG) == page2pa(pp2));
	assert(pp2->pp_ref == 1);

	// should be no free memory
	assert(page_alloc(&pp) == -E_NO_MEM);

	printk("start page_insert\n");
	// should be able to map pp2 at BY2PG because it's already there
	assert(page_insert(boot_pgdir, 0, pp2, BY2PG, 0) == 0);
	assert(va2pa(boot_pgdir, BY2PG) == page2pa(pp2));
	assert(pp2->pp_ref == 1);

	// pp2 should NOT be on the free list
	// could happen in ref counts are handled sloppily in page_insert
	assert(page_alloc(&pp) == -E_NO_MEM);

	// should not be able to map at PDMAP because need free page for page table
	assert(page_insert(boot_pgdir, 0, pp0, PDMAP, 0) < 0);

	// insert pp1 at BY2PG (replacing pp2)
	assert(page_insert(boot_pgdir, 0, pp1, BY2PG, 0) == 0);

	// should have pp1 at both 0 and BY2PG, pp2 nowhere, ...
	assert(va2pa(boot_pgdir, 0x0) == page2pa(pp1));
	assert(va2pa(boot_pgdir, BY2PG) == page2pa(pp1));
	// ... and ref counts should reflect this
	assert(pp1->pp_ref == 2);
	printk("pp2->pp_ref %d\n", pp2->pp_ref);
	assert(pp2->pp_ref == 0);
	printk("end page_insert\n");

	// pp2 should be returned by page_alloc
	assert(page_alloc(&pp) == 0 && pp == pp2);

	// unmapping pp1 at 0 should keep pp1 at BY2PG
	page_remove(boot_pgdir, 0, 0x0);
	assert(va2pa(boot_pgdir, 0x0) == ~0);
	assert(va2pa(boot_pgdir, BY2PG) == page2pa(pp1));
	assert(pp1->pp_ref == 1);
	assert(pp2->pp_ref == 0);

	// unmapping pp1 at BY2PG should free it
	page_remove(boot_pgdir, 0, BY2PG);
	assert(va2pa(boot_pgdir, 0x0) == ~0);
	assert(va2pa(boot_pgdir, BY2PG) == ~0);
	assert(pp1->pp_ref == 0);
	assert(pp2->pp_ref == 0);

	// so it should be returned by page_alloc
	assert(page_alloc(&pp) == 0 && pp == pp1);

	// should be no free memory
	assert(page_alloc(&pp) == -E_NO_MEM);

	// forcibly take pp0 back
	assert(PTE_ADDR(boot_pgdir[0]) == page2pa(pp0));
	boot_pgdir[0] = 0;
	assert(pp0->pp_ref == 1);
	pp0->pp_ref = 0;

	// give free list back
	page_free_list = fl;

	// free the pages we took
	page_free(pp0);
	page_free(pp1);
	page_free(pp2);

	printk("page_check() succeeded!\n");
}


/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** lab02-extra-off
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct Page_list page_free_swappable_list;
static u_char* disk_alloc();
static void disk_free(u_char* pdisk);

static const int PAGE_ENTRY_CNT = 1024;

void swap_init()
{
	LIST_INIT(&page_free_swappable_list);
	for (int i = SWAP_PAGE_BASE; i < SWAP_PAGE_END; i += BY2PG)
	{
		struct Page* pp = pa2page(i);
		LIST_REMOVE(pp, pp_link);
		LIST_INSERT_HEAD(&page_free_swappable_list, pp, pp_link);
	}
}

// Interface for 'Passive Swap Out'
struct Page* swap_alloc(Pde* pgdir, u_int asid)
{
	// Step 1: Ensure free page
	if (LIST_EMPTY(&page_free_swappable_list))
	{
		// printk("Not enough free swappable page\n");
		
		/* Your Code Here (1/3) */
		struct Page* pp = pa2page(SWAP_PAGE_BASE);
		u_char* da = disk_alloc();
		
		for (int pdx = 0; pdx < PAGE_ENTRY_CNT; pdx++)
		{
			if (!(pgdir[pdx] & PTE_V))
				continue;
			/*
			 * The address of page table is kernel address, but the 
			 * content of page table entry is physical address.
			 */
			Pte* pgtbl = (Pte*)KADDR(PTE_ADDR(pgdir[pdx]));
			for (int ptx = 0; ptx < PAGE_ENTRY_CNT; ptx++)
			{
				if (!(pgtbl[ptx] & PTE_V))
					continue;
				if (PPN(pgtbl[ptx]) == page2ppn(pp))
				{
					u_long va = (pdx << PDSHIFT) | (ptx << PGSHIFT);
					u_long perm = PTE_PERM(perm);
					perm &= ~PTE_V;
					perm |= PTE_SWP;
					pgtbl[ptx] = PTE_ADDR(da) | perm;
					tlb_invalidate(asid, va);
				}
			}
		}

		memcpy(da, (void*)page2kva(pp), BY2PG);

		LIST_INSERT_HEAD(&page_free_swappable_list, pp, pp_link);
	}

	// Step 2: Get a free page and clear it
	struct Page* pp = LIST_FIRST(&page_free_swappable_list);
	LIST_REMOVE(pp, pp_link);
	memset((void*)page2kva(pp), 0, BY2PG);

	return pp;
}

// Interfaces for 'Active Swap In'
static int is_swapped(Pde* pgdir, u_long va)
{
	/* Your Code Here (2/3) */
	Pte* pte;

	// va = ROUNDDOWN(va, BY2PG);
	pgdir_walk(pgdir, va, 0, &pte);

	return pte && (*pte & PTE_SWP) && !(*pte & PTE_V);
}

static void swap(Pde* pgdir, u_int asid, u_long va)
{
	// printk("INVOKE: swap()\n");

	/* Your Code Here (3/3) */
	// va = ROUNDDOWN(va, BY2PG);
	struct Page* pp = swap_alloc(pgdir, asid);
	Pte* pte;

	/*
	 * Find the pte corresponding to given va.
	 * Notice that, this va is one of the pte that mapped to the swapped out
	 * page, while all pte(s) that mapped to the swapped page contains the
	 * da we want.
	 */
	page_lookup(pgdir, va, &pte);
	assert(pte);

	u_char* da = (u_char*)PTE_ADDR(*pte);
	memcpy((void*)page2kva(pp), da, BY2PG);

	for (int pdx = 0; pdx < PAGE_ENTRY_CNT; pdx++)
	{
		if (!(pgdir[pdx] & PTE_V))
			continue;

		Pte* pgtbl = (Pte*)KADDR(PTE_ADDR(pgdir[pdx]));
		for (int ptx = 0; ptx < PAGE_ENTRY_CNT; ptx++)
		{
			if (!((pgtbl[ptx] & PTE_SWP) && !(pgtbl[ptx] & PTE_V)))
				continue;
			if (PPN(pgtbl[ptx]) == PPN(da))
			{
				u_long va = (pdx << PDSHIFT) | (ptx << PGSHIFT);
				u_long perm = PTE_PERM(pgtbl[ptx]);
				PTE_SET(perm, PTE_V);
				PTE_CLR(perm, PTE_SWP);
				pgtbl[ptx] = page2pa(pp) | perm;
				tlb_invalidate(asid, va);
			}
		}
	}

	disk_free(da);
}

Pte swap_lookup(Pde* pgdir, u_int asid, u_long va)
{
	// Step 1: If corresponding page is swapped out, swap it in
	if (is_swapped(pgdir, va))
	{
		swap(pgdir, asid, va);
	}

	// Step 2: Look up page table element.
	Pte* ppte;
	page_lookup(pgdir, va, &ppte);

	// Step 3: Return
	return ppte == NULL ? 0 : *ppte;
}

// Disk Simulation (Do not modify)
u_char swap_disk[SWAP_DISK_NPAGE * BY2PG] __attribute__((aligned(BY2PG)));
u_char swap_disk_used[SWAP_DISK_NPAGE];

static u_char* disk_alloc()
{
	int alloc = 0;
	for (; alloc < SWAP_DISK_NPAGE && swap_disk_used[alloc]; alloc++)
	{
		;
	}
	assert(alloc < SWAP_DISK_NPAGE);
	swap_disk_used[alloc] = 1;
	return &swap_disk[alloc * BY2PG];
}

static void disk_free(u_char* pdisk)
{
	int offset = pdisk - swap_disk;
	assert(offset % BY2PG == 0);
	swap_disk_used[offset / BY2PG] = 0;
}