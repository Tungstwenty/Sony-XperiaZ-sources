/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2010-2012, The Linux Foundation. All rights reserved.
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

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/socket.h>

#include <glib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include "textfile.h"
#include "adapter.h"
#include "device.h"
#include "glib-helper.h"
#include "storage.h"
#include "log.h"

#define IOT_SPECIAL_MAPPING_FILE "iop_device_list.conf"

struct match {
	GSList *keys;
	char *pattern;
};

static inline int create_filename(char *buf, size_t size,
				const bdaddr_t *bdaddr, const char *name)
{
	char addr[18];

	ba2str(bdaddr, addr);

	return create_name(buf, size, STORAGEDIR, addr, name);
}

int read_device_alias(const char *src, const char *dst, char *alias, size_t size)
{
	char filename[PATH_MAX + 1], *tmp;
	int err;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "aliases");

	tmp = textfile_get(filename, dst);
	if (!tmp)
		return -ENXIO;

	err = snprintf(alias, size, "%s", tmp);

	free(tmp);

	return err < 0 ? -EIO : 0;
}

int write_device_alias(const char *src, const char *dst, const char *alias)
{
	char filename[PATH_MAX + 1];

	create_name(filename, PATH_MAX, STORAGEDIR, src, "aliases");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	return textfile_put(filename, dst, alias);
}

int write_discoverable_timeout(bdaddr_t *bdaddr, int timeout)
{
	char filename[PATH_MAX + 1], str[32];

	snprintf(str, sizeof(str), "%d", timeout);

	create_filename(filename, PATH_MAX, bdaddr, "config");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	return textfile_put(filename, "discovto", str);
}

int read_discoverable_timeout(const char *src, int *timeout)
{
	char filename[PATH_MAX + 1], *str;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "config");

	str = textfile_get(filename, "discovto");
	if (!str)
		return -ENOENT;

	if (sscanf(str, "%d", timeout) != 1) {
		free(str);
		return -ENOENT;
	}

	free(str);

	return 0;
}

int write_pairable_timeout(bdaddr_t *bdaddr, int timeout)
{
	char filename[PATH_MAX + 1], str[32];

	snprintf(str, sizeof(str), "%d", timeout);

	create_filename(filename, PATH_MAX, bdaddr, "config");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	return textfile_put(filename, "pairto", str);
}

int read_pairable_timeout(const char *src, int *timeout)
{
	char filename[PATH_MAX + 1], *str;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "config");

	str = textfile_get(filename, "pairto");
	if (!str)
		return -ENOENT;

	if (sscanf(str, "%d", timeout) != 1) {
		free(str);
		return -ENOENT;
	}

	free(str);

	return 0;
}

int write_device_mode(bdaddr_t *bdaddr, const char *mode)
{
	char filename[PATH_MAX + 1];

	create_filename(filename, PATH_MAX, bdaddr, "config");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (strcmp(mode, "off") != 0)
		textfile_put(filename, "onmode", mode);

	return textfile_put(filename, "mode", mode);
}

int read_device_mode(const char *src, char *mode, int length)
{
	char filename[PATH_MAX + 1], *str;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "config");

	str = textfile_get(filename, "mode");
	if (!str)
		return -ENOENT;

	strncpy(mode, str, length);
	mode[length - 1] = '\0';

	free(str);

	return 0;
}

int read_on_mode(const char *src, char *mode, int length)
{
	char filename[PATH_MAX + 1], *str;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "config");

	str = textfile_get(filename, "onmode");
	if (!str)
		return -ENOENT;

	strncpy(mode, str, length);
	mode[length - 1] = '\0';

	free(str);

	return 0;
}

int write_local_name(bdaddr_t *bdaddr, const char *name)
{
	char filename[PATH_MAX + 1], str[249];
	int i;

	memset(str, 0, sizeof(str));
	for (i = 0; i < 248 && name[i]; i++)
		if ((unsigned char) name[i] < 32 || name[i] == 127)
			str[i] = '.';
		else
			str[i] = name[i];

	create_filename(filename, PATH_MAX, bdaddr, "config");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	return textfile_put(filename, "name", str);
}

int read_local_name(bdaddr_t *bdaddr, char *name)
{
	char filename[PATH_MAX + 1], *str;
	int len;

	create_filename(filename, PATH_MAX, bdaddr, "config");

	str = textfile_get(filename, "name");
	if (!str)
		return -ENOENT;

	len = strlen(str);
	if (len > 248)
		str[248] = '\0';
	strcpy(name, str);

	free(str);

	return 0;
}


int write_le_params(bdaddr_t *src, bdaddr_t *dst, struct bt_le_params *params)
{
	char filename[PATH_MAX + 1];
	char peer[18];
	char value[sizeof(struct bt_le_params) * 3];

	create_filename(filename, PATH_MAX, src, "le_params");
	ba2str(dst, peer);

	if (!params) {
		return textfile_del(filename, peer);
	}

	memset(value, 0, sizeof(value));

	snprintf(value, sizeof(value), "%2.2X %2.2X %4.4X %4.4X %4.4X "
					"%4.4X %4.4X %4.4X %4.4X %4.4X %4.4X",
					params->prohibit_remote_chg,
					params->filter_policy,
					params->scan_interval,
					params->scan_window,
					params->interval_min,
					params->interval_max,
					params->latency,
					params->supervision_timeout,
					params->min_ce_len,
					params->max_ce_len,
					params->conn_timeout);

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	return textfile_put(filename, peer, value);
}

struct bt_le_params *read_le_params(bdaddr_t *src, bdaddr_t *dst)
{
	char filename[PATH_MAX + 1], *value;
	char peer[18];
	struct bt_le_params *params = NULL;
	int cnt;
	int prohibit_remote_chg;
	int filter_policy;
	int scan_interval;
	int scan_window;
	int interval_min;
	int interval_max;
	int latency;
	int supervision_timeout;
	int min_ce_len;
	int max_ce_len;
	int conn_timeout;

	create_filename(filename, PATH_MAX, src, "le_params");
	ba2str(dst, peer);

	value = textfile_get(filename, peer);
	if (!value)
		return NULL;

