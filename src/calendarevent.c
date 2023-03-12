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
#include "calendarevent.h"


struct _CalendarEvent
{
    GObject parent_instance; //parent
    gint    eventid; //field
    gchar* summary; //field
    gchar* location; //field
    gchar* description; //field

    gint    startyear; //field
    gint    startmonth; //field
    gint    startday; //field
    gint    starthour; //field
    gint    startmin; //field

    gint    endyear; //field
    gint    endmonth; //field
    gint    endday; //field
    gint    endhour; //field
    gint    endmin; //field

    gint    isyearly; //field
    gint    isallday; //field
    gint    ispriority; //field
    gint    hasreminder; //field
    gint    remindermin; //field
};

G_DEFINE_TYPE (CalendarEvent, calendar_event, G_TYPE_OBJECT);

//define properties
enum {
    PROP_0,
    PROP_EVENTID,
    PROP_SUMMARY,
    PROP_LOCATION,
    PROP_DESCRIPTION,
    PROP_STARTYEAR,
    PROP_STARTMONTH,
    PROP_STARTDAY,
    PROP_STARTHOUR,
    PROP_STARTMIN,
    PROP_ENDYEAR,
    PROP_ENDMONTH,
    PROP_ENDDAY,
    PROP_ENDHOUR,
    PROP_ENDMIN,
    PROP_ISYEARLY,
    PROP_ISALLDAY,
    PROP_ISPRIORITY,
    PROP_HASREMINDER,
    PROP_REMINDERMIN,
    LAST_PROP
};


static GParamSpec *properties[LAST_PROP];


static void calendar_event_get_property(GObject *object,
                                        guint   prop_id,
                                        GValue  *value,
                                        GParamSpec *pspec)
{
    CalendarEvent *self =(CalendarEvent *)object;

    switch (prop_id)
    {
        case PROP_EVENTID:
             g_value_set_int(value, calendar_event_get_eventid(self)); //boxing
            break;
        case PROP_SUMMARY:
            g_value_set_string(value, calendar_event_get_summary(self)); //boxing
            break;
         case PROP_LOCATION:
            g_value_set_string(value, calendar_event_get_location(self)); //boxing
            break;
         case PROP_DESCRIPTION:
            g_value_set_string(value, calendar_event_get_description(self)); //boxing
            break;
         case PROP_STARTYEAR:
             g_value_set_int(value, calendar_event_get_start_year(self)); //boxing
            break;
          case PROP_STARTMONTH:
             g_value_set_int(value, calendar_event_get_start_month(self)); //boxing
            break;
         case PROP_STARTDAY:
             g_value_set_int(value, calendar_event_get_start_day(self)); //boxing
            break;
         case PROP_STARTHOUR:
             g_value_set_int(value, calendar_event_get_start_hour(self)); //boxing
            break;
        case PROP_STARTMIN:
             g_value_set_int(value, calendar_event_get_start_min(self)); //boxing
            break;

         case PROP_ENDYEAR:
             g_value_set_int(value, calendar_event_get_end_year(self)); //boxing
            break;
          case PROP_ENDMONTH:
             g_value_set_int(value, calendar_event_get_end_month(self)); //boxing
            break;
         case PROP_ENDDAY:
             g_value_set_int(value, calendar_event_get_end_day(self)); //boxing
            break;
         case PROP_ENDHOUR:
             g_value_set_int(value, calendar_event_get_end_hour(self)); //boxing
            break;
        case PROP_ENDMIN:
             g_value_set_int(value, calendar_event_get_end_min(self)); //boxing
            break;

            //--------

         case PROP_ISYEARLY:
             g_value_set_int(value, calendar_event_get_is_yearly(self)); //boxing
            break;
          case PROP_ISALLDAY:
             g_value_set_int(value, calendar_event_get_is_allday(self)); //boxing
            break;
         case PROP_ISPRIORITY:
             g_value_set_int(value, calendar_event_get_is_priority(self)); //boxing
            break;
         case PROP_HASREMINDER:
             g_value_set_int(value, calendar_event_get_has_reminder(self)); //boxing
            break;
        case PROP_REMINDERMIN:
             g_value_set_int(value, calendar_event_get_reminder_min(self)); //boxing
            break;



    }
}

