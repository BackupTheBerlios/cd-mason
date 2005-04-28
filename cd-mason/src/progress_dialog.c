#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h> /*for kill*/
#else
#  warning "Couldn't find the header file for kill()"
#endif
#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  warning "Couldn't find the header file for strcmp()"
#endif
#include <signal.h> /*for kill*/
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include "main.h"
#include "cdrecord_iface.h"

void create_progress_dialog (guint blank_mode)
{
	GtkWidget *new_widget, *b_progress, *status, *expander, *alignment,
		  *scrolled, *speed;
	GtkBox	*vbox, *hbox;
	GList	*children;

	new_widget = gtk_dialog_new_with_buttons (_("Burning Progress"), 
			gui.MainWindow,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gui.progress_dialog = GTK_DIALOG(new_widget);
	children = GTK_BOX(gui.progress_dialog->action_area)->children;
	gtk_widget_grab_focus(((GtkBoxChild *)(children->data))->widget);
	alignment = gtk_alignment_new(.5,0,1,1);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 6, 0, 6, 6);
	gtk_widget_show(alignment);
	gtk_container_add(GTK_CONTAINER(gui.progress_dialog->vbox), alignment);
	new_widget = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(new_widget);
	gtk_container_add(GTK_CONTAINER(alignment), new_widget);
	vbox = GTK_BOX(new_widget);
	
	status = gtk_label_new ("");
	gtk_box_pack_start(vbox, status, FALSE, FALSE, 6);
	gtk_widget_show(status);
	gui.p_status = GTK_LABEL(status);
	gtk_label_set_line_wrap (gui.p_status, TRUE);

	if (!blank_mode)
	{
		b_progress = gtk_progress_bar_new();
		gtk_box_pack_start(vbox, b_progress, FALSE, FALSE, 6);
		gtk_widget_show(b_progress);
		gui.p_bar = GTK_PROGRESS_BAR(b_progress);

		new_widget = gtk_hbox_new(FALSE, 3);
		gtk_box_pack_start(vbox, new_widget, FALSE, FALSE, 0);
		gtk_widget_show(new_widget);
		hbox = GTK_BOX(new_widget);

		new_widget = gtk_progress_bar_new();
		gui.p_buf = GTK_PROGRESS_BAR(new_widget);
		gtk_progress_bar_set_fraction(gui.p_buf, 1);
		gtk_progress_bar_set_text(gui.p_buf, _("Drive Buffer"));
		gtk_widget_set_size_request(new_widget, -1, 18);
		gtk_box_pack_start(hbox, new_widget, TRUE, TRUE, 0);
		gtk_widget_show(new_widget);

		new_widget = gtk_progress_bar_new();
		gui.p_fifo = GTK_PROGRESS_BAR(new_widget);
		gtk_progress_bar_set_fraction(gui.p_fifo, 1);
		gtk_progress_bar_set_text(gui.p_fifo, _("FIFO Buffer"));
		gtk_widget_set_size_request(new_widget, -1, 18);
		gtk_box_pack_start(hbox, new_widget, TRUE, TRUE, 0);
		gtk_widget_show(new_widget);

		new_widget = gtk_frame_new(NULL);
		gtk_box_pack_start(hbox, new_widget, FALSE, FALSE, 0);
		gtk_widget_show(new_widget);
		speed = gtk_label_new("0.0x");
		gui.p_speed = GTK_LABEL(speed);
		gtk_label_set_width_chars(gui.p_speed, 5);
		gtk_container_add(GTK_CONTAINER(new_widget), speed);
		gtk_widget_show(speed);
	}
	
	/*Text Area*/
	gui.log_buf = gtk_text_buffer_new(NULL);
	gtk_text_buffer_get_start_iter(gui.log_buf, &gui.end_iter);
	
	expander = gtk_expander_new(_("Details"));
	g_signal_connect(expander, "activate", 
			G_CALLBACK(remember_window_size), 
			GTK_WINDOW(gui.progress_dialog));
	gtk_box_pack_end(vbox, expander, TRUE, TRUE, 3);
	gtk_widget_show(expander);

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(expander), scrolled);
	gtk_widget_show(scrolled);
	
	new_widget = gtk_text_view_new_with_buffer(gui.log_buf);
	gui.t_view = GTK_TEXT_VIEW(new_widget);
	gtk_text_view_set_wrap_mode(gui.t_view, GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(gui.t_view, FALSE);
	gtk_text_view_set_cursor_visible(gui.t_view, FALSE);
	gtk_container_add (GTK_CONTAINER(scrolled), new_widget);
	gtk_widget_show(new_widget);
}

void error_dialog(const gchar *format, const gchar *secondary)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (gui.MainWindow,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			format);
	if (secondary)
		gtk_message_dialog_format_secondary_text(
				GTK_MESSAGE_DIALOG(dialog), secondary);
	
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

