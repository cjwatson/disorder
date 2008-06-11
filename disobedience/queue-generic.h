/*
 * This file is part of DisOrder
 * Copyright (C) 2006-2008 Richard Kettlewell
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
#ifndef QUEUE_GENERIC_H
#define QUEUE_GENERIC_H

/** @brief Definition of a column */
struct queue_column {
  /** @brief Column name */
  const char *name;

  /** @brief Compute value for this column */
  const char *(*value)(const struct queue_entry *q,
                       const char *data);

  /** @brief Passed to value() */
  const char *data;

  /** @brief Flags word */
  unsigned flags;
};

/** @brief Ellipsize column if too wide */
#define COL_ELLIPSIZE 0x0001

/** @brief Set expand property */
#define COL_EXPAND 0x0002

/** @brief Right-algin column */
#define COL_RIGHT 0x0004

/** @brief An item in the queue's popup menu */
struct queue_menuitem {
  /** @brief Menu item name */
  const char *name;

  /** @brief Called to activate the menu item */
  void (*activate)(GtkMenuItem *menuitem,
                   gpointer user_data);
  
  /** @brief Called to determine whether the menu item is usable.
   *
   * Returns @c TRUE if it should be sensitive and @c FALSE otherwise.
   */
  int (*sensitive)(struct queuelike *ql);

  /** @brief Signal handler ID */
  gulong handlerid;

  /** @brief Widget for menu item */
  GtkWidget *w;
};

/** @brief Definition of a queue-like window */
struct queuelike {

  /* Things filled in by the caller: */

  /** @brief Name for this tab */
  const char *name;
  
  /** @brief Initialization function */
  void (*init)(void);

  /** @brief Columns */
  const struct queue_column *columns;

  /** @brief Number of columns in this queuelike */
  int ncolumns;

  /** @brief Items for popup menu */
  struct queue_menuitem *menuitems;

  /** @brief Number of menu items */
  int nmenuitems;

  /* Dynamic state: */

  /** @brief The head of the queue */
  struct queue_entry *q;

  /* Things created by the implementation: */
  
  /** @brief The list store */
  GtkListStore *store;

  /** @brief The tree view */
  GtkWidget *view;

  /** @brief The selection */
  GtkTreeSelection *selection;
  
  /** @brief The popup menu */
  GtkWidget *menu;
};

extern struct queuelike ql_queue;
extern struct queuelike ql_recent;
extern struct queuelike ql_added;

extern time_t last_playing;

int ql_selectall_sensitive(struct queuelike *ql);
void ql_selectall_activate(GtkMenuItem *menuitem,
                           gpointer user_data);
int ql_selectnone_sensitive(struct queuelike *ql);
void ql_selectnone_activate(GtkMenuItem *menuitem,
                            gpointer user_data);
int ql_properties_sensitive(struct queuelike *ql);
void ql_properties_activate(GtkMenuItem *menuitem,
                            gpointer user_data);
int ql_scratch_sensitive(struct queuelike *ql);
void ql_scratch_activate(GtkMenuItem *menuitem,
                         gpointer user_data);
int ql_remove_sensitive(struct queuelike *ql);
void ql_remove_activate(GtkMenuItem *menuitem,
                        gpointer user_data);
int ql_play_sensitive(struct queuelike *ql);
void ql_play_activate(GtkMenuItem *menuitem,
                      gpointer user_data);
gboolean ql_button_release(GtkWidget *widget,
                           GdkEventButton *event,
                           gpointer user_data);
GtkWidget *init_queuelike(struct queuelike *ql);
void ql_update_list_store(struct queuelike *ql) ;
void ql_update_row(struct queue_entry *q,
                   GtkTreeIter *iter);
void ql_new_queue(struct queuelike *ql,
                  struct queue_entry *newq);
const char *column_when(const struct queue_entry *q,
                        const char *data);
const char *column_who(const struct queue_entry *q,
                       const char *data);
const char *column_namepart(const struct queue_entry *q,
                            const char *data);
const char *column_length(const struct queue_entry *q,
                          const char *data);
struct tabtype *ql_tabtype(struct queuelike *ql);
struct queue_entry *ql_iter_to_q(GtkTreeModel *model,
                                 GtkTreeIter *iter);

#endif /* QUEUE_GENERIC_H */

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