	/* cnt must == number of members (10) or is invalid */
	cnt = sscanf(value, "%2X %2X %4X %4X %4X %4X %4X %4X %4X %4X %4X",
				&prohibit_remote_chg,
				&filter_policy,
				&scan_interval,
				&scan_window,
				&interval_min,
				&interval_max,
				&latency,
				&supervision_timeout,
				&min_ce_len,
				&max_ce_len,
				&conn_timeout);

	if (cnt != 11)
		goto done;

	params = g_malloc(sizeof(struct bt_le_params));
	if (!params)
		goto done;

	params->prohibit_remote_chg = (uint8_t) prohibit_remote_chg;
	params->filter_policy = (uint8_t) filter_policy;
	params->scan_interval = (uint16_t) scan_interval;
	params->scan_window = (uint16_t) scan_window;
	params->interval_min = (uint16_t) interval_min;
	params->interval_max = (uint16_t) interval_max;
	params->latency = (uint16_t) latency;
	params->supervision_timeout = (uint16_t) supervision_timeout;
	params->min_ce_len = (uint16_t) min_ce_len;
	params->max_ce_len = (uint16_t) max_ce_len;
	params->conn_timeout = (uint16_t) conn_timeout;

done:
	g_free(value);
	return params;
}

int write_local_class(bdaddr_t *bdaddr, uint8_t *class)
{
	char filename[PATH_MAX + 1], str[9];

	sprintf(str, "0x%2.2x%2.2x%2.2x", class[2], class[1], class[0]);

	create_filename(filename, PATH_MAX, bdaddr, "config");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	return textfile_put(filename, "class", str);
}

int read_local_class(bdaddr_t *bdaddr, uint8_t *class)
{
	char filename[PATH_MAX + 1], tmp[3], *str;
	int i;

	create_filename(filename, PATH_MAX, bdaddr, "config");

	str = textfile_get(filename, "class");
	if (!str)
		return -ENOENT;

	memset(tmp, 0, sizeof(tmp));
	for (i = 0; i < 3; i++) {
		memcpy(tmp, str + (i * 2) + 2, 2);
		class[2 - i] = (uint8_t) strtol(tmp, NULL, 16);
	}

	free(str);

	return 0;
}

int write_remote_class(bdaddr_t *local, bdaddr_t *peer, uint32_t class)
{
	char filename[PATH_MAX + 1], addr[18], str[9];

	create_filename(filename, PATH_MAX, local, "classes");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(peer, addr);
	sprintf(str, "0x%6.6x", class);

	return textfile_put(filename, addr, str);
}

int read_remote_class(bdaddr_t *local, bdaddr_t *peer, uint32_t *class)
{
	char filename[PATH_MAX + 1], addr[18], *str;

	create_filename(filename, PATH_MAX, local, "classes");

	ba2str(peer, addr);

	str = textfile_get(filename, addr);
	if (!str)
		return -ENOENT;

	if (sscanf(str, "%x", class) != 1) {
		free(str);
		return -ENOENT;
	}

	free(str);

	return 0;
}

int write_device_name(bdaddr_t *local, bdaddr_t *peer, char *name)
{
	char filename[PATH_MAX + 1], addr[18], str[249];
	int i;

	memset(str, 0, sizeof(str));
	for (i = 0; i < 248 && name[i]; i++)
		if ((unsigned char) name[i] < 32 || name[i] == 127)
			str[i] = '.';
		else
			str[i] = name[i];

	create_filename(filename, PATH_MAX, local, "names");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(peer, addr);
	return textfile_put(filename, addr, str);
}

int read_device_name(const char *src, const char *dst, char *name)
{
	char filename[PATH_MAX + 1], *str;
	int len;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "names");

	str = textfile_get(filename, dst);
	if (!str)
		return -ENOENT;

	len = strlen(str);
	if (len > 248)
		str[248] = '\0';
	strcpy(name, str);

	free(str);

	return 0;
}

int write_remote_eir(bdaddr_t *local, bdaddr_t *peer, uint8_t *data)
{
	char filename[PATH_MAX + 1], addr[18], str[481];
	int i;

	memset(str, 0, sizeof(str));
	for (i = 0; i < HCI_MAX_EIR_LENGTH; i++)
		sprintf(str + (i * 2), "%2.2X", data[i]);

	create_filename(filename, PATH_MAX, local, "eir");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(peer, addr);
	return textfile_put(filename, addr, str);
}

int read_remote_eir(bdaddr_t *local, bdaddr_t *peer, uint8_t *data)
{
	char filename[PATH_MAX + 1], addr[18], *str;
	int i;

	create_filename(filename, PATH_MAX, local, "eir");

	ba2str(peer, addr);

	str = textfile_get(filename, addr);
	if (!str)
		return -ENOENT;

	if (!data) {
		free(str);
		return 0;
	}

	if (strlen(str) < 480) {
		free(str);
		return -EIO;
	}

	for (i = 0; i < HCI_MAX_EIR_LENGTH; i++)
		sscanf(str + (i * 2), "%02hhX", &data[i]);

	free(str);

	return 0;
}

int read_version_info(bdaddr_t *local, bdaddr_t *peer, uint16_t *lmp_ver)
{
	char filename[PATH_MAX + 1], addr[18], *str, *ver_str, *subver_str, *ver;
	int len;

	create_filename(filename, PATH_MAX, local, "manufacturers");

	ba2str(peer, addr);

	str = textfile_get(filename, addr);
	if (!str)
		return -ENOENT;

	if (!lmp_ver) {
		free(str);
		return -ENOENT;
	}

	ver_str = strchr(str, ' ');
	if (!ver_str) {
		free(str);
		return -ENOENT;
	}
	*(ver_str++) = 0;

	subver_str = strchr(ver_str, ' ');
	if (!subver_str) {
		free(str);
		return -ENOENT;
	}
	*(subver_str++) = 0;

	len = subver_str-ver_str+1;

	ver = g_malloc(len);

	memcpy(ver, ver_str, len-1);
	ver[len-1] = '\0';


	if (lmp_ver)
		*lmp_ver = (uint16_t) strtol(ver, NULL, 10);

	free(ver);
	return 0;
}

