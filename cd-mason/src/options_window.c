/***************************************************************************
 *            options_window.c
 *            
 *  Wed 2 Mar 00:23:13 2005
 *  Copyright  2005  Felix Braun
 *  hazzl@gmx.net
 ****************************************************************************/
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib/gi18n.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "preferences.h"
#include "main.h"

void toggle_multisession(GtkToggleButton *button, GtkComboBox *combo)
{
	if (gtk_toggle_button_get_active(button))
	{
		/* set writing mode to TAO because it is required by
		 * multisession, then lock the selector
		 */
		gtk_combo_box_set_active (combo, MODE_TAO);
		gtk_widget_set_sensitive (GTK_WIDGET(combo), FALSE);
	}
	else
	{
		gtk_widget_set_sensitive (GTK_WIDGET(combo), TRUE);
	}
}

void show_options()
{
	GtkWidget	*new_widget, *container, *nb_widget, *label; 
	GtkToggleButton	*burnfree, *overburn, *multisession, *dummy,
			*pad, *immed, *swab, *forcespeed;
	GtkNotebook	*notebook;
	GtkDialog	*dialog;
	GtkBox		*vbox, *hbox;
	GtkComboBox	*mode_box, *isofs_box;
	gint		result;
	
	new_widget = gtk_dialog_new_with_buttons (_("Options"),
			gui.MainWindow,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_PREFERENCES, GTK_RESPONSE_HELP,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
                        NULL);
	dialog = GTK_DIALOG(new_widget);
	
	container = gtk_alignment_new(.5, 0.0, 1.0, 0.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(container), 0, 0, 6, 6);
	gtk_widget_show(container);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), container, TRUE, TRUE, 6);
	nb_widget = gtk_notebook_new();
	notebook = GTK_NOTEBOOK(nb_widget);
	gtk_container_add(GTK_CONTAINER(container), nb_widget);
	
	label = gtk_label_new(_("Writing Mode"));
	new_widget = gtk_vbox_new(FALSE,3);
	gtk_widget_show(new_widget);
	vbox = GTK_BOX(new_widget);
	gtk_notebook_append_page(notebook, new_widget, label);

	new_widget = gtk_check_button_new_with_mnemonic(_("_Burnfree"));
	burnfree = GTK_TOGGLE_BUTTON (new_widget);
	gtk_toggle_button_set_active(burnfree, current_cd.burnfree);
	gtk_box_pack_start(vbox, new_widget, FALSE, FALSE, 3);
	
	new_widget = gtk_check_button_new_with_mnemonic(_("O_verburn"));
	overburn = GTK_TOGGLE_BUTTON (new_widget);
	gtk_toggle_button_set_active(overburn, current_cd.overburn);
	gtk_box_pack_start(vbox, new_widget, FALSE, FALSE, 3);
	
	new_widget = gtk_check_button_new_with_mnemonic(_("_Multisession"));
	multisession = GTK_TOGGLE_BUTTON(new_widget);
	gtk_toggle_button_set_active(multisession, current_cd.multisession);
	gtk_box_pack_start(vbox, new_widget, FALSE, FALSE, 3);
	
	new_widget = gtk_check_button_new_with_mnemonic(_("_Dummy write"));
	dummy = GTK_TOGGLE_BUTTON(new_widget);
	gtk_toggle_button_set_active(dummy, current_cd.dummy);
	gtk_box_pack_start(vbox, new_widget, FALSE, FALSE, 3);
	
	container = gtk_expander_new(_("Advanced Options"));
	gtk_box_pack_start(vbox, container, FALSE, FALSE, 3);
	g_signal_connect(container, "activate", 
			G_CALLBACK(remember_window_size), 
			GTK_WINDOW(dialog));
	new_widget = gtk_hbox_new (TRUE, 6 );
	hbox = GTK_BOX(new_widget);
	gtk_container_add(GTK_CONTAINER(container), new_widget);
	new_widget = gtk_vbox_new(FALSE,3);
	gtk_box_pack_start (hbox, new_widget, FALSE, FALSE, 6);
	vbox = GTK_BOX(new_widget);
	
	new_widget = gtk_check_button_new_with_mnemonic(_("Add _padding"));
	pad = GTK_TOGGLE_BUTTON(new_widget);
	gtk_toggle_button_set_active(pad, current_cd.pad);
	gtk_box_pack_start(vbox, new_widget, FALSE, FALSE, 3);
	
	new_widget = gtk_check_button_new_with_mnemonic(_("Add SCSI _Immed"));
	immed = GTK_TOGGLE_BUTTON(new_widget);
	gtk_toggle_button_set_active(immed, current_cd.immed);
	gtk_box_pack_start(vbox, new_widget, FALSE, FALSE, 3);
	
	new_widget = gtk_check_button_new_with_mnemonic(
			_("Swap audio byte order"));
	swab = GTK_TOGGLE_BUTTON(new_widget);
	gtk_toggle_button_set_active(swab, current_cd.swab);
	gtk_widget_set_sensitive (new_widget, 
			current_cd.cd_type == CD_TYPE_AUDIO);
	gtk_box_pack_start(vbox, new_widget, FALSE, FALSE, 3);
	
	new_widget = gtk_check_button_new_with_mnemonic(_("Use _Forcespeed"));
	forcespeed = GTK_TOGGLE_BUTTON(new_widget);
	gtk_toggle_button_set_active(forcespeed, current_cd.forcespeed);
	gtk_box_pack_start(vbox, new_widget, FALSE, FALSE, 3);

	new_widget = gtk_vbox_new(FALSE, 3);
	vbox = GTK_BOX(new_widget);
	gtk_box_pack_start(hbox, new_widget, FALSE, FALSE, 0);
	label = gtk_label_new(_("Writing mode:"));
	gtk_box_pack_start(vbox, label, FALSE, FALSE, 0);
	new_widget = gtk_combo_box_new_text();
	mode_box = GTK_COMBO_BOX(new_widget);
	gtk_combo_box_insert_text(mode_box, MODE_DAO, _("Disk-at-Once (DAO)"));
	gtk_combo_box_insert_text(mode_box, MODE_TAO, _("Track-at-Once (TAO)"));
	gtk_combo_box_insert_text(mode_box, MODE_RAW, _("Raw-Writing (raw96r)"));
	gtk_combo_box_set_active(mode_box, current_cd.write_mode);
	gtk_widget_set_sensitive(new_widget,
			!gtk_toggle_button_get_active(multisession));
	g_signal_connect (GTK_TOGGLE_BUTTON(multisession), "toggled", 
		G_CALLBACK(toggle_multisession), mode_box);
	gtk_box_pack_start(vbox, new_widget, FALSE, FALSE, 0);

	label = gtk_label_new(_("Filesystem Options"));
	new_widget = gtk_vbox_new(FALSE,3);
	if (current_cd.cd_type != CD_TYPE_DATA)
		gtk_widget_set_sensitive (new_widget, FALSE);
	vbox = GTK_BOX(new_widget);
	gtk_notebook_append_page(notebook, new_widget, label);
	if (current_cd.cd_type != CD_TYPE_DATA)
		gtk_widget_set_sensitive (label, FALSE);

	new_widget = gtk_label_new(_("Create CD for:"));
	gtk_label_set_justify(GTK_LABEL(new_widget), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (vbox, new_widget, FALSE, FALSE, 0);
	new_widget = gtk_combo_box_new_text();
	gtk_box_pack_start (vbox, new_widget, FALSE, FALSE, 0);
	isofs_box = GTK_COMBO_BOX(new_widget);
	gtk_combo_box_append_text(isofs_box, _("Windows + Unix"));
	gtk_combo_box_append_text(isofs_box, _("Unix only"));
	gtk_combo_box_append_text(isofs_box, _("MS-DOS"));
	gtk_combo_box_append_text(isofs_box, _("Custom"));
	gtk_combo_box_set_active (isofs_box, 0);
	new_widget = gtk_expander_new(_("Advanced Options"));
	gtk_box_pack_start (vbox, new_widget, TRUE, TRUE, 3);

	label = gtk_label_new(_("Boot Options"));
	new_widget = gtk_vbox_new(FALSE,3);
	vbox = GTK_BOX(new_widget);
	gtk_notebook_append_page(notebook, new_widget, label);
	if (current_cd.cd_type != CD_TYPE_DATA)
		gtk_widget_set_sensitive (label, FALSE);
	
	label = gtk_label_new(_("ISO Identifiers"));
	new_widget = gtk_hbox_new(FALSE,3);
	if (current_cd.cd_type != CD_TYPE_DATA)
		gtk_widget_set_sensitive (new_widget, FALSE);
	hbox = GTK_BOX(new_widget);
	gtk_notebook_append_page(notebook, new_widget, label);
	if (current_cd.cd_type != CD_TYPE_DATA)
		gtk_widget_set_sensitive (label, FALSE);
	
	new_widget = gtk_vbox_new(TRUE, 3);
	gtk_box_pack_start(hbox, new_widget, FALSE, FALSE, 3);
	vbox = GTK_BOX(new_widget);
	new_widget = gtk_vbox_new(TRUE, 3);
	gtk_box_pack_start(hbox, new_widget, TRUE, TRUE, 3);
	hbox = GTK_BOX(new_widget); /* it's really a second V-box */
	
	label = gtk_label_new(_("Volume ID"));
	gtk_box_pack_start(vbox, label, FALSE, FALSE, 3);
	new_widget = gtk_entry_new_with_max_length(32);
	gtk_box_pack_start(hbox, new_widget, TRUE, TRUE, 3);
	
	label = gtk_label_new(_("Copyright"));
	gtk_box_pack_start(vbox, label, FALSE, FALSE, 3);
	new_widget = gtk_entry_new_with_max_length(37);
	gtk_box_pack_start(hbox, new_widget, TRUE, TRUE, 3);

	label = gtk_label_new(_("Publisher"));
	gtk_box_pack_start(vbox, label, FALSE, FALSE, 3);
	new_widget = gtk_entry_new_with_max_length(128);
	gtk_box_pack_start(hbox, new_widget, TRUE, TRUE, 3);

	label = gtk_label_new(_("Preparer"));
	gtk_box_pack_start(vbox, label, FALSE, FALSE, 3);
	new_widget = gtk_entry_new_with_max_length(128);
	gtk_box_pack_start(hbox, new_widget, TRUE, TRUE, 3);

	label = gtk_label_new(_("Abstract"));
	gtk_box_pack_start(vbox, label, FALSE, FALSE, 3);
	new_widget = gtk_entry_new_with_max_length(37);
	gtk_box_pack_start(hbox, new_widget, TRUE, TRUE, 3);

	label = gtk_label_new(_("Bibliography"));
	gtk_box_pack_start(vbox, label, FALSE, FALSE, 3);
	new_widget = gtk_entry_new_with_max_length(37);
	gtk_box_pack_start(hbox, new_widget, TRUE, TRUE, 3);

	gtk_widget_show_all(nb_widget);
	
	do {
		result = gtk_dialog_run(dialog);
		switch (result) {
			case GTK_RESPONSE_HELP:
				show_prefs(NULL, GTK_WINDOW(dialog));
				break;
			case GTK_RESPONSE_OK:
				current_cd.burnfree =
					gtk_toggle_button_get_active(burnfree);
				current_cd.overburn =
					gtk_toggle_button_get_active(overburn);
				current_cd.multisession =
				     gtk_toggle_button_get_active(multisession);
				current_cd.dummy =
					gtk_toggle_button_get_active(dummy);
				current_cd.pad =
					gtk_toggle_button_get_active(pad);
				current_cd.immed =
					gtk_toggle_button_get_active(immed);
				current_cd.swab =
					gtk_toggle_button_get_active(swab);
				current_cd.forcespeed =
				       gtk_toggle_button_get_active(forcespeed);
				current_cd.write_mode =
					gtk_combo_box_get_active(mode_box);
			/*default:
			 	do nothing */
		}
	} while (result == GTK_RESPONSE_HELP);
	gtk_widget_destroy (GTK_WIDGET(dialog));
}
