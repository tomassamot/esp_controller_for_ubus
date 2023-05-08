#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <libserialport.h>

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
static void set_return_values(struct ubus_context *ctx, struct ubus_request_data *req,struct blob_buf *b, char return_text[]);

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
    [PORT_DEVICE] = {.name = "port", .type=BLOBMSG_TYPE_STRING},
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
	
	int r;
	ssize_t cnt;

	blob_buf_init(&b, 0);

	a1 = blobmsg_open_array(&b, "devices");
	
	struct sp_port **port_list;

	enum sp_return result = sp_list_ports(&port_list);
	if (result != SP_OK) {
		syslog(LOG_WARNING, "sp_list_ports() failed!");
		return -1;
	}

	for (int i = 0; port_list[i] != NULL; i++) {
		struct sp_port *port = port_list[i];

		int vendor_id;
		int product_id;
		result = sp_get_port_usb_vid_pid(port, &vendor_id, &product_id);
		if(result != SP_OK){
			syslog(LOG_WARNING, "Failed to get vendor id and product id");
			continue;
		}

		char *port_name = sp_get_port_name(port);

		if(vendor_id == 4292 && product_id == 60000){
			void *t = blobmsg_open_table(&b, "device");
			blobmsg_add_string(&b, "port", port_name);
			blobmsg_add_u32(&b, "vendor_id", vendor_id);
			blobmsg_add_u32(&b, "product_id", product_id);
			blobmsg_close_table(&b, t);
		}
	}

	sp_free_port_list(port_list);

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
	char return_text[1024];
	
	blobmsg_parse(myesp_onoff_policy, _ONOFF_MAX, tb, blob_data(msg), blob_len(msg));
	

	//if (!tb[PORT_ONOFF] || !tb[PIN])
	//	return UBUS_STATUS_INVALID_ARGUMENT;
	if(!tb[PORT_ONOFF]){
		sprintf(return_text, "Incorrect port given");
		set_return_values(ctx, req, &b, return_text);
		return;
	}
	if(!tb[PIN]){
		sprintf(return_text, "Incorrect pin given");
		set_return_values(ctx, req, &b, return_text);
		return;
	}

	
	char command[80];
	sprintf(command, "echo '{\"action\": \"on\", \"pin\": %d}' > %s", blobmsg_get_u32(tb[PIN]), blobmsg_get_string(tb[PORT_ONOFF]));

	system(command);
		
	sprintf(return_text, "Successfully sent 'on' command");
	
	set_return_values(ctx, req, &b, return_text);
	return;
}


static int myesp_off(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[_ONOFF_MAX];
	struct blob_buf b = {};
	char return_text[1024];
	
	blobmsg_parse(myesp_onoff_policy, _ONOFF_MAX, tb, blob_data(msg), blob_len(msg));
	

	//if (!tb[PORT_ONOFF] || !tb[PIN])
	//	return UBUS_STATUS_INVALID_ARGUMENT;
	if(!tb[PORT_ONOFF]){
		sprintf(return_text, "Incorrect port given");
		set_return_values(ctx, req, &b, return_text);
		return;
	}
	if(!tb[PIN]){
		sprintf(return_text, "Incorrect pin given");
		set_return_values(ctx, req, &b, return_text);
		return;
	}

	
	char command[80];
	sprintf(command, "echo '{\"action\": \"off\", \"pin\": %d}' > %s", blobmsg_get_u32(tb[PIN]), blobmsg_get_string(tb[PORT_ONOFF]));

	system(command);
		
	sprintf(return_text, "Successfully sent 'off' command");
	
	set_return_values(ctx, req, &b, return_text);
	return;

	// blob_buf_init(&b, 0);
	// blobmsg_add_string(&b, "response", return_text);
	// ubus_send_reply(ctx, req, b.head);
	// blob_buf_free(&b);
}

static void set_return_values(struct ubus_context *ctx, struct ubus_request_data *req,struct blob_buf *b, char return_text[])
{
	blob_buf_init(b, 0);
	blobmsg_add_string(b, "response", return_text);
	ubus_send_reply(ctx, req, b->head);
	blob_buf_free(b);
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


