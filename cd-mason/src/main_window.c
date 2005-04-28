#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "main.h"
#include "menus.h"
#include "tree_view.h"
#include "progress_dialog.h"
#include "options_window.h"

void run_addfile_dialog(gpointer self, gpointer data)
{
	GtkWidget *dialog;
	GtkFileChooser *chooser;
	GSList *filenames;
	
	dialog = gtk_file_chooser_dialog_new (_("Add File(s)"),
				      gui.MainWindow,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT,
				      NULL);
	chooser = GTK_FILE_CHOOSER(dialog);
	gtk_file_chooser_set_select_multiple (chooser, TRUE);
	gtk_file_chooser_set_local_only (chooser, TRUE);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			filenames = gtk_file_chooser_get_filenames (chooser);
			g_slist_foreach(filenames, (GFunc)add_file, NULL);
			g_slist_free(filenames);
		}

	gtk_widget_destroy (dialog);
}

void set_button_image (GtkWidget *menu_item, gpointer type)
{
	GtkWidget *child;
	GtkWidget *old_image;
	GtkWidget *new_image;
	GdkPixbuf *pixbuf;

	current_cd.cd_type = GPOINTER_TO_UINT(type);
	if (current_cd.cd_type == CD_TYPE_AUDIO)
		current_cd.pad = TRUE;
	if (current_cd.cd_type == CD_TYPE_DATA)
		current_cd.write_mode = MODE_TAO;
	current_cd.cd_type_chosen = TRUE; /* this will be unset later if we were
					     called from set_guessed_cd_type */

	old_image = gtk_image_menu_item_get_image (
			GTK_IMAGE_MENU_ITEM(menu_item));
	pixbuf = gtk_image_get_pixbuf (GTK_IMAGE(old_image));
	new_image = gtk_image_new_from_pixbuf (pixbuf);
	child = gtk_bin_get_child (GTK_BIN(gui.cd_type_button));
	if (child)
		gtk_widget_destroy (child);
	gtk_container_add (gui.cd_type_button, new_image);
	gtk_widget_show(new_image);
}
			
static void
type_menu_position (GtkMenu  *menu,
			  gint     *x,
			  gint     *y,
			  gboolean *push_in,
			  gpointer user_data)
{
  GtkWidget *active;
  GtkWidget *child;
  GtkWidget *widget;
  GtkRequisition requisition;
  GList *children;
  GdkScreen *screen;
  gint screen_width, screen_height;
  gint menu_xpos;
  gint menu_ypos;
  gint menu_width, menu_height;

  widget = GTK_WIDGET(user_data);

  gtk_widget_get_child_requisition (GTK_WIDGET (menu), &requisition);
  menu_width = requisition.width;
  menu_height = requisition.height;

  active = gtk_menu_get_active (menu);
  gdk_window_get_origin (widget->window, &menu_xpos, &menu_ypos);

  menu_xpos += widget->allocation.x;
  menu_ypos += widget->allocation.y + widget->allocation.height / 2 - 2;

  if (active != NULL)
    {
      gtk_widget_get_child_requisition (active, &requisition);
      menu_ypos -= requisition.height / 2;
    }

  children = GTK_MENU_SHELL (menu)->children;
  while (children)
    {
      child = children->data;

      if (active == child)
	break;

      if (GTK_WIDGET_VISIBLE (child))
	{
	  gtk_widget_get_child_requisition (child, &requisition);
	  menu_ypos -= requisition.height;
	}

      children = children->next;
    }

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    menu_xpos = menu_xpos + widget->allocation.width - menu_width;

  /* Clamp the position on screen */
  screen = gtk_widget_get_screen(widget);
  screen_width = gdk_screen_get_width (screen);
  
  if (menu_xpos < 0)
    menu_xpos = 0;
  else if ((menu_xpos + menu_width) > screen_width)
    menu_xpos = screen_width - menu_width;

  screen_height = gdk_screen_get_height (screen);
  if (menu_ypos < 0)
	  menu_ypos = 0;
  else if ((menu_ypos + menu_height) > screen_height)
	  menu_ypos = screen_height - menu_height;
			  
  *x = menu_xpos;
  *y = menu_ypos;
  *push_in = TRUE;
}

