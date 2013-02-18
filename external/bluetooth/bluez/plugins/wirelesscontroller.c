/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright 2011,2012 Sony Corporation
 *  Copyright (C) 2012 Sony Mobile Communications AB.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <glib.h>
#include <gdbus.h>
#include "error.h"
#include "plugin.h"
#include "adapter.h"
#include "log.h"
#include <utils/Log.h>

#include "device.h"
#include "manager.h"
#include "storage.h"
#include <bluetooth/sdp_lib.h>

#include <sys/ioctl.h>

#define WIRELESS_CONTROLLER_INTERFACE "com.sony.nfx.WirelessController"
#define VENDOR 0x054c
#define PRODUCT 0x0268
#define HID_UUID "00001124-0000-1000-8000-00805f9b34fb"
#define check_address(address) bachk(address)

static DBusConnection *connection;
static struct btd_adapter *g_adapter;
static const char *any_path;

gboolean is_wirelesscontroller(const bdaddr_t *src, const bdaddr_t *dst)
{
	char srcaddr[18], dstaddr[18];
	uint16_t vendor=0, product=0;

	ba2str(src, srcaddr);
	ba2str(dst, dstaddr);
	read_device_id(srcaddr, dstaddr, NULL, &vendor, &product, NULL);

	if (vendor != VENDOR || product != PRODUCT) {
		return FALSE;
	}

	return TRUE;
}

static gboolean set_wirelesscontroller_paring_info(struct btd_adapter *adapter,
                                 const char *name,
                                 const char *address,
                                 guint32 vendor_id,
                                 guint32 product_id,
                                 const char *pnp_record)
{
	DBusConnection *conn;
	sdp_record_t *rec;
	struct btd_device *device;
	bdaddr_t src, dst;
	char srcaddr[18];
	unsigned char key[16];

	/* Write device name */
	str2ba(address, &dst);
	adapter_get_address(adapter, &src);
	ba2str(&src, srcaddr);
	write_device_name(&src, &dst, (char *) name);

	/* Store the device's SDP record */
	rec = record_from_string(pnp_record);
	store_record(srcaddr, address, rec);
	sdp_record_free(rec);

	/* Set the device id */
	store_device_id(srcaddr, address, 0xffff, vendor_id, product_id, 0);

	/* Write trust */
	write_trust(srcaddr, address, "[all]", TRUE);

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (conn == NULL) {
		ALOGE("%s: Failed to get on the bus", __func__);
		return FALSE;
	}

	/* Create device */
	device = adapter_get_device(conn, adapter, address);
	if (device == NULL) {
		ALOGE("%s: Failed to get the device", __func__);
		dbus_connection_unref(conn);
		return FALSE;
	}

	device_set_paired(device, TRUE);
	device_set_temporary(device, FALSE);
	device_set_name(device, name);
	btd_device_add_uuid(device, HID_UUID);

	/* Write dummy linkkey */
	memset(key, 0, sizeof(key));
	write_link_key(&src, &dst, key, 0, 4);

	dbus_connection_unref(conn);

	return TRUE;
}

static DBusMessage *create_device_WirelessController(DBusConnection *conn,
					DBusMessage *msg, void *data)
{
	//struct btd_adapter *adapter;
	struct btd_device *device;
	int adapter_id;

	const char* devicename;
	const char* deviceaddress;
	const char* pnp;

	/* get arguments */
	dbus_message_get_args(msg, NULL,
				DBUS_TYPE_STRING, &devicename,
				DBUS_TYPE_STRING, &deviceaddress,
				DBUS_TYPE_STRING, &pnp,
				DBUS_TYPE_INVALID);

	/* check address */
	if (check_address(deviceaddress) < 0){
		ALOGE("%s: check_address error", __func__);
		return dbus_message_new_method_return(msg);
	}

	/* Look for the default adapter */
	//adapter_id = manager_get_default_adapter();
	//adapter = manager_find_adapter_by_id(adapter_id);

	/* Is this device already paired? */
	if (adapter_find_device(g_adapter, deviceaddress)) {
		ALOGD("%s: This device is already paired.", __func__);
		return dbus_message_new_method_return(msg);
	}

	/* Set Paring info to Bluez. */
	set_wirelesscontroller_paring_info(g_adapter,
							devicename,
							deviceaddress,
							VENDOR, PRODUCT,
							pnp);

	return dbus_message_new_method_return(msg);
}

static int switch_role(int deviceDescriptor,
						const bdaddr_t* deviceBdAddress,
						int role)
{
	return hci_switch_role(deviceDescriptor, deviceBdAddress, role, 1000);
}

