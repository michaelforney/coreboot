#include <console/console.h>
#include <arch/smp/mpspec.h>
#include <arch/io.h>
#include <device/pci.h>
#include <string.h>
#include <stdint.h>
#if CONFIG_LOGICAL_CPUS==1
#include <cpu/amd/multicore.h>
#endif
#include <cpu/amd/amdk8_sysconf.h>

extern  unsigned char bus_bcm5780[7];
extern  unsigned char bus_bcm5785_0;
extern  unsigned char bus_bcm5785_1;
extern  unsigned char bus_bcm5785_1_1;
extern  unsigned apicid_bcm5785[3];

extern  unsigned sbdn2;

static void *smp_write_config_table(void *v)
{
        struct mp_config_table *mc;
	int i, bus_isa;

        mc = (void *)(((char *)v) + SMP_FLOATING_TABLE_LEN);

	mptable_init(mc, LAPIC_ADDR);

        smp_write_processors(mc);

	get_bus_conf();

	mptable_write_buses(mc, NULL, &bus_isa);

/*I/O APICs:	APIC ID	Version	State		Address*/
        {
                device_t dev = 0;
		struct resource *res;
		for(i=0; i<3; i++) {
	                dev = dev_find_device(0x1166, 0x0235, dev);
        	        if (dev) {
				res = find_resource(dev, PCI_BASE_ADDRESS_0);
				if (res) {
					smp_write_ioapic(mc, apicid_bcm5785[i], 0x11, res->base);
				}
        	        }
		}

	}

	mptable_add_isa_interrupts(mc, bus_isa, apicid_bcm5785[0], 0);

//IDE
	outb(0x02, 0xc00); outb(0x0e, 0xc01);

	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_EDGE|MP_IRQ_POLARITY_HIGH,  bus_bcm5785_0, ((1+sysconf.sbdn)<<2)|1, apicid_bcm5785[0], 0xe); // IDE

//SATA
	outb(0x07, 0xc00); outb(0x0f, 0xc01);
        smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5785_1, (0x0e<<2)|0, apicid_bcm5785[0], 0xf);

//USB
	outb(0x01, 0xc00); outb(0x0a, 0xc01);
        for(i=0;i<3;i++) {
                smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5785_0, ((2+sysconf.sbdn)<<2)|i, apicid_bcm5785[0], 0xa); //
        }



        /* enable int */
        /* why here? must get the BAR and PCI command bit 1 set before enable it ....*/
        {
                device_t dev;
                dev = dev_find_device(0x1166, 0x0205, 0);
                if(dev) {
                        uint32_t dword;
                        dword = pci_read_config32(dev, 0x6c);
                        dword |= (1<<4); // enable interrupts
                        pci_write_config32(dev, 0x6c, dword);

                }

        }

//First pci-x slot (on bcm5785) under bus_bcm5785_1:d.0
        for(i=0;i<4;i++) {
                smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5785_1_1, (4<<2)|i, apicid_bcm5785[1], 2 + (0+i)%4); //
        }


//pci slot (on bcm5785)
        for(i=0;i<4;i++) {
                smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5785_0, (4<<2)|i, apicid_bcm5785[1], i%2); //
        }


//onboard ati
        smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5785_0, (5<<2)|0, apicid_bcm5785[1], 0x1);

//PCI-X on bcm5780
        for(i=0;i<4;i++) {
                smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5780[1], (4<<2)|i, apicid_bcm5785[1], 6 + (0+i)%4); //
        }

        for(i=0;i<4;i++) {
                smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5780[1], (5<<2)|i, apicid_bcm5785[1], 6 + (1+i)%4); //
        }

//onboard Broadcom
        for(i=0;i<2;i++) {
                smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5780[2], (4<<2)|i, apicid_bcm5785[1], 0xa + (0+i)%4); //
        }


// First PCI-E x8
        for(i=0;i<4;i++) {
                smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5780[5], (0<<2)|i, apicid_bcm5785[1], 0xe); //
        }


// Second PCI-E x8
        for(i=0;i<4;i++) {
                smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5780[3], (0<<2)|i, apicid_bcm5785[1], 0xc); //
        }


// Third PCI-E x1
        for(i=0;i<4;i++) {
                smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_bcm5780[4], (0<<2)|i, apicid_bcm5785[1], 0xd); //
        }

/*Local Ints:	Type	Polarity    Trigger	Bus ID	 IRQ	APIC ID	PIN#*/
	smp_write_lintsrc(mc, mp_ExtINT, MP_IRQ_TRIGGER_EDGE|MP_IRQ_POLARITY_HIGH, bus_isa, 0x0, MP_APIC_ALL, 0x0);
	smp_write_lintsrc(mc, mp_NMI, MP_IRQ_TRIGGER_EDGE|MP_IRQ_POLARITY_HIGH, bus_isa, 0x0, MP_APIC_ALL, 0x1);
	/* There is no extension information... */

	/* Compute the checksums */
	mc->mpe_checksum = smp_compute_checksum(smp_next_mpc_entry(mc), mc->mpe_length);
	mc->mpc_checksum = smp_compute_checksum(mc, mc->mpc_length);
	printk(BIOS_DEBUG, "Wrote the mp table end at: %p - %p\n",
		mc, smp_next_mpe_entry(mc));
	return smp_next_mpe_entry(mc);
}

unsigned long write_smp_table(unsigned long addr)
{
	void *v;
	v = smp_write_floating_table(addr);
	return (unsigned long)smp_write_config_table(v);
}
