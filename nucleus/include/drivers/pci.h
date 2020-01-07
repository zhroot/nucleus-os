#ifndef _PCI_H_
#define _PCI_H_

struct pci_cfg
{
   unsigned int	 vendorID;
   unsigned int	 deviceID;
   unsigned int	 command_reg;
   unsigned int	 status_reg;
   unsigned char revisionID;
   unsigned char progIF;
   unsigned char subclass;
   unsigned char classcode;
   unsigned char cacheline_size;
   unsigned char latency;
   unsigned char header_type;
   unsigned char BIST;
	struct
	{
	 unsigned long base_address0;
	 unsigned long base_address1;
	 unsigned long base_address2;
	 unsigned long base_address3;
	 unsigned long base_address4;
	 unsigned long base_address5;
	 unsigned long CardBus_CIS;
	 unsigned int  subsystem_vendorID;
	 unsigned int  subsystem_deviceID;
	 unsigned long expansion_ROM;
	 unsigned char  cap_ptr;
	 unsigned char  reserved1[3];
	 unsigned long reserved2[1];
	 unsigned char  interrupt_line;
	 unsigned char  interrupt_pin;
	 unsigned char  min_grant;
	 unsigned char  max_latency;
	 unsigned long device_specific[48];
	 } nonbridge;
	struct
	{
	 unsigned long base_address0;
	 unsigned long base_address1;
	 unsigned char  primary_bus;
	 unsigned char  secondary_bus;
	 unsigned char  subordinate_bus;
	 unsigned char  secondary_latency;
	 unsigned char  IO_base_low;
	 unsigned char  IO_limit_low;
	 unsigned int  secondary_status;
	 unsigned int  memory_base_low;
	 unsigned int  memory_limit_low;
	 unsigned int  prefetch_base_low;
	 unsigned int  prefetch_limit_low;
	 unsigned long prefetch_base_high;
	 unsigned long prefetch_limit_high;
	 unsigned int  IO_base_high;
	 unsigned int  IO_limit_high;
	 unsigned long reserved2[1];
	 unsigned long expansion_ROM;
	 unsigned char  interrupt_line;
	 unsigned char  interrupt_pin;
	 unsigned int  bridge_control;
	 unsigned long device_specific[48];
	 } bridge;
	struct
	{
	 unsigned long ExCa_base;
	 unsigned char  cap_ptr;
	 unsigned char  reserved05;
	 unsigned int  secondary_status;
	 unsigned char  PCI_bus;
	 unsigned char  CardBus_bus;
	 unsigned char  subordinate_bus;
	 unsigned char  latency_timer;
	 unsigned long memory_base0;
	 unsigned long memory_limit0;
	 unsigned long memory_base1;
	 unsigned long memory_limit1;
	 unsigned int  IObase_0low;
	 unsigned int  IObase_0high;
	 unsigned int  IOlimit_0low;
	 unsigned int  IOlimit_0high;
	 unsigned int  IObase_1low;
	 unsigned int  IObase_1high;
	 unsigned int  IOlimit_1low;
	 unsigned int  IOlimit_1high;
	 unsigned char  interrupt_line;
	 unsigned char  interrupt_pin;
	 unsigned int  bridge_control;
	 unsigned int  subsystem_vendorID;
	 unsigned int  subsystem_deviceID;
	 unsigned long legacy_baseaddr;
	 unsigned long cardbus_reserved[14];
	 unsigned long vendor_specific[32];
	 } cardbus;
};

struct subclass_info
{
	int subclass_code;
	const char *subclass_name;
};

static const char * const class_names[] =
   {
    "reserved",		// 00
    "disk",		// 01
    "network",		// 02
    "display",		// 03
    "multimedia",	// 04
    "memory",		// 05
    "bridge",		// 06
    "communication",	// 07
    "system peripheral",// 08
    "input",		// 09
    "docking station",	// 0A
    "CPU",		// 0B
    "serial bus",	// 0C
   };

static const struct subclass_info subclass_info_01[] =
   {
     { 0x00, "SCSI" },
     { 0x01, "IDE" },
     { 0x02, "floppy" },
     { 0x03, "IPI"},
     { 0x04, "RAID" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_02[] =
   {
     { 0x00, "Ethernet" },
     { 0x01, "TokenRing" },
     { 0x02, "FDDI" },
     { 0x03, "ATM" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_03[] =
   {
     { 0x00, "VGA" },
     { 0x01, "SuperVGA" },
     { 0x02, "XGA" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_04[] =
   {
     { 0x00, "video" },
     { 0x01, "audio" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_05[] =
   {
     { 0x00, "RAM" },
     { 0x01, "Flash memory" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_06[] =
   {
     { 0x00, "CPU/PCI" },
     { 0x01, "PCI/ISA" },
     { 0x02, "PCI/EISA" },
     { 0x03, "PCI/MCA" },
     { 0x04, "PCI/PCI" },
     { 0x05, "PCI/PCMCIA" },
     { 0x06, "PCI/NuBus" },
     { 0x07, "PCI/CardBus" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_07[] =
   {
     { 0x00, "serial" },
     { 0x01, "parallel" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_08[] =
   {
     { 0x00, "PIC" },
     { 0x01, "DMAC" },
     { 0x02, "timer" },
     { 0x03, "RTC" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_09[] =
   {
     { 0x00, "keyboard" },
     { 0x01, "digitizer" },
     { 0x02, "mouse" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_0A[] =
   {
     { 0x00, "generic" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_0B[] =
   {
     { 0x00, "386" },
     { 0x01, "486" },
     { 0x02, "Pentium" },
     { 0x03, "P6" },
     { 0x10, "Alpha" },
     { 0x40, "coproc" },
     { 0x80, "other" },
     { -1, 0 },
   };

static const struct subclass_info subclass_info_0C[] =
   {
     { 0x00, "Firewire" },
     { 0x01, "ACCESS.bus" },
     { 0x02, "SSA" },
     { 0x03, "USB" },
     { 0x04, "Fiber Channel" },
     { 0x80, "other" },
     { -1, 0 },
   };

extern unsigned char cfg_mech;
extern unsigned char cfg_max;

char GetSpecialVendor(unsigned int vendor);
char GetSpecialDevice(unsigned int vendor, unsigned int device);
char CheckPCI(void);
char check_pci_bios(void);
void GetPCIList(void);

extern struct pci_cfg pci_list[];

#endif
