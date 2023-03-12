/***************************************************************************
 *   Author Alan Crispin                                                   *
 *   crispinalan@gmail.com                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation.                                         *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
#ifndef DISPLAYITEM_H
#define DISPLAYITEM_H


#include <gtk/gtk.h>

G_BEGIN_DECLS

#define DISPLAY_TYPE_ITEM (display_item_get_type ())

G_DECLARE_FINAL_TYPE (DisplayItem, display_item, DISPLAY, ITEM, GObject)

static void display_item_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void display_item_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void display_item_finalize (GObject *obj);

G_END_DECLS

#endif //DISPLAYITEM_H