static void calendar_event_set_property(GObject *object,
                                        guint   prop_id,
                                        const GValue  *value,
                                        GParamSpec *pspec)
{
    CalendarEvent *self =(CalendarEvent *)object;

    switch (prop_id)
    {
        case PROP_EVENTID:
            calendar_event_set_eventid(self, g_value_get_int(value));
            break;
        case PROP_SUMMARY:
            calendar_event_set_summary(self, g_value_get_string(value));
            break;
         case PROP_LOCATION:
            calendar_event_set_location(self, g_value_get_string(value));
            break;
         case PROP_DESCRIPTION:
            calendar_event_set_description(self, g_value_get_string(value));
            break;
          case PROP_STARTYEAR:
            calendar_event_set_start_year(self, g_value_get_int(value));
            break;
          case PROP_STARTMONTH:
            calendar_event_set_start_month(self, g_value_get_int(value));
            break;
         case PROP_STARTDAY:
            calendar_event_set_start_day(self, g_value_get_int(value));
            break;
        case PROP_STARTHOUR:
            calendar_event_set_start_hour(self, g_value_get_int(value));
            break;
         case PROP_STARTMIN:
            calendar_event_set_start_min(self, g_value_get_int(value));
            break;
         case PROP_ENDYEAR:
            calendar_event_set_end_year(self, g_value_get_int(value));
            break;
          case PROP_ENDMONTH:
            calendar_event_set_end_month(self, g_value_get_int(value));
            break;
         case PROP_ENDDAY:
            calendar_event_set_end_day(self, g_value_get_int(value));
            break;
        case PROP_ENDHOUR:
            calendar_event_set_end_hour(self, g_value_get_int(value));
            break;
         case PROP_ENDMIN:
            calendar_event_set_end_min(self, g_value_get_int(value));
            break;

            case PROP_ISYEARLY:
            calendar_event_set_is_yearly(self, g_value_get_int(value));
            break;
          case PROP_ISALLDAY:
            calendar_event_set_is_allday(self, g_value_get_int(value));
            break;
         case PROP_ISPRIORITY:
            calendar_event_set_is_priority(self, g_value_get_int(value));
            break;
        case PROP_HASREMINDER:
            calendar_event_set_has_reminder(self, g_value_get_int(value));
            break;
         case PROP_REMINDERMIN:
            calendar_event_set_reminder_min(self, g_value_get_int(value));
            break;

    }
}



