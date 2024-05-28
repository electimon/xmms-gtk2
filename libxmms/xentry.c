/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
 * Small modification of the entry widget where keyboard navigation
 * works even when the entry is not editable.
 * Copyright 2003 Haavard Kvaalen <havardk@xmms.org>
 */

#include <ctype.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdki18n.h>
#include <gtk/gtkmain.h>
#include "xentry.h"

static gint gtk_entry_key_press           (GtkWidget *widget,
					   GdkEventKey *event);
static void gtk_entry_move_cursor         (GtkEditable *editable, int x);

static void gtk_move_forward_character    (GtkEntry *entry);
static void gtk_move_backward_character   (GtkEntry *entry);
static void gtk_move_forward_word         (GtkEntry *entry);
static void gtk_move_backward_word        (GtkEntry *entry);
static void gtk_move_beginning_of_line    (GtkEntry *entry);
static void gtk_move_end_of_line          (GtkEntry *entry);


static const XmmsGtkTextFunction control_keys[26] = {
	(XmmsGtkTextFunction)gtk_move_beginning_of_line,    /* a */
	(XmmsGtkTextFunction)gtk_move_backward_character,   /* b */
	(XmmsGtkTextFunction)gtk_editable_copy_clipboard,   /* c */
	NULL,                                           /* d */
	(XmmsGtkTextFunction)gtk_move_end_of_line,          /* e */
	(XmmsGtkTextFunction)gtk_move_forward_character,    /* f */
};

static const XmmsGtkTextFunction alt_keys[26] = {
	NULL,                                           /* a */
	(XmmsGtkTextFunction)gtk_move_backward_word,        /* b */
	NULL,                                           /* c */
	NULL,                                           /* d */
	NULL,                                           /* e */
	(XmmsGtkTextFunction)gtk_move_forward_word,         /* f */
};


static void xmms_entry_class_init(GtkEntryClass *class)
{
	GtkWidgetClass *widget_class = (GtkWidgetClass*) class;

	widget_class->key_press_event = gtk_entry_key_press;
}

