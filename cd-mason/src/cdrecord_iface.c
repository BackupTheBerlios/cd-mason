#define _GNU_SOURCE 1 /* for stpcpy */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef STDC_HEADERS
#  include <string.h>
#  include <stdio.h>
#else
#  warning "Header file for memcpy(), strncpy(), stpcpy(), snprintf()?"
#endif
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#else
#  warning "where is the definition of strtol() and friends?"
#endif
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "main.h"
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#else
#  warning "sys/types.h not needed for regexen?"
#endif
#ifdef HAVE_REGEX_H
#  include <regex.h>
#else
#  warning "definition of regex functions not in regex.h?"
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#else
#  warning "definition of dup2() and STDIN_FILENO not found."
#endif
#define B_LEN 256

typedef struct {
	regex_t	rx;
	void 	(* match_func) (const regex_t *rx, const regmatch_t *regmatch, 
			gchar **line, gsize *linelen);
} parse_t;

void register_parser (
		const gchar *rx_str,
		void (*func) (const regex_t *rx, const regmatch_t *regmatch,
			gchar **line, gsize *linelen))
{
	parse_t	*parser;
	int	result;

	parser = g_chunk_new(parse_t, current_cd.parse_chunk);
	if(G_UNLIKELY((result = regcomp (&(parser->rx), rx_str, REG_EXTENDED))))
		g_error("failed to compile <%s> error: %d", rx_str, result);
	parser->match_func = func;
	current_cd.parse_list = g_slist_append (current_cd.parse_list, parser);
}

inline void midstr (gchar *dest, gchar *src, 
		gsize maxlen, gsize s_offset, gsize e_offset)
{
	gsize len = e_offset - s_offset;
	
	maxlen--;
	if (len > maxlen) len = maxlen;
	strncpy (dest, (gchar *)src + s_offset, len);
	*((gchar *)dest + len) = '\0';
}

void start_write (const regex_t *rx, const regmatch_t regmatch[],
		gchar **line, gsize *linelen)
{
	gchar	status[255];
	gchar	speed[5];
	gchar	mode[30];
	gchar	session[30];
	
	midstr (speed, *line, 5, regmatch[1].rm_so, regmatch[1].rm_eo);
	midstr (mode, *line, 30, regmatch[2].rm_so, regmatch[2].rm_eo);
	midstr (session, *line, 30, regmatch[3].rm_so, regmatch[3].rm_eo);
	
	snprintf (status, 255, _("I'm going to write a CD at speed %s in %s mode for %s session."), speed, mode, session);
	gtk_label_set_text(gui.p_status, status);
}
void init_countdown (const regex_t *rx, const regmatch_t regmatch[],
		gchar **line, gsize *linelen)
{
	gchar status[256];
	gchar timeout[5];
	midstr (timeout, *line, 5, regmatch[1].rm_so, regmatch[1].rm_eo);
	snprintf (status, 256,  _("%s\nStarting in %s seconds."),
			gtk_label_get_text(gui.p_status), timeout);
	gtk_label_set_text(gui.p_status, status);
}
void next_countdown (const regex_t *rx, const regmatch_t *regmatch,
		gchar **line, gsize *linelen)
{
	gchar status[256];
	gchar first[256];
	gchar timeout [5];

	midstr (timeout, *line, 5, regmatch[1].rm_so, regmatch[1].rm_eo);
	strncpy (status, gtk_label_get_text(gui.p_status), 256);
	midstr (first, status, 256, 0, strchr(status, '\n') - status);
	snprintf (status, 256, _("%s\nStarting in %s seconds."),
		 first, timeout);
	gtk_label_set_text(gui.p_status, status);
	*linelen = 0; /*do not display countdown in log*/
}

void pregap (const regex_t *rx, const regmatch_t regmatch[],
		gchar **line, gsize *linelen)
{
	current_cd.burn_in_progress = TRUE;
	gtk_label_set_text(gui.p_status, _("Writing lead-in"));
}

