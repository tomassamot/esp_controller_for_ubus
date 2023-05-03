#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <stdlib.h>

#include "becomedaemon.h"

	

static int myesp_off(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);
static int myesp_on(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);
static int myesp_devices(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);


enum{
    ALL_DEVICES,
    _ALL_DEVICES_MAX
};
enum{
    PORT_DEVICE,
    VENDOR_ID,
    PRODUCT_ID,
    _DEVICE_MAX,
};

enum{
    PORT_ONOFF,
    PIN,
    _ONOFF_MAX,
};

static const struct blobmsg_policy myesp_all_devices_policy[] = {
    [ALL_DEVICES] = {.name = "devices", .type = BLOBMSG_TYPE_ARRAY}
};
static const struct blobmsg_policy myesp_device_policy[] = {
    [PORT_DEVICE] = {.name = "port", .type=BLOBMSG_TYPE_INT32},
    [VENDOR_ID] = {.name = "vendor_id", .type=BLOBMSG_TYPE_STRING},
    [PRODUCT_ID] = {.name = "product_id", .type=BLOBMSG_TYPE_STRING},
};

static const struct blobmsg_policy myesp_onoff_policy[] = {
    [PORT_ONOFF] = {.name = "port", .type = BLOBMSG_TYPE_STRING},
    [PIN] = {.name = "pin", .type = BLOBMSG_TYPE_INT32},
};

static struct ubus_method myesp_methods[] = {
    UBUS_METHOD_NOARG("devices", myesp_devices),
    UBUS_METHOD("on", myesp_on, myesp_onoff_policy),
    UBUS_METHOD("off", myesp_off, myesp_onoff_policy),
};

static struct ubus_object_type myesp_object_type = 
    UBUS_OBJECT_TYPE("myesp", myesp_methods);

static struct ubus_object myesp_object = {
    .name = "myesp",
    .type = &myesp_object_type,
    .methods = myesp_methods,
    .n_methods = ARRAY_SIZE(myesp_methods),
};

static int myesp_devices(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	char port[5], vendor_id[5], product_id[6];
	strcpy(product_id, "");
	void *a1;
	struct blob_buf b = {};

	blob_buf_init(&b, 0);

	a1 = blobmsg_open_array(&b, "devices");

	FILE *cmd = popen("lsusb", "r");
	char buf[256];
	while (fgets(buf, sizeof(buf), cmd) != 0) {
		void *t1 = blobmsg_open_table(&b, "device");
		char *token = strtok(buf, " ");
		int i = 0;

		strcpy(port, "");
		strcpy(vendor_id, "");
		strcpy(product_id, "");
		while(token != NULL){
			if(i == 1){
				sprintf(port, "USB%ld", strtol(token, NULL, 32)-1);
			}
            else if(i == 5){
                char* token_token = strtok(token, ":");
				sprintf(vendor_id, "%s", token_token);
                token_token = strtok(NULL, ":");
				sprintf(product_id, "%s", token_token);
				product_id[4] = '\0';	
            }
			token = strtok(NULL, " ");
			i++;
		}
		blobmsg_add_string(&b, "port", port);
		blobmsg_add_string(&b, "vendor_id", vendor_id);
		blobmsg_add_string(&b, "product_id", product_id);
		
		blobmsg_close_table(&b, t1);
	}

	blobmsg_close_array(&b, a1);

	ubus_send_reply(ctx, req, b.head);

	blob_buf_free(&b);
}
static int myesp_on(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
    struct blob_attr *tb[_ONOFF_MAX];
	struct blob_buf b = {};
	
	blobmsg_parse(myesp_onoff_policy, _ONOFF_MAX, tb, blob_data(msg), blob_len(msg));
	
	if (!tb[PORT_ONOFF] || !tb[PIN])
		return UBUS_STATUS_INVALID_ARGUMENT;
	
	char command[80];
	sprintf(command, "echo '{\"action\": \"on\", \"pin\": %d}' > /dev/tty%s", blobmsg_get_u32(tb[PIN]), blobmsg_get_string(tb[PORT_ONOFF]));
	
	int ret = system(command);

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "return_code", ret);
	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);
}
static int myesp_off(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[_ONOFF_MAX];
	struct blob_buf b = {};
	
	blobmsg_parse(myesp_onoff_policy, _ONOFF_MAX, tb, blob_data(msg), blob_len(msg));
	
	if (!tb[PORT_ONOFF] || !tb[PIN])
		return UBUS_STATUS_INVALID_ARGUMENT;
	
	char command[80];
	sprintf(command, "echo '{\"action\": \"off\", \"pin\": %d}' > /dev/tty%s", blobmsg_get_u32(tb[PIN]), blobmsg_get_string(tb[PORT_ONOFF]));
	
	int ret = system(command);

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "return_code", ret);
	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);
}

int main(int argc, char **argv)
{
	struct ubus_context *ctx;

	int ret = become_daemon();
	if(ret)
		return EXIT_FAILURE;

	uloop_init();

	ctx = ubus_connect(NULL);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);
	ubus_add_object(ctx, &myesp_object);
	uloop_run();

	ubus_free(ctx);
	uloop_done();

	return 0;
}


