#include <console/console.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <string.h>
#include <stdint.h>
#if CONFIG_LOGICAL_CPUS==1
#include <cpu/amd/multicore.h>
#endif

#include <cpu/amd/amdk8_sysconf.h>
#include <stdlib.h>

// Global variables for MB layouts and these will be shared by irqtable mptable and acpi_tables
//busnum is default
        unsigned char bus_ck804_0; //1
        unsigned char bus_ck804_1; //2
        unsigned char bus_ck804_2; //3
        unsigned char bus_ck804_3; //4
        unsigned char bus_ck804_4; //5
        unsigned char bus_ck804_5; //6
        unsigned char bus_ck804b_0;//a
        unsigned char bus_ck804b_1;//b
        unsigned char bus_ck804b_2;//c
        unsigned char bus_ck804b_3;//d
        unsigned char bus_ck804b_4;//e
        unsigned char bus_ck804b_5;//f
        unsigned apicid_ck804;
        unsigned apicid_ck804b;

unsigned pci1234x[] =
{        //Here you only need to set value in pci1234 for HT-IO that could be installed or not
	 //You may need to preset pci1234 for HTIO board, please refer to src/northbridge/amd/amdk8/get_sblk_pci1234.c for detail
        0x0000ff0, // SB chain
        0x0000000, // HTX
        0x0000110, // io04
        0x0000010, // HTX
//        0x0000ff0,
//        0x0000ff0,
//        0x0000ff0,
//        0x0000ff0
};
unsigned hcdnx[] =
{ //HT Chain device num, actually it is unit id base of every ht device in chain, assume every chain only have 4 ht device at most
	0x20202020,
	0x20202020,
        0x20202020,
        0x20202020,
//        0x20202020,
//        0x20202020,
//        0x20202020,
//        0x20202020,
};

unsigned sbdnb;

static unsigned get_bus_conf_done = 0;

void get_bus_conf(void)
{

	unsigned apicid_base;
	unsigned sbdn;

        device_t dev;
        int i;

        if(get_bus_conf_done==1) return; //do it only once

        get_bus_conf_done = 1;

        sysconf.hc_possible_num = ARRAY_SIZE(pci1234x);
        for(i=0;i<sysconf.hc_possible_num; i++) {
                sysconf.pci1234[i] = pci1234x[i];
                sysconf.hcdn[i] = hcdnx[i];
        }

        get_sblk_pci1234();

	sysconf.sbdn = (sysconf.hcdn[0] & 0xff); // first byte of first chain
	sbdn = sysconf.sbdn;

	bus_ck804_0 = (sysconf.pci1234[0] >> 16) & 0xff;

                /* CK804 */
                dev = dev_find_slot(bus_ck804_0, PCI_DEVFN(sbdn + 0x09,0));
                if (dev) {
                        bus_ck804_1 = pci_read_config8(dev, PCI_SECONDARY_BUS);
                        bus_ck804_4 = pci_read_config8(dev, PCI_SUBORDINATE_BUS);
                        bus_ck804_4++;
                }
                else {
                        printk(BIOS_DEBUG, "ERROR - could not find PCI 1:%02x.0, using defaults\n", sbdn + 0x09);

                        bus_ck804_1 = 2;
                        bus_ck804_4 = 3;
                }

                dev = dev_find_slot(bus_ck804_0, PCI_DEVFN(sbdn + 0x0d, 0));
                if (dev) {
                        bus_ck804_4 = pci_read_config8(dev, PCI_SECONDARY_BUS);
                        bus_ck804_5 = pci_read_config8(dev, PCI_SUBORDINATE_BUS);
                        bus_ck804_5++;
                }
                else {
                        printk(BIOS_DEBUG, "ERROR - could not find PCI 1:%02x.0, using defaults\n", sbdn + 0x0d);

                        bus_ck804_5 = bus_ck804_4 + 1;
                }

                dev = dev_find_slot(bus_ck804_0, PCI_DEVFN(sbdn+ 0x0e,0));
                if (dev) {
                        bus_ck804_5 = pci_read_config8(dev, PCI_SECONDARY_BUS);
                }
                else {
                        printk(BIOS_DEBUG, "ERROR - could not find PCI 1:%02x.0, using defaults\n", sbdn+ 0x0e);
                }

                /* CK804b */

	if(sysconf.pci1234[2] & 0x0f) { //if the second cpu is installed
		sbdnb = (sysconf.hcdn[2] & 0xff);
		bus_ck804b_0 = (sysconf.pci1234[2]>>16) & 0xff;

                dev = dev_find_slot(bus_ck804b_0, PCI_DEVFN(sbdnb + 0x0d, 0));
                if (dev)
                {
                    bus_ck804b_4 = pci_read_config8(dev, PCI_SECONDARY_BUS);
                    bus_ck804b_5 = pci_read_config8(dev, PCI_SUBORDINATE_BUS);
                    bus_ck804b_5++;
                }
                else
                {
                    printk(BIOS_DEBUG, "ERROR - could not find PCI %02x:%02x.0, using defaults\n", bus_ck804b_0, sbdnb + 0x0d);

                    bus_ck804b_5 = bus_ck804b_4 + 1;
                }

                dev = dev_find_slot(bus_ck804b_0, PCI_DEVFN(sbdnb + 0x0e,0));
                if (dev) {
                        bus_ck804b_5 = pci_read_config8(dev, PCI_SECONDARY_BUS);
                }
                else {
                        printk(BIOS_DEBUG, "ERROR - could not find PCI %02x:%02x.0, using defaults\n", bus_ck804b_0,sbdnb+0x0e);
                        bus_ck804b_5 = bus_ck804b_4+1;
                }
	}


/*I/O APICs:	APIC ID	Version	State		Address*/
#if CONFIG_LOGICAL_CPUS==1
	apicid_base = get_apicid_base(2);
#else
	apicid_base = CONFIG_MAX_PHYSICAL_CPUS;
#endif
	apicid_ck804 = apicid_base+0;
        apicid_ck804b = apicid_base+1;
}