void track_progress (const regex_t *rx, const regmatch_t regmatch[],
		gchar **line, gsize *linelen)
{
	gint track_size, written, fifo, buf, written_offset;
	gchar track[4];
	gchar status[30];

	midstr(track, *line, 4, regmatch[1].rm_so, regmatch[1].rm_eo);
	snprintf(status, 30, _("Writing track %s"), track);
	gtk_label_set_text(gui.p_status, status);
	
	if (regmatch[2].rm_so == regmatch[2].rm_eo)
	{
		/* the optional subexpression "(([[:digit:]]+) of +)?" did not
		 * match
		 */
		written_offset = regmatch[4].rm_so;
		track_size = -1;
	}
	else
	{
		written_offset = regmatch[3].rm_so;
		track_size = strtol ((char*)*line+regmatch[4].rm_so, NULL, 10);
	}
	
	written = strtol ((char *)*line+written_offset, NULL, 10);
	gtk_progress_bar_set_fraction (gui.p_bar, 
		(gdouble) ((written + current_cd.already_written)
			    << 20) / (gdouble) current_cd.total_size);
	if (written == track_size)
		current_cd.already_written += track_size;
	
	fifo = strtol ((char *)*line+regmatch[5].rm_so, NULL, 10);
	gtk_progress_bar_set_fraction (gui.p_fifo, (gdouble)fifo / 100.0);
	
	buf = strtol ((char *)*line+regmatch[6].rm_so, NULL, 10);
	gtk_progress_bar_set_fraction (gui.p_buf, (gdouble)buf / 100.0);
	
	midstr (status, *line, 30, regmatch[7].rm_so, regmatch[7].rm_eo);
	gtk_label_set_text(gui.p_speed, status);
	**line = '\n';
}

void fixating (const regex_t *rx, const regmatch_t regmatch[],
		gchar **line, gsize *linelen)
{
	gtk_progress_bar_set_fraction (gui.p_bar, 1.0);
	gtk_label_set_text(gui.p_status, _("Fixating..."));
}

void burning_done (const regex_t *rx, const regmatch_t regmatch[],
		gchar **line, gsize *linelen)
{
	GtkBox *action_area;
	GtkWidget *button;
	
	gtk_label_set_text(gui.p_status, _("Done."));
	current_cd.burn_in_progress = FALSE;
	current_cd.already_written = 0;
	g_io_channel_shutdown (current_cd.cdr.err, FALSE, NULL);
	g_io_channel_shutdown (current_cd.cdr.out, FALSE, NULL);
	g_source_remove(current_cd.cdr.out_source);
	g_source_remove(current_cd.cdr.err_source);
	action_area = GTK_BOX(gui.progress_dialog->action_area);
	gtk_widget_destroy (
			((GtkBoxChild *)(action_area->children->data))->widget);
	button = gtk_dialog_add_button (
			gui.progress_dialog, GTK_STOCK_CLOSE, GTK_RESPONSE_OK);
	gtk_widget_grab_focus (button);
}

void no_disk (const regex_t *rx, const regmatch_t regmatch[],
		gchar **line, gsize *linelen)
{
	GtkBox *action_area;
	GtkWidget *button;
	
	gtk_label_set_text(gui.p_status, _("There seems to be no CD in the burner"));
	current_cd.burn_in_progress = FALSE;
	current_cd.already_written = 0;
	g_io_channel_shutdown (current_cd.cdr.err, FALSE, NULL);
	g_io_channel_shutdown (current_cd.cdr.out, FALSE, NULL);
	g_source_remove(current_cd.cdr.out_source);
	g_source_remove(current_cd.cdr.err_source);
	action_area = GTK_BOX(gui.progress_dialog->action_area);
	gtk_widget_destroy (
			((GtkBoxChild *)(action_area->children->data))->widget);
	button = gtk_dialog_add_button (
			gui.progress_dialog, GTK_STOCK_CLOSE, GTK_RESPONSE_OK);
	gtk_widget_grab_focus (button);
}
	