int write_version_info(bdaddr_t *local, bdaddr_t *peer, uint16_t manufacturer,
					uint8_t lmp_ver, uint16_t lmp_subver)
{
	char filename[PATH_MAX + 1], addr[18], str[16];

	memset(str, 0, sizeof(str));
	sprintf(str, "%d %d %d", manufacturer, lmp_ver, lmp_subver);

	create_filename(filename, PATH_MAX, local, "manufacturers");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(peer, addr);
	return textfile_put(filename, addr, str);
}

int write_features_info(bdaddr_t *local, bdaddr_t *peer,
				unsigned char *page1, unsigned char *page2)
{
	char filename[PATH_MAX + 1], addr[18];
	char str[] = "0000000000000000 0000000000000000";
	char *old_value;
	int i;

	ba2str(peer, addr);

	create_filename(filename, PATH_MAX, local, "features");
	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	old_value = textfile_get(filename, addr);

	if (page1)
		for (i = 0; i < 8; i++)
			sprintf(str + (i * 2), "%2.2X", page1[i]);
	else if (old_value && strlen(old_value) >= 16)
		strncpy(str, old_value, 16);

	if (page2)
		for (i = 0; i < 8; i++)
			sprintf(str + 17 + (i * 2), "%2.2X", page2[i]);
	else if (old_value && strlen(old_value) >= 33)
		strncpy(str + 17, old_value + 17, 16);

	free(old_value);

	return textfile_put(filename, addr, str);
}

static int decode_bytes(const char *str, unsigned char *bytes, size_t len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		if (sscanf(str + (i * 2), "%02hhX", &bytes[i]) != 1)
			return -EINVAL;
	}

	return 0;
}

int read_remote_features(bdaddr_t *local, bdaddr_t *peer,
				unsigned char *page1, unsigned char *page2)
{
	char filename[PATH_MAX + 1], addr[18], *str;
	size_t len;
	int err;

	if (page1 == NULL && page2 == NULL)
		return -EINVAL;

	create_filename(filename, PATH_MAX, local, "features");

	ba2str(peer, addr);

	str = textfile_get(filename, addr);
	if (!str)
		return -ENOENT;

	len = strlen(str);

	err = -ENOENT;

	if (page1 && len >= 16)
		err = decode_bytes(str, page1, 8);

	if (page2 && len >= 33)
		err = decode_bytes(str + 17, page2, 8);

	free(str);

	return err;
}

int write_lastseen_info(bdaddr_t *local, bdaddr_t *peer, struct tm *tm)
{
	char filename[PATH_MAX + 1], addr[18], str[24];

	memset(str, 0, sizeof(str));
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S %Z", tm);

	create_filename(filename, PATH_MAX, local, "lastseen");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(peer, addr);
	return textfile_put(filename, addr, str);
}

int write_lastused_info(bdaddr_t *local, bdaddr_t *peer, struct tm *tm)
{
	char filename[PATH_MAX + 1], addr[18], str[24];

	memset(str, 0, sizeof(str));
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S %Z", tm);

	create_filename(filename, PATH_MAX, local, "lastused");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(peer, addr);
	return textfile_put(filename, addr, str);
}

int write_link_key(bdaddr_t *local, bdaddr_t *peer, unsigned char *key, uint8_t type, int length)
{
	char filename[PATH_MAX + 1], addr[18], str[38];
	int i;

	memset(str, 0, sizeof(str));
	for (i = 0; i < 16; i++)
		sprintf(str + (i * 2), "%2.2X", key[i]);
	sprintf(str + 32, " %d %d", type, length);

	create_filename(filename, PATH_MAX, local, "linkkeys");

	create_file(filename, S_IRUSR | S_IWUSR);

	ba2str(peer, addr);

	if (length < 0) {
		char *tmp = textfile_get(filename, addr);
		if (tmp) {
			if (strlen(tmp) > 34)
				memcpy(str + 34, tmp + 34, 3);
			free(tmp);
		}
	}

	return textfile_put(filename, addr, str);
}

int read_link_key(bdaddr_t *local, bdaddr_t *peer, unsigned char *key, uint8_t *type)
{
	char filename[PATH_MAX + 1], addr[18], tmp[3], *str;
	int i;

	create_filename(filename, PATH_MAX, local, "linkkeys");
	DBG("%s", filename);

	ba2str(peer, addr);
	str = textfile_get(filename, addr);
	if (!str)
		return -ENOENT;

	if (!key) {
		free(str);
		DBG("no key");
		return 0;
	}

	memset(tmp, 0, sizeof(tmp));
	for (i = 0; i < 16; i++) {
		memcpy(tmp, str + (i * 2), 2);
		key[i] = (uint8_t) strtol(tmp, NULL, 16);
	}

	if (type) {
		memcpy(tmp, str + 33, 2);
		*type = (uint8_t) strtol(tmp, NULL, 10);
	}

	free(str);
	DBG("key found");

	return 0;
}

static void find_hash_by_str(char *key, char *value, void *data)
{
	char *io = data;

	/* See if we already found Addr in lekeys */
	if (io[sizeof(uint32_t) * 2] == '\0')
		return;

	if (strstr(value, io))
		memcpy(io, key, sizeof(uint32_t) * 2 + 1);
}

