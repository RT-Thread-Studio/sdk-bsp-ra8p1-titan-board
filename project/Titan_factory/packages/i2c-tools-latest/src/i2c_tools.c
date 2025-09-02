#include "i2c_utils.h"

static void i2c(int argc,char *argv[])
{
    if(argc > 2)
    {
        #ifdef I2C_TOOLS_USE_SW_I2C
            if( i2c_init( strtonum(argv[ARG_SDA_POS]), strtonum(argv[ARG_SCL_POS])) )
            {
                rt_kprintf("[i2c] failed to find bus with sda=%s scl=%s\n", argv[ARG_SDA_POS], argv[ARG_SCL_POS]);
                return;
            }
        #else
            if(i2c_init(argv[ARG_BUS_NAME_POS]))
            {
                rt_kprintf("[i2c] failed to find bus %s\n", argv[ARG_BUS_NAME_POS]);
                return;
            }
        #endif

        if(!strcmp(argv[ARG_CMD_POS], "scan"))
        {
            rt_uint32_t start_addr = 0x00;
            rt_uint32_t stop_addr  = 0x80;
        #ifdef I2C_TOOLS_USE_SW_I2C
            if(argc >= 5)
            {
                if (sscanf(argv[4], "%x", &start_addr) != 1) {
                    rt_kprintf("[i2c] Invalid start address: %s\n", argv[4]);
                    return;
                }
                if(argc > 5)
                {
                    if (sscanf(argv[5], "%x", &stop_addr) != 1) {
                        rt_kprintf("[i2c] Invalid stop address: %s\n", argv[5]);
                        return;
                    }
                }
            }
        #else
            if(argc >= 4)
            {
                if (sscanf(argv[3], "%x", &start_addr) != 1) {
                    rt_kprintf("[i2c] Invalid start address: %s\n", argv[3]);
                    return;
                }
                if(argc > 4)
                {
                    if (sscanf(argv[4], "%x", &stop_addr) != 1) {
                        rt_kprintf("[i2c] Invalid stop address: %s\n", argv[4]);
                        return;
                    }
                }
            }
        #endif
            if (start_addr >= 0x80 || stop_addr > 0x80) {
                rt_kprintf("[i2c] The addresses only range from 0x00 to 0x7F\n");
                return;
            }
            if (start_addr >= stop_addr) {
                rt_kprintf("[i2c] The stop address should be higher than the start address\n");
                return;
            }
            i2c_scan(start_addr, stop_addr);
            return;
        }

        if(argc < 5)
        {
            i2c_help();
            return;
        }

        if(!strcmp(argv[1], "read"))
        {
            rt_uint8_t buffer[I2C_TOOLS_BUFFER_SIZE];
            rt_uint8_t len = 1;
            if( argc == ARG_READ_MAX)
            {
                len = atoi(argv[ARG_READ_MAX - 1]);
            }
            if(i2c_read(strtonum(argv[ARG_ADDR_POS]), strtonum(argv[ARG_DATA_POS]), buffer, len) == len)
            {
                rt_kprintf("[ ");
                for(rt_uint8_t i = 0; i < len; i++)
                {
                    rt_kprintf("0x%02X", buffer[i]);
                    if(i != (len-1))
                    {
                        rt_kprintf(", ");
                    }
                }
                rt_kprintf(" ]\n");
            }
            else
            {
                #ifdef I2C_TOOLS_USE_SW_I2C
                    rt_kprintf("[i2c] read from bus failed with sda=%s scl=%s\n", argv[ARG_SDA_POS], argv[ARG_SCL_POS]);
                #else
                    rt_kprintf("[i2c] read from %s failed\n", argv[ARG_BUS_NAME_POS]);
                #endif
            }
        }

        else if(!strcmp(argv[1], "write"))
        {
            unsigned char buffer[I2C_TOOLS_BUFFER_SIZE];
            for(int i=0; i<(argc-ARG_DATA_POS); i++)
            {
                buffer[i] = strtonum(argv[i+ARG_DATA_POS]);
            }
            if(!i2c_write(strtonum(argv[ARG_DATA_POS-1]), buffer, argc-ARG_DATA_POS))
            {
                #ifdef I2C_TOOLS_USE_SW_I2C
                    rt_kprintf("[i2c] write to bus failed with sda=%s scl=%s\n", argv[ARG_SDA_POS], argv[ARG_SCL_POS]);
                #else
                    rt_kprintf("[i2c] write to %s failed\n", argv[ARG_BUS_NAME_POS]);
                #endif
            }
        }
    }
    else
    {
        i2c_help();
    }
}
MSH_CMD_EXPORT(i2c, i2c tools);