void average_speed (const regex_t *rx, const regmatch_t regmatch[],
		gchar **line, gsize *linelen)
{
	gchar speed[8];

	midstr(speed, *line, 8, regmatch[1].rm_so, regmatch[1].rm_eo);
	gtk_label_set_text (gui.p_speed, speed);
}
	
void probe_cd(const regex_t *rx, const regmatch_t regmatch[],
		gchar **line, gsize *linelen)
{
	gtk_label_set_text (gui.p_status, _("Probing CD"));
}

void start_blank(const regex_t *rx, const regmatch_t regmatch[],
		gchar **line, gsize *linelen)
{
	gtk_label_set_text (gui.p_status, _("Minimally blanking CD..."));
}

void init_parse_list (void)
{
 current_cd.parse_chunk = g_mem_chunk_create(parse_t, 8, G_ALLOC_ONLY);
 
 register_parser (
 "^\rTrack ([[:digit:]]+): +(([[:digit:]]+) of +)?([[:digit:]]+) MB written \\(fifo +([[:digit:]]+)%\\) \\[buf +([[:digit:]]+)%\\] +([[:digit:]]+.[[:digit:]]x)\\.",
 track_progress);
 
 register_parser (
   "^Starting to write .* speed ([[:digit:]]+) in (.*) mode for (.*) session\\.",
   start_write);
 
 register_parser (
   "^Last chance to quit, .* in ([[:digit:]]+) seconds\\.",
   init_countdown);

 register_parser (
   "^\b+ +([[:digit:]]+) seconds\\.",
   next_countdown);

 register_parser (
  "^Writing pregap",
  pregap);

 register_parser (
  "^Device type",
  probe_cd
  );

 register_parser (
 "^Fixating\\.\\.\\.",
 fixating);

 register_parser (
 "fifo had .* puts",
 burning_done);

 register_parser (
 "No disk",
 no_disk); 
 
 register_parser (
 "^Average write speed +([[:digit:]]+.[[:digit:]]x)",
 average_speed);

 register_parser (
 "^Blanking PMA",
 start_blank);

 register_parser (
 "^Blanking time:",
 burning_done);
}

/* We want to parse cdrecords output in lines. However, not all lines end in
 * 0x0a because cdrecord outputs graphically. That is it overwrites some lines.
 * Therefore, valid line terminators are: '\n', "n.", "s." and "x."
 * A line is defined as all characters occuring between any of these three 
 * line terminators.
 *
 * returns FALSE if no line terminators were found.
 */
gboolean split_line (gchar *in, 
		const gsize *in_len, 
		gchar *out, 
		gsize *out_len)
{
	gsize	i=0;
	gboolean terminator_found=FALSE;

	while ((!terminator_found) && (i < *in_len))
	{
		i++;
		*out=*(in++);
		/* change latin-1 encoded "ö" (0xf6) to "o" (0x6f) 
		 * because otherwise it's not valid UTF-8
		 */
		if (*out==(gchar)0xf6)
		{
			*out=(gchar)0x6f;
		} else if (*out=='\n') 
		{
			terminator_found = TRUE;
		} else if ((*out=='.') && (i > 1))
		{
			if ((*(out-1)=='n') 
					|| (*(out-1)=='s')
					|| (*(out-1)=='x'))
				terminator_found = TRUE;
		}
		out++;
	} 
	*out = '\0';
	*out_len=i;
	return terminator_found;
}

void parse_line (gchar **line, gsize *linelen)
{
	regmatch_t	match[10];
	GSList		*curr;
	parse_t		*parser;

	for(curr = current_cd.parse_list; curr; curr=curr->next)
	{
		parser = (parse_t *)curr->data;
		if (G_UNLIKELY(regexec (&(parser->rx), *line, 
					10, match, 0) == 0))
		{
			parser->match_func (&(parser->rx), match,
					line, linelen);
			break;
		}
	}
	if (*linelen)
	{
		gtk_text_buffer_insert(gui.log_buf, &gui.end_iter, 
				*line, *linelen);
		gtk_text_view_scroll_to_iter( gui.t_view, &gui.end_iter, 
			0.0, FALSE, 0.0, 0.0);
	}
}

