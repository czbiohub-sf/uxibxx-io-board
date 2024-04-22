#define DATA_IO_EP_SIZE 16

#define NOTIF_EP_ADDR (ENDPOINT_DIR_IN | 1)
#define NOTIF_EP_SIZE 8

#define DIN_EP_ADDR (ENDPOINT_DIR_IN | 2)
#define DIN_EP_SIZE DATA_IO_EP_SIZE
#define DIN_POLL_INT_MS 5 // TODO is this meaningful?

#define DOUT_EP_ADDR (ENDPOINT_DIR_OUT | 3)
#define DOUT_EP_SIZE DATA_IO_EP_SIZE
#define DOUT_POLL_INT_MS 5 // TODO is this meaningful?
