#include <gtk/gtkoldeditable.h>
#include "gtkcompat.h"

void gtk_old_editable_set_position (GtkOldEditable *editable,
                                           gint position)
{
  GtkOldEditableClass *klass = GTK_OLD_EDITABLE_GET_CLASS (editable);

  klass->set_position (GTK_OLD_EDITABLE (editable), position);
}

gint gtk_old_editable_get_position (GtkOldEditable *editable)
{
  return GTK_OLD_EDITABLE (editable)->current_pos;
}

void gtk_old_editable_copy_clipboard (GtkOldEditable *editable)
{
  GtkOldEditableClass *klass = GTK_OLD_EDITABLE_GET_CLASS (editable);

  klass->copy_clipboard (GTK_OLD_EDITABLE (editable));
}

void gtk_old_editable_select_region (GtkOldEditable *editable,
                                                gint start_pos, gint end_pos)
{
  GtkOldEditableClass *klass = GTK_OLD_EDITABLE_GET_CLASS (editable);

  klass->set_selection (GTK_OLD_EDITABLE (editable), start_pos, end_pos);
}