int write_le_key(bdaddr_t *local, bdaddr_t *peer, uint8_t addr_type,
		uint32_t *hash, unsigned char *key, uint8_t key_type,
		uint8_t length, uint8_t auth, uint8_t dlen, uint8_t *data)
{
	char filename[PATH_MAX + 1], hashkey[9];
	uint32_t this_hash;
	uint8_t mask, old_mask, old_offset;
	char *keystr, *newstr;
	int i, new_len, err = 0;

	if (!local || !peer || !hash || !key)
		return 0;

	this_hash = *hash;

	create_filename(filename, PATH_MAX, local, "lekeys");

	create_file(filename, S_IRUSR | S_IWUSR);

	/* Find hash by BDADDR */
	if (!this_hash) {
		char tmp[18];

		ba2str(peer, tmp);
		textfile_foreach(filename, find_hash_by_str, tmp);

		if (tmp[sizeof(uint32_t) * 2] == '\0') {
			this_hash = (uint32_t) strtol(tmp, NULL, 16);
		}
	}

	/* Generate new hash */
	if (!this_hash) {
		char *tmp = textfile_get(filename, "lasthash");
		if (tmp) {
			this_hash = (uint32_t) strtol(tmp, NULL, 16);
			free(tmp);
		}
		this_hash++;
		sprintf(hashkey, "%8.8X", this_hash);
		err = textfile_put(filename, "lasthash", hashkey);
		if (err)
			return err;
	} else {
		sprintf(hashkey, "%8.8X", this_hash);
	}

	switch (key_type) {
	case KEY_TYPE_LTK:
		mask = LE_STORE_LTK;
		break;
	case KEY_TYPE_IRK:
		mask = LE_STORE_IRK;
		break;
	case KEY_TYPE_CSRK:
		mask = LE_STORE_CSRK;
		break;
	default:
		return 0;
	}

	newstr = g_malloc0(LE_KEY_LEN);
	keystr = textfile_get(filename, hashkey);
	if (keystr) {
		addr_type = (uint8_t) strtol(&keystr[18], NULL, 16);
		old_mask = (uint8_t) strtol(&keystr[18+3], NULL, 16);
		length = (uint8_t) strtol(&keystr[18+3+3], NULL, 16);
		auth = (uint8_t) strtol(&keystr[18+3+3+3], NULL, 16);
		mask |= old_mask;
		keystr[17] = '\0';
		sprintf(newstr, "%s %2.2X %2.2X %2.2X %2.2X",
				keystr, addr_type, mask, length, auth);
	} else {
		old_mask = 0;
		ba2str(peer, newstr);
		sprintf(&newstr[17], " %2.2X %2.2X %2.2X %2.2X",
				addr_type, mask, length, auth);
	}

	old_offset = new_len = strlen(newstr);

	if (mask & LE_STORE_LTK) {
		if ((key_type == KEY_TYPE_LTK) && (dlen == 10) && data) {
			newstr[new_len++] = ' ';
			for (i = 0; i < 16; i++) {
				sprintf(&newstr[new_len], "%2.2X", key[i]);
				new_len += 2;
			}
			newstr[new_len++] = ' ';
			for (i = 0; i < 10; i++) {
				sprintf(&newstr[new_len], "%2.2X", data[i]);
				new_len += 2;
			}
		} else
			memcpy(&newstr[new_len], &keystr[old_offset],
					LE_KEY_LTK_LEN-1);

		new_len = strlen(newstr);

		if (old_mask & LE_STORE_LTK)
			old_offset += LE_KEY_LTK_LEN;
	}

	if (mask & LE_STORE_IRK) {
		if ((key_type == KEY_TYPE_IRK) && (dlen == 7) && data) {
			newstr[new_len++] = ' ';
			for (i = 0; i < 16; i++) {
				sprintf(&newstr[new_len], "%2.2X", key[i]);
				new_len += 2;
			}
			sprintf(&newstr[new_len], " %2.2X ", data[0]);
			new_len += 4;
			ba2str((bdaddr_t *)&data[1], &newstr[new_len]);
		} else
			memcpy(&newstr[new_len], &keystr[old_offset],
					LE_KEY_IRK_LEN-1);

		new_len = strlen(newstr);

		if (old_mask & LE_STORE_IRK)
			old_offset += LE_KEY_IRK_LEN;
	}

	if (mask & LE_STORE_CSRK) {
		if ((key_type == KEY_TYPE_CSRK) && (dlen == 4) && data) {
			newstr[new_len++] = ' ';
			for (i = 0; i < 16; i++) {
				sprintf(&newstr[new_len], "%2.2X", key[i]);
				new_len += 2;
			}
			newstr[new_len++] = ' ';
			for (i = 0; i < 4; i++) {
				sprintf(&newstr[new_len], "%2.2X", data[i]);
				new_len += 2;
			}
		} else
			memcpy(&newstr[new_len], &keystr[old_offset],
					LE_KEY_CSRK_LEN-1);
	}

	err = textfile_put(filename, hashkey, newstr);

	if (keystr)
		free(keystr);

	free(newstr);

	if (!err)
		*hash = this_hash;

	return err;
}

int read_le_key(bdaddr_t *local, bdaddr_t *peer, uint8_t *addr_type,
		uint32_t *hash, uint8_t *length, uint8_t *auth,
		unsigned char *key, uint8_t key_type, uint8_t *dlen,
		uint8_t *data, uint8_t max_dlen)
{
	char filename[PATH_MAX + 1], hashkey[9], *keystr;
	uint32_t this_hash;
	uint8_t this_addr_type, this_type, mask;
	int i, len;
	int found = 0;

	if (!local || !peer) {
		DBG("%p %p", local, peer);
		return -ENOENT;
	}

	this_type = key_type;
	this_hash = hash ? *hash : 0;

	create_filename(filename, PATH_MAX, local, "lekeys");

	/* default to LTK if unrecognized */
	switch (this_type) {
	case KEY_TYPE_IRK:
	case KEY_TYPE_CSRK:
		break;
	default:
		this_type = KEY_TYPE_LTK;
	}

	/* Find hash by BDADDR */
	if (!this_hash) {
		char tmp[18];

		ba2str(peer, tmp);
		textfile_foreach(filename, find_hash_by_str, tmp);

		if (tmp[sizeof(uint32_t) * 2] == '\0') {
			this_hash = (uint32_t) strtol(tmp, NULL, 16);
		}
	}

	if (!this_hash) {
		DBG("!this_hash");
		return -ENOENT;
	}

	sprintf(hashkey, "%8.8X", this_hash);
	keystr = textfile_get(filename, hashkey);

	if (!keystr) {
		DBG("!keystr for %s", hashkey);
		return -ENOENT;
	}

	DBG("keystr: %s", keystr);

	this_addr_type = (uint8_t) strtol(&keystr[18], NULL, 16);

	mask = (uint8_t) strtol(&keystr[18+3], NULL, 16);

	/* Get Length, or if only verifying existance, mark as found */
	if (length)
		*length = (uint8_t) strtol(&keystr[18+3+3], NULL, 16);
	else
		found = 1;

	if (auth)
		*auth = (uint8_t) strtol(&keystr[18+3+3+3], NULL, 16);

	len = LE_KEY_HDR_LEN;

	if (mask & LE_STORE_LTK) {
		if (this_type == KEY_TYPE_LTK) {
			if ((max_dlen >= 10) && data && dlen) {
				char tmp[] = {0, 0, 0};

				for (i = 0; i < 16; i++) {
					memcpy(tmp, &keystr[len], 2);
					key[i] = (uint8_t) strtol(tmp, NULL, 16);
					len+=2;
				}

				len++;

				for (i = 0; i < 10; i++) {
					memcpy(tmp, &keystr[len], 2);
					data[i] = (uint8_t) strtol(tmp, NULL, 16);
					len+=2;
				}
				*dlen = 10;
				found = 1;
			}
			goto done;
		}
		len += LE_KEY_LTK_LEN;
	}

	if (mask & LE_STORE_IRK) {
		if (this_type == KEY_TYPE_IRK) {
			if ((max_dlen >= 7) && data && dlen) {
				char tmp[] = {0, 0, 0};

				for (i = 0; i < 16; i++) {
					memcpy(tmp, &keystr[len], 2);
					key[i] = (uint8_t) strtol(tmp, NULL, 16);
					len+=2;
				}

				len++;

				for (i = 0; i < 7; i++) {
					data[i] = (uint8_t) strtol(&keystr[len], NULL, 16);
					len+=3;
				}
				*dlen = 7;
				found = 1;
			}
			goto done;
		}
		len += LE_KEY_IRK_LEN;
	}

	if (mask & LE_STORE_CSRK) {
		if (this_type == KEY_TYPE_CSRK) {
			if ((max_dlen >= 10) && data && dlen) {
				char tmp[] = {0, 0, 0};

				for (i = 0; i < 16; i++) {
					memcpy(tmp, &keystr[len], 2);
					key[i] = (uint8_t) strtol(tmp, NULL, 16);
					len+=2;
				}

				len++;

				for (i = 0; i < 4; i++) {
					memcpy(tmp, &keystr[len], 2);
					data[i] = (uint8_t) strtol(tmp, NULL, 16);
					len+=2;
				}
				*dlen = 4;
				found = 1;
			}
		}
	}

done:
	free(keystr);

	if (found) {
		DBG("found");
		return 0;
	} else {
		DBG("!found");
		return -ENOENT;
	}
}