static int change_mode_to_sniff(int deviceDescriptor,
								const bdaddr_t* deviceBdAddress,
								int maxInterval,
								int minInterval,
								int attempt,
								int timeout)
{
	int ret = -1;

	/* get hci connection handle */
	struct hci_conn_info_req *cr;
	cr = g_malloc0(sizeof(*cr) + sizeof(struct hci_conn_info));
	cr->type = ACL_LINK;

	bacpy(&cr->bdaddr, deviceBdAddress);
	ret = ioctl(deviceDescriptor, HCIGETCONNINFO, cr);
	if(ret < 0) {
		ALOGE("%s: failed to get hci connection info [%d]", __func__, ret);
		g_free(cr);
		return ret;
	}
	uint16_t connectionHandle = cr->conn_info->handle;

	g_free(cr);

	/* change mode to sniff */
	struct hci_request rq;
	evt_mode_change rp;
	sniff_mode_cp cp;

	cp.handle       = connectionHandle;
	cp.max_interval = maxInterval;
	cp.min_interval = minInterval;
	cp.attempt      = attempt;
	cp.timeout      = timeout;

	memset(&rq, 0, sizeof(rq));
	rq.ogf    = OGF_LINK_POLICY;
	rq.ocf    = OCF_SNIFF_MODE;
	rq.event  = EVT_MODE_CHANGE;
	rq.cparam = &cp;
	rq.clen   = SNIFF_MODE_CP_SIZE;
	rq.rparam = &rp;
	rq.rlen   = EVT_MODE_CHANGE_SIZE;

	ret = hci_send_req(deviceDescriptor, &rq, 1000);

	return ret;

}

static DBusMessage *change_mode_WirelessController(DBusConnection *conn,
					DBusMessage *msg, void *data)
{
	struct btd_device *device;

	dbus_uint32_t max_interval;
	dbus_uint32_t min_interval;
	dbus_uint32_t attempt;
	dbus_uint32_t timeout;
	const char* deviceaddress;
	dbus_uint32_t role;

	/* get arguments */
	dbus_message_get_args(msg, NULL,
				DBUS_TYPE_UINT32, &max_interval,
				DBUS_TYPE_UINT32, &min_interval,
				DBUS_TYPE_UINT32, &attempt,
				DBUS_TYPE_UINT32, &timeout,
				DBUS_TYPE_STRING, &deviceaddress,
				DBUS_TYPE_UINT32, &role,
				DBUS_TYPE_INVALID);

	/* check address */
	if (check_address(deviceaddress) < 0){
		ALOGE("%s: check_address error", __func__);
		return dbus_message_new_method_return(msg);
	}

	/* open device */
	uint16_t dev_id = adapter_get_dev_id(g_adapter);
	int dd = hci_open_dev(dev_id);
	if(dd < 0) {
		ALOGD("%s: hci_open_dev error dev_id:%d", __func__, dev_id);
		return dbus_message_new_method_return(msg);
	}

	/* change bluetooth device address format sring to bdaddr_t */
	bdaddr_t deviceBdAddress;
	str2ba(deviceaddress, &deviceBdAddress);

	/* switch role */
	switch_role(dd, &deviceBdAddress, role);

	/* chage mode to sniff */
	change_mode_to_sniff(dd, &deviceBdAddress, max_interval, min_interval, attempt, timeout);

	/* close device */
	hci_close_dev(dd);

	return dbus_message_new_method_return(msg);
}

static GDBusMethodTable wirelesscontroller_methods[] = {
	{ "CreateDeviceWirelessController",	"sss",	"",	create_device_WirelessController } ,
	{ "ChangeModeWirelessController",	"uuuusu",	"",	change_mode_WirelessController } ,
	{ }
};

static int register_interface(const char *path, struct btd_adapter *adapter)
{
	g_adapter = adapter;

	if (g_dbus_register_interface(connection, path, WIRELESS_CONTROLLER_INTERFACE,
				wirelesscontroller_methods, NULL, NULL, g_adapter, NULL) == FALSE) {
		ALOGE("%s: D-Bus failed to register %s interface", __func__, WIRELESS_CONTROLLER_INTERFACE);
		return -EIO;
	}

	return 0;
}

static void unregister_interface(const char *path)
{
	g_adapter = NULL;

	g_dbus_unregister_interface(connection, path, WIRELESS_CONTROLLER_INTERFACE);
}

static int wirelesscontroller_probe(struct btd_adapter *adapter)
{
	register_interface(adapter_get_path(adapter), adapter);
	return 0;
}

static void wirelesscontroller_remove(struct btd_adapter *adapter)
{
	unregister_interface(adapter_get_path(adapter));
}

static struct btd_adapter_driver wirelesscontroller = {
	.name	= "wirelesscontroller",
	.probe	= wirelesscontroller_probe,
	.remove= wirelesscontroller_remove,
};

static int wirelesscontroller_init(void)
{
	int err;

	connection = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (connection == NULL)
		return -EIO;

	any_path = btd_adapter_any_request_path();
	if (any_path != NULL) {
		if (register_interface(any_path, NULL) < 0) {
			btd_adapter_any_release_path();
			any_path = NULL;
		}
	}

	err = btd_register_adapter_driver(&wirelesscontroller);
	if (err < 0) {
		dbus_connection_unref(connection);
		return err;
	}

	return 0;
}

static void wirelesscontroller_exit(void)
{
	btd_unregister_adapter_driver(&wirelesscontroller);

	if (any_path != NULL) {
		unregister_interface(any_path);

		btd_adapter_any_release_path();
		any_path = NULL;
	}

	dbus_connection_unref(connection);
}

BLUETOOTH_PLUGIN_DEFINE(wirelesscontroller, VERSION,
		BLUETOOTH_PLUGIN_PRIORITY_DEFAULT, wirelesscontroller_init, wirelesscontroller_exit)
