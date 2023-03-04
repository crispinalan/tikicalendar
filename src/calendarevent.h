#ifndef CALENDAREVENT_H
#define CALENDAREVENT_H


#include <glib-object.h>


G_BEGIN_DECLS

#define CALENDAR_TYPE_EVENT (calendar_event_get_type ())

G_DECLARE_FINAL_TYPE (CalendarEvent, calendar_event, CALENDAR, EVENT, GObject)

//getters and setters

gint calendar_event_get_eventid(CalendarEvent *self);
void calendar_event_set_eventid(CalendarEvent *self, gint event_id);

const gchar* calendar_event_get_summary(CalendarEvent *self);
void calendar_event_set_summary(CalendarEvent *self, const gchar* summary);

const gchar* calendar_event_get_location(CalendarEvent *self);
void calendar_event_set_location(CalendarEvent *self, const gchar* location);

const gchar* calendar_event_get_description(CalendarEvent *self);
void calendar_event_set_description(CalendarEvent *self, const gchar* description);

gint calendar_event_get_start_year(CalendarEvent *self);
void calendar_event_set_start_year(CalendarEvent *self, gint start_year);

gint calendar_event_get_start_month(CalendarEvent *self);
void calendar_event_set_start_month(CalendarEvent *self, gint start_month);

gint calendar_event_get_start_day(CalendarEvent *self);
void calendar_event_set_start_day(CalendarEvent *self, gint start_day);

gint calendar_event_get_start_hour(CalendarEvent *self);
void calendar_event_set_start_hour(CalendarEvent *self, gint start_hour);

gint calendar_event_get_start_min(CalendarEvent *self);
void calendar_event_set_start_min(CalendarEvent *self, gint start_min);


gint calendar_event_get_end_year(CalendarEvent *self);
void calendar_event_set_end_year(CalendarEvent *self, gint end_year);

gint calendar_event_get_end_month(CalendarEvent *self);
void calendar_event_set_end_month(CalendarEvent *self, gint end_month);

gint calendar_event_get_end_day(CalendarEvent *self);
void calendar_event_set_end_day(CalendarEvent *self, gint end_day);

gint calendar_event_get_end_hour(CalendarEvent *self);
void calendar_event_set_end_hour(CalendarEvent *self, gint end_hour);

gint calendar_event_get_end_min(CalendarEvent *self);
void calendar_event_set_end_min(CalendarEvent *self, gint end_min);


gint calendar_event_get_is_yearly(CalendarEvent *self);
void calendar_event_set_is_yearly(CalendarEvent *self, gint is_yearly);

gint calendar_event_get_is_allday(CalendarEvent *self);
void calendar_event_set_is_allday(CalendarEvent *self, gint is_allday);

gint calendar_event_get_is_priority(CalendarEvent *self);
void calendar_event_set_is_priority(CalendarEvent *self, gint is_priority);

gint calendar_event_get_has_reminder(CalendarEvent *self);
void calendar_event_set_has_reminder(CalendarEvent *self, gint has_reminder);

gint calendar_event_get_reminder_min(CalendarEvent *self);
void calendar_event_set_reminder_min(CalendarEvent *self, gint reminder_mins);





G_END_DECLS

#endif //CALENDAREVENT_H
