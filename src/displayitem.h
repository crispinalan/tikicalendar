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
