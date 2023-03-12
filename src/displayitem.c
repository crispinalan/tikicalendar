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

#include "displayitem.h"

//define the class struct
struct _DisplayItem
{
    GObject parent_instance; //parent is GObject
    char *label; //label to be displayed in listview
    int id; //id
    int starttime;  //for sorting
};


G_DEFINE_TYPE (DisplayItem, display_item, G_TYPE_OBJECT)

enum
{
  PROP_LABEL = 1, //Prop label must start with 1
  PROP_ID,
  PROP_STARTTIME, //for sorting
  LAST_PROPERTY //the size of the GParamSpec properties array
};

static GParamSpec *properties[LAST_PROPERTY] = { NULL, };


static void display_item_class_init (DisplayItemClass *class)
{

	// GObjectClass is the class structure for the GObject type
	// G_OBJECT_CLASS casts the DisplayItemClass called class into a GObjectClass structure

	GObjectClass *object_class = G_OBJECT_CLASS (class);

	//the object_class has virtual functions called get_property and set_property
	//so you override these to provide your own getter and setter implementations

	//override the get_property of the object_class
	object_class->get_property = display_item_get_property;
	//override the set property of the object_class
	object_class->set_property = display_item_set_property;
	object_class->finalize = display_item_finalize;

	//property label used for listview display  and make it read writable
	properties[PROP_LABEL] = g_param_spec_string ("label", "label", "used for list view display",
												  NULL, G_PARAM_READWRITE);
	properties[PROP_ID] = g_param_spec_int ("id", "id", "id",
											0, G_MAXINT, 0, G_PARAM_READWRITE);

	properties[PROP_STARTTIME] = g_param_spec_int ("starttime", "starttime", "starttime",
												   0, G_MAXINT, 0, G_PARAM_READWRITE);


	//install the properties using the object_class, the number of proerties and the properties
	g_object_class_install_properties (object_class, LAST_PROPERTY, properties);
}

static void display_item_init (DisplayItem *self)
{
  
}

static void display_item_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec){


  DisplayItem *obj = (DisplayItem *)object;
  //property_id has to map to property enumeration
  switch (property_id)
    {
    case PROP_LABEL:
    g_value_set_string (value, obj->label);
    break;
    case PROP_ID:
    g_value_set_int (value, obj->id);
    break;
    case PROP_STARTTIME: //for sorting
    g_value_set_int (value, obj->starttime);
    break;
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
    }
}

static void display_item_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec   *pspec){

  DisplayItem *obj = (DisplayItem *)object;

  //setting the display object properties
  switch (property_id)
    {
    case PROP_LABEL:
      g_free (obj->label);
      obj->label = g_value_dup_string (value); //label defines what is displayed
      break;
    case PROP_ID:
      obj->id = g_value_get_int (value); //get the int from the GValue
      break;
     case PROP_STARTTIME:
      obj->starttime = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void display_item_finalize (GObject *obj)
{
  DisplayItem *object = (DisplayItem *)obj;

  g_free (object->label);

  G_OBJECT_CLASS (display_item_parent_class)->finalize (obj);
}






