#include "vms.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int expected_exit_status() { return 0; }

void test() {
    vms_init();

    void* l2 = vms_new_page();
    void* l1 = vms_new_page();
    void* l0 = vms_new_page();
    void* p0 = vms_new_page();

    void* virtual_address = (void*) 0xABC123;
    uint64_t* l2_entry = vms_page_table_pte_entry(l2, virtual_address, 2);
    vms_pte_set_ppn(l2_entry, vms_page_to_ppn(l1));
    vms_pte_valid_set(l2_entry);

    uint64_t* l1_entry = vms_page_table_pte_entry(l1, virtual_address, 1);
    vms_pte_set_ppn(l1_entry, vms_page_to_ppn(l0));
    vms_pte_valid_set(l1_entry);

    uint64_t* l0_entry = vms_page_table_pte_entry(l0, virtual_address, 0);
    vms_pte_set_ppn(l0_entry, vms_page_to_ppn(p0));
    vms_pte_valid_set(l0_entry);
    vms_pte_read_set(l0_entry);
    vms_pte_write_set(l0_entry);

    vms_set_root_page_table(l2);

    vms_write(virtual_address, 1);

    void* forked_l2 = vms_fork_copy();
    vms_set_root_page_table(forked_l2);

    //printf("Before\n");
    assert(l2 != forked_l2);
    // printf("l2 did fork well\n");
    // printf("Used Pages: %d\n", vms_get_used_pages());
    assert(vms_get_used_pages() == 8);
    // printf("Get used pages\n");
    // printf("Read value: %lX\n",(uint64_t) virtual_address);
    // printf("Read value: %lX\n",(uint64_t) vms_read(virtual_address));
    assert(vms_read(virtual_address) == 1);
    //printf("VMS read\n");
    vms_write(virtual_address, 2);
    assert(vms_read(virtual_address) == 2);
    //printf("VMS Write\n");
    vms_set_root_page_table(l2);
    assert(vms_read(virtual_address) == 1);
    //printf("VMS read\n");
    
}
