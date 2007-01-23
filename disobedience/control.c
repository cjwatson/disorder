/*
 * This file is part of DisOrder.
 * Copyright (C) 2006 Richard Kettlewell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "disobedience.h"

/* Forward declartions ----------------------------------------------------- */

struct icon;

static void update_pause(const struct icon *);
static void update_play(const struct icon *);
static void update_scratch(const struct icon *);
static void update_random_enable(const struct icon *);
static void update_random_disable(const struct icon *);
static void update_enable(const struct icon *);
static void update_disable(const struct icon *);
static void clicked_icon(GtkButton *, gpointer);

static double left(double v, double b);
static double right(double v, double b);
static double volume(double l, double r);
static double balance(double l, double r);

static void volume_adjusted(GtkAdjustment *a, gpointer user_data);
static gchar *format_volume(GtkScale *scale, gdouble value);
static gchar *format_balance(GtkScale *scale, gdouble value);

/* Control bar ------------------------------------------------------------- */

static int suppress_set_volume;
/* Guard against feedback loop in volume control */

static struct icon {
  const char *icon;
  const char *tip;
  void (*clicked)(GtkButton *button, gpointer userdata);
  void (*update)(const struct icon *i);
  int (*action)(disorder_eclient *c,
                disorder_eclient_no_response *completed,
                void *v);
  GtkWidget *button;
} icons[] = {
  { "pause.png", "Pause playing track", clicked_icon, update_pause,
    disorder_eclient_pause, 0 },
  { "play.png", "Resume playing track", clicked_icon, update_play,
    disorder_eclient_resume, 0 },
  { "cross.png", "Cancel playing track", clicked_icon, update_scratch,
    disorder_eclient_scratch_playing, 0 },
  { "random.png", "Enable random play", clicked_icon, update_random_enable,
    disorder_eclient_random_enable, 0 },
  { "randomcross.png", "Disable random play", clicked_icon, update_random_disable,
    disorder_eclient_random_disable, 0 },
  { "notes.png", "Enable play", clicked_icon, update_enable,
    disorder_eclient_enable, 0 },
  { "notescross.png", "Disable play", clicked_icon, update_disable,
    disorder_eclient_disable, 0 },
};
#define NICONS (int)(sizeof icons / sizeof *icons)

GtkAdjustment *volume_adj, *balance_adj;

/* Create the control bar */
 GtkWidget *control_widget(void) {
  GtkWidget *hbox = gtk_hbox_new(FALSE, 1), *vbox;
  GtkWidget *content;
  GdkPixbuf *pb;
  GtkWidget *v, *b;
  GtkTooltips *tips = gtk_tooltips_new();
  int n;

  D(("control_widget"));
  for(n = 0; n < NICONS; ++n) {
    icons[n].button = gtk_button_new();
    if((pb = find_image(icons[n].icon)))
      content = gtk_image_new_from_pixbuf(pb);
    else
      content = gtk_label_new(icons[n].icon);
    gtk_container_add(GTK_CONTAINER(icons[n].button), content);
    gtk_tooltips_set_tip(tips, icons[n].button, icons[n].tip, "");
    g_signal_connect(G_OBJECT(icons[n].button), "clicked",
                     G_CALLBACK(icons[n].clicked), &icons[n]);
    /* pop the icon in a vbox so it doesn't get vertically stretch if there are
     * taller things in the control bar */
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), icons[n].button, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
  }
  /* create the adjustments for the volume control */
  volume_adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, goesupto,
                                                 goesupto / 20, goesupto / 20,
                                                 0));
  balance_adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, -1, 1,
                                                  0.2, 0.2, 0));
  /* the volume control */
  v = gtk_hscale_new(volume_adj);
  b = gtk_hscale_new(balance_adj);
  gtk_scale_set_digits(GTK_SCALE(v), 10);
  gtk_scale_set_digits(GTK_SCALE(b), 10);
  gtk_widget_set_size_request(v, 192, -1);
  gtk_widget_set_size_request(b, 192, -1);
  gtk_tooltips_set_tip(tips, v, "Volume", "");
  gtk_tooltips_set_tip(tips, b, "Balance", "");
  gtk_box_pack_start(GTK_BOX(hbox), v, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), b, FALSE, TRUE, 0);
  /* space updates rather than hammering the server */
  gtk_range_set_update_policy(GTK_RANGE(v), GTK_UPDATE_DELAYED);
  gtk_range_set_update_policy(GTK_RANGE(b), GTK_UPDATE_DELAYED);
  /* notice when the adjustments are changed */
  g_signal_connect(G_OBJECT(volume_adj), "value-changed",
                   G_CALLBACK(volume_adjusted), 0);
  g_signal_connect(G_OBJECT(balance_adj), "value-changed",
                   G_CALLBACK(volume_adjusted), 0);
  /* format the volume/balance values ourselves */
  g_signal_connect(G_OBJECT(v), "format-value",
                   G_CALLBACK(format_volume), 0);
  g_signal_connect(G_OBJECT(b), "format-value",
                   G_CALLBACK(format_balance), 0);
  return hbox;
}