gboolean validate_data_cd()
{
	GtkTreeIter iter;
	gint num = 0;
	guchar vfs_type;

	gtk_tree_model_get_iter_first( current_cd.tree.model, &iter);
	do {
		num++;
		gtk_tree_model_get (current_cd.tree.model, &iter,
				VFS_TYPE_COL, &vfs_type, -1 );
		if ((vfs_type != GNOME_VFS_FILE_TYPE_REGULAR)
				&& (vfs_type != GNOME_VFS_FILE_TYPE_DIRECTORY))
		{
			error_dialog(_("Unsupported file type"), _("Sorry, at the moment I can only create data CDs consisting exclusively of flat files and directories (no links or other funky stuff). If you want to help out, patches are always welcome ;-)"));
			return FALSE;
		}
	} while (gtk_tree_model_iter_next (current_cd.tree.model, &iter));
	if (num > 250)
	{
		error_dialog (_("Too many files"), _("Currently you are limited to a total of 250 files on a CD. Sorry for the inconvenience."));
		return FALSE;
	}
	return TRUE;
}

gboolean validate_audio_cd()
{
	GtkTreeIter iter;
	gint num = 0;
	gchar *mime_type, *filename;

	gtk_tree_model_get_iter_first( current_cd.tree.model, &iter);
	do {
		num ++;
		gtk_tree_model_get (current_cd.tree.model, &iter,
				DESTNAME_COL, &filename,
				MIME_TYPE_COL, &mime_type,
				-1);
		if (strcmp(mime_type, "audio/x-wav") != 0)
		{
			error_dialog(_("Unhandled audio type"),_("At the moment CD-Mason can only write WAV-files on an audio CD."));
			return FALSE;
		}
	} while (gtk_tree_model_iter_next (current_cd.tree.model, &iter));
	if (num > 99)
	{
		error_dialog (_("Too many tracks"),
			_("An audio CD can have no more than 99 tracks."));
		return FALSE;
	}
	return TRUE;
}

gboolean validate_iso_cd()
{
	GtkTreeIter iter;
	gchar *mime_type;
	
	if (gtk_tree_model_iter_n_children (current_cd.tree.model, NULL) > 1)
	{
		error_dialog (_("Too many files"),_("To burn an ISO-Image please add only one file to this CD."));
		return FALSE;
	}
	gtk_tree_model_get_iter_first(current_cd.tree.model, &iter);
	gtk_tree_model_get(current_cd.tree.model, &iter,
			MIME_TYPE_COL, &mime_type, -1);
	if (strcmp(mime_type, "application/x-cd-image") != 0)
	{
		error_dialog (_("Invalid file type"),
			_("The file added to this CD is not an ISO-image."));
		return FALSE;
	}
	return TRUE;
}

gboolean cd_is_valid()
{
	GtkTreeIter iter;
	
	if (!gtk_tree_model_get_iter_first(current_cd.tree.model, &iter))
	{
		error_dialog(_("The current CD Project is empty."),_("To add files drag them into the main window or press the \"Add\" button."));
	return FALSE;
	}
	switch (current_cd.cd_type) {
	case CD_TYPE_DATA:
		return validate_data_cd();
	case CD_TYPE_AUDIO:
		return validate_audio_cd();
	case CD_TYPE_ISO:
		return validate_iso_cd();
	}
	return TRUE;
}

void run_progress_dialog(GtkWidget *button, gpointer data)
{
	gint result, blank_mode, really;
	cdr_handle *cdr = &(current_cd.cdr);

	blank_mode = 0;

	if(GPOINTER_TO_UINT(data) == 1)
	{
		GtkWidget *menu, *active;
		GSList *blank_list, *element;
		
		menu = gtk_menu_tool_button_get_menu(
				GTK_MENU_TOOL_BUTTON(button));
		active = gtk_menu_get_active (GTK_MENU(menu));
		blank_list = gtk_radio_menu_item_get_group (
				GTK_RADIO_MENU_ITEM(active));
		element = g_slist_find (blank_list, active);
		blank_mode = g_slist_position (blank_list, element) + 1;
		g_message("Blank mode %d", blank_mode);
		goto cont;
	}	
	if (cd_is_valid())
	{
cont:
		create_progress_dialog(blank_mode);
		spawn_cdrecord(blank_mode);
		do
		{
			really = GTK_RESPONSE_YES;
			result = gtk_dialog_run(gui.progress_dialog);
			if (current_cd.burn_in_progress && (!current_cd.dummy))
			{
				GtkWidget *dia2;
				
				dia2 = gtk_message_dialog_new (
						GTK_WINDOW(gui.progress_dialog),
						GTK_DIALOG_MODAL,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_YES_NO,
						_("Really abort burning?"));
				gtk_message_dialog_format_secondary_text (
						GTK_MESSAGE_DIALOG(dia2),
						_("If you abort a burning operation, the medium will be destroyed. Are you sure you want to do that?"));
				really = gtk_dialog_run (GTK_DIALOG(dia2));
				gtk_widget_destroy(dia2);
			}
		} while (really != GTK_RESPONSE_YES);
		
		/*FIXME: cdr ist beendet und dann wird das Fenster geschlossen*/
		if (GTK_RESPONSE_CANCEL == result || 
			GTK_RESPONSE_DELETE_EVENT == result) 
		{
			current_cd.burn_in_progress = FALSE;
			current_cd.already_written = 0;
			kill (cdr->pid, SIGTERM);
			if (cdr->iso_pid)
				kill (cdr->iso_pid, SIGTERM);
			g_io_channel_shutdown (cdr->err, FALSE, NULL);
			g_io_channel_shutdown (cdr->out, FALSE, NULL);
			g_source_remove(cdr->out_source);
			g_source_remove(cdr->err_source);
		};
		gtk_widget_destroy((GtkWidget*)gui.progress_dialog);
	}
}