int delete_le_keys(bdaddr_t *local, bdaddr_t *peer, uint32_t hash)
{
	char filename[PATH_MAX + 1], hashkey[9];

	if (!local)
		return -ENOENT;

	create_filename(filename, PATH_MAX, local, "lekeys");

	if (!hash && peer) {
		char tmp[18];

		ba2str(peer, tmp);
		DBG("Finding LE device by Addr: %s", tmp);
		textfile_foreach(filename, find_hash_by_str, tmp);

		DBG("Found: %s???", tmp);
		if (tmp[sizeof(uint32_t) * 2] == '\0') {
			hash = (uint32_t) strtol(tmp, NULL, 16);
		}
	}

	if (!hash)
		return 0;

	sprintf(hashkey, "%8.8X", hash);
	DBG("Hash: %s???", hashkey);
	textfile_del(filename, hashkey);

	return 0;
}

uint32_t read_le_hash(bdaddr_t *local, bdaddr_t *peer, uint8_t *mid, uint8_t len)
{
	char filename[PATH_MAX + 1];
	uint32_t hash = 0;

	create_filename(filename, PATH_MAX, local, "lekeys");

	create_file(filename, S_IRUSR | S_IWUSR);

	/* Obtain hash from Addr */
	if (peer) {
		char tmp[18];

		ba2str(peer, tmp);
		textfile_foreach(filename, find_hash_by_str, tmp);

		if (tmp[sizeof(uint32_t) * 2] == '\0') {
			hash = (uint32_t) strtol(tmp, NULL, 16);
		}
	}

	/* Obtain hash from Master ID */
	if (!hash && len && mid) {
		char io[21];
		int i;

		for (i = 0; i < 10; i++)
			sprintf(io + (i * 2), "%2.2X", mid[i]);

		textfile_foreach(filename, find_hash_by_str, io);

		if (io[sizeof(uint32_t) * 2] == '\0')
			hash = (uint32_t) strtol(io, NULL, 16);
	}

	/* Previously unseen device: Allocate hash */
	if (!hash) {
		char hashstr[9];
		char *str = textfile_get(filename, "lasthash");

		if (str) {
			hash = (uint32_t) strtol(str, NULL, 16);
			free(str);
		}
		hash++;
		sprintf(hashstr, "%8.8X", hash);
		textfile_put(filename, "lasthash", hashstr);
	}

	return hash;
}

int read_pin_code(bdaddr_t *local, bdaddr_t *peer, char *pin)
{
	char filename[PATH_MAX + 1], addr[18], *str;
	int len;

	create_filename(filename, PATH_MAX, local, "pincodes");

	ba2str(peer, addr);
	str = textfile_get(filename, addr);
	if (!str)
		return -ENOENT;

	strncpy(pin, str, 16);
	len = strlen(pin);

	free(str);

	return len;
}

static GSList *service_string_to_list(char *services)
{
	GSList *l = NULL;
	char *start = services;
	int i, finished = 0;

	for (i = 0; !finished; i++) {
		if (services[i] == '\0')
			finished = 1;

		if (services[i] == ' ' || services[i] == '\0') {
			services[i] = '\0';
			l = g_slist_append(l, start);
			start = services + i + 1;
		}
	}

	return l;
}

static char *service_list_to_string(GSList *services)
{
	char str[1024];
	int len = 0;

	if (!services)
		return g_strdup("");

	memset(str, 0, sizeof(str));

	while (services) {
		int ret;
		char *ident = services->data;

		ret = snprintf(str + len, sizeof(str) - len - 1, "%s%s",
				ident, services->next ? " " : "");

		if (ret > 0)
			len += ret;

		services = services->next;
	}

	return g_strdup(str);
}