void read_channel(GIOChannel *channel, 
		gchar **line,
		gsize *linelen)
{
	gsize buflen=0;
	gchar buf[B_LEN];
	gchar *buffer=buf;
	GError *gerror=NULL;
	GIOStatus status;

	if (*linelen)
		memcpy(buffer, *line, *linelen);
	status = g_io_channel_read_chars(
			channel, 
			&buf[*linelen], B_LEN-1-*linelen, &buflen, 
			&gerror);
	buflen += *linelen;
	if (status == G_IO_STATUS_ERROR) {
		g_message("error %s", gerror->message);
		g_error_free(gerror);
	}
	if (buflen) {
		while (split_line (buffer, &buflen, *line, linelen))
		{
			buffer = (gchar *)(buffer+*linelen);
			buflen -= *linelen;
			parse_line (line, linelen);
			*linelen=0;
		}
	}
}


gboolean read_cdr_out(GIOChannel *channel, 
		const GIOCondition condition)
{
	static gsize partial_out_len=0;
	static gchar partial_out_line[B_LEN];
	static gchar *out_linep;

	out_linep = partial_out_line;
	read_channel(channel, &out_linep, &partial_out_len);
	return TRUE;
}
	
gboolean read_cdr_err (GIOChannel *channel, 
		GIOCondition condition)
{
	static gsize partial_err_len=0;
	static gchar partial_err_line[B_LEN];
	static gchar *err_linep;

	err_linep = partial_err_line;
	read_channel(channel, &err_linep, &partial_err_len);
	return TRUE;
}

void redirect_stdin(gpointer user_data)
{
	dup2 (GPOINTER_TO_INT(user_data), STDIN_FILENO);
}

gint spawn_mkisofs()
{
	GtkTreeIter iter;
	gint argc, iso_stdout;
	gchar *child_argv[256];
	
	argc = 0;
	child_argv[argc++] = cfg.mkiso_path;
	child_argv[argc++] = "-quiet";
	child_argv[argc++] = "-J";
	child_argv[argc++] = "-r";
	child_argv[argc++] = "-l";
	
	gtk_tree_model_get_iter_first (current_cd.tree.model, &iter);
	do {
		gtk_tree_model_get (current_cd.tree.model, 
				&iter, SRCNAME_COL, 
				&child_argv[argc++], -1);
	} while (gtk_tree_model_iter_next (current_cd.tree.model, &iter));
	child_argv[argc] = NULL; /* end marker */
	
	g_spawn_async_with_pipes(
			NULL,	/* working directory*/
			child_argv,
			NULL,	/* environment*/
			0,	/* flags */
			NULL,	/* child_setup */
			NULL,	/* user data for child setup*/
			&(current_cd.cdr.iso_pid),
			NULL,	/* child's stdin */
			&iso_stdout,
			NULL,	/* child's stderr */
			NULL	/* spawing error */
			);
	return iso_stdout;
}
		

