/***************************************************************************
 *            tree-view.c
 *
 *  Sun Jun  6 12:18:13 2004
 *  Copyright  2004  User
 *  Email
 ****************************************************************************/
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib/gi18n.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  warning "Where is the header file for strcmp()?"
#endif
#include "main.h"

static gboolean (*default_row_drop_possible)();
enum {
		TARGET_URI,
		TARGET_ROW,
	};
static GtkTargetEntry  	dest_targets[] = {
		{"text/uri-list", 0, TARGET_URI},
		{"GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, TARGET_ROW},
        };
static GtkTargetEntry source_targets[] = {
		{"GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, TARGET_ROW},
	};	
static gchar *audio_types[] = { 
		"audio/x-wav",
		"audio/basic",
		"audio/x-mp3",
		"application/ogg",
		NULL
};
static gchar *dirstring;

gboolean can_be_dropped_into(GtkTreePath *dest)	
{
	/* FIXME: for the moment we don't handle re-arranging of files on a
	 * DATA CD. We could simply disable the tree view as a drag source but
	 * then I'd forget how to fix it again ;-) So simply disallow dropping
	 * files into directories for the moment
	 */
	return FALSE;
#if 0
	GtkTreeIter	iter;
	

	if (gtk_tree_model_get_iter (current_cd.tree.model,
				&iter, dest))
	{
		guchar type;
		gtk_tree_model_get (current_cd.tree.model, &iter,
					VFS_TYPE_COL, &type,
					-1);
		return (type == GNOME_VFS_FILE_TYPE_DIRECTORY);
  	}
	else
		return FALSE;
#endif
}

void create_new_tree_row(GtkTreePath *dest,
		GtkTreeViewDropPosition drop_pos,
		GtkTreeIter *new_iterp)
{
	GtkTreeIter old_iter;
	
	if (dest && gtk_tree_model_get_iter(
				current_cd.tree.model, 
				&old_iter, dest))
	{
		/* The following tests depend on specific numeric values for the
		 * GtkTreeViewDropPosition because we do bit operations to
		 * determine whether the items shall be dropped into/before the
		 * given drop_pos.
		 * Specificly:	bit 0 => before /after
		 * 		bit 1 => not into / into
		 */
			
		if ((drop_pos & GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) && 
				(can_be_dropped_into (dest))) 
			gtk_tree_store_append (current_cd.tree.store,
					new_iterp,
					&old_iter);
		else
		{
			if (drop_pos & GTK_TREE_VIEW_DROP_AFTER)
				gtk_tree_store_insert_after(
						current_cd.tree.store,
						new_iterp, 
						NULL, 
						&old_iter);
			else	
				gtk_tree_store_insert_before(
						current_cd.tree.store,
						new_iterp,
						NULL,
						&old_iter);
		}
	}
	else
		gtk_tree_store_append (current_cd.tree.store, new_iterp, NULL);
}

void set_type(guchar type)
{
	switch (type) {
		case CD_TYPE_DATA:
			gtk_menu_item_activate (gui.data_type_item);
			break;
		case CD_TYPE_AUDIO:
			gtk_menu_item_activate (gui.audio_type_item);
			break;
		case CD_TYPE_ISO:
			gtk_menu_item_activate (gui.iso_type_item);
			break;
		}
	return;
}

void set_guessed_cd_type (guchar new_type)
{
	if (current_cd.cd_type_chosen)
		return;
	switch (current_cd.cd_type) {
		case CD_TYPE_UNKNOWN:
			set_type(new_type);
			break;
		case CD_TYPE_AUDIO:
			if (CD_TYPE_AUDIO == new_type)
				break;
			/* else fall through */
		case CD_TYPE_ISO:
			set_type(CD_TYPE_DATA);
	}
	current_cd.cd_type_chosen = FALSE;
}

void update_cd_type (gchar *mime_type)
{
	gchar	**audio_type;
	
	if ( 0 == strcmp (mime_type, "application/x-cd-image"))
		set_guessed_cd_type(CD_TYPE_ISO);
	else
	{
		for (audio_type = audio_types; *audio_type; audio_type++)
		{
			if ( 0 == strcmp (mime_type, *audio_type))
			{
				set_guessed_cd_type(CD_TYPE_AUDIO);
				return;	
			}
		}
		set_guessed_cd_type(CD_TYPE_DATA);
	}	
}

void update_cd_size (gint64 size)
{
	if (size < 0 && -size >= current_cd.total_size)
	{
		current_cd.total_size = 0;
		current_cd.cd_type = CD_TYPE_UNKNOWN;
		current_cd.cd_type_chosen = FALSE;
	}
	else
		current_cd.total_size += size;
	gtk_progress_bar_set_text(gui.size_bar, 
			gnome_vfs_format_file_size_for_display
				(current_cd.total_size));
}

void add_file(gchar *uri,
		GtkTreePath *dest_path, 
		GtkTreeViewDropPosition drop_pos)
{
	GnomeVFSFileInfo *info;
	GnomeVFSResult	result;
	gchar		*fullpath;
	GtkTreeIter	new_iter;
	
	info = gnome_vfs_file_info_new();
	result = gnome_vfs_get_file_info (uri, info,
		GNOME_VFS_FILE_INFO_DEFAULT |GNOME_VFS_FILE_INFO_GET_MIME_TYPE);
	
	if (result != GNOME_VFS_OK)
	{
		g_message("No GNOME_VFS_INFO!");
		return;
	}
	
	if (!(info->flags & GNOME_VFS_FILE_FLAGS_LOCAL))
	{
		g_message("Not a local file!");
		return;
	}
	
	create_new_tree_row(dest_path, drop_pos, &new_iter);
	fullpath = g_filename_from_uri(uri, NULL, NULL);
	if(!fullpath) fullpath = uri;
	gtk_tree_store_set (current_cd.tree.store, &new_iter,
		DESTNAME_COL,
		  g_filename_display_basename(fullpath),
		LEN_COL,
		  (guint) info->size, 
		LENSTR_COL, /*FIXME: what to do with links?*/
		  info->type == GNOME_VFS_FILE_TYPE_DIRECTORY ? 
		  	dirstring :
			gnome_vfs_format_file_size_for_display (info->size),
		SRCNAME_COL,
		  fullpath,
		MIME_TYPE_COL,
		  info->mime_type,
		VFS_TYPE_COL,
		  info->type,
		-1);
	update_cd_size((gint64)info->size);
	update_cd_type(info->mime_type);
}

void del_file(void)
{
	GtkTreeIter		iter;
	GtkTreePath		*path;
	GtkTreeSelection	*selection;
	GList			*path_list, *i, *ref_list=NULL;
	guint			size;

	selection = gtk_tree_view_get_selection(gui.treeview);
	path_list = gtk_tree_selection_get_selected_rows(selection, 
			&current_cd.tree.model);
	for (i = path_list; i != NULL; i = g_list_next(i))
	{
		ref_list = g_list_append (
				ref_list,
				gtk_tree_row_reference_new (
					current_cd.tree.model,
					(GtkTreePath *) i->data));
	}
	g_list_foreach (path_list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (path_list);
	for (i = ref_list; i != NULL; i=g_list_next(i))
	{
		
		if (gtk_tree_row_reference_valid (
					(GtkTreeRowReference *) i->data))
		{
			path = gtk_tree_row_reference_get_path (
					(GtkTreeRowReference *) i->data);
			gtk_tree_model_get_iter (
					current_cd.tree.model, &iter, path);
			gtk_tree_model_get (
					current_cd.tree.model, &iter, 
					LEN_COL, &size, -1);	
			update_cd_size(-(gint64)size);
			gtk_tree_store_remove(current_cd.tree.store, &iter);
		}
	}
	g_list_free (ref_list);
}

void drag_receive(GtkWidget *widget,
                  GdkDragContext *drag_context,
                  gint x,
                  gint y,
                  GtkSelectionData *data,
                  guint info,
                  guint time,
                  gpointer uri_type)
{
	if (G_UNLIKELY(data->type == (GdkAtom) uri_type))
	{
		gchar	**uris, **uri;
		GtkTreePath	*dest_path;
		GtkTreeViewDropPosition drop_pos;
		
		gtk_tree_view_get_dest_row_at_pos(gui.treeview, x, y, 
				&dest_path, &drop_pos);
		uris = g_uri_list_extract_uris(data->data);
		for (uri = uris; *uri; uri++)
		{
			add_file(*uri, dest_path, drop_pos);
		}
		g_strfreev(uris);
		gtk_tree_path_free(dest_path);
		gtk_drag_finish(drag_context, TRUE, FALSE, time);
	}
}

static gboolean can_drop_here (GtkTreeDragDest  *drag_dest,
				GtkTreePath      *dest_path,
				GtkSelectionData *selection_data)
{
  gboolean rv = TRUE;
  GtkTreePath	*parent;
  
  if (gtk_tree_path_get_depth(dest_path) > 1)
  { 	
	parent = gtk_tree_path_copy (dest_path);
  	if (gtk_tree_path_up(parent))
		rv = can_be_dropped_into(parent);
  	gtk_tree_path_free(parent);
  }
  
  if (rv)
	  return default_row_drop_possible
		  (drag_dest, dest_path, selection_data);
  else
	  return rv;
}

GtkTreeView *create_treeview (void)
{
	GtkTreeViewColumn	*col;
	GtkCellRenderer		*text_renderer;
	GtkTreeView		*treeview;
	GtkTreeDragDestIface *iface;

	dirstring = _("Directory");

	/* Set up storage space */
	current_cd.tree.store = gtk_tree_store_new ( NUM_COLS,
				G_TYPE_STRING,	/*Name on CD (UTF8)*/
				G_TYPE_UINT,	/*Length (in sectors)*/
				G_TYPE_STRING,	/*Len. human readable */
				G_TYPE_STRING,	/*Name on HD (local)*/
				G_TYPE_STRING,	/*MIME type*/
				G_TYPE_UCHAR	/*GNOME_VFS_FileType*/
			      );

	/* Create GUI */
	treeview = GTK_TREE_VIEW(gtk_tree_view_new_with_model (
			current_cd.tree.model));

	gtk_tree_selection_set_mode (
			gtk_tree_view_get_selection(treeview),
			GTK_SELECTION_MULTIPLE);

	text_renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes (
			_("Destination"),
			text_renderer, "text", DESTNAME_COL, NULL);
	gtk_tree_view_column_set_resizable(col, TRUE);
	gtk_tree_view_append_column (treeview, col);
	
	col = gtk_tree_view_column_new_with_attributes(
			_("Size"),
		 	text_renderer,	"text", LENSTR_COL, NULL);
	gtk_tree_view_append_column (treeview, col);
	
	/* Set up Drag & Drop */
	gtk_tree_view_enable_model_drag_source (treeview,
				GDK_BUTTON1_MASK,
				source_targets,
				G_N_ELEMENTS (source_targets),
				GDK_ACTION_MOVE);
	gtk_tree_view_enable_model_drag_dest (treeview, 
				dest_targets,
				G_N_ELEMENTS(dest_targets),
				GDK_ACTION_COPY | GDK_ACTION_MOVE);
	
	g_signal_connect(GTK_WIDGET(treeview), "drag-data-received",
			G_CALLBACK(drag_receive),
			(gpointer) gdk_atom_intern("text/uri-list",TRUE));
	if ((iface = GTK_TREE_DRAG_DEST_GET_IFACE(
				GTK_TREE_DRAG_DEST(current_cd.tree.store))))
	{
		default_row_drop_possible = iface->row_drop_possible;
		iface->row_drop_possible = can_drop_here;
	}
	return (treeview);
}
