#include <cstdio>
#include <cstring>
#include <libusb-1.0/libusb.h>

#include <unistd.h>

#include <vector>
#include <string>

static void printdev(libusb_device *dev);

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

/*
 * "Hello World"-like example for USB and libusb usage; mostly copied from
 * http://www.dreamincode.net/forums/topic/148707-introduction-to-using-libusb-10/
 */
int
main(void) {
    libusb_device **devs = NULL;
    libusb_device *mydev = NULL;
    libusb_device_handle *dev = NULL;
    libusb_context *ctx = NULL;
    int rc, cnt, cfgnum;
    // unsigned char txBuf[32] = "Hallo, Welt!";
    std::string even("abc"), odd("xyz");
    std::vector<uint8_t> txBuf(4), rxBuf(4);

    int txLen = 0, rxLen = 0;

    rc = libusb_init(&ctx);
    if (rc < 0) {
        printf("libusb init error 0x%.08x\n", rc);
        goto out;
    }

    // libusb_set_debug(ctx, 3);
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);

    cnt = libusb_get_device_list(ctx, &devs);
    if(cnt < 0) {
        printf("Failed to obtain list of devices. Error 0x%.08x\n", cnt);
        goto out;
    }

    printf("Found %i devices\n", cnt);
    for (ssize_t i = 0; i < cnt; i++) {
        libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(devs[i], &desc) < 0)
            continue;

        // printf("idVendor=%#x\tidProduct=%#x\n", desc.idVendor, desc.idProduct);
        if ((desc.idVendor == 0xdead) && (desc.idProduct == 0xbeef)) {
            printdev(devs[i]); //print specs of this device
            mydev = devs[i];
            break;
        }
    }

    if (mydev == NULL) {
        printf("Failed to find USB device.\n");
        goto out;
    }

    rc = libusb_open(mydev, &dev);
    if (rc != 0) {
        printf("Failed to open USB device. rc=%i\n", rc);
    }

    rc = libusb_get_configuration(dev, &cfgnum);
    if (rc != 0) {
        printf("Failed to get configuration. rc=%i\n", rc);
        goto out;
    }
    printf("Configuration is %i\n", cfgnum);

    if (!cfgnum) {
        rc = libusb_set_configuration(dev, 1);
        if (rc != 0) {
            printf("Failed to set configuration. rc=%i\n", rc);
            goto out;
        }
    }

    rc = libusb_claim_interface(dev, 1);
    if (rc != 0) {
        printf("Failed to claim interface. rc=%i\n", rc);
        goto out;
    }

    sleep(1);

    for (unsigned i = 0; i < 10; i++) {
        if ((i % 2) == 0) {
            txBuf.assign(even.begin(), even.end());
        } else {
            txBuf.assign(odd.begin(), odd.end());
        }

        rxBuf.clear();
    
        rc = libusb_bulk_transfer(dev, 1, txBuf.data(), txBuf.size(), &txLen, 2500);
        if (rc != 0) {
            printf("[# %i] Bulk Tx transfer failed. rc=%i\n", i, rc);
            goto out;
        }

        if (txLen != 0) {
            printf("[# %i] Tx of %i bytes complete.\n", i, txLen);
        }

        rc = libusb_bulk_transfer(dev, 0x81, rxBuf.data(), txBuf.size(), &rxLen, 500);
        if (rc != 0) {
            printf("[# %i] Bulk Rx transfer failed. rc=%i\n", i, rc);
            goto out;
        }

        if (txBuf == txBuf) {
            printf("[# %i] Read %i bytes:\n%s\n", i, rxLen, rxBuf.data());
        }
    }

#if 0
    rc = libusb_control_transfer(dev,
      (1 << 7) | (0 << 5) | (0 << 0), /* Direction: Device to Host, Type: Standard, Recipient: Device */
      0x06, /* Get Descriptor */
      0x0100, /* Device Descriptor */
      0x0203,
      rxBuf,
      64,
      2000
      );
    if (rc < 0) {
        printf("Control transfer failed. rc=%i\n", rc);
        goto out;
    } else {
        printf("Control transfer: Received %i Bytes.\n", rc);
    }
#endif

out:
    if (dev != NULL) {
        libusb_release_interface(dev, 1);
        libusb_close(dev);
    }

    if (devs != NULL) {
        libusb_free_device_list(devs, 1);
    }

    if (ctx != NULL) {
        libusb_exit(ctx);
    }

    txLen = rxLen = 0;

    return (rc);
}

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

static void
printdev(libusb_device *dev) {
    libusb_device_descriptor desc;
    libusb_config_descriptor *config = NULL;

    const libusb_interface *inter = NULL;
    const libusb_interface_descriptor *interdesc = NULL;
    const libusb_endpoint_descriptor *epdesc = NULL;

    int rc = libusb_get_device_descriptor(dev, &desc);
    if (rc < 0) {
        printf("%s(dev=%p): Failed to get device descriptor. rc=0x%.08x\n", __func__, dev, rc);
        goto out;
    }

    printf("Device Class: 0x%.08x\n", desc.bDeviceClass);
    printf("VendorID: 0x%.04x\n", desc.idVendor);
    printf("ProductID: 0x%.04x\n", desc.idProduct);

    printf("Number of possible configurations: %i\n", desc.bNumConfigurations);

    for (uint8_t cfgNum = 0; cfgNum < desc.bNumConfigurations; cfgNum++) {
        printf("\n *** Configuration %i ***\n", cfgNum);

        libusb_get_config_descriptor(dev, 0, &config);
        printf("Number of Interfaces: %i\n", config->bNumInterfaces);

        for(uint8_t i = 0; i < config->bNumInterfaces; i++) {
            inter = &config->interface[i];

            printf("Number of alternate settings: %i\n", inter->num_altsetting);

            for(int j = 0; j < inter->num_altsetting; j++) {
                interdesc = &inter->altsetting[j];

                printf("\tInterface Number: %i\n", interdesc->bInterfaceNumber);
                printf("\tNumber of endpoints: %i\n", interdesc->bNumEndpoints);

                for(uint8_t k=0; k< interdesc->bNumEndpoints; k++) {
                    epdesc = &interdesc->endpoint[k];

                    printf("\t\tDescriptor Type: %i\n", epdesc->bDescriptorType);
                    printf("\t\tEP Address: 0x%.02x\n", epdesc->bEndpointAddress);
                    printf("\t\tAttributes: 0x%.02x\n", epdesc->bmAttributes);
                }
            }
        }

        if (config != NULL) {
            libusb_free_config_descriptor(config);
            config = NULL;
        }
        
        printf("\n");
    }
    
out:
    return;
}
