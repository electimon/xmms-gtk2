void gtk_old_editable_set_position (GtkOldEditable *editable,
					gint position);

gint gtk_old_editable_get_position (GtkOldEditable *editable);

void gtk_old_editable_copy_clipboard (GtkOldEditable *editable);

void gtk_old_editable_select_region (GtkOldEditable *editable,
					gint start_pos, gint end_pos);
