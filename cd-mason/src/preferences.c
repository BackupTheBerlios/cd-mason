/***************************************************************************
 *            preferences.c
 *            
 *  Thu 31 Mar 13:47:46 2005
 *  Copyright  2005  Felix Braun
 *  hazzl@gmx.net
 ****************************************************************************/
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef STDC_HEADERS
#  include <stdio.h>
#else
#  warn "Where is the definition of snprintf()?"
#endif
#include <glib/gi18n.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "main.h"


void cpy_cfg (gchar **dest, const gchar *src)
{
	g_free(*dest);
	*dest = g_strdup(src);
}

void show_prefs(GtkWidget *self, GtkWindow *parent)
{
	GtkWidget	*new_widget, *container;
	GtkEntry	*dev, *cdr, *mkiso, *mp3, *ogg;
	GtkDialog	*dialog;
	GtkBox		*hbox, *vbox1, *vbox2;
	GtkComboBox	*speed_box;
	gchar		speed[4];
	gint		result, i;
	const 		guchar speed_array[] = { 0, 1, 2, 4, 8, 12, 16, 20, 24,
							32, 40, 48, 52, 0 };

	new_widget = gtk_dialog_new_with_buttons (_("Device Preferences"),
			parent,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
                        NULL);
	dialog = GTK_DIALOG(new_widget);
	
	container = gtk_alignment_new(.5, 0.0, 1.0, 0.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(container), 0, 0, 6, 6);
	gtk_box_pack_start(GTK_BOX(dialog->vbox), container, TRUE, TRUE, 6);
	
	new_widget= gtk_hbox_new(FALSE, 3);
	gtk_container_add (GTK_CONTAINER(container), new_widget);
	hbox = GTK_BOX(new_widget);
	new_widget = gtk_vbox_new(TRUE, 3);
	gtk_box_pack_start (hbox, new_widget, FALSE, FALSE, 3);
	vbox1 = GTK_BOX(new_widget);
	new_widget = gtk_vbox_new(FALSE, 3);
	gtk_box_pack_start (hbox, new_widget, TRUE, TRUE, 3);
	vbox2 = GTK_BOX(new_widget);
	
	new_widget = gtk_label_new(_("CD-Writer Device"));
	gtk_box_pack_start (vbox1, new_widget, FALSE, FALSE, 3);
	new_widget = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (vbox2, new_widget, TRUE, TRUE, 0);
	hbox = GTK_BOX (new_widget);
	new_widget = gtk_entry_new();
	gtk_box_pack_start (hbox, new_widget, TRUE, TRUE, 0);
	dev = GTK_ENTRY(new_widget);
	gtk_entry_set_text (dev, cfg.burner_dev);
	new_widget = gtk_label_new(_("Burning Speed"));
	gtk_box_pack_start (hbox, new_widget, FALSE, FALSE, 3);
	new_widget = gtk_combo_box_new_text();
	gtk_box_pack_start (hbox, new_widget, FALSE, FALSE, 0);
	speed_box = GTK_COMBO_BOX(new_widget);
	gtk_combo_box_append_text (speed_box, _("Automatic"));
	if (cfg.max_speed == 0) gtk_combo_box_set_active (speed_box, 0);
	for (i=1; speed_array[i] != 0; i++)
	{
		snprintf (speed, 4, "%dx", speed_array[i]);
		gtk_combo_box_append_text (speed_box, speed);
		if (cfg.max_speed == speed_array[i])
			gtk_combo_box_set_active (speed_box, i);
	}

	new_widget = gtk_label_new(_("cdrecord"));
	gtk_box_pack_start (vbox1, new_widget, FALSE, FALSE, 0);
	new_widget = gtk_entry_new();
	gtk_box_pack_start (vbox2, new_widget, TRUE, TRUE, 3);
	cdr = GTK_ENTRY(new_widget);
	gtk_entry_set_text (cdr, cfg.cdrec_path);

	new_widget = gtk_label_new(_("mkisofs"));
	gtk_box_pack_start (vbox1, new_widget, FALSE, FALSE, 3);
	new_widget = gtk_entry_new();
	gtk_box_pack_start (vbox2, new_widget, TRUE, TRUE, 3);
	mkiso = GTK_ENTRY(new_widget);
	gtk_entry_set_text (mkiso, cfg.mkiso_path);
	
	new_widget = gtk_label_new(_("madplay"));
	gtk_box_pack_start (vbox1, new_widget, FALSE, FALSE, 3);
	new_widget = gtk_entry_new();
	gtk_box_pack_start (vbox2, new_widget, TRUE, TRUE, 3);
	mp3 = GTK_ENTRY(new_widget);
	gtk_entry_set_text (mp3, cfg.mp3dec);
	
	new_widget = gtk_label_new(_("oggdec"));
	gtk_box_pack_start (vbox1, new_widget, FALSE, FALSE, 3);
	new_widget = gtk_entry_new();
	gtk_box_pack_start (vbox2, new_widget, TRUE, TRUE, 3);
	ogg = GTK_ENTRY(new_widget);
	gtk_entry_set_text (ogg, cfg.oggdec);
	
	gtk_widget_show_all(container);
	
	result = gtk_dialog_run(dialog);
	if (result == GTK_RESPONSE_OK) {
		cpy_cfg(&cfg.burner_dev, gtk_entry_get_text(dev));
		cpy_cfg(&cfg.cdrec_path, gtk_entry_get_text(cdr));
		cpy_cfg(&cfg.mkiso_path, gtk_entry_get_text(mkiso));
		cpy_cfg(&cfg.mp3dec, gtk_entry_get_text(mp3));
		cpy_cfg(&cfg.oggdec, gtk_entry_get_text(ogg));
		cfg.max_speed =
			speed_array[gtk_combo_box_get_active(speed_box)];
		}
	gtk_widget_destroy (GTK_WIDGET(dialog));
}