void display_type_menu (GtkWidget *button, GtkMenuShell *menuShell)
{
	gtk_widget_show_all(GTK_WIDGET(menuShell));
	gtk_menu_popup (GTK_MENU(menuShell), NULL, NULL, (GtkMenuPositionFunc)type_menu_position, button, 0, gtk_get_current_event_time());
	gtk_button_released(GTK_BUTTON(button));
}

GtkWidget *
create_MainWindow (void)
{
	GtkWidget *MainWindowWidget;
	GtkWidget *new_widget;
	GtkBox *vbox, *hbox1, *hbox2;
	GtkToolbar *toolbar;
	GtkToolItem *addfile_button, *remove_button, *clearcdrw_button,
		    *opts_button, *separator;
	GtkWidget *clearcdrw_image;
	GtkWidget *scrolledwindow;
	GtkWidget *type_button, *type_menuitem, *type_image;
	GtkMenuShell *type_menu;
	GtkProgressBar *progressbar;
	GtkWidget *write_cd_button, *wcd_image, *wcd_label;
	GdkPixbuf *wcd_pixbuf;
	gint	wcd_x, wcd_y;
	GtkTooltips *tooltips, *other_tooltips;
	GtkMenu *blank_menu;
	GSList	*blank_group=NULL;

	/* Main Window */
	MainWindowWidget = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gui.MainWindow = GTK_WINDOW(MainWindowWidget);
	gtk_window_set_title (gui.MainWindow, PACKAGE_NAME);
	gtk_window_set_icon_from_file(gui.MainWindow, "cd-mason.svg", NULL);
	gtk_window_set_default_size(gui.MainWindow, 400, 310);
	
	other_tooltips = gtk_tooltips_new();

	new_widget = gtk_vbox_new (FALSE, 0);
	vbox = GTK_BOX(new_widget);
	gtk_container_add (GTK_CONTAINER (MainWindowWidget), new_widget);
	
	/* Menus */
	new_widget = create_menubar();
	gtk_box_pack_start (vbox, new_widget, FALSE, FALSE, 0);

	/* Toolbar */
	tooltips = gtk_tooltips_new ();
	new_widget = gtk_toolbar_new ();
	gtk_widget_grab_focus(new_widget);	
	gtk_box_pack_start (vbox, new_widget, FALSE, FALSE, 0);
	toolbar = GTK_TOOLBAR(new_widget);
	gtk_toolbar_set_tooltips (toolbar, TRUE);
	gtk_toolbar_set_style (toolbar, GTK_TOOLBAR_BOTH);
	
	addfile_button = gtk_tool_button_new_from_stock (GTK_STOCK_ADD);
	gtk_tool_item_set_tooltip (addfile_button, tooltips,
					_("Add file(s) to the CD"),
					_("This will open a file selection dialog which allows you to add files to the CD"));
	gtk_toolbar_insert (toolbar, addfile_button,-1);
	g_signal_connect(addfile_button, "clicked",
					G_CALLBACK(run_addfile_dialog), 
					NULL);
	
	remove_button = gtk_tool_button_new_from_stock (GTK_STOCK_REMOVE);
	gtk_tool_item_set_tooltip (remove_button, tooltips,
			_("Remove selected file(s) from CD"),
			_("To remove items select them in the list then press this button"));
	gtk_toolbar_insert (toolbar, remove_button, -1);
	g_signal_connect(remove_button, "clicked",
			G_CALLBACK(del_file),
			NULL);
	
	opts_button = gtk_tool_button_new_from_stock (GTK_STOCK_PROPERTIES);
	gtk_tool_item_set_tooltip (opts_button, tooltips,
			_("Adjust burning options"),
			_("You can select advanced options for the creation of the CD by pressing this button"));
	gtk_toolbar_insert (toolbar, opts_button, -1);
	g_signal_connect(opts_button, "clicked",
			G_CALLBACK(show_options), NULL);
	
	separator = gtk_separator_tool_item_new();
	gtk_tool_item_set_expand (separator, TRUE);
	gtk_separator_tool_item_set_draw (
			GTK_SEPARATOR_TOOL_ITEM(separator), FALSE);
	gtk_toolbar_insert (toolbar, separator, -1);
	
	blank_menu = GTK_MENU(gtk_menu_new());
	new_widget = gtk_radio_menu_item_new_with_label (blank_group,
			_("Fast blanking"));
	blank_group = gtk_radio_menu_item_get_group (
			GTK_RADIO_MENU_ITEM(new_widget));
	gtk_menu_append (blank_menu, new_widget);
	new_widget = gtk_radio_menu_item_new_with_label (blank_group,
			_("Thorough blanking"));
	blank_group = gtk_radio_menu_item_get_group (
			GTK_RADIO_MENU_ITEM(new_widget));
	gtk_menu_append (blank_menu, new_widget);
	new_widget = gtk_radio_menu_item_new_with_label (blank_group,
			_("Blank last session"));
	blank_group = gtk_radio_menu_item_get_group (
			GTK_RADIO_MENU_ITEM(new_widget));
	gtk_menu_append (blank_menu, new_widget);
	new_widget = gtk_radio_menu_item_new_with_label (blank_group,
			_("Unclose last session"));
	blank_group = gtk_radio_menu_item_get_group (
			GTK_RADIO_MENU_ITEM(new_widget));
	gtk_menu_append (blank_menu, new_widget);
	new_widget = gtk_radio_menu_item_new_with_label (blank_group,
			_("Blank a track"));
	blank_group = gtk_radio_menu_item_get_group (
			GTK_RADIO_MENU_ITEM(new_widget));
	gtk_menu_append (blank_menu, new_widget);
	clearcdrw_image = gtk_image_new_from_file("clear-cdrw.png");
	clearcdrw_button = gtk_menu_tool_button_new (clearcdrw_image,
					_("Clear CD-RW"));
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(clearcdrw_button),
					GTK_WIDGET(blank_menu));
	gtk_widget_show_all(GTK_WIDGET(blank_menu));
	gtk_menu_tool_button_set_arrow_tooltip (
			GTK_MENU_TOOL_BUTTON(clearcdrw_button), tooltips,
			_("Set blanking mode"),
			_("CDs can be blanked in different ways. If you press this arrow, you will able to chose which method should be used."));
	gtk_tool_item_set_tooltip (clearcdrw_button, tooltips,
					_("Clear CD-RW medium"),
					_("This will tell the selected CD drive to erase the medium currently inserted"));
	gtk_toolbar_insert (toolbar, clearcdrw_button,-1);
	g_signal_connect(clearcdrw_button, "clicked", 
			G_CALLBACK(run_progress_dialog), GUINT_TO_POINTER(1));
	
	/* TreeView */
	new_widget = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(new_widget), GTK_SHADOW_OUT);
	gtk_box_pack_start (vbox, new_widget, TRUE, TRUE, 0);
	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER(new_widget), scrolledwindow);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	
	gui.treeview = create_treeview();
	new_widget = GTK_WIDGET(gui.treeview);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), new_widget);
	
	new_widget = gtk_hbox_new (FALSE, 0);
	hbox1 = GTK_BOX(new_widget);
	gtk_box_pack_start (vbox, new_widget, FALSE, TRUE, 0);

	/* Type Button */
	type_button = gtk_button_new();
	gui.cd_type_button = GTK_CONTAINER(type_button);
	gtk_widget_set_size_request (type_button, 32, -1);
	gtk_box_pack_start (hbox1, type_button, FALSE, TRUE, 0);
	gtk_tooltips_set_tip (other_tooltips, type_button,
			_("Type of CD to create"),
			_("You can set the CD type using this button, otherwise the computer will try to determine the type automatically."));

	type_menu = GTK_MENU_SHELL(gtk_menu_new());
	type_menuitem = gtk_image_menu_item_new_with_label(_("Data"));
	type_image = gtk_image_new_from_file ("type_data.png");
	gtk_image_menu_item_set_image (
			GTK_IMAGE_MENU_ITEM(type_menuitem), type_image);
	g_signal_connect(type_menuitem, "activate", 
			G_CALLBACK(set_button_image), 
			GUINT_TO_POINTER(CD_TYPE_DATA));
	gtk_menu_shell_append (type_menu, type_menuitem);
	gui.data_type_item = GTK_MENU_ITEM(type_menuitem);
	
	type_menuitem = gtk_image_menu_item_new_with_label(_("Audio"));
	type_image = gtk_image_new_from_file ("type_audio.png");
	gtk_image_menu_item_set_image (
			GTK_IMAGE_MENU_ITEM(type_menuitem), type_image);
	g_signal_connect(type_menuitem, "activate", 
			G_CALLBACK(set_button_image), 
			GUINT_TO_POINTER(CD_TYPE_AUDIO));
	gtk_menu_shell_append (type_menu, type_menuitem);
	gui.audio_type_item = GTK_MENU_ITEM(type_menuitem);

	type_menuitem = gtk_image_menu_item_new_with_label(_("CD Image"));
	type_image = gtk_image_new_from_file ("type_iso.png");
	gtk_image_menu_item_set_image (
			GTK_IMAGE_MENU_ITEM(type_menuitem), type_image);
	g_signal_connect(type_menuitem, "activate", 
			G_CALLBACK(set_button_image),
			GUINT_TO_POINTER(CD_TYPE_ISO));
	gtk_menu_shell_append (type_menu, type_menuitem);
	gui.iso_type_item = GTK_MENU_ITEM(type_menuitem);
	
	g_signal_connect(type_button, "pressed",
					G_CALLBACK(display_type_menu), 
					type_menu);
	g_signal_connect(type_button, "clicked",
					G_CALLBACK(display_type_menu), 
					type_menu);

	/* Progress Bar */
	new_widget = gtk_progress_bar_new ();
	gtk_box_pack_start (hbox1, new_widget, TRUE, TRUE, 0);
	progressbar = GTK_PROGRESS_BAR (new_widget);
	gtk_progress_bar_set_text (progressbar, _("Empty"));
	gui.size_bar = progressbar;

	/* Write CD */
	write_cd_button = gtk_button_new ();
	gtk_tooltips_set_tip (other_tooltips, write_cd_button,
			_("Start writing the CD"),
			_("This button will open a dialog which will let you set further options for writing the files to a CD."));
	gtk_box_pack_start (hbox1, write_cd_button, FALSE, TRUE, 0);
	g_signal_connect(write_cd_button, "clicked",
					G_CALLBACK(run_progress_dialog), 
					NULL);

	new_widget = gtk_hbox_new (FALSE, 2);
	hbox2 = GTK_BOX(new_widget);
	gtk_container_add (GTK_CONTAINER (write_cd_button), new_widget); 
	gtk_icon_size_lookup (GTK_ICON_SIZE_BUTTON, &wcd_x, &wcd_y);
	wcd_pixbuf = gdk_pixbuf_new_from_file_at_size (
			"cd-mason.svg", wcd_x, wcd_y, NULL);
	wcd_image = gtk_image_new_from_pixbuf(wcd_pixbuf);
	
	gtk_box_pack_start (hbox2, wcd_image, FALSE, FALSE, 0);
	
	wcd_label = gtk_label_new (_("Write CD"));
	gtk_box_pack_start (hbox2, wcd_label, FALSE, FALSE, 0);

	gtk_window_add_accel_group (gui.MainWindow, accel_group);

	g_signal_connect (MainWindowWidget, "destroy",
		    G_CALLBACK (quit_application), NULL);

	return MainWindowWidget;
}
