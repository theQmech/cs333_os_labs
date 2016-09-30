6750 // See MultiProcessor Specification Version 1.[14]
6751 
6752 struct mp {             // floating pointer
6753   uchar signature[4];           // "_MP_"
6754   void *physaddr;               // phys addr of MP config table
6755   uchar length;                 // 1
6756   uchar specrev;                // [14]
6757   uchar checksum;               // all bytes must add up to 0
6758   uchar type;                   // MP system config type
6759   uchar imcrp;
6760   uchar reserved[3];
6761 };
6762 
6763 struct mpconf {         // configuration table header
6764   uchar signature[4];           // "PCMP"
6765   ushort length;                // total table length
6766   uchar version;                // [14]
6767   uchar checksum;               // all bytes must add up to 0
6768   uchar product[20];            // product id
6769   uint *oemtable;               // OEM table pointer
6770   ushort oemlength;             // OEM table length
6771   ushort entry;                 // entry count
6772   uint *lapicaddr;              // address of local APIC
6773   ushort xlength;               // extended table length
6774   uchar xchecksum;              // extended table checksum
6775   uchar reserved;
6776 };
6777 
6778 struct mpproc {         // processor table entry
6779   uchar type;                   // entry type (0)
6780   uchar apicid;                 // local APIC id
6781   uchar version;                // local APIC verison
6782   uchar flags;                  // CPU flags
6783     #define MPBOOT 0x02           // This proc is the bootstrap processor.
6784   uchar signature[4];           // CPU signature
6785   uint feature;                 // feature flags from CPUID instruction
6786   uchar reserved[8];
6787 };
6788 
6789 struct mpioapic {       // I/O APIC table entry
6790   uchar type;                   // entry type (2)
6791   uchar apicno;                 // I/O APIC id
6792   uchar version;                // I/O APIC version
6793   uchar flags;                  // I/O APIC flags
6794   uint *addr;                  // I/O APIC address
6795 };
6796 
6797 
6798 
6799 
6800 // Table entry types
6801 #define MPPROC    0x00  // One per processor
6802 #define MPBUS     0x01  // One per bus
6803 #define MPIOAPIC  0x02  // One per I/O APIC
6804 #define MPIOINTR  0x03  // One per bus interrupt source
6805 #define MPLINTR   0x04  // One per system interrupt source
6806 
6807 
6808 
6809 
6810 
6811 
6812 
6813 
6814 
6815 
6816 
6817 
6818 
6819 
6820 
6821 
6822 
6823 
6824 
6825 
6826 
6827 
6828 
6829 
6830 
6831 
6832 
6833 
6834 
6835 
6836 
6837 
6838 
6839 
6840 
6841 
6842 
6843 
6844 
6845 
6846 
6847 
6848 
6849 
6850 // Blank page.
6851 
6852 
6853 
6854 
6855 
6856 
6857 
6858 
6859 
6860 
6861 
6862 
6863 
6864 
6865 
6866 
6867 
6868 
6869 
6870 
6871 
6872 
6873 
6874 
6875 
6876 
6877 
6878 
6879 
6880 
6881 
6882 
6883 
6884 
6885 
6886 
6887 
6888 
6889 
6890 
6891 
6892 
6893 
6894 
6895 
6896 
6897 
6898 
6899 