int write_trust(const char *src, const char *addr, const char *service,
		gboolean trust)
{
	char filename[PATH_MAX + 1], *str;
	GSList *services = NULL, *match;
	gboolean trusted;
	int ret;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "trusts");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	str = textfile_caseget(filename, addr);
	if (str)
		services = service_string_to_list(str);

	match = g_slist_find_custom(services, service, (GCompareFunc) strcmp);
	trusted = match ? TRUE : FALSE;

	/* If the old setting is the same as the requested one, we're done */
	if (trusted == trust) {
		g_slist_free(services);
		free(str);
		return 0;
	}

	if (trust)
		services = g_slist_append(services, (void *) service);
	else
		services = g_slist_remove(services, match->data);

	/* Remove the entry if the last trusted service was removed */
	if (!trust && !services)
		ret = textfile_casedel(filename, addr);
	else {
		char *new_str = service_list_to_string(services);
		ret = textfile_caseput(filename, addr, new_str);
		free(new_str);
	}

	g_slist_free(services);

	free(str);

	return ret;
}

gboolean read_trust(const bdaddr_t *local, const char *addr, const char *service)
{
	char filename[PATH_MAX + 1], *str;
	GSList *services;
	gboolean ret;

	create_filename(filename, PATH_MAX, local, "trusts");

	str = textfile_caseget(filename, addr);
	if (!str)
		return FALSE;

	services = service_string_to_list(str);

	if (g_slist_find_custom(services, service, (GCompareFunc) strcmp))
		ret = TRUE;
	else
		ret = FALSE;

	g_slist_free(services);
	free(str);

	return ret;
}

struct trust_list {
	GSList *trusts;
	const char *service;
};

static void append_trust(char *key, char *value, void *data)
{
	struct trust_list *list = data;

	if (strstr(value, list->service))
		list->trusts = g_slist_append(list->trusts, g_strdup(key));
}

GSList *list_trusts(bdaddr_t *local, const char *service)
{
	char filename[PATH_MAX + 1];
	struct trust_list list;

	create_filename(filename, PATH_MAX, local, "trusts");

	list.trusts = NULL;
	list.service = service;

	if (textfile_foreach(filename, append_trust, &list) < 0)
		return NULL;

	return list.trusts;
}

int write_device_profiles(bdaddr_t *src, bdaddr_t *dst, const char *profiles)
{
	char filename[PATH_MAX + 1], addr[18];

	if (!profiles)
		return -EINVAL;

	create_filename(filename, PATH_MAX, src, "profiles");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(dst, addr);
	return textfile_put(filename, addr, profiles);
}

int delete_entry(bdaddr_t *src, const char *storage, const char *key)
{
	char filename[PATH_MAX + 1];

	create_filename(filename, PATH_MAX, src, storage);

	return textfile_del(filename, key);
}

int store_record(const gchar *src, const gchar *dst, sdp_record_t *rec)
{
	char filename[PATH_MAX + 1], key[28];
	sdp_buf_t buf;
	int err, size, i;
	char *str;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "sdp");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	snprintf(key, sizeof(key), "%17s#%08X", dst, rec->handle);

	if (sdp_gen_record_pdu(rec, &buf) < 0)
		return -1;

	size = buf.data_size;

	str = g_malloc0(size*2+1);

	for (i = 0; i < size; i++)
		sprintf(str + (i * 2), "%02X", buf.data[i]);

	err = textfile_put(filename, key, str);

	free(buf.data);
	free(str);

	return err;
}

sdp_record_t *record_from_string(const gchar *str)
{
	sdp_record_t *rec;
	int size, i, len;
	uint8_t *pdata;
	char tmp[3];

	size = strlen(str)/2;
	pdata = g_malloc0(size);

	tmp[2] = 0;
	for (i = 0; i < size; i++) {
		memcpy(tmp, str + (i * 2), 2);
		pdata[i] = (uint8_t) strtol(tmp, NULL, 16);
	}

	rec = sdp_extract_pdu(pdata, size, &len);
	free(pdata);

	return rec;
}


sdp_record_t *fetch_record(const gchar *src, const gchar *dst,
						const uint32_t handle)
{
	char filename[PATH_MAX + 1], key[28], *str;
	sdp_record_t *rec;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "sdp");

	snprintf(key, sizeof(key), "%17s#%08X", dst, handle);

	str = textfile_get(filename, key);
	if (!str)
		return NULL;

	rec = record_from_string(str);
	free(str);

	return rec;
}

int delete_record(const gchar *src, const gchar *dst, const uint32_t handle)
{
	char filename[PATH_MAX + 1], key[28];

	create_name(filename, PATH_MAX, STORAGEDIR, src, "sdp");

	snprintf(key, sizeof(key), "%17s#%08X", dst, handle);

	return textfile_del(filename, key);
}

struct record_list {
	sdp_list_t *recs;
	const gchar *addr;
};

static void create_stored_records_from_keys(char *key, char *value,
							void *user_data)
{
	struct record_list *rec_list = user_data;
	const gchar *addr = rec_list->addr;
	sdp_record_t *rec;

	if (strncmp(key, addr, 17))
		return;

	rec = record_from_string(value);

	rec_list->recs = sdp_list_append(rec_list->recs, rec);
}

void delete_all_records(const bdaddr_t *src, const bdaddr_t *dst)
{
	sdp_list_t *records, *seq;
	char srcaddr[18], dstaddr[18];

	ba2str(src, srcaddr);
	ba2str(dst, dstaddr);

	records = read_records(src, dst);

	for (seq = records; seq; seq = seq->next) {
		sdp_record_t *rec = seq->data;
		delete_record(srcaddr, dstaddr, rec->handle);
	}

	if (records)
		sdp_list_free(records, (sdp_free_func_t) sdp_record_free);
}

sdp_list_t *read_records(const bdaddr_t *src, const bdaddr_t *dst)
{
	char filename[PATH_MAX + 1];
	struct record_list rec_list;
	char srcaddr[18], dstaddr[18];

	ba2str(src, srcaddr);
	ba2str(dst, dstaddr);

	rec_list.addr = dstaddr;
	rec_list.recs = NULL;

	create_name(filename, PATH_MAX, STORAGEDIR, srcaddr, "sdp");
	textfile_foreach(filename, create_stored_records_from_keys, &rec_list);

	return rec_list.recs;
}

