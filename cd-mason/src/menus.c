#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "preferences.h"
#include "main.h"

GtkWidget *
create_menubar (void)
{
	GtkWidget *menubar;
	GtkWidget *file_menuitem;
	GtkWidget *file_menu;
	GtkWidget *m_file_new;
	GtkWidget *m_file_open;
	GtkWidget *m_file_save;
	GtkWidget *m_file_saveas;
	GtkWidget *m_file_quit;
	GtkWidget *edit_menuitem;
	GtkWidget *edit_menu;
	GtkWidget *m_edit_cut;
	GtkWidget *m_edit_copy;
	GtkWidget *m_edit_paste;
	GtkWidget *m_edit_delete;
	GtkWidget *m_edit_prefs;
	GtkWidget *view_menuitem;
	GtkWidget *view_menu;
	GtkWidget *help_menuitem;
	GtkWidget *help_menu;
	GtkWidget *m_help_about;
	GtkContainer *container;
	
		menubar = gtk_menu_bar_new ();

		file_menuitem = gtk_menu_item_new_with_mnemonic (_("_File"));
		gtk_container_add (GTK_CONTAINER (menubar), file_menuitem);

		file_menu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menuitem),
					   file_menu);
		container = GTK_CONTAINER (file_menu);
		m_file_new =
			gtk_image_menu_item_new_from_stock ("gtk-new",
							    accel_group);
		gtk_container_add (container, m_file_new);

		m_file_open =
			gtk_image_menu_item_new_from_stock ("gtk-open",
							    accel_group);
		gtk_container_add (container, m_file_open);

		m_file_save =
			gtk_image_menu_item_new_from_stock ("gtk-save",
							    accel_group);
		gtk_container_add (container, m_file_save);

		m_file_saveas =
			gtk_image_menu_item_new_from_stock ("gtk-save-as",
							    accel_group);
		gtk_container_add (container, m_file_saveas);

		gtk_container_add (container,
				   gtk_separator_menu_item_new());
		m_file_quit =
			gtk_image_menu_item_new_from_stock ("gtk-quit",
							    accel_group);
		gtk_container_add (container, m_file_quit);
		
		g_signal_connect (m_file_quit, "activate",
		    G_CALLBACK (quit_application), NULL);

		edit_menuitem = gtk_menu_item_new_with_mnemonic (_("_Edit"));
		gtk_container_add (GTK_CONTAINER (menubar), edit_menuitem);

		edit_menu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (edit_menuitem),
					   edit_menu);
		container = GTK_CONTAINER(edit_menu);

		m_edit_cut =
			gtk_image_menu_item_new_from_stock ("gtk-cut",
							    accel_group);
		gtk_container_add (container, m_edit_cut);

		m_edit_copy =
			gtk_image_menu_item_new_from_stock ("gtk-copy",
							    accel_group);
		gtk_container_add (container, m_edit_copy);

		m_edit_paste =
			gtk_image_menu_item_new_from_stock ("gtk-paste",
							    accel_group);
		gtk_container_add (container, m_edit_paste);

		m_edit_delete =
			gtk_image_menu_item_new_from_stock ("gtk-delete",
							    accel_group);
		gtk_container_add (container, m_edit_delete);

		gtk_container_add (container,
				gtk_separator_menu_item_new());
		m_edit_prefs =
			gtk_image_menu_item_new_from_stock ("gtk-preferences",
					accel_group);
		gtk_container_add (container, m_edit_prefs);
		
		g_signal_connect (m_edit_prefs, "activate",
		    G_CALLBACK (show_prefs), gui.MainWindow);

		view_menuitem = gtk_menu_item_new_with_mnemonic (_("_View"));
		gtk_container_add (GTK_CONTAINER (menubar), view_menuitem);

		view_menu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (view_menuitem),
					   view_menu);

		help_menuitem = gtk_menu_item_new_with_mnemonic (_("_Help"));
		gtk_container_add (GTK_CONTAINER (menubar), help_menuitem);

		help_menu = gtk_menu_new ();
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_menuitem),
					   help_menu);

		m_help_about = gtk_image_menu_item_new_from_stock ("gtk-about",
				accel_group);
		gtk_container_add (GTK_CONTAINER (help_menu), m_help_about);
		
		g_signal_connect (m_help_about, "activate",
		    G_CALLBACK (run_about_dialog), NULL);
	return menubar;
}