GtkType xmms_entry_get_type(void)
{
	static GtkType entry_type = 0;

	if (!entry_type)
	{
		static const GtkTypeInfo entry_info = {
			"XmmsEntry",
			sizeof (XmmsEntry),
			sizeof (XmmsEntryClass),
			(GtkClassInitFunc) xmms_entry_class_init,
			NULL,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		entry_type = gtk_type_unique(GTK_TYPE_ENTRY, &entry_info);
	}

	return entry_type;
}

GtkWidget* xmms_entry_new(void)
{
	return GTK_WIDGET(gtk_type_new(XMMS_TYPE_ENTRY));
}

static int gtk_entry_key_press(GtkWidget *widget, GdkEventKey *event)
{
	GtkEntry *entry;
	GtkEditable *editable;

	int return_val;
	guint initial_pos, sel_start_pos, sel_end_pos;
	gboolean has_selection;
	int extend_selection;
	gboolean extend_start = FALSE;

	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(XMMS_IS_ENTRY(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	entry = GTK_ENTRY(widget);
	editable = GTK_EDITABLE(widget);
	return_val = FALSE;

	if (gtk_editable_get_editable(editable))
		/* Let the regular entry handler do it */
		return FALSE;

	initial_pos = gtk_editable_get_position(editable);

	extend_selection = event->state & GDK_SHIFT_MASK;

	has_selection = gtk_editable_get_selection_bounds(editable, &sel_start_pos, &sel_end_pos);
	/* TODO what to do when there is no selection? */

	if (extend_selection)
	{
		if (sel_start_pos == sel_end_pos)
		{
			sel_start_pos = initial_pos;
			sel_end_pos = initial_pos;
		}

		extend_start = (initial_pos == sel_start_pos);
	}

	switch (event->keyval)
	{
		case GDK_Insert:
			return_val = TRUE;
			if (event->state & GDK_CONTROL_MASK)
				gtk_editable_copy_clipboard(editable);
			break;
		case GDK_Home:
			return_val = TRUE;
			gtk_move_beginning_of_line(entry);
			break;
		case GDK_End:
			return_val = TRUE;
			gtk_move_end_of_line(entry);
			break;
		case GDK_Left:
			return_val = TRUE;
			if (!extend_selection && sel_start_pos != sel_end_pos)
			{
				gtk_editable_set_position(editable,
							  MIN(sel_start_pos,
							      sel_end_pos));
				/* Force redraw below */
				initial_pos = -1;
			}
			else
				gtk_move_backward_character(entry);
			break;
		case GDK_Right:
			return_val = TRUE;
			if (!extend_selection && sel_start_pos != sel_end_pos)
			{
				gtk_editable_set_position(editable,
							  MAX(sel_start_pos,
							      sel_end_pos));
				/* Force redraw below */
				initial_pos = -1;
			}
			else
				gtk_move_forward_character(entry);
			break;
		case GDK_Return:
			return_val = TRUE;
			gtk_widget_activate(widget);
			break;
		default:
			if ((event->keyval >= 0x20) && (event->keyval <= 0xFF))
			{
				int key = event->keyval;

				if (key >= 'A' && key <= 'Z')
					key -= 'A' - 'a';

				if (key >= 'a' && key <= 'z')
					key -= 'a';
				else
					break;

				if (event->state & GDK_CONTROL_MASK)
				{
					if (control_keys[key])
					{
						(*control_keys[key])(editable,
								     event->time);
						return_val = TRUE;
					}
					break;
				}
				else if (event->state & GDK_MOD1_MASK)
				{
					if (alt_keys[key])
					{
						(*alt_keys[key])(editable,
								 event->time);
						return_val = TRUE;
					}
					break;
				}
			}
	}

	if (return_val && (gtk_editable_get_position(editable) != initial_pos))
	{
		if (extend_selection)
		{
			int cpos = gtk_editable_get_position(editable);
			if (cpos < sel_start_pos)
				sel_start_pos = cpos;
			else if (cpos > sel_end_pos)
				sel_end_pos = cpos;
			else
			{
				if (extend_start)
					sel_start_pos = cpos;
				else
					sel_end_pos = cpos;
			}
		}
		else
		{
			sel_start_pos = 0;
			sel_end_pos = 0;
		}

		gtk_editable_select_region(editable, sel_start_pos, sel_end_pos);
	}

	return return_val;
}

static void gtk_entry_move_cursor (GtkEditable *editable, int x)
{
	int set, pos = gtk_editable_get_position(editable);
	if (pos + x < 0)
		set = 0;
	else
		set = pos + x;
	gtk_editable_set_position(editable, set);
}

static void gtk_move_forward_character(GtkEntry *entry)
{
	gtk_entry_move_cursor(GTK_EDITABLE(entry), 1);
}

static void gtk_move_backward_character (GtkEntry *entry)
{
	gtk_entry_move_cursor(GTK_EDITABLE(entry), -1);
}

static void gtk_move_forward_word (GtkEntry *entry)
{
	GtkEditable *editable;
	gchar *text;
	int i;

	editable = GTK_EDITABLE (entry);

	/* Prevent any leak of information */
	if (!gtk_entry_get_visibility(entry))
	{
		gtk_editable_set_position(GTK_EDITABLE(entry), -1);
		return;
	}

	if (entry->text && (gtk_editable_get_position(editable) < entry->text_length))
	{
		text = entry->text;
		i = gtk_editable_get_position(editable);

		if (
		    !isalnum(text[i]))
			for (; i < entry->text_length; i++)
			{
				if (isalnum(text[i]))
					break;
			}

		for (; i < entry->text_length; i++)
		{
			if (isalnum(text[i]))
				break;
		}

		gtk_editable_set_position(GTK_EDITABLE(entry), i);
	}
}

static void gtk_move_backward_word(GtkEntry *entry)
{
	GtkEditable *editable;
	gchar *text;
	int i;

	editable = GTK_EDITABLE (entry);

	/* Prevent any leak of information */
	if (!gtk_entry_get_visibility(entry))
	{
		gtk_editable_set_position(GTK_EDITABLE(entry), 0);
		return;
	}

	if (entry->text && gtk_editable_get_position(editable) > 0)
	{
		text = entry->text;
		i = gtk_editable_get_position(editable);

		if (
		    !isalnum(text[i]))
			for (; i >= 0; i--)
			{
				if (gdk_iswalnum(text[i]))
					break;
			}
		for (; i >= 0; i--)
		{
			if (
			    !isalnum(text[i]))
			{
				i++;
				break;
			}
		}

		if (i < 0)
			i = 0;

		gtk_editable_set_position(GTK_EDITABLE(entry), i);
	}
}

static void gtk_move_beginning_of_line (GtkEntry *entry)
{
	gtk_editable_set_position(GTK_EDITABLE(entry), 0);
}

static void gtk_move_end_of_line (GtkEntry *entry)
{
	gtk_editable_set_position(GTK_EDITABLE(entry), -1);
}