sdp_record_t *find_record_in_list(sdp_list_t *recs, const char *uuid)
{
	sdp_list_t *seq;

	for (seq = recs; seq; seq = seq->next) {
		sdp_record_t *rec = (sdp_record_t *) seq->data;
		sdp_list_t *svcclass = NULL;
		sdp_list_t *svcclass_ext = NULL;
		char *uuid_str;

		if (sdp_get_service_classes(rec, &svcclass) < 0)
			continue;

		/* Extract the uuid */
		svcclass_ext = svcclass;
		do {
			uuid_str = bt_uuid2string(svcclass_ext->data);
			if (!uuid_str)
				continue;

			if (!strcasecmp(uuid_str, uuid)) {
				sdp_list_free(svcclass, free);
				free(uuid_str);
				return rec;
			}

			free(uuid_str);

		} while (svcclass_ext = svcclass_ext->next);

		sdp_list_free(svcclass, free);
	}
	return NULL;
}

int store_device_id(const gchar *src, const gchar *dst,
				const uint16_t source, const uint16_t vendor,
				const uint16_t product, const uint16_t version)
{
	char filename[PATH_MAX + 1], str[20];

	create_name(filename, PATH_MAX, STORAGEDIR, src, "did");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	snprintf(str, sizeof(str), "%04X %04X %04X %04X", source,
						vendor, product, version);

	return textfile_put(filename, dst, str);
}

static int read_device_id_from_did(const gchar *src, const gchar *dst,
					uint16_t *source, uint16_t *vendor,
					uint16_t *product, uint16_t *version)
{
	char filename[PATH_MAX + 1];
	char *str, *vendor_str, *product_str, *version_str;

	create_name(filename, PATH_MAX, STORAGEDIR, src, "did");

	str = textfile_get(filename, dst);
	if (!str)
		return -ENOENT;

	vendor_str = strchr(str, ' ');
	if (!vendor_str) {
		free(str);
		return -ENOENT;
	}
	*(vendor_str++) = 0;

	product_str = strchr(vendor_str, ' ');
	if (!product_str) {
		free(str);
		return -ENOENT;
	}
	*(product_str++) = 0;

	version_str = strchr(product_str, ' ');
	if (!version_str) {
		free(str);
		return -ENOENT;
	}
	*(version_str++) = 0;

	if (source)
		*source = (uint16_t) strtol(str, NULL, 16);
	if (vendor)
		*vendor = (uint16_t) strtol(vendor_str, NULL, 16);
	if (product)
		*product = (uint16_t) strtol(product_str, NULL, 16);
	if (version)
		*version = (uint16_t) strtol(version_str, NULL, 16);

	free(str);

	return 0;
}

int read_device_id(const gchar *srcaddr, const gchar *dstaddr,
					uint16_t *source, uint16_t *vendor,
					uint16_t *product, uint16_t *version)
{
	uint16_t lsource, lvendor, lproduct, lversion;
	sdp_list_t *recs;
	sdp_record_t *rec;
	bdaddr_t src, dst;
	int err;

	err = read_device_id_from_did(srcaddr, dstaddr, &lsource,
						vendor, product, version);
	if (!err) {
		if (lsource == 0xffff)
			err = -ENOENT;

		return err;
	}

	str2ba(srcaddr, &src);
	str2ba(dstaddr, &dst);

	recs = read_records(&src, &dst);
	rec = find_record_in_list(recs, PNP_UUID);

	if (rec) {
		sdp_data_t *pdlist;

		pdlist = sdp_data_get(rec, SDP_ATTR_VENDOR_ID_SOURCE);
		lsource = pdlist ? pdlist->val.uint16 : 0x0000;

		pdlist = sdp_data_get(rec, SDP_ATTR_VENDOR_ID);
		lvendor = pdlist ? pdlist->val.uint16 : 0x0000;

		pdlist = sdp_data_get(rec, SDP_ATTR_PRODUCT_ID);
		lproduct = pdlist ? pdlist->val.uint16 : 0x0000;

		pdlist = sdp_data_get(rec, SDP_ATTR_VERSION);
		lversion = pdlist ? pdlist->val.uint16 : 0x0000;

		err = 0;
	}

	sdp_list_free(recs, (sdp_free_func_t)sdp_record_free);

	if (err) {
		/* FIXME: We should try EIR data if we have it, too */

		/* If we don't have the data, we don't want to go through the
		 * above search every time. */
		lsource = 0xffff;
		lvendor = 0x0000;
		lproduct = 0x0000;
		lversion = 0x0000;
	}

	store_device_id(srcaddr, dstaddr, lsource, lvendor, lproduct, lversion);

	if (err)
		return err;

	if (source)
		*source = lsource;
	if (vendor)
		*vendor = lvendor;
	if (product)
		*product = lproduct;
	if (version)
		*version = lversion;

	return 0;
}

int write_device_pairable(bdaddr_t *bdaddr, gboolean mode)
{
	char filename[PATH_MAX + 1];

	create_filename(filename, PATH_MAX, bdaddr, "config");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	return textfile_put(filename, "pairable", mode ? "yes" : "no");
}

int read_device_pairable(bdaddr_t *bdaddr, gboolean *mode)
{
	char filename[PATH_MAX + 1], *str;

	create_filename(filename, PATH_MAX, bdaddr, "config");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	str = textfile_get(filename, "pairable");
	if (!str)
		return -ENOENT;

	*mode = strcmp(str, "yes") == 0 ? TRUE : FALSE;

	free(str);

	return 0;
}

gboolean read_blocked(const bdaddr_t *local, const bdaddr_t *remote)
{
	char filename[PATH_MAX + 1], *str, addr[18];

	create_filename(filename, PATH_MAX, local, "blocked");

	ba2str(remote, addr);

	str = textfile_caseget(filename, addr);
	if (!str)
		return FALSE;

	free(str);

	return TRUE;
}

int write_blocked(const bdaddr_t *local, const bdaddr_t *remote,
							gboolean blocked)
{
	char filename[PATH_MAX + 1], addr[18];

	create_filename(filename, PATH_MAX, local, "blocked");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(remote, addr);

	if (blocked == FALSE)
		return textfile_casedel(filename, addr);

	return textfile_caseput(filename, addr, "");
}

int write_device_services(const bdaddr_t *sba, const bdaddr_t *dba,
							const char *services)
{
	char filename[PATH_MAX + 1], addr[18];

	create_filename(filename, PATH_MAX, sba, "primary");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(dba, addr);

	return textfile_put(filename, addr, services);
}

