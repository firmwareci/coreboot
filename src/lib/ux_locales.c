/* SPDX-License-Identifier: GPL-2.0-only */

#include <cbfs.h>
#include <console/console.h>
#include <security/vboot/misc.h>
#include <stddef.h>
#include <string.h>
#include <ux_locales.h>
#include <vb2_api.h>

#define LANG_ID_MAX 100
#define LANG_ID_LEN 3
#define PRERAM_LOCALES_NAME "preram_locales"

/*
 * Devices which support early vga have the capability to show localized text in
 * Code Page 437 encoding. (see src/drivers/pc80/vga/vga_font_8x16.c)
 *
 * preram_locales located in CBFS is an uncompressed file located in either RO
 * or RW CBFS. It contains the localization information in the following format:
 *
 * [string_name_1] [\x00]
 * [language_id_1] [\x00] [localized_string_1] [\x00]
 * [language_id_2] [\x00] [localized_string_2] [\x00] ...
 * [string_name_2] [\x00] ...
 *
 * This file contains tools to locate the file and search for localized strings
 * with specific language ID.
 */

/* Cached state for map (locales_get_map) and unmap (ux_locales_unmap). */
struct preram_locales_state {
	void *data;
	size_t size;
	bool initialized;
};

static struct preram_locales_state cached_state;

void ux_locales_unmap(void)
{
	if (cached_state.initialized) {
		if (cached_state.data)
			cbfs_unmap(cached_state.data);
		cached_state.initialized = false;
		cached_state.size = 0;
		cached_state.data = NULL;
	}
}

/* Get the map address of preram_locales. */
static void *locales_get_map(size_t *size_out, bool unmap)
{
	if (cached_state.initialized) {
		*size_out = cached_state.size;
		return cached_state.data;
	}
	cached_state.initialized = true;
	cached_state.data = cbfs_ro_map(PRERAM_LOCALES_NAME,
					&cached_state.size);
	*size_out = cached_state.size;
	return cached_state.data;
}

/* Move to the next string in the data. Strings are separated by 0x00. */
static size_t move_next(const char *data, size_t offset, size_t size)
{
	if (offset >= size)
		return size;
	while (offset < size && data[offset] != '\0')
		offset++;
	offset++;
	return offset;
}

/* Find the next occurrence of the specific string. */
static size_t search_for(const char *data, size_t offset, size_t size,
			 const char *str)
{
	if (offset >= size)
		return size;
	while (offset < size) {
		if (!strncmp(data + offset, str, size - offset))
			return offset;
		offset = move_next(data, offset, size);
	}
	return size;
}

/* Find the next occurrence of the integer ID, where ID is less than 100. */
static size_t search_for_id(const char *data, size_t offset, size_t size,
			    int id)
{
	if (id >= LANG_ID_MAX)
		return offset;
	char int_to_str[LANG_ID_LEN] = {};
	snprintf(int_to_str, LANG_ID_LEN, "%d", id);
	return search_for(data, offset, size, int_to_str);
}

const char *ux_locales_get_text(const char *name)
{
	const char *data;
	size_t size, offset, name_offset, next;
	uint32_t lang_id;

	data = locales_get_map(&size, false);
	if (!data) {
		printk(BIOS_INFO, "%s: %s not found.\n", __func__,
		       PRERAM_LOCALES_NAME);
		return NULL;
	}

	/* Get the language ID from vboot API. */
	lang_id = vb2api_get_locale_id(vboot_get_context());
	/* Validity check: Language ID should smaller than LANG_ID_MAX. */
	if (lang_id >= LANG_ID_MAX) {
		printk(BIOS_INFO, "%s: ID %d too big; fallback to 0.\n",
		       __func__, lang_id);
		lang_id = 0;
	}

	printk(BIOS_INFO, "%s: Search for %s with language ID: %u\n",
	       __func__, name, lang_id);

	offset = 0;
	/* Search for name. */
	offset = search_for(data, offset, size, name);
	if (offset >= size) {
		printk(BIOS_INFO, "%s: Name %s not found.\n", __func__, name);
		return NULL;
	}
	name_offset = offset;

	/* Search for language ID. */
	offset = search_for_id(data,  name_offset, size, lang_id);
	/* Language ID not supported; fallback to English if the current
	 * language is not English (0). */
	if (offset >= size) {
		/*
		 * Since we only support a limited charset, it is very normal
		 * that a language is not supported and we fallback here
		 * silently.
		 */
		if (lang_id != 0)
			offset = search_for_id(data, name_offset, size, 0);
		if (offset >= size) {
			printk(BIOS_INFO, "%s: Neither %d nor 0 found.\n",
			       __func__, lang_id);
			return NULL;
		}
	}

	offset =  move_next(data, offset, size);
	if (offset >= size)
		return NULL;

	/* Validity check that the returned string must be NULL terminated. */
	next = move_next(data, offset, size) - 1;
	if (next >= size || data[next] != '\0') {
		printk(BIOS_INFO, "%s: %s is not NULL terminated.\n",
		       __func__, PRERAM_LOCALES_NAME);
		return NULL;
	}

	return data + offset;
}