/* Update the control bar after some kind of state change */
void control_update(void) {
  int n;
  double l, r;

  D(("control_update"));
  for(n = 0; n < NICONS; ++n)
    icons[n].update(&icons[n]);
  l = volume_l / 100.0;
  r = volume_r / 100.0;
  ++suppress_set_volume;;
  gtk_adjustment_set_value(volume_adj, volume(l, r) * goesupto);
  gtk_adjustment_set_value(balance_adj, balance(l, r));
  --suppress_set_volume;
}

static void update_icon(GtkWidget *button, 
                        int visible, int attribute((unused)) usable) {
  (visible ? gtk_widget_show : gtk_widget_hide)(button);
  /* TODO: show usability */
}

static void update_pause(const struct icon *icon) {
  int visible = !(last_state & DISORDER_TRACK_PAUSED);
  int usable = playing;                 /* TODO: might be a lie */
  update_icon(icon->button, visible, usable);
}

static void update_play(const struct icon *icon) {
  int visible = !!(last_state & DISORDER_TRACK_PAUSED);
  int usable = playing;
  update_icon(icon->button, visible, usable);
}

static void update_scratch(const struct icon *icon) {
  int visible = 1;
  int usable = playing;
  update_icon(icon->button, visible, usable);
}

static void update_random_enable(const struct icon *icon) {
  int visible = !(last_state & DISORDER_RANDOM_ENABLED);
  int usable = 1;
  update_icon(icon->button, visible, usable);
}

static void update_random_disable(const struct icon *icon) {
  int visible = !!(last_state & DISORDER_RANDOM_ENABLED);
  int usable = 1;
  update_icon(icon->button, visible, usable);
}

static void update_enable(const struct icon *icon) {
  int visible = !(last_state & DISORDER_PLAYING_ENABLED);
  int usable = 1;
  update_icon(icon->button, visible, usable);
}

static void update_disable(const struct icon *icon) {
  int visible = !!(last_state & DISORDER_PLAYING_ENABLED);
  int usable = 1;
  update_icon(icon->button, visible, usable);
}

static void clicked_icon(GtkButton attribute((unused)) *button,
                         gpointer userdata) {
  const struct icon *icon = userdata;

  icon->action(client, 0, 0);
}

static void volume_adjusted(GtkAdjustment attribute((unused)) *a,
                            gpointer attribute((unused)) user_data) {
  double v = gtk_adjustment_get_value(volume_adj) / goesupto;
  double b = gtk_adjustment_get_value(balance_adj);

  if(suppress_set_volume)
    /* This is the result of an update from the server, not a change from the
     * user.  Don't feedback! */
    return;
  D(("volume_adjusted"));
  /* force to 'stereotypical' values */
  v = nearbyint(100 * v) / 100;
  b = nearbyint(5 * b) / 5;
  /* Set the volume.  We don't want a reply, we'll get the actual new volume
   * from the log. */
  disorder_eclient_volume(client, 0,
                          nearbyint(left(v, b) * 100),
                          nearbyint(right(v, b) * 100),
                          0);
}

/* Called to format the volume value */
static gchar *format_volume(GtkScale attribute((unused)) *scale,
                            gdouble value) {
  char s[32];

  snprintf(s, sizeof s, "%.1f", (double)value);
  return xstrdup(s);
}

/* Called to format the balance value. */
static gchar *format_balance(GtkScale attribute((unused)) *scale,
                             gdouble value) {
  char s[32];

  if(fabs(value) < 0.1)
    return xstrdup("0");
  snprintf(s, sizeof s, "%+.1f", (double)value);
  return xstrdup(s);
}

/* Volume mapping.  We consider left, right, volume to be in [0,1]
 * and balance to be in [-1,1].
 * 
 * First, we just have volume = max(left, right).
 *
 * Balance we consider to linearly represent the amount by which the quieter
 * channel differs from the louder.  In detail:
 *
 *  if right > left then balance > 0:
 *   balance = 0 => left = right  (as an endpoint, not an instance)
 *   balance = 1 => left = 0
 *   fitting to linear, left = right * (1 - balance)
 *                so balance = 1 - left / right
 *   (right > left => right > 0 so no division by 0.)
 * 
 *  if left > right then balance < 0:
 *   balance = 0 => right = left  (same caveat as above)
 *   balance = -1 => right = 0
 *   again fitting to linear, right = left * (1 + balance)
 *                       so balance = right / left - 1
 *   (left > right => left > 0 so no division by 0.)
 *
 *  if left = right then we just have balance = 0.
 *
 * Thanks to Clive and Andrew.
 */

static double max(double x, double y) {
  return x > y ? x : y;
}

static double left(double v, double b) {
  if(b > 0)                             /* volume = right */
    return v * (1 - b);
  else                                  /* volume = left */
    return v;
}

static double right(double v, double b) {
  if(b > 0)                             /* volume = right */
    return v;
  else                                  /* volume = left */
    return v * (1 + b);
}

static double volume(double l, double r) {
  return max(l, r);
}

static double balance(double l, double r) {
  if(l > r)
    return r / l - 1;
  else if(r > l)
    return 1 - l / r;
  else                                  /* left = right */
    return 0;
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
/* arch-tag:IEbGnYlX8cqOFjY1EXlXBA */
