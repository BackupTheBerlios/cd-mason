#ifndef PROJECT_H
#define PROJECT_H	1
#include <glib/gtypes.h>
typedef struct {
	GPid	pid, iso_pid;
	GIOChannel *out, *err;
	guint	out_source, err_source;
} cdr_handle;
typedef struct {
	guint64 total_size;
	guint	cd_type :2;
	guint	write_mode :3;
	guint	cd_type_chosen :1;
	guint	burn_in_progress :1;
	guint	burnfree :1;
	guint	overburn :1;
	guint	multisession :1;
	guint	dummy :1;
	guint	pad :1;
	guint	immed :1;
	guint	swab :1;
	guint	forcespeed :1;
	guint	already_written;
	/*gchar	*input_charset;
	gchar	*output_charset;*/
	GMemChunk *parse_chunk;
	GSList	*parse_list;
	union	{
		GtkTreeStore *store;
		GtkTreeModel *model;
	} tree;
	cdr_handle cdr;
} CDM_CD;

typedef struct {
	GtkWindow *MainWindow;
	GtkTreeView *treeview;
	GtkContainer *cd_type_button;
	GtkProgressBar *size_bar;
	GtkMenuItem *data_type_item;
	GtkMenuItem *audio_type_item;
	GtkMenuItem *iso_type_item;
	GtkDialog *progress_dialog;
	GtkLabel *p_status, *p_speed;
	GtkProgressBar *p_bar, *p_buf, *p_fifo;
	GtkTextBuffer *log_buf;
	GtkTextView *t_view;
	GtkTextIter end_iter;
	gint height, width;
} CDM_GUI;

/* Maximum 0-3 */
enum {
	CD_TYPE_UNKNOWN,
	CD_TYPE_DATA,
	CD_TYPE_AUDIO,
	CD_TYPE_ISO
};

/* Maximum 0-7 !*/
enum {
	MODE_DAO = 0,
	MODE_TAO,
	MODE_RAW
};
	


enum {
	DESTNAME_COL,
	LEN_COL,
	LENSTR_COL,
	SRCNAME_COL,
	MIME_TYPE_COL,
	VFS_TYPE_COL,
	NUM_COLS
};

typedef struct {
	gchar *burner_dev;
	gchar *cdrec_path;
	gchar *mkiso_path;
	guchar max_speed;
	gchar *mp3dec;
	gchar *oggdec;
} CDM_config;

#endif /* PROJECT_H */