void spawn_cdrecord(gint cdr_mode)
{
	gchar *mode_opt[] = {
		"-dao",
		"-tao",
		"-raw96r"
	};
	gchar *child_argv[128];
	gint c_stderr, c_stdout, argc;
	gpointer iso_stdout;
	GSpawnFlags flags;
	GSpawnChildSetupFunc child_setup;
	gchar driveropts[35]= "driveropts=";
	gchar device[64];
	gchar speed[9];
	gchar *opts;
	GtkTreeIter iter;
	cdr_handle *cdr = &(current_cd.cdr);
	
	child_setup = NULL;
	iso_stdout = NULL;
	flags = 0;
	argc = 0;
	
	child_argv[argc++] = cfg.cdrec_path;
	child_argv[argc++] = "-v";
	snprintf (device, 64, "dev=%s", cfg.burner_dev);
	child_argv[argc++] = device;
	child_argv[argc++] = "fs=16m";
	if(cdr_mode)
	{
		gchar *blank_mode[] = {
			"track", "unclose", "session", "all", "fast" };
		gchar blank[35] = "blank=";

		opts = &blank[6];
		strncpy (opts, blank_mode[cdr_mode-1], 29);
		child_argv[argc++] = blank;
	}
	else
	{
		if (cfg.max_speed)
		{
			snprintf (speed, 9, "speed=%d", cfg.max_speed);
			child_argv[argc++] = speed;
		}
		child_argv[argc++] = mode_opt[current_cd.write_mode];
		if (current_cd.dummy) child_argv[argc++] = "-dummy";
		if (current_cd.overburn) child_argv[argc++] = "-overburn";
		if (current_cd.multisession) child_argv[argc++] = "-multi";
		if (current_cd.immed) child_argv[argc++] = "-immed";
		if (current_cd.pad) child_argv[argc++] = "-pad";
		if (current_cd.swab) child_argv[argc++] = "-swab";
		opts = &driveropts[11];
		if (current_cd.burnfree) opts = stpcpy(opts, "burnfree");
		if (current_cd.forcespeed) {
			if (opts != &driveropts[11]) opts = stpcpy(opts, ",");
			opts = stpcpy (opts, "forcespeed");
		}
		if (opts != &driveropts[11]) child_argv[argc++] = driveropts;
		switch (current_cd.cd_type) {
		case CD_TYPE_ISO:
			gtk_tree_model_get_iter_first (
					current_cd.tree.model, &iter);
			gtk_tree_model_get (current_cd.tree.model, 
						&iter,
						SRCNAME_COL, 
						&child_argv[argc++], -1);
			break;
		case CD_TYPE_AUDIO:
			child_argv[argc++] = "-audio";
			gtk_tree_model_get_iter_first (
					current_cd.tree.model, &iter);
			do {
				gtk_tree_model_get (current_cd.tree.model, 
						&iter,
						SRCNAME_COL, 
						&child_argv[argc++], -1);
			} while (gtk_tree_model_iter_next (
						current_cd.tree.model, &iter));
			break;
		case CD_TYPE_DATA:
			child_argv[argc++] = "-data";
			child_argv[argc++] = "-"; /* read ISO from stdin */
			child_setup = redirect_stdin;
			iso_stdout = GINT_TO_POINTER(spawn_mkisofs ());
			flags = G_SPAWN_LEAVE_DESCRIPTORS_OPEN;
		}
	}
	child_argv[argc]=NULL; /* end marker */
	
	g_spawn_async_with_pipes(
			NULL, /*working directory*/
			child_argv,
			NULL, /*environment*/
			flags,	
			child_setup,
			iso_stdout, /*user data for child setup*/
			&(cdr->pid),
			NULL,  /* child's stdin (this is a return location)
				* we do the pipe setup with mkisofs in
				* the child setup function
				*/
			&c_stdout,
			&c_stderr,
			NULL /*spawing error*/
			);
	init_parse_list();
	cdr->out = g_io_channel_unix_new (c_stdout);
	g_io_channel_set_encoding(cdr->out, NULL ,NULL); /* we want raw data */
	g_io_channel_set_buffered (cdr->out, FALSE);
	cdr->out_source = g_io_add_watch (cdr->out, G_IO_IN,
			(GIOFunc) read_cdr_out,	NULL);
	cdr->err = g_io_channel_unix_new (c_stderr);
	g_io_channel_set_encoding(cdr->err, NULL ,NULL);
	g_io_channel_set_buffered (cdr->err, FALSE);
	cdr->err_source = g_io_add_watch (cdr->err, G_IO_IN,
			(GIOFunc) read_cdr_err, NULL);
}