static void filter_keys(char *key, char *value, void *data)
{
	struct match *match = data;
	const char *address = match->pattern;

	/* Each key contains: MAC#handle*/
	if (strncasecmp(key, address, 17) == 0)
		match->keys = g_slist_append(match->keys, g_strdup(key));
}

int delete_device_service(const bdaddr_t *sba, const bdaddr_t *dba)
{
	GSList *l;
	struct match match;
	char filename[PATH_MAX + 1], address[18];
	int err;

	create_filename(filename, PATH_MAX, sba, "primary");

	memset(address, 0, sizeof(address));
	ba2str(dba, address);

	err = textfile_del(filename, address);
	if (err < 0)
		return err;

	/* Deleting all characteristics of a given address */
	memset(&match, 0, sizeof(match));
	match.pattern = address;

	create_filename(filename, PATH_MAX, sba, "characteristic");
	err = textfile_foreach(filename, filter_keys, &match);
	if (err < 0)
		return err;

	for (l = match.keys; l; l = l->next) {
		const char *key = l->data;
		textfile_del(filename, key);
	}

	g_slist_foreach(match.keys, (GFunc) g_free, NULL);
	g_slist_free(match.keys);

	/* Deleting all attributes values of a given address */
	memset(&match, 0, sizeof(match));
	match.pattern = address;

	create_filename(filename, PATH_MAX, sba, "attributes");
	err = textfile_foreach(filename, filter_keys, &match);
	if (err < 0)
		return err;

	for (l = match.keys; l; l = l->next) {
		const char *key = l->data;
		textfile_del(filename, key);
	}

	g_slist_foreach(match.keys, (GFunc) g_free, NULL);
	g_slist_free(match.keys);

	return 0;
}

char *read_device_services(const bdaddr_t *sba, const bdaddr_t *dba)
{
	char filename[PATH_MAX + 1], addr[18];

	create_filename(filename, PATH_MAX, sba, "primary");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(dba, addr);

	return textfile_caseget(filename, addr);
}

int write_device_characteristics(const bdaddr_t *sba, const bdaddr_t *dba,
					uint16_t handle, const char *chars)
{
	char filename[PATH_MAX + 1], addr[18], key[23];

	create_filename(filename, PATH_MAX, sba, "characteristic");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(dba, addr);

	snprintf(key, sizeof(key), "%17s#%04X", addr, handle);

	return textfile_put(filename, key, chars);
}

char *read_device_characteristics(const bdaddr_t *sba, const bdaddr_t *dba,
							uint16_t handle)
{
	char filename[PATH_MAX + 1], addr[18], key[23];

	create_filename(filename, PATH_MAX, sba, "characteristic");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(dba, addr);

	snprintf(key, sizeof(key), "%17s#%04X", addr, handle);

	return textfile_caseget(filename, key);
}

int write_device_attribute(const bdaddr_t *sba, const bdaddr_t *dba,
					uint16_t handle, const char *chars)
{
	char filename[PATH_MAX + 1], addr[18], key[23];

	create_filename(filename, PATH_MAX, sba, "attributes");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(dba, addr);

	snprintf(key, sizeof(key), "%17s#%04X", addr, handle);

	return textfile_put(filename, key, chars);
}

int read_device_attributes(const bdaddr_t *sba, textfile_cb func, void *data)
{
	char filename[PATH_MAX + 1];

	create_filename(filename, PATH_MAX, sba, "attributes");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	return textfile_foreach(filename, func, data);
}

int write_device_type(const bdaddr_t *sba, const bdaddr_t *dba,
						device_type_t type)
{
	char filename[PATH_MAX + 1], addr[18], chars[3];

	create_filename(filename, PATH_MAX, sba, "types");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(dba, addr);

	snprintf(chars, sizeof(chars), "%2.2X", type);

	return textfile_put(filename, addr, chars);
}

device_type_t read_device_type(const bdaddr_t *sba, const bdaddr_t *dba)
{
	char filename[PATH_MAX + 1], addr[18], *chars;
	device_type_t type;

	create_filename(filename, PATH_MAX, sba, "types");

	create_file(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	ba2str(dba, addr);

	chars = textfile_caseget(filename, addr);
	if (chars == NULL)
		return DEVICE_TYPE_UNKNOWN;

	type = strtol(chars, NULL, 16);

	free(chars);

	return type;
}

int read_special_map_devaddr(char *category, bdaddr_t *peer, uint8_t *match)
{
	char filename[PATH_MAX+1], addr[18], *str, *temp, *next_adrList;

	ba2str(peer, addr);

	if (!category || !match) {
		DBG("category or match is NULL");
		return -ENOENT;
	}

	snprintf(filename, PATH_MAX, "%s/%s", CONFIGDIR, IOT_SPECIAL_MAPPING_FILE);

	str = textfile_get(filename, category);
	if (!str) {
		DBG("category %s is not available in iop file",category);
		return -ENOENT;
	}

	*match = 0;
	temp = str;

	while (temp) {
		if (strncasecmp(temp, addr, 8) == 0) {
			DBG("match found");
			*match = 1;
			break;
		}

		next_adrList = strchr(temp, ';');
		if (!next_adrList)
			break;

		temp = next_adrList + 1;
	}
	free(str);

	return 0;
}

int read_special_map_devname(char *category, char *name, uint8_t *match)
{
	char filename[PATH_MAX+1], *str, *temp, *next_adrList;
	int len;

	if (!category || !match || !name) {
		DBG("one of the input arguments is null");
		return -ENOENT;
	}

	len = strlen(name);
	if (!len) {
		DBG("input name length is 0");
		return -ENOENT;
	}

	snprintf(filename, PATH_MAX, "%s/%s", CONFIGDIR, IOT_SPECIAL_MAPPING_FILE);
	str = textfile_get(filename, category);
	if (!str) {
		DBG("category %s is not available in iop file", category);
		return -ENOENT;
	}

	*match = 0;
	temp = str;

	while (temp) {
		if ((strlen(temp) >= len) &&
			(strncasecmp(temp, name, len) == 0) &&
			(temp[len] == ';' || temp[len] == '\0')) {
			DBG("match exist");
			*match = 1;
			break;
		}

		next_adrList = strchr(temp, ';');
		if (!next_adrList)
			break;

		temp = next_adrList + 1;
	}

	free(str);

	return 0;
}