static void calendar_event_class_init (CalendarEventClass *klass)
{
    //make class constructor static meaning not available outside this class

    GObjectClass *object_class =G_OBJECT_CLASS(klass);

    //override these
    object_class->get_property = calendar_event_get_property;
    object_class->set_property = calendar_event_set_property;

    //properties
    properties[PROP_EVENTID] =
    g_param_spec_int("eventid",
                     "eventid",
                     "The event id",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);


    properties[PROP_SUMMARY] =
    g_param_spec_string("summary",
                        "Summary",
                        "The event summary",
                        NULL,
                        (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));


    properties[PROP_LOCATION] =
    g_param_spec_string("location",
                        "Location",
                        "The event location",
                        NULL,
                        (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    properties[PROP_DESCRIPTION] =
    g_param_spec_string("description",
                        "Description",
                        "The event description",
                        NULL,
                        (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    properties[PROP_STARTYEAR] =
    g_param_spec_int("startyear",
                     "startyear",
                     "The event start year",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_STARTMONTH] =
    g_param_spec_int("startmonth",
                     "startmonth",
                     "The event start month",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_STARTDAY] =
    g_param_spec_int("startday",
                     "startday",
                     "The event start day",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_STARTHOUR] =
    g_param_spec_int("starthour",
                     "starthour",
                     "The event start hour",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_STARTMIN] =
    g_param_spec_int("startmin",
                     "startmin",
                     "The event start minute",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);


    properties[PROP_ENDYEAR] =
    g_param_spec_int("endyear",
                     "endyear",
                     "The event end year",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_ENDMONTH] =
    g_param_spec_int("endmonth",
                     "endmonth",
                     "The event end month",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_ENDDAY] =
    g_param_spec_int("endday",
                     "endday",
                     "The event end day",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_ENDHOUR] =
    g_param_spec_int("endhour",
                     "endhour",
                     "The event end hour",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_ENDMIN] =
    g_param_spec_int("endmin",
                     "endmin",
                     "The event end minute",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    //-------------

    properties[PROP_ISYEARLY] =
    g_param_spec_int("isyearly",
                     "isyearly",
                     "The event repeats yearly",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_ISALLDAY] =
    g_param_spec_int("isallday",
                     "isallday",
                     "This is an all day event",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_ISPRIORITY] =
    g_param_spec_int("ispriority",
                     "ispriority",
                     "The event has high priority",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_HASREMINDER] =
    g_param_spec_int("hasreminder",
                     "hasreminder",
                     "The event has a reminder",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);

    properties[PROP_REMINDERMIN] =
    g_param_spec_int("remindermin",
                     "remindermin",
                     "The reminder minutes",
                     0,G_MAXINT,0,
                     G_PARAM_READWRITE);


    g_object_class_install_properties(object_class, LAST_PROP, properties);


}

static void calendar_event_init (CalendarEvent *self)
{
  
}


//getters and setters

gint calendar_event_get_eventid(CalendarEvent *self){
    return self->eventid;
}

void calendar_event_set_eventid(CalendarEvent *self, gint event_id)
{
    self->eventid =event_id;
}


const gchar* calendar_event_get_summary(CalendarEvent *self){

    return self->summary;

}
void calendar_event_set_summary(CalendarEvent *self, const gchar* summary){


    if(g_strcmp0(summary, self->summary))
    { //check not the same

        g_free(self->summary);
        self->summary =g_strdup(summary);
    }

}


const gchar* calendar_event_get_location(CalendarEvent *self){

    return self->location;

}


void calendar_event_set_location(CalendarEvent *self, const gchar* location){


    if(g_strcmp0(location, self->location))
    { //check not the same

        g_free(self->location);
        self->location =g_strdup(location);
    }

}

const gchar* calendar_event_get_description(CalendarEvent *self){

    return self->description;

}


void calendar_event_set_description(CalendarEvent *self, const gchar* description){


    if(g_strcmp0(description, self->description))
    { //check not the same

        g_free(self->description);
        self->description =g_strdup(description);
    }

}


gint calendar_event_get_start_year(CalendarEvent *self){
    return self->startyear;
}
void calendar_event_set_start_year(CalendarEvent *self, gint start_year)
{
    self->startyear =start_year;
}

gint calendar_event_get_start_month(CalendarEvent *self){
    return self->startmonth;
}
void calendar_event_set_start_month(CalendarEvent *self, gint start_month)
{
    self->startmonth =start_month;
}

gint calendar_event_get_start_day(CalendarEvent *self){
    return self->startday;
}
void calendar_event_set_start_day(CalendarEvent *self, gint start_day)
{
    self->startday =start_day;
}

gint calendar_event_get_start_hour(CalendarEvent *self){
    return self->starthour;
}
void calendar_event_set_start_hour(CalendarEvent *self, gint start_hour)
{
    self->starthour =start_hour;
}

gint calendar_event_get_start_min(CalendarEvent *self){
    return self->startmin;
}
void calendar_event_set_start_min(CalendarEvent *self, gint start_min)
{
    self->startmin =start_min;
}

//--------------------------------------------------

gint calendar_event_get_end_year(CalendarEvent *self){
    return self->endyear;
}
void calendar_event_set_end_year(CalendarEvent *self, gint end_year)
{
    self->endyear =end_year;
}

gint calendar_event_get_end_month(CalendarEvent *self){
    return self->endmonth;
}
void calendar_event_set_end_month(CalendarEvent *self, gint end_month)
{
    self->endmonth =end_month;
}

gint calendar_event_get_end_day(CalendarEvent *self){
    return self->endday;
}
void calendar_event_set_end_day(CalendarEvent *self, gint end_day)
{
    self->endday =end_day;
}

gint calendar_event_get_end_hour(CalendarEvent *self){
    return self->endhour;
}
void calendar_event_set_end_hour(CalendarEvent *self, gint end_hour)
{
    self->endhour =end_hour;
}

gint calendar_event_get_end_min(CalendarEvent *self){
    return self->endmin;
}
void calendar_event_set_end_min(CalendarEvent *self, gint end_min)
{
    self->endmin =end_min;
}

//----------------------------------------------------------------

gint calendar_event_get_is_yearly(CalendarEvent *self){
    return self->isyearly;
}
void calendar_event_set_is_yearly(CalendarEvent *self, gint is_yearly)
{
    self->isyearly =is_yearly;
}

gint calendar_event_get_is_allday(CalendarEvent *self){
    return self->isallday;
}
void calendar_event_set_is_allday(CalendarEvent *self, gint is_allday)
{
    self->isallday =is_allday;
}

gint calendar_event_get_is_priority(CalendarEvent *self){
    return self->ispriority;
}
void calendar_event_set_is_priority(CalendarEvent *self, gint is_priority)
{
    self->ispriority =is_priority;
}

gint calendar_event_get_has_reminder(CalendarEvent *self){
    return self->hasreminder;
}
void calendar_event_set_has_reminder(CalendarEvent *self, gint has_reminder)
{
    self->hasreminder =has_reminder;
}

gint calendar_event_get_reminder_min(CalendarEvent *self){
    return self->remindermin;
}
void calendar_event_set_reminder_min(CalendarEvent *self, gint reminder_min)
{
    self->remindermin =reminder_min;
}
