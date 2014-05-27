/*
 * cell.c - functions for table handling at the cell level
 *
 * Copyright (C) 2014 Ondrej Oprala <ooprala@redhat.com>
 * Copyright (C) 2014 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

/**
 * SECTION: cell
 * @title: Cell
 * @short_description: cell API
 *
 * An API to access and modify per-cell data and information. Note that cell is
 * always part of the line. If you destroy (un-reference) a line than it
 * destroys all line cells too.
 */


#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "smartcolsP.h"

/*
 * The cell has no ref-counting, free() and new() functions. All is
 * handled by libscols_line.
 */

/**
 * scols_reset_cell:
 * @ce: pointer to a struct libscols_cell instance
 *
 * Frees the cell's internal data and resets its status.
 *
 * Returns: 0, a negative value in case of an error.
 */
int scols_reset_cell(struct libscols_cell *ce)
{
	assert(ce);

	if (!ce)
		return -EINVAL;

	free(ce->data);
	free(ce->color);
	memset(ce, 0, sizeof(*ce));
	return 0;
}

/**
 * scols_cell_set_data:
 * @ce: a pointer to a struct libscols_cell instance
 * @str: data (used for scols_printtable())
 *
 * Stores a copy of the @str in @ce.
 *
 * Returns: 0, a negative value in case of an error.
 */
int scols_cell_set_data(struct libscols_cell *ce, const char *str)
{
	char *p = NULL;

	assert(ce);

	if (!ce)
		return -EINVAL;
	if (str) {
		p = strdup(str);
		if (!p)
			return -ENOMEM;
	}
	free(ce->data);
	ce->data = p;
	return 0;
}

/**
 * scols_cell_refer_data:
 * @ce: a pointer to a struct libscols_cell instance
 * @str: data (used for scols_printtable())
 *
 * Adds a reference to @str to @ce. The pointer is deallocated by
 * scols_reset_cell() or scols_unref_line(). This function is mostly designed
 * for situations when the data for the cell are already composed in allocated
 * memory (e.g. asprintf()) to avoid extra unnecessary strdup().
 *
 * Returns: 0, a negative value in case of an error.
 */
int scols_cell_refer_data(struct libscols_cell *ce, char *str)
{
	assert(ce);

	if (!ce)
		return -EINVAL;
	free(ce->data);
	ce->data = str;
	return 0;
}

/**
 * scols_cell_get_data:
 * @ce: a pointer to a struct libscols_cell instance
 *
 * Returns: data in @ce or NULL.
 */
const char *scols_cell_get_data(const struct libscols_cell *ce)
{
	assert(ce);
	return ce ? ce->data : NULL;
}

/**
 * scols_cell_set_userdata:
 * @ce: a pointer to a struct libscols_cell instance
 * @data: private user data
 *
 * Returns: 0, a negative value in case of an error.
 */
int scols_cell_set_userdata(struct libscols_cell *ce, void *data)
{
	assert(ce);

	if (!ce)
		return -EINVAL;
	ce->userdata = data;
	return 0;
}

/**
 * scols_cell_get_userdata
 *
 * @ce: a pointer to a struct libscols_cell instance
 *
 * Returns: user data
 */
void *scols_cell_get_userdata(struct libscols_cell *ce)
{
	return ce ? ce->userdata : NULL;
}

/**
 * scols_cmpstr_cells:
 * @a: pointer to cell
 * @b: pointer to cell
 *
 * Compares cells data by strcmp(). The function is designed for
 * scols_column_set_cmpfunc() and scols_sort_table().
 *
 * Returns: follows strcmp() return values.
 */
int scols_cmpstr_cells(struct libscols_cell *a,
		       struct libscols_cell *b,
		       __attribute__((__unused__)) void *data)
{
	const char *adata, *bdata;

	if (a == b)
		return 0;

	adata = scols_cell_get_data(a);
	bdata = scols_cell_get_data(b);

	if (adata == NULL && bdata == NULL)
		return 0;
	if (adata == NULL)
		return -1;
	if (bdata == NULL)
		return 1;
	return strcmp(adata, bdata);
}

/**
 * scols_cell_set_color:
 * @ce: a pointer to a struct libscols_cell instance
 * @color: color name or ESC sequence
 *
 * Set the color of @ce to @color.
 *
 * Returns: 0, a negative value in case of an error.
 */
int scols_cell_set_color(struct libscols_cell *ce, const char *color)
{
	char *p = NULL;

	assert(ce);

	if (!ce)
		return -EINVAL;
	if (color) {
		if (isalpha(*color)) {
			color = color_sequence_from_colorname(color);

			if (!color)
				return -EINVAL;
		}
		p = strdup(color);
		if (!p)
			return -ENOMEM;
	}
	free(ce->color);
	ce->color = p;
	return 0;
}

/**
 * scols_cell_get_color:
 * @ce: a pointer to a struct libscols_cell instance
 *
 * Returns: the current color of @ce.
 */
const char *scols_cell_get_color(const struct libscols_cell *ce)
{
	assert(ce);
	return ce ? ce->color : NULL;
}

/**
 * scols_cell_copy_content:
 * @dest: a pointer to a struct libscols_cell instance
 * @src: a pointer to an immutable struct libscols_cell instance
 *
 * Copy the contents of @src into @dest.
 *
 * Returns: 0, a negative value in case of an error.
 */
int scols_cell_copy_content(struct libscols_cell *dest,
			    const struct libscols_cell *src)
{
	int rc;

	assert(dest);
	assert(src);

	rc = scols_cell_set_data(dest, scols_cell_get_data(src));
	if (!rc)
		rc = scols_cell_set_color(dest, scols_cell_get_color(src));
	if (!rc)
		dest->userdata = src->userdata;
	return rc;
}