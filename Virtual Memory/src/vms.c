#include "vms.h"

#include "mmu.h"
#include "pages.h"

#include <bits/types/locale_t.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void page_fault_handler(void* virtual_address, int level, void* page_table) {
    uint64_t* l0_page_table_entry = vms_page_table_pte_entry(page_table, virtual_address, level);
    
    if (!vms_pte_valid(l0_page_table_entry)) {
        // Handle the page fault due to an invalid page
        printf("Page Fault: Invalid access at L%d virtual address %p\n", level, virtual_address);
        return;
    }

    if (!vms_pte_write(l0_page_table_entry)) {
        // Create a copy of the page and make it writable
        void* page = vms_ppn_to_page(vms_pte_get_ppn(l0_page_table_entry));
        void* copy_page = vms_new_page();
        memcpy(copy_page, page, PAGE_SIZE);
        
        vms_pte_set_ppn(l0_page_table_entry, vms_page_to_ppn(copy_page));
        vms_pte_write_set(l0_page_table_entry);
    } else {
        // Handle other page fault cases
        printf("Page Fault: Invalid access at L%d virtual address %p\n", level, virtual_address);
    }
}

void vms_set_all_pages_read_only(void* page_table, int level) {
    for (int i = 0; i < NUM_PTE_ENTRIES; ++i) {
        uint64_t* entry = vms_page_table_pte_entry_from_index(page_table, i);

        if (vms_pte_valid(entry)) {
            if (level > 0) {
                // Recursively set child page tables as read-only
                void* child_page_table = vms_ppn_to_page(vms_pte_get_ppn(entry));
                vms_set_all_pages_read_only(child_page_table, level - 1);
            }
            // Only set the PTE as read-only at levels above 0
            if (level > 0) {
                vms_pte_write_clear(entry);
            }
        }
    }
}


void Forkhelper(void* parent, void *child, int level) {
    for (int i = 0; i < NUM_PTE_ENTRIES; ++i) {
        uint64_t* entry = vms_page_table_pte_entry_from_index(parent, i);
        uint64_t* child_PTR = vms_page_table_pte_entry_from_index(child, i);
        *child_PTR = *entry;

        if (vms_pte_valid(entry)) {
            if (level == 0) {
                void* parent_subpage = vms_ppn_to_page(vms_pte_get_ppn(entry));
                void* child_subpage = vms_new_page();
                // Update the parent entry to point to the new child page table
                
                memcpy(child_subpage, parent_subpage, PAGE_SIZE);
                vms_pte_set_ppn(child_PTR, vms_page_to_ppn(child_subpage));
                
                vms_pte_valid_set(child_PTR);
                printf("Level 0\n");
                //printf("Level 0 PPN %0lX\n", vms_pte_get_ppn(entryCopy));
            }else if (level > 0){
                  // Recursively copy the child page tables
                void* parent_subpage = vms_ppn_to_page(vms_pte_get_ppn(entry));
                void* child_subpage = vms_new_page();
                // Update the parent entry to point to the new child page table
                
                vms_pte_set_ppn(child_PTR, vms_page_to_ppn(child_subpage));
                Forkhelper(parent_subpage, child_subpage, level - 1);

                //vms_pte_valid_set(entryCopy);
                printf("Level %d (done)\n", level);
                //printf("Level %d PPN %0lX\n",level, vms_pte_get_ppn(entryCopy));
            }
        }
    }
}


void* vms_fork_copy() {
    void* parent_l2 = vms_get_root_page_table();
    void* child = vms_new_page();
    
    Forkhelper(parent_l2, child, 2);

    return child;
}

void* vms_fork_copy_on_write() {
    void* parent_l2 = vms_get_root_page_table();
    void* child = vms_new_page();
    
    Forkhelper(parent_l2, child, 2);

    // Mark all pages as read-only
    vms_set_all_pages_read_only(child, 2);

    return child;
}

