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

/*
 * Use MAKEFILE to compile
 *
*/

#include <gtk/gtk.h>
#include <glib/gstdio.h>  //needed for g_mkdir
#include <math.h>  //compile with -lm

#include "displayitem.h"
#include "calendarevent.h"

#define CONFIG_DIRNAME "tikical-gtk4"
#define CONFIG_FILENAME "tikical-config-026"

static GMutex lock;

//declarations

static void create_header (GtkWindow *window);

//listbox display
static GtkWidget *create_widget (gpointer item, gpointer user_data);
static void add_separator (GtkListBoxRow *row, GtkListBoxRow *before, gpointer data);
static void callbk_row_activated (GtkListBox  *listbox,	 GtkListBoxRow *row, gpointer user_data);

//helpers
gchar* convert_number_to_cardinal_string(int number);
gchar* convert_number_to_ordinal_string(int number);


int compare (const void * a, const void * b);
gboolean check_day_events_for_overlap();


//gchar* get_css_string();
GDate* calculate_easter(gint year);
gboolean is_public_holiday(int day);
char* get_holiday_str(int day);

int get_number_of_events();

//helpers
static char* remove_semicolons (const char *text);
static char* remove_punctuations(const char *text);

//Event Dialogs
static void callbk_check_button_allday_toggled (GtkCheckButton *check_button, gpointer user_data);
static void callbk_add(GtkButton *button, gpointer  user_data);
static void callbk_new_event(GtkButton *button, gpointer  user_data);
static void callbk_update(GtkButton *button, gpointer  user_data);
static void callbk_edit_event(GtkButton *button, gpointer  user_data);


//Speak

static void speak_events();
//static void speak_events(gpointer  user_data);
static void speak_month_events(gpointer  user_data);
static void callbk_speak(GSimpleAction* action, GVariant *parameter,gpointer user_data);
static void callbk_speak_month(GSimpleAction* action, GVariant *parameter,gpointer user_data);

//Actions

static void callbk_about(GSimpleAction* action, GVariant *parameter, gpointer user_data);
static void callbk_info(GSimpleAction *action, GVariant *parameter,  gpointer user_data);
static void callbk_preferences(GSimpleAction* action, GVariant *parameter,gpointer user_data);
static void callbk_home(GSimpleAction* action, GVariant *parameter, gpointer user_data);
static void callbk_delete(GSimpleAction* action, GVariant *parameter,  gpointer user_data);
static void callbk_quit(GSimpleAction* action,G_GNUC_UNUSED GVariant *parameter, gpointer user_data);
static void callbk_delete_selected(GtkButton *button, gpointer  user_data);

//calendar callbks
static void callbk_calendar_day_selected(GtkCalendar *calendar, gpointer user_data);
static void callbk_calendar_next_month(GtkCalendar *calendar, gpointer user_data);
static void callbk_calendar_prev_month(GtkCalendar *calendar, gpointer user_data);
static void callbk_calendar_next_year(GtkCalendar *calendar, gpointer user_data);
static void callbk_calendar_prev_year(GtkCalendar *calendar, gpointer user_data);

static void update_date(GtkCalendar *calendar, gpointer user_data);

static void display_events(int year, int month, int day);
static int compare_times (gconstpointer a, gconstpointer b, gpointer data);


void add_calendar_marks_for_current_month(GtkCalendar *calendar);

//helpers
gchar* convert_number_to_cardinal_string(int number);
gchar* convert_number_to_ordinal_string(int number);


void save_csv_file(); //need a new approach
void load_csv_file();
void reload_csv_file();
int file_exists(const char *file_name);

//config
static char * m_config_file = NULL;

static GListStore *m_store=NULL;   //m_store is a Gio GListStore store
GArray *evt_arry; //event g_array

//int max_records=5000;
//CalendarEvent *event_store=NULL;

static int m_id_selection=-1;
static int m_row_index=-1; //listbox row index

static int m_store_size=0;

//calendar
static int m_12hour_format=1; //am pm hour format
static int m_show_end_time=0; //show end_time
static int m_show_location=0; //show location
static int m_holidays=1; //holidays

static int m_talk =1;
static int m_talk_at_startup =1;
static int m_talk_event_number=1;
static int m_talk_time=1;
static int m_talk_location=1;
static int m_talk_priority=1;

static int m_talk_month_event_dates=1;

static int m_speed = 150; //espeak words per minute

static int m_reset_preferences=0;


static int m_today_year=0;
static int m_today_month=0;
static int m_today_day=0;


//locals member variables

static const char* m_summary ="summary";
static const char* m_location ="";
static const char* m_description ="todo";
static int m_start_year=0;
static int m_start_month=0;
static int m_start_day=0;
static float m_start_time=0.0; //float
static int m_start_hour=0;
static int m_start_min=0;
static int m_end_year=0; //multiday plumbing (not yet implemented)
static int m_end_month=0;
static int m_end_day=0;
static float m_end_time=0.0; //float
static int m_end_hour=0;
static int m_end_min=0;
static int m_priority=0;
static int m_is_yearly=0;
static int m_is_allday=0;


static int m_has_reminder=0; //plumbing
static int m_reminder_min=0;

//---------------------------------------------------------------------
// map menu actions to callbacks

const GActionEntry app_actions[] = {
  { "speak", callbk_speak },
  { "month_speak", callbk_speak_month},
  { "home", callbk_home}
};


//-------------------------------------------------------------------------------
// Save load config file
//-------------------------------------------------------------------------------

static void config_load_default()
{
	//talking
	m_talk=1;
	m_talk_at_startup=1;
	m_talk_event_number=1;
	m_talk_time=1;
	m_talk_location=0;
	m_talk_priority=0;

	//calendar
	m_12hour_format=1;
	m_show_end_time=0;
	m_show_location=1;
	m_holidays=1;

}

static void config_read()
{
	// Clean up previously loaded configuration values
	m_talk=1;
	m_talk_at_startup=1;
	m_talk_event_number=1;
	m_talk_time=1;
	m_talk_location=0;
	m_talk_priority=0;

	m_12hour_format=1;
	m_show_end_time=0;
	m_show_location=1;
	m_holidays=1;


	// Load keys from keyfile
	GKeyFile * kf = g_key_file_new();
	g_key_file_load_from_file(kf, m_config_file, G_KEY_FILE_NONE, NULL);

	//talk
	m_talk = g_key_file_get_integer(kf, "calendar_settings", "talk", NULL);
	m_talk_at_startup=g_key_file_get_integer(kf, "calendar_settings", "talk_startup", NULL);
	m_talk_event_number=g_key_file_get_integer(kf, "calendar_settings", "talk_event_number", NULL);
	m_talk_time=g_key_file_get_integer(kf, "calendar_settings", "talk_time", NULL);
	m_talk_location=g_key_file_get_integer(kf, "calendar_settings", "talk_location", NULL);
	m_talk_priority=g_key_file_get_integer(kf, "calendar_settings", "talk_priority", NULL);

	//calendar
	m_12hour_format=g_key_file_get_integer(kf, "calendar_settings", "hour_format", NULL);
	m_show_end_time = g_key_file_get_integer(kf, "calendar_settings", "show_end_time", NULL);
	m_show_location = g_key_file_get_integer(kf, "calendar_settings", "show_location", NULL);

	m_holidays = g_key_file_get_integer(kf, "calendar_settings", "holidays", NULL);

	g_key_file_free(kf);

}

void config_write()
{

	GKeyFile * kf = g_key_file_new();

	//talk
	g_key_file_set_integer(kf, "calendar_settings", "talk", m_talk);
	g_key_file_set_integer(kf, "calendar_settings", "talk_startup", m_talk_at_startup);
	g_key_file_set_integer(kf, "calendar_settings", "talk_event_number", m_talk_event_number);
	g_key_file_set_integer(kf, "calendar_settings", "talk_time", m_talk_time);
	g_key_file_set_integer(kf, "calendar_settings", "talk_location", m_talk_location);
	g_key_file_set_integer(kf, "calendar_settings", "talk_priority", m_talk_priority);

	g_key_file_set_integer(kf, "calendar_settings", "hour_format", m_12hour_format);
	g_key_file_set_integer(kf, "calendar_settings", "show_end_time", m_show_end_time);
	g_key_file_set_integer(kf, "calendar_settings", "show_location", m_show_location);

	g_key_file_set_integer(kf, "calendar_settings", "holidays", m_holidays);


	gsize length;
	gchar * data = g_key_file_to_data(kf, &length, NULL);
	g_file_set_contents(m_config_file, data, -1, NULL);
	g_free(data);
	g_key_file_free(kf);

}

void config_initialize() {

	gchar * config_dir = g_build_filename(g_get_user_config_dir(), CONFIG_DIRNAME, NULL);
	m_config_file = g_build_filename(config_dir, CONFIG_FILENAME, NULL);

	// Make sure config directory exists
	if(!g_file_test(config_dir, G_FILE_TEST_IS_DIR))
		g_mkdir(config_dir, 0777);
	    //g_mkdir(config_dir, 0666);
	// If a config file doesn't exist, create one with defaults otherwise

	// read the existing one
	if(!g_file_test(m_config_file, G_FILE_TEST_EXISTS))
	{
		config_load_default();
		config_write();
	}
	else
	{
		config_read();
	}

	g_free(config_dir);

}

//-------------------------------------------------------------------------------
// Debugging helper functions
//--------------------------------------------------------------------------------

void print_array(GArray *a) {

	gint evt_id=0;
	gchar *summary_str="";
	gchar *location_str="";

	gint start_year=0;
	gint start_month=0;
	gint start_day=0;
	gint start_hour=0;
	gint start_min=0;

	gchar *line="";

    g_print("Array contents\n");

    for (int i=0; i<a->len; i++) {

		line="";

		CalendarEvent *evt =g_array_index(a, CalendarEvent*, i);

		g_object_get (evt, "eventid", &evt_id, NULL);
		g_object_get (evt, "summary", &summary_str, NULL);
		g_object_get (evt, "location", &location_str, NULL);

		g_object_get (evt, "startyear", &start_year, NULL);
		g_object_get (evt, "startmonth", &start_month, NULL);
		g_object_get (evt, "startday", &start_day, NULL);
		g_object_get (evt, "starthour", &start_hour, NULL);
		g_object_get (evt, "startmin", &start_min, NULL);

		gchar *id_str = g_strdup_printf("%d", evt_id);

		gchar *start_year_str = g_strdup_printf("%d", start_year);
		gchar *start_month_str = g_strdup_printf("%d", start_month);
		gchar *start_day_str = g_strdup_printf("%d", start_day);
		gchar *start_hour_str = g_strdup_printf("%d", start_hour);
		gchar *start_min_str = g_strdup_printf("%d", start_min);


		line= g_strconcat(
			line,
			id_str," ",
			start_day_str,"-",
			start_month_str,"-",
			start_year_str ," ",
			start_hour_str,":",
			start_min_str," ",
			summary_str," ",
			location_str," ",
			NULL);

		g_print("%s\n",line);

    }
}
//---------------------------------------------------------------------
// Debugging GlistStore helper functions
//---------------------------------------------------------------------

void print_gliststore(GListStore *store) {

    guint n_items;
    GListModel *model;
    gint id_value;
    gchar* label_value;
    g_print("ListStore contents\n");

    model = G_LIST_MODEL (store);
    n_items = g_list_model_get_n_items (G_LIST_MODEL (model));

    for (int i=0; i<n_items; i++) {
        DisplayItem *test = g_list_model_get_item (model, i);
        g_object_get (test, "id", &id_value, NULL);
        g_object_get (test, "label", &label_value, NULL);
        g_print("store at index %d: id = %d label =%s\n",i, id_value, label_value);
    }
}

gchar* get_gliststore_labels(GListStore *store) {


	gchar* events_str="";
	guint n_items;
    GListModel *model;
    gint id_value;
    gchar* label_value;
    //g_print("ListStore contents\n");

    model = G_LIST_MODEL (store);
    n_items = g_list_model_get_n_items (G_LIST_MODEL (model));

    for (int i=0; i<n_items; i++) {
        DisplayItem *test = g_list_model_get_item (model, i);
        g_object_get (test, "id", &id_value, NULL);
        g_object_get (test, "label", &label_value, NULL);
       // g_print("store at index %d: id = %d label =%s\n",i, id_value, label_value);

		events_str=g_strconcat(events_str, " ", label_value, " ",NULL);
    }

	return events_str;
}

int get_gliststore_number(GListStore *store) {

	guint n_items;
    GListModel *model;
    model = G_LIST_MODEL (store);
    n_items = g_list_model_get_n_items (G_LIST_MODEL (model));

	return n_items;

}

//--------------------------------------------------------------------
// Remove unwanted characters
//--------------------------------------------------------------------

static char* remove_semicolons (const char *text)
{
	GString *str;
	const char *p;
	str = g_string_new ("");
	p = text;
	while (*p)
	{
	gunichar cp = g_utf8_get_char(p);
	if ( cp != ';' ){ //do not allow semicolons into database
	g_string_append_unichar (str, *p);
	}//if
	++p;
	}

	return g_string_free (str, FALSE);
}

static char* remove_punctuations(const char *text)
{
	GString *str;
	const char *p;
	str = g_string_new ("");
	p = text;
	while (*p)
	{
	gunichar cp = g_utf8_get_char(p);
	if ( cp != '\'' ){ //remove all apostrophes as cause tts errors
	g_string_append_unichar (str, *p);
	}//if
	++p;
	}

	return g_string_free (str, FALSE);
}


// static void callbk_print_array(GSimpleAction* action, GVariant *parameter,gpointer user_data)
// {
//
// 	//GtkWidget *window =user_data;
// 	print_array(evt_arry);
//
// }
//---------------------------------------------------------------------------------
//--------------------------------------------------------------------
// calendar functions
//---------------------------------------------------------------------

static void mark_holidays_on_calendar(GtkCalendar *calendar, gint month, gint year)
{

	//g_print("Entering mark holidays on calendar\n");

	//gtk_calendar_clear_marks(GTK_CALENDAR(calendar));

	if (month==1) {
	gtk_calendar_mark_day(calendar,1); //day =1 //new year
   }

   GDate *edate =calculate_easter(year);
   gint eday = g_date_get_day(edate);
   gint emonth =g_date_get_month(edate);

   if(month==emonth)
   {
	   gtk_calendar_mark_day(calendar,eday);
   }

   g_date_subtract_days(edate,2);
   gint efday = g_date_get_day(edate); //ef=easter friday
   gint efmonth =g_date_get_month(edate);

   if(month==efmonth)
   {
	   gtk_calendar_mark_day(calendar,efday);
   }

   g_date_add_days(edate,3);
   gint emonday = g_date_get_day(edate); //easter monday
   gint emmonth =g_date_get_month(edate);

   if(month==emmonth)
   {
	   gtk_calendar_mark_day(calendar,emonday);
   }

   if (month==12) {

	  gtk_calendar_mark_day(calendar, 25); //christmas day
	  gtk_calendar_mark_day(calendar, 26); //boxing
	}
}


void set_event_marks_on_calendar(GtkCalendar *calendar) {

	gtk_calendar_clear_marks(GTK_CALENDAR (calendar));

	for (int i=0; i<evt_arry->len; i++) {

		gint start_year=0;
		gint start_month=0;
		gint start_day=0;
		gint is_yearly=0;
		gchar *day_str="";
		gchar *mark_str="";

		CalendarEvent *evt =g_array_index(evt_arry, CalendarEvent*, i);

		g_object_get (evt, "startyear", &start_year, NULL);
		g_object_get (evt, "startmonth", &start_month, NULL);
		g_object_get (evt, "startday", &start_day, NULL);
		g_object_get (evt, "isyearly", &is_yearly, NULL);

		if(m_start_year==start_year && m_start_month ==start_month)
		{

			day_str = g_strdup_printf("%d",start_day);
			mark_str =g_strconcat(mark_str,"Adding mark for day ",day_str,"  ",NULL);
			//g_print("%s\n",mark_str);
			gtk_calendar_mark_day(GTK_CALENDAR(calendar), start_day);
		}


	} //for evt_arry

}

gchar* get_marks_on_calendar(GtkCalendar *calendar) {

	//for testing
	gboolean marked_day=FALSE;
	gint mark_count =0;
	gchar *day_str="";
	gchar *result_str="";
	//get days in month
	int num_month_days =g_date_get_days_in_month (m_start_month, m_start_year);
	//g_print("Days in month %d = %d\n",m_start_month, num_month_days);

	for (int day=1; day<=num_month_days; day++)
	{
		if(gtk_calendar_get_day_is_marked(GTK_CALENDAR(calendar),day)) {

			mark_count++;
			//gchar *day_str = g_strdup_printf("%d",day);
			//str =g_strconcat(str,"Day ",day_str," is marked\n",NULL);
			day_str =g_strconcat(day_str, convert_number_to_ordinal_string(day), " ", NULL);
		}

	}

	if (mark_count==0) {
		result_str = "No events this month ";
	}
	else if(mark_count==1) {
		result_str=g_strconcat(result_str, "You have one event this month on the ", day_str," ", NULL);
	}
	else {

		//gchar *mark_count_str = g_strdup_printf("%d",mark_count);
		result_str=g_strconcat(result_str, "You have events on the ", day_str," this month ", NULL);
	}

	//g_print("%s\n",result_str);

	return result_str;
}

int  get_total_number_of_events(){

	return evt_arry->len;
}

int  get_number_of_day_events(){

	int event_count=0;

	for (int i=0; i<evt_arry->len; i++) {

		gint start_year=0;
		gint start_month=0;
		gint start_day=0;
		gint is_yearly=0;

		CalendarEvent *evt =g_array_index(evt_arry, CalendarEvent*, i);

		g_object_get (evt, "startyear", &start_year, NULL);
		g_object_get (evt, "startmonth", &start_month, NULL);
		g_object_get (evt, "startday", &start_day, NULL);
		g_object_get (evt, "isyearly", &is_yearly, NULL);

		//normal events
		if(m_start_year==start_year && m_start_month ==start_month && m_start_day==start_day)
		{
			event_count++;
		}//if
		//repeat year events
		if(m_start_year!= start_year && m_start_month ==start_month && m_start_day==start_day && is_yearly==1)
		{
			event_count++;
		}

	}//for

	return event_count;

}

//---------------------------------------------------------------------
// calculate easter
//---------------------------------------------------------------------

GDate* calculate_easter(gint year) {

	GDate *edate;

	gint Yr = year;
    gint a = Yr % 19;
    gint b = Yr / 100;
    gint c = Yr % 100;
    gint d = b / 4;
    gint e = b % 4;
    gint f = (b + 8) / 25;
    gint g = (b - f + 1) / 3;
    gint h = (19 * a + b - d - g + 15) % 30;
    gint i = c / 4;
    gint k = c % 4;
    gint L = (32 + 2 * e + 2 * i - h - k) % 7;
    gint m = (a + 11 * h + 22 * L) / 451;
    gint month = (h + L - 7 * m + 114) / 31;
    gint day = ((h + L - 7 * m + 114) % 31) + 1;
	edate = g_date_new_dmy(day, month, year);

	return edate;
}
//--------------------------------------------------------------------
// public holidays
//---------------------------------------------------------------------
gboolean is_public_holiday(int day) {

// UK public holidays
// New Year's Day: 1 January (DONE)
// Good Friday: March or April  (DONE)
// Easter Monday: March or April (DONE)
// Early May: First Monday of May (DONE)
// Spring Bank Holiday: Last Monday of May (DONE)
// Summer Bank Holiday: Last Monday of August (DONE)
// Christmas Day: 25 December (DONE)
// Boxing day: 26 December (DONE)

	//markup public holidays
	if (m_start_month==1 && day ==1) {
	//new year
	 return TRUE;
	}

	if (m_start_month==12 && day==25) {
	//christmas day
	return TRUE;
	}

	if (m_start_month==12 && day==26) {
	//boxing day
	return TRUE;
	}

	if (m_start_month == 5) {
     //May complicated
     GDate *first_monday_may;
     first_monday_may = g_date_new_dmy(1, m_start_month, m_start_year);

     while (g_date_get_weekday(first_monday_may) != G_DATE_MONDAY)
       g_date_add_days(first_monday_may,1);

     int may_day = g_date_get_day(first_monday_may);

     if( day==may_day) return TRUE;
     //else return FALSE;

     int days_in_may =g_date_get_days_in_month (m_start_month, m_start_year);
     int plus_days = 0;

     if (may_day + 28 <= days_in_may) {
       plus_days = 28;
     } else {
       plus_days = 21;
     }

     GDate *spring_bank =g_date_new_dmy (may_day, m_start_month, m_start_year);

     g_date_add_days(spring_bank,plus_days);

     int spring_bank_day = g_date_get_day(spring_bank);

     if (g_date_valid_dmy (spring_bank_day,m_start_month,m_start_year) && day ==spring_bank_day)
     return TRUE;
	} //m_start_month==5 (may)

	GDate *easter_date =calculate_easter(m_start_year);
	int easter_day = g_date_get_day(easter_date);
	int easter_month =g_date_get_month(easter_date);

	if(m_start_month==easter_month && day == easter_day)
	{
	//easter day
	return TRUE;
	}

	g_date_subtract_days(easter_date,2);
	int easter_friday = g_date_get_day(easter_date);
	int easter_friday_month =g_date_get_month(easter_date);

	if(m_start_month==easter_friday_month && day ==easter_friday)
	{
	//easter friday
	return TRUE;
	}

	g_date_add_days(easter_date,3);
	int easter_monday = g_date_get_day(easter_date); //easter monday
	int easter_monday_month =g_date_get_month(easter_date);

	if(m_start_month==easter_monday_month && day ==easter_monday)
	{
	//easter monday
	return TRUE;
	}

	if (m_start_month == 8) {
      //August complicated
    GDate *first_monday_august;
     first_monday_august = g_date_new_dmy(1, m_start_month, m_start_year);

     while (g_date_get_weekday(first_monday_august) != G_DATE_MONDAY)
       g_date_add_days(first_monday_august,1);

     int august_day = g_date_get_day(first_monday_august);


     int days_in_august =g_date_get_days_in_month (m_start_month, m_start_year);
     int plus_days = 0;

     if (august_day + 28 <= days_in_august) {
       plus_days = 28;
     } else {
       plus_days = 21;
     }

     GDate *august_bank =g_date_new_dmy (august_day, m_start_month, m_start_year);

     g_date_add_days(august_bank,plus_days);

     int august_bank_day = g_date_get_day(august_bank);

     if (g_date_valid_dmy (august_bank_day,m_start_month,m_start_year) && day ==august_bank_day)
     return TRUE;


    } //m_start_month==8

	return FALSE;
}

char* get_holiday_str(int day) {

// UK public holidays
// New Year's Day: 1 January (DONE)
// Good Friday: March or April  (DONE)
// Easter Monday: March or April (DONE)
// Early May: First Monday of May (TODO)
// Spring Bank Holiday: Last Monday of May (DONE)
// Summer Bank Holiday: Last Monday of August (DONE)
// Christmas Day: 25 December (DONE)
// Boxing day: 26 December (DONE)

	//markup public holidays
	if (m_start_month==1 && day ==1) {
	return "New Year";
	}

	if (m_start_month==12 && day==25) {
	//christmas day
	return "Christmas Day";
	}

	if (m_start_month==12 && day==26) {
	//boxing day
	return "Boxing Day";
	}

	if (m_start_month == 5) {
     //May complicated
     GDate *first_monday_may;
     first_monday_may = g_date_new_dmy(1, m_start_month, m_start_year);


     while (g_date_get_weekday(first_monday_may) != G_DATE_MONDAY)
       g_date_add_days(first_monday_may,1);

     int may_day = g_date_get_day(first_monday_may);

     if( day==may_day) return "Public Holiday"; //may bank holiday

     int days_in_may =g_date_get_days_in_month (m_start_month, m_start_year);

     int plus_days = 0;

     if (may_day + 28 <= days_in_may) {
       plus_days = 28;
     } else {
       plus_days = 21;
     }

     GDate *spring_bank =g_date_new_dmy (may_day, m_start_month, m_start_year);
     g_date_add_days(spring_bank,plus_days);
     int spring_bank_day = g_date_get_day(spring_bank);
     if (g_date_valid_dmy (spring_bank_day,m_start_month,m_start_year) && day ==spring_bank_day)
     return "Public Holiday";   //spring bank holiday

	} //m_start_month ==5 (May)

	GDate *easter_date =calculate_easter(m_start_year);
	int easter_day = g_date_get_day(easter_date);
	int easter_month =g_date_get_month(easter_date);

	if(m_start_month==easter_month && day == easter_day)
	{
	//easter day
	return "Easter Day";
	}

	g_date_subtract_days(easter_date,2);
	int easter_friday = g_date_get_day(easter_date);
	int easter_friday_month =g_date_get_month(easter_date);

	if(m_start_month==easter_friday_month && day ==easter_friday)
	{
	//easter friday
	return "Easter Friday";
	}

	g_date_add_days(easter_date,3);
	int easter_monday = g_date_get_day(easter_date); //easter monday
	int easter_monday_month =g_date_get_month(easter_date);

	if(m_start_month==easter_monday_month && day ==easter_monday)
	{
	//easter monday
	return "Easter Monday";
	}

	if (m_start_month == 8) {
      //August complicated
    GDate *first_monday_august;
     first_monday_august = g_date_new_dmy(1, m_start_month, m_start_year);

     while (g_date_get_weekday(first_monday_august) != G_DATE_MONDAY)
       g_date_add_days(first_monday_august,1);

     int august_day = g_date_get_day(first_monday_august);


     int days_in_august =g_date_get_days_in_month (m_start_month, m_start_year);
     int plus_days = 0;

     if (august_day + 28 <= days_in_august) {
       plus_days = 28;
     } else {
       plus_days = 21;
     }

     GDate *august_bank =g_date_new_dmy (august_day, m_start_month, m_start_year);

     g_date_add_days(august_bank,plus_days);

     int august_bank_day = g_date_get_day(august_bank);

     if (g_date_valid_dmy (august_bank_day,m_start_month,m_start_year) && day ==august_bank_day)
     return "Public Holiday";   //august bank holiday

    } //m_start_month==8

	return "";
}


//-----------------------------------------------------------------
// Speak callbks
//------------------------------------------------------------------

static void callbk_speak(GSimpleAction* action, GVariant *parameter,gpointer user_data){

	GtkWidget *window = user_data;

	GtkWidget *calendar =g_object_get_data(G_OBJECT(window), "window-calendar-key");
	speak_events(window);

}

static void callbk_speak_month(GSimpleAction* action, GVariant *parameter,gpointer user_data){

	//g_print("speak month called\n");
	GtkWidget *window = user_data;
	//GtkWidget *calendar =g_object_get_data(G_OBJECT(window), "window-calendar-key");
	speak_month_events(window);
	//speak_events();

}


//-----------------------------------------------------------------

gchar* convert_number_to_ordinal_string(int number)
{
	gchar* result =NULL;

	switch (number) {
		case 1:
		result ="First";
		break;
		case 2:
		result ="Second";
		break;
		case 3:
		result ="Third";
		break;
		case 4:
		result ="Fourth";
		break;
		case 5:
		result ="Fifth";
		break;
		case 6:
		result ="Sixth";
		break;
		case 7:
		result ="Seventh";
		break;
		case 8:
		result ="Eighth";
		break;
		case 9:
		result ="Ninth";
		break;
		case 10:
		result ="Tenth";
		break;
		case 11:
		result ="Eleventh";
		break;
		case 12:
		result ="Twelfth";
		break;
		case 13:
		result ="Thirteenth";
		break;
		case 14:
		result ="Fourteenth";
		break;
		case 15:
		result ="Fifteenth";
		break;
		case 16:
		result ="Sixteenth";
		break;
		case 17:
		result ="Seventeenth";
		break;
		case 18:
		result ="Eighteenth";
		break;
		case 19:
		result ="Nineteenth";
		break;
		case 20:
		result ="Twentieth";
		break;
		case 21:
		result ="Twenty first";
		break;
		case 22:
		result ="Twenty second";
		break;
		case 23:
		result ="Twenty third";
		break;
		case 24:
		result ="Twenty fourth";
		break;
		case 25:
		result ="Twenty fifth";
		break;
		case 26:
		result ="Twenty sixth";
		break;
		case 27:
		result ="Twenty seven";
		break;
		case 28:
		result ="Twenty eighth";
		break;
		case 29:
		result ="Twenty ninth";
		break;
		case 30:
		result ="Thirtieth";
		break;
		case 31:
		result ="Thirty first";
		break;
		default:
		result ="zero";
		break;
	}
}


gchar* convert_number_to_cardinal_string(int number) {

	gchar* result =NULL;

	switch (number) {
		case 1:
		result ="one";
		break;
		case 2:
		result ="two";
		break;
		case 3:
		result ="three";
		break;
		case 4:
		result ="four";
		break;
		case 5:
		result ="five";
		break;
		case 6:
		result ="six";
		break;
		case 7:
		result ="seven";
		break;
		case 8:
		result ="eight";
		break;
		case 9:
		result ="nine";
		break;
		case 10:
		result ="ten";
		break;
		case 11:
		result ="eleven";
		break;
		case 12:
		result ="twelve";
		break;
		case 13:
		result ="thirteen";
		break;
		case 14:
		result ="fourteen";
		break;
		case 15:
		result ="fifteen";
		break;
		case 16:
		result ="sixteen";
		break;
		case 17:
		result ="seventeen";
		break;
		case 18:
		result ="eighteen";
		break;
		case 19:
		result ="nineteen";
		break;
		case 20:
		result ="twenty";
		break;
		case 21:
		result ="twenty one";
		break;
		case 22:
		result ="twenty two";
		break;
		case 23:
		result ="twenty three";
		break;
		case 24:
		result ="twenty four";
		break;
		case 25:
		result ="twenty five";
		break;
		case 26:
		result ="twenty six";
		break;
		case 27:
		result ="twenty seven";
		break;
		case 28:
		result ="twenty eight";
		break;
		case 29:
		result ="twenty nine";
		break;
		case 30:
		result ="thirty";
		break;
		case 31:
		result ="thirty one";
		break;
		case 32:
		result ="thirty two";
		break;
		case 33:
		result ="thirty three";
		break;
		case 34:
		result ="thirty four";
		break;
		case 35:
		result ="thirty five";
		break;
		case 36:
		result ="thirty six";
		break;
		case 37:
		result ="thirty seven";
		break;
		case 38:
		result ="thirty eight";
		break;
		case 39:
		result ="thirty nine";
		break;
		case 40:
		result ="forty";
		break;
		case 41:
		result ="forty one";
		break;
		case 42:
		result ="forty two";
		break;
		case 43:
		result ="forty three";
		break;
		case 44:
		result ="forty four";
		break;
		case 45:
		result ="forty five";
		break;
		case 46:
		result ="forty six";
		break;
		case 47:
		result ="forty seven";
		break;
		case 48:
		result ="forty eight";
		break;
		case 49:
		result ="forty nine";
		break;
		case 50:
		result ="fifty";
		break;
		case 51:
		result ="fifty one";
		break;
		case 52:
		result ="fifty two";
		break;
		case 53:
		result ="fifty three";
		break;
		case 54:
		result ="fifty four";
		break;
		case 55:
		result ="fifty five";
		break;
		case 56:
		result ="fifty six";
		break;
		case 57:
		result ="fifty seven";
		break;
		case 58:
		result ="fifty eight";
		break;
		case 59:
		result ="fifty nine";
		break;

		default:
		result ="zero";
		break;

	}

	return result;
}


//---------------------------------------------------------------------
// Speaking
//---------------------------------------------------------------------
static gpointer thread_speak_func(gpointer user_data)
{

	char* text =user_data;
	gchar * command_str ="flite";

	if (g_file_test(g_build_filename("/usr/bin/", "flite", NULL), G_FILE_TEST_IS_REGULAR)) {

		command_str= g_strconcat(command_str," -t "," '",text,"' ", NULL);
		system(command_str);
	}

	else {
		g_print("flite not installed\n");
		//removed messagbox as being depreciated in gtk 4.10
	}
    g_mutex_unlock (&lock); //thread mutex unlock
    return NULL;
}

static void speak_month_events(gpointer  user_data) {

	if(m_talk==0) return;

	gchar* speak_str ="";

	GtkWidget *window = user_data;
	GtkWidget *calendar =g_object_get_data(G_OBJECT(window), "window-calendar-key");

	gchar * mark_str =get_marks_on_calendar(GTK_CALENDAR(calendar));

	speak_str=g_strconcat(speak_str," ", mark_str, NULL);

	GThread *thread_speak;
	if(m_talk) {
	g_mutex_lock (&lock);
	thread_speak = g_thread_new(NULL, thread_speak_func, speak_str);
	}
	g_thread_unref (thread_speak);

}



//---------------------------------------------------------------------
// speak events
//---------------------------------------------------------------------
static void speak_events() {

	if(m_talk==0) return;


	char* speak_str ="";
	char* speak_str_date ="";
	char* speak_str_events="";
	char* weekday_str="";
	char* day_month_str="";
	char * holiday_str="";

	GDate* day_date;
	day_date = g_date_new_dmy(m_start_day, m_start_month, m_start_year);
	GDateWeekday weekday =g_date_get_weekday(day_date);

	switch(weekday)
	{
		case G_DATE_MONDAY:
			weekday_str="Monday";
			break;
		case G_DATE_TUESDAY:
			weekday_str="Tuesday";
			break;
		case G_DATE_WEDNESDAY:
			weekday_str="Wednesday";
			break;
		case G_DATE_THURSDAY:
			weekday_str="Thursday";
			break;
		case G_DATE_FRIDAY:
			weekday_str="Friday";
			break;
		case G_DATE_SATURDAY:
			weekday_str="Saturday";
			break;
		case G_DATE_SUNDAY:
			weekday_str="Sunday";
			break;
		default:
			weekday_str="Unknown";
	}//switch

	gchar* day_str =convert_number_to_ordinal_string(m_start_day);

	//gchar* day_str =g_strdup_printf("%d",m_day);

	day_month_str =g_strconcat(day_month_str,weekday_str," ", day_str, " ", NULL);

	switch(m_start_month)
	{
		case G_DATE_JANUARY:
			day_month_str =g_strconcat(day_month_str,"January ", NULL);
			break;
		case G_DATE_FEBRUARY:
			day_month_str =g_strconcat(day_month_str,"February ", NULL);
			break;
		case G_DATE_MARCH:
			day_month_str =g_strconcat(day_month_str,"March ", NULL);
			break;
		case G_DATE_APRIL:
			day_month_str =g_strconcat(day_month_str,"April ", NULL);
			break;
		case G_DATE_MAY:
			day_month_str =g_strconcat(day_month_str,"May ", NULL);
			break;
		case G_DATE_JUNE:
			day_month_str =g_strconcat(day_month_str,"June ", NULL);
			break;
		case G_DATE_JULY:
			day_month_str =g_strconcat(day_month_str,"July ", NULL);
			break;
		case G_DATE_AUGUST:
			day_month_str =g_strconcat(day_month_str,"August ", NULL);
			break;
		case G_DATE_SEPTEMBER:
			day_month_str =g_strconcat(day_month_str,"September ", NULL);
			break;
		case G_DATE_OCTOBER:
			day_month_str =g_strconcat(day_month_str,"October ", NULL);
			break;
		case G_DATE_NOVEMBER:
			day_month_str =g_strconcat(day_month_str,"November ", NULL);
			break;
		case G_DATE_DECEMBER:
			day_month_str =g_strconcat(day_month_str,"December ", NULL);
			break;
		default:
			day_month_str =g_strconcat(day_month_str,"Unknown ", NULL);
	}

	//speak_str= g_strconcat(speak_str, day_month_year_str,NULL);
	speak_str_date =g_strconcat(speak_str_date, day_month_str, NULL);


	if (m_holidays && is_public_holiday(m_start_day)) {
		holiday_str= get_holiday_str(m_start_day);
		speak_str_date =g_strconcat(speak_str_date, holiday_str, ".  ", NULL);
	}


	//Speak from listbox (easy way but limited)
	//count day events
	// 	int event_count=get_gliststore_number(m_store);
	// 	if(m_talk_event_number)	{
	// 		if (event_count==0) speak_str_events =g_strconcat(speak_str_events, " No events ", NULL);
	// 		else {
	// 			char* event_number_str = g_strdup_printf("%d", event_count);
	// 			speak_str_events =g_strconcat(speak_str_events, " You have ",event_number_str, "events  ", NULL);
	// 		}
	// 	}
	// 	//print_gliststore(m_store);
	// 	//get liststore events diplayed in listbox
	// 	gchar* glist_str = get_gliststore_labels(m_store);
	// 	speak_str_events =g_strconcat(speak_str_events, " ", glist_str, NULL);
	// 	speak_str=g_strconcat(speak_str_date, speak_str_events, NULL);

	//Get day events and process (hard way but options)

	//--------------------------------------------------------------------------------------------
	// create time sorted day events array
	//---------------------------------------------------------------------------------------------

	GArray *day_events_arry =g_array_new(FALSE, FALSE, sizeof(CALENDAR_TYPE_EVENT));

	int sort_time =0;

	for (int i=0; i<evt_arry->len; i++) {

		gint start_year=0;
		gint start_month=0;
		gint start_day=0;
		gint is_yearly=0;

		gint start_hour=0;
		gint start_min=0;

		CalendarEvent *evt =g_array_index(evt_arry, CalendarEvent*, i);

		g_object_get (evt, "startyear", &start_year, NULL);
		g_object_get (evt, "startmonth", &start_month, NULL);
		g_object_get (evt, "startday", &start_day, NULL);
		g_object_get (evt, "isyearly", &is_yearly, NULL);
		g_object_get (evt, "starthour", &start_hour, NULL);
		g_object_get (evt, "startmin", &start_min, NULL);

		//normal events
		if(m_start_year==start_year && m_start_month ==start_month && m_start_day==start_day)
		{
			int etime=start_hour*60*60 + 60*start_min; //seconds
			//g_print("etime = %d start_time = %d\n", etime, sort_time);

			if(etime >sort_time) {
				//g_print("append value\n");
				g_array_append_val(day_events_arry, evt);
				sort_time=etime;
			}
			else {
				//g_print("prepend value\n");
				g_array_prepend_val(day_events_arry, evt);
			}
		}//if
		//repeaters -repeating year events
		if(m_start_year!= start_year && m_start_month ==start_month && m_start_day==start_day && is_yearly==1)
		{
			int etime=start_hour*60*60 + 60*start_min; //seconds
			//g_print("etime = %d start_time = %d\n", etime, sort_time);

			if(etime >sort_time) {
				//g_print("append value\n");
				g_array_append_val(day_events_arry, evt);
				sort_time=etime;
			}
			else {
				//g_print("prepend value\n");
				g_array_prepend_val(day_events_arry, evt);
			}
		}

	}//for evt_arry


	//loop through day events

	//g_array_sort (day_events_arry, compare_fn);
	//g_print("day_events_arry\n");
	//print_array(day_events_arry); //debugging

	int event_count =day_events_arry->len;

	if(m_talk_event_number)	{
		if (event_count==0) {
			speak_str_events =g_strconcat(speak_str_events, " No events ", NULL);
		}
		else if(event_count==1) {
			speak_str_events =g_strconcat(speak_str_events, " One event ", NULL);
		}
		else {
			gchar* event_number_str = g_strdup_printf("%d", event_count);
			speak_str_events =g_strconcat(speak_str_events, " You have ",event_number_str, "events  ", NULL);
		}
	}



	for (int i=0; i<day_events_arry->len; i++) {


		gint evt_id=0;
		gchar *summary_str="";
		gchar *location_str="";

		gint start_year=0;
		gint start_month=0;
		gint start_day=0;
		gint start_hour=0;
		gint start_min=0;

		gint end_year=0;
		gint end_month=0;
		gint end_day=0;
		gint end_hour=0;
		gint end_min=0;

		gint is_yearly=0;
		gint is_allday=0;
		gint is_priority=0;
		gint has_reminder=0;
		gint reminder_min=0;

		CalendarEvent *evt =g_array_index(day_events_arry, CalendarEvent*, i);

		g_object_get (evt, "eventid", &evt_id, NULL);
		g_object_get (evt, "summary", &summary_str, NULL);
		g_object_get (evt, "location", &location_str, NULL);
		//g_object_get (evt, "description", &description_str, NULL);
		g_object_get (evt, "startyear", &start_year, NULL);
		g_object_get (evt, "startmonth", &start_month, NULL);
		g_object_get (evt, "startday", &start_day, NULL);
		g_object_get (evt, "starthour", &start_hour, NULL);
		g_object_get (evt, "startmin", &start_min, NULL);
		g_object_get (evt, "endyear", &end_year, NULL);
		g_object_get (evt, "endmonth", &end_month, NULL);
		g_object_get (evt, "endday", &end_day, NULL);
		g_object_get (evt, "endhour", &end_hour, NULL);
		g_object_get (evt, "endmin", &end_min, NULL);
		g_object_get (evt, "isyearly", &is_yearly, NULL);
		g_object_get (evt, "isallday", &is_allday, NULL);
		g_object_get (evt, "ispriority", &is_priority, NULL);

		//create speak events string
		//speak_str_events="";

		gchar *start_time_str="";
		gchar *time_str="";
		//gchar *summary_str="";
		//gchar *location_str="";
		gchar *starthour_str = "";
		gchar *startmin_str = "";
		gchar *endhour_str = "";
		gchar *endmin_str = "";
		gchar *ampm_str ="";

		//time str first

		if (is_allday) {
			time_str="All day Event. ";
		}
		else {


			if(m_12hour_format) {

				if (start_hour >=13 && start_hour<=23) {
					int s_hour= start_hour-12;
					ampm_str="p.m. ";
					starthour_str =convert_number_to_cardinal_string(s_hour);
				}
				else {
					ampm_str="a.m. ";
					starthour_str =convert_number_to_cardinal_string(start_hour);
				}
			} //12
			else {
				starthour_str =convert_number_to_cardinal_string(start_hour);
			}//24


			time_str = g_strconcat(time_str, starthour_str, "  ", NULL);

			startmin_str =convert_number_to_cardinal_string(start_min);

			if(start_min ==0) {
				time_str = g_strconcat(time_str," ",NULL);
			}
			else if (start_min >0 && start_min <10){
				time_str = g_strconcat(time_str, "zero ", startmin_str,".   ",NULL);
			}
			else {
				time_str = g_strconcat(time_str, startmin_str,".  ",NULL);
			}

			time_str = g_strconcat(time_str, ampm_str,".  ",NULL);
		} //else not allday


		summary_str =remove_punctuations(summary_str);

		location_str =remove_punctuations(location_str);

		if (m_talk_time){
			speak_str_events= g_strconcat(speak_str_events, time_str, summary_str, ". ", NULL);
		}
		else {
			speak_str_events= g_strconcat(speak_str_events, summary_str, ".  ", NULL);
		}

		if(m_talk_location) {
			speak_str_events= g_strconcat(speak_str_events, location_str,". ", NULL);
		}


		if(m_talk_priority && is_priority) {
			speak_str_events=g_strconcat(speak_str_events, " high priority.  ", NULL);
		}

	}//day_events for loop

	speak_str=g_strconcat(speak_str_date, speak_str_events, NULL);

	//Thread speaking

	GThread *thread_speak;
	if(m_talk) {
		g_mutex_lock (&lock);
		thread_speak = g_thread_new(NULL, thread_speak_func, speak_str);
	}
	g_thread_unref (thread_speak);

}


//--------------------------------------------------------------------------------
// Add event
//---------------------------------------------------------------------------------

static void callbk_check_button_allday_toggled (GtkCheckButton *check_button, gpointer user_data)
{

 	GtkWidget *spin_button_start_time;
 	GtkWidget *spin_button_end_time;

 	spin_button_start_time =g_object_get_data(G_OBJECT(user_data), "cb_allday_spin_start_time_key");
	spin_button_end_time= g_object_get_data(G_OBJECT(user_data), "cb_allday_spin_end_time_key");

 	if (gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button))){
	gtk_widget_set_sensitive(spin_button_start_time,FALSE);
	gtk_widget_set_sensitive(spin_button_end_time,FALSE);
	}
	else{
	gtk_widget_set_sensitive(spin_button_start_time,TRUE);
	gtk_widget_set_sensitive(spin_button_end_time,TRUE);

	}

}



static void callbk_add(GtkButton *button, gpointer  user_data){

    //g_print("add button clicked");
	GtkWidget *window = user_data;

	GtkWidget *calendar =g_object_get_data(G_OBJECT(window), "window-calendar-key");
	GtkWidget *label_date =g_object_get_data(G_OBJECT(window), "window-label-date-key");

	GtkWidget *dialog = g_object_get_data(G_OBJECT(button), "dialog-key");

	GtkEntryBuffer *buffer_summary;
	GtkWidget *entry_summary = g_object_get_data(G_OBJECT(button), "entry-summary-key");

	GtkEntryBuffer *buffer_location;
	GtkWidget *entry_location = g_object_get_data(G_OBJECT(button), "entry-location-key");


	GtkWidget *spin_button_start_time= g_object_get_data(G_OBJECT(button), "spin-start-time-key");
    GtkWidget *spin_button_end_time= g_object_get_data(G_OBJECT(button), "spin-end-time-key");

	GtkWidget *check_button_allday= g_object_get_data(G_OBJECT(button), "check-button-allday-key");
    GtkWidget *check_button_isyearly= g_object_get_data(G_OBJECT(button), "check-button-isyearly-key");
    GtkWidget *check_button_priority= g_object_get_data(G_OBJECT(button), "check-button-priority-key");



	buffer_summary = gtk_entry_get_buffer (GTK_ENTRY(entry_summary));
	m_summary= gtk_entry_buffer_get_text (buffer_summary);

	m_summary =remove_semicolons(m_summary);
	//m_summary =remove_punctuations(m_summary);

	buffer_location = gtk_entry_get_buffer (GTK_ENTRY(entry_location));
	m_location= gtk_entry_buffer_get_text (buffer_location);
	m_location =remove_semicolons(m_location);
	m_description ="todo";

	float start_time =gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_button_start_time));
	float end_time =gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_button_end_time));

	//convert to start hour and min
	float integral_part, fractional_part;
	fractional_part = modff(start_time, &integral_part);
	int start_hour =(int) integral_part; //start_hour
	fractional_part=round(fractional_part *100);
	int start_min=(int) (fractional_part); //start_min
	//convert to end hour and min
	float integral_part_end, fractional_part_end;
	fractional_part_end = modff(end_time, &integral_part_end);
	int end_hour =(int) integral_part_end; //end_hour
	fractional_part_end=round(fractional_part_end *100);
	int end_min=(int) (fractional_part_end); //start_min

	//g_print("m_summary = %s\n",m_summary);
	//g_print("m_location = %s\n",m_location);

	int isyearly=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_isyearly));
    int isallday=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_allday));
	int priority=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_priority));


	CalendarEvent *evt= g_object_new(CALENDAR_TYPE_EVENT,0);

	//int id_idx =(evt_arry->len) +1;

	int id_idx =evt_arry->len;

    g_object_set (evt, "eventid", id_idx, NULL);
    g_object_set (evt, "summary", g_strdup(m_summary), NULL);
    g_object_set (evt, "location", g_strdup(m_location), NULL);
    g_object_set (evt, "description", g_strdup(m_description), NULL);
    g_object_set (evt, "startyear", m_start_year, NULL);
    g_object_set (evt, "startmonth", m_start_month, NULL);
    g_object_set (evt, "startday", m_start_day, NULL);
    g_object_set (evt, "starthour", start_hour, NULL);
    g_object_set (evt, "startmin", start_min, NULL);
    g_object_set (evt, "endyear", m_end_year, NULL); //to do
    g_object_set (evt, "endmonth",m_end_month, NULL);
    g_object_set (evt, "endday", m_end_day, NULL);
    g_object_set (evt, "endhour", end_hour, NULL);
    g_object_set (evt, "endmin", end_min, NULL);
    g_object_set (evt, "isyearly", isyearly, NULL);
    g_object_set (evt, "isallday", isallday, NULL);
    g_object_set (evt, "ispriority", priority, NULL);
    g_object_set (evt, "hasreminder", 0, NULL);
    g_object_set (evt, "remindermin", 30, NULL);

    g_array_append_val(evt_arry, evt);

	m_id_selection=-1;
	update_date(GTK_CALENDAR(calendar),label_date);
	display_events(m_start_year,m_start_month,m_start_day);

	set_event_marks_on_calendar(GTK_CALENDAR(calendar));
	gtk_window_destroy(GTK_WINDOW(dialog));
	//gtk_window_close(GTK_WINDOW(window));

}

static void callbk_new_event(GtkButton *button, gpointer  user_data){

  GtkWidget *window =user_data;

  GtkWidget *dialog;
  gint response;
  GtkWidget *grid;
  GtkWidget *box;
  GtkWidget *label_date;
  GtkEntryBuffer *buffer;

  GtkWidget *label_entry_summary;
  GtkWidget *entry_summary;

  GtkWidget *label_location;
  GtkWidget *entry_location;

  //Start time
  GtkWidget *label_start_time;
  GtkWidget *spin_button_start_time;
  GtkWidget *box_start_time;

  //End time
  GtkWidget *label_end_time;
  GtkWidget *spin_button_end_time;
  GtkWidget *box_end_time;


  //Check buttons
  GtkWidget *check_button_allday;
  GtkWidget *check_button_isyearly;
  GtkWidget *check_button_priority;

  GtkWidget *button_add;

  GDate *event_date;
  event_date = g_date_new();
  event_date = g_date_new_dmy(m_start_day,m_start_month,m_start_year);
  int event_day= g_date_get_day(event_date);
  int event_month= g_date_get_month(event_date);
  int event_year= g_date_get_year(event_date);

  gchar * date_str="Event Date: ";
  gchar *day_str = g_strdup_printf("%d",event_day);
  gchar *month_str = g_strdup_printf("%d",event_month);
  gchar *year_str = g_strdup_printf("%d",event_year);

  date_str =g_strconcat(date_str,day_str,"-",month_str,"-",year_str,NULL);

  dialog =gtk_window_new(); //gtk_dialog_new_with_buttons to be deprecated gtk4.10

  gtk_window_set_title (GTK_WINDOW (dialog), "New Event");
  gtk_window_set_default_size(GTK_WINDOW(dialog),350,100);

  box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);
  gtk_window_set_child (GTK_WINDOW (dialog), box);

  button_add = gtk_button_new_with_label ("Add");
  g_signal_connect (button_add, "clicked", G_CALLBACK (callbk_add), window);

  label_date =gtk_label_new(date_str);

  label_entry_summary =gtk_label_new("Event Summary");
  entry_summary =gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entry_summary),100);

  label_location =gtk_label_new("Location");
  entry_location =gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entry_location),100);

  gtk_box_append(GTK_BOX(box), label_date);
  gtk_box_append(GTK_BOX(box), label_entry_summary);
  gtk_box_append(GTK_BOX(box), entry_summary);
  gtk_box_append(GTK_BOX(box), label_location);
  gtk_box_append(GTK_BOX(box), entry_location);


  g_object_set_data(G_OBJECT(button_add), "dialog-key",dialog);
  g_object_set_data(G_OBJECT(button_add), "entry-summary-key",entry_summary);
  g_object_set_data(G_OBJECT(button_add), "entry-location-key",entry_location);

  //--------------------------------------------------------
  // Start time spin buttons
  //---------------------------------------------------------

  GtkAdjustment *adjustment_start;
  //value,lower,upper,step_increment,page_increment,page_size
  adjustment_start = gtk_adjustment_new (8.00, 0.0, 23.59, 1.0, 1.0, 0.0);
  //start time spin
  label_start_time =gtk_label_new("Start Time (24 hour) ");
  spin_button_start_time = gtk_spin_button_new (adjustment_start, 1.0, 2);
  box_start_time=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);
  gtk_box_append (GTK_BOX(box_start_time),label_start_time);
  gtk_box_append (GTK_BOX(box_start_time),spin_button_start_time);
  gtk_box_append(GTK_BOX(box), box_start_time);

  g_object_set_data(G_OBJECT(button_add), "spin-start-time-key",spin_button_start_time);

  //end time spin
   GtkAdjustment *adjustment_end;
  adjustment_end = gtk_adjustment_new (8.00, 0.0, 23.59, 1.0, 1.0, 0.0);
  label_end_time =gtk_label_new("End Time (24 hour) ");
  spin_button_end_time = gtk_spin_button_new (adjustment_end, 1.0, 2);
  box_end_time=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);
  gtk_box_append (GTK_BOX(box_end_time),label_end_time);
  gtk_box_append (GTK_BOX(box_end_time),spin_button_end_time);
  gtk_box_append(GTK_BOX(box), box_end_time);

  g_object_set_data(G_OBJECT(button_add), "spin-end-time-key",spin_button_end_time);

  //check buttons
  check_button_allday = gtk_check_button_new_with_label ("Is All Day");

  g_object_set_data(G_OBJECT(check_button_allday), "cb_allday_spin_start_time_key",spin_button_start_time);
  g_object_set_data(G_OBJECT(check_button_allday), "cb_allday_spin_end_time_key",spin_button_end_time);

  g_signal_connect_swapped (GTK_CHECK_BUTTON(check_button_allday), "toggled",
					G_CALLBACK (callbk_check_button_allday_toggled), check_button_allday);

  check_button_isyearly = gtk_check_button_new_with_label ("Is Yearly");
  check_button_priority = gtk_check_button_new_with_label ("Is High Priority");
  gtk_box_append(GTK_BOX(box), check_button_allday);
  gtk_box_append(GTK_BOX(box), check_button_isyearly);
  gtk_box_append(GTK_BOX(box), check_button_priority);

  gtk_box_append(GTK_BOX(box), button_add);

  g_object_set_data(G_OBJECT(button_add), "check-button-allday-key",check_button_allday);
  g_object_set_data(G_OBJECT(button_add), "check-button-isyearly-key",check_button_isyearly);
  g_object_set_data(G_OBJECT(button_add), "check-button-priority-key",check_button_priority);

  gtk_window_present (GTK_WINDOW (dialog));

}

//----------------------------------------------------------------------
// Edit event
//----------------------------------------------------------------------


static void callbk_update_event(GtkButton *button, gpointer  user_data){

	//g_print("update event");

	GtkWidget *window = user_data;
	GtkWidget *calendar =g_object_get_data(G_OBJECT(window), "window-calendar-key");
	GtkWidget *label_date =g_object_get_data(G_OBJECT(window), "window-label-date-key");

	GtkWidget *dialog = g_object_get_data(G_OBJECT(button), "dialog-key");


	GtkEntryBuffer *buffer_summary;
	GtkWidget *entry_summary = g_object_get_data(G_OBJECT(button), "entry-summary-key");

	GtkEntryBuffer *buffer_location;
	GtkWidget *entry_location = g_object_get_data(G_OBJECT(button), "entry-location-key");


    GtkWidget *check_button_allday= g_object_get_data(G_OBJECT(button), "check-button-allday-key");
    GtkWidget  *check_button_isyearly= g_object_get_data(G_OBJECT(button), "check-button-isyearly-key");
    GtkWidget  *check_button_priority= g_object_get_data(G_OBJECT(button), "check-button-priority-key");

	GtkWidget *spin_button_start_time= g_object_get_data(G_OBJECT(button), "spin-start-time-key");
    GtkWidget *spin_button_end_time= g_object_get_data(G_OBJECT(button), "spin-end-time-key");

	buffer_summary = gtk_entry_get_buffer (GTK_ENTRY(entry_summary));
	m_summary= gtk_entry_buffer_get_text (buffer_summary);
	m_summary =remove_semicolons(m_summary);

	buffer_location = gtk_entry_get_buffer (GTK_ENTRY(entry_location));
	m_location= gtk_entry_buffer_get_text (buffer_location);
	m_location =remove_semicolons(m_location);

	m_description="todo";

	float start_time =gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_button_start_time));
	float end_time =gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_button_end_time));


	//convert to start hour and min
	float integral_part, fractional_part;
	fractional_part = modff(start_time, &integral_part);
	m_start_hour =(int) integral_part; //start_hour
	fractional_part=round(fractional_part *100);
	m_start_min=(int) (fractional_part); //start_min
	//convert to end hour and min
	float integral_part_end, fractional_part_end;
	fractional_part_end = modff(end_time, &integral_part_end);
	m_end_hour =(int) integral_part_end; //end_hour
	fractional_part_end=round(fractional_part_end *100);
	m_end_min=(int) (fractional_part_end); //start_min

	m_is_yearly=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_isyearly));
	m_is_allday=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_allday));
	m_priority=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_priority));


	gint arry_index=0;

	//insert change into event array

	for (int i=0; i<evt_arry->len; i++) {


		CalendarEvent *evt =g_array_index(evt_arry, CalendarEvent*, i);

		g_object_get (evt, "eventid", &arry_index, NULL);

		if(arry_index==m_id_selection){
			//found event now insert changes
			g_object_set (evt, "eventid", arry_index, NULL);
			g_object_set (evt, "summary", g_strdup(m_summary), NULL);
			g_object_set (evt, "location", g_strdup(m_location), NULL);
			g_object_set (evt, "description", g_strdup(m_description), NULL);
			g_object_set (evt, "startyear", m_start_year, NULL);
			g_object_set (evt, "startmonth", m_start_month, NULL);
			g_object_set (evt, "startday", m_start_day, NULL);
			g_object_set (evt, "starthour", m_start_hour, NULL);
			g_object_set (evt, "startmin", m_start_min, NULL);
			g_object_set (evt, "endyear", m_end_year, NULL); //to do
			g_object_set (evt, "endmonth",m_end_month, NULL);
			g_object_set (evt, "endday", m_end_day, NULL);
			g_object_set (evt, "endhour", m_end_hour, NULL);
			g_object_set (evt, "endmin", m_end_min, NULL);
			g_object_set (evt, "isyearly", m_is_yearly, NULL);
			g_object_set (evt, "isallday", m_is_allday, NULL);
			g_object_set (evt, "ispriority", m_priority, NULL);
			g_object_set (evt, "hasreminder", m_has_reminder, NULL);
			g_object_set (evt, "remindermin", m_reminder_min, NULL);


			evt_arry= g_array_remove_index(evt_arry,arry_index);
            g_array_insert_val(evt_arry,arry_index,evt);
			//g_array_remove_index(evt_arry,arry_index+1);

		}//if

	}//for

	m_row_index=-1;
	m_id_selection=-1;
	update_date(GTK_CALENDAR(calendar),label_date);
	display_events(m_start_year,m_start_month,m_start_day);
	set_event_marks_on_calendar(GTK_CALENDAR(calendar));
	gtk_window_destroy(GTK_WINDOW(dialog));
}

//---------------------------------------------------------------------
// callback edit event
//---------------------------------------------------------------------
static void callbk_edit_event(GtkButton *button, gpointer  user_data){


	if (m_id_selection==-1) return;

	GtkWindow *window =user_data;


	GtkWidget *dialog;
	gint response;
	GtkWidget *grid;
	GtkWidget *box;
	GtkWidget *label_date;
	GtkEntryBuffer *buffer_summary;
	GtkEntryBuffer *buffer_location;
	//GtkEntryBuffer *buffer_description;

	GtkWidget *label_entry_summary;
	GtkWidget *entry_summary;

	GtkWidget *label_location;
	GtkWidget *entry_location;


	//Start time
	GtkWidget *label_start_time;
	GtkWidget *spin_button_start_time;
	GtkWidget *box_start_time;

	//End time
	GtkWidget *label_end_time;
	GtkWidget *spin_button_end_time;
	GtkWidget *box_end_time;

	//Check buttons
	GtkWidget *check_button_allday;
	GtkWidget *check_button_isyearly;
	GtkWidget *check_button_priority;

	GtkWidget *button_update;

	GDate *event_date;
	event_date = g_date_new();
	event_date = g_date_new_dmy(m_start_day,m_start_month,m_start_year);
	int event_day= g_date_get_day(event_date);
	int event_month= g_date_get_month(event_date);
	int event_year= g_date_get_year(event_date);

	gchar * date_str="Event Date: ";
	gchar *day_str = g_strdup_printf("%d",event_day);
	gchar *month_str = g_strdup_printf("%d",event_month);
	gchar *year_str = g_strdup_printf("%d",event_year);

	date_str =g_strconcat(date_str,day_str,"-",month_str,"-",year_str,NULL);


	dialog =gtk_window_new(); //gtk_dialog_new_with_buttons  deprecated gtk4.10

	gtk_window_set_title (GTK_WINDOW (dialog), "Edit Event");
	gtk_window_set_default_size(GTK_WINDOW(dialog),350,100);

	box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);
	gtk_window_set_child (GTK_WINDOW (dialog), box);


	gint evt_id=0;
	gchar *summary_str="";
	gchar *location_str="";

	gint start_year=0;
	gint start_month=0;
	gint start_day=0;
	gint start_hour=0;
	gint start_min=0;

	gint end_year=0;
	gint end_month=0;
	gint end_day=0;
	gint end_hour=0;
	gint end_min=0;

	gint is_yearly=0;
	gint is_allday=0;
	gint is_priority=0;
	gint has_reminder=0;
	gint reminder_min=0;



	for (int i=0; i<evt_arry->len; i++) {


		CalendarEvent *evt =g_array_index(evt_arry, CalendarEvent*, i);

		g_object_get (evt, "eventid", &evt_id, NULL);

		if(evt_id==m_id_selection){

			g_object_get (evt, "summary", &summary_str, NULL);
			g_object_get (evt, "location", &location_str, NULL);
			//g_object_get (evt, "description", &description_str, NULL); //plumbing for future updates
			//g_object_get (evt, "startyear", &start_year, NULL);
			//g_object_get (evt, "startmonth", &start_month, NULL);
			//g_object_get (evt, "startday", &start_day, NULL);
			g_object_get (evt, "starthour", &start_hour, NULL);
			g_object_get (evt, "startmin", &start_min, NULL);
			//g_object_get (evt, "endyear", &end_year, NULL);
			//g_object_get (evt, "endmonth", &end_month, NULL);
			//g_object_get (evt, "endday", &end_day, NULL);
			g_object_get (evt, "endhour", &end_hour, NULL);
			g_object_get (evt, "endmin", &end_min, NULL);
			g_object_get (evt, "isyearly", &is_yearly, NULL);
			g_object_get (evt, "isallday", &is_allday, NULL);
			g_object_get (evt, "ispriority", &is_priority, NULL);
			//g_object_get (evt, "hasreminder", &has_reminder, NULL);
			//g_object_get (evt, "remindermin", &reminder_min, NULL);

			m_summary =g_strdup(summary_str);
			m_location =g_strdup(location_str);
			//m_description ="todo";
			//m_start_year=start_year;
			//m_start_month=start_month;
			//m_start_day=start_day;

			m_start_hour=start_hour;
			m_start_min=start_min;
			//m_end_year=0; //multiday not yet implemented
			//m_end_month=0;
			//m_end_day=0;

			m_end_hour=end_hour;
			m_end_min=end_min;

			m_is_yearly=is_yearly;
			m_is_allday=is_allday;
			m_priority=is_priority;
			break;

		} //if
	}


	label_date =gtk_label_new(date_str);

	button_update = gtk_button_new_with_label ("Update");
    g_signal_connect (button_update, "clicked", G_CALLBACK (callbk_update_event), window);

	label_entry_summary =gtk_label_new("Event Summary");
	entry_summary =gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_summary),100);
	buffer_summary=gtk_entry_buffer_new(m_summary,-1); //show  event summary
	gtk_entry_set_buffer(GTK_ENTRY(entry_summary),buffer_summary);

	label_location =gtk_label_new("Location");
	entry_location =gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_location),100);
	buffer_location=gtk_entry_buffer_new(m_location,-1); //show location
	gtk_entry_set_buffer(GTK_ENTRY(entry_location),buffer_location);


	gtk_box_append(GTK_BOX(box), label_date);
	gtk_box_append(GTK_BOX(box), label_entry_summary);
	gtk_box_append(GTK_BOX(box), entry_summary);
	gtk_box_append(GTK_BOX(box), label_location);
	gtk_box_append(GTK_BOX(box), entry_location);

	g_object_set_data(G_OBJECT(button_update), "dialog-key",dialog);

	g_object_set_data(G_OBJECT(button_update), "entry-summary-key",entry_summary);
	g_object_set_data(G_OBJECT(button_update), "entry-location-key",entry_location);
	g_object_set_data(G_OBJECT(button_update), "dialog-window-key",window);


	//start time spin
	GtkAdjustment *adjustment_start;
	adjustment_start = gtk_adjustment_new (8.00, 0.0, 23.59, 1.0, 1.0, 0.0);
	//start time spin
	label_start_time =gtk_label_new("Start Time (24 hour) ");
	spin_button_start_time = gtk_spin_button_new (adjustment_start, 1.0, 2);

	//convert hour-min to float
	//double d = wholeNumber + (double)decimal / 100;

	m_start_time=m_start_hour+(float)m_start_min/100;

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_start_time),m_start_time);
	box_start_time=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);
	gtk_box_append (GTK_BOX(box_start_time),label_start_time);
	gtk_box_append (GTK_BOX(box_start_time),spin_button_start_time);
	gtk_box_append(GTK_BOX(box), box_start_time);

	g_object_set_data(G_OBJECT(button_update), "spin-start-time-key",spin_button_start_time);

	//end time spin
	GtkAdjustment *adjustment_end;
	//value,lower,upper,step_increment,page_increment,page_size
	adjustment_end = gtk_adjustment_new (8.00, 0.0, 23.59, 1.0, 1.0, 0.0);
	label_end_time =gtk_label_new("End Time (24 hour) ");
	spin_button_end_time = gtk_spin_button_new (adjustment_end, 1.0, 2);

	m_end_time=m_end_hour+(float)m_end_min/100;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_end_time),m_end_time);
	box_end_time=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);

	gtk_box_append (GTK_BOX(box_end_time),label_end_time);
	gtk_box_append (GTK_BOX(box_end_time),spin_button_end_time);
	gtk_box_append(GTK_BOX(box), box_end_time);

	g_object_set_data(G_OBJECT(button_update), "spin-end-time-key",spin_button_end_time);

	//check buttons
	check_button_allday = gtk_check_button_new_with_label ("Is All Day");
	//gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_allday), e.is_allday);

	g_object_set_data(G_OBJECT(check_button_allday), "cb_allday_spin_start_time_key",spin_button_start_time);
	g_object_set_data(G_OBJECT(check_button_allday), "cb_allday_spin_end_time_key",spin_button_end_time);

	g_signal_connect_swapped (GTK_CHECK_BUTTON(check_button_allday), "toggled",
							  G_CALLBACK (callbk_check_button_allday_toggled), check_button_allday);

	if(m_is_allday) {
		gtk_widget_set_sensitive(spin_button_start_time,FALSE);
		gtk_widget_set_sensitive(spin_button_end_time,FALSE);
	}
	else {
		gtk_widget_set_sensitive(spin_button_start_time,TRUE);
		gtk_widget_set_sensitive(spin_button_end_time,TRUE);
	}

	check_button_isyearly = gtk_check_button_new_with_label("Is Yearly");
	check_button_priority = gtk_check_button_new_with_label ("Is High Priority");
	gtk_box_append(GTK_BOX(box), check_button_allday);
	gtk_box_append(GTK_BOX(box), check_button_isyearly);
	gtk_box_append(GTK_BOX(box), check_button_priority);
	gtk_box_append(GTK_BOX(box), button_update);

	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_isyearly), m_is_yearly);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_allday), m_is_allday);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_priority), m_priority);

	g_object_set_data(G_OBJECT(button_update), "check-button-allday-key",check_button_allday);
	g_object_set_data(G_OBJECT(button_update), "check-button-isyearly-key",check_button_isyearly);
	g_object_set_data(G_OBJECT(button_update), "check-button-priority-key",check_button_priority);

	gtk_window_present (GTK_WINDOW (dialog));

}


//---------------------------------------------------------------------
// callback delete selected
//---------------------------------------------------------------------
static void callbk_delete_selected(GtkButton *button, gpointer  user_data){

	if (m_row_index==-1) return;

	GtkWindow *window =user_data;
	GtkWidget *calendar =g_object_get_data(G_OBJECT(window), "window-calendar-key");
	GtkWidget *label_date =g_object_get_data(G_OBJECT(window), "window-label-date-key");

	gint arry_index=0;

	//insert change into event array

	for (int i=0; i<evt_arry->len; i++) {


		CalendarEvent *evt =g_array_index(evt_arry, CalendarEvent*, i);

		g_object_get (evt, "eventid", &arry_index, NULL);


		if(arry_index==m_id_selection){
			//found so now remove
			//g_print("delete event found\n");

			g_array_remove_index_fast(evt_arry,arry_index);
			//evt_arry= g_array_remove_index_fast(evt_arry,arry_index);
			//g_array_unref(evt_arry);
			save_csv_file();

			evt_arry =g_array_new(FALSE, FALSE, sizeof(CALENDAR_TYPE_EVENT));
			load_csv_file();

		}
	}

	m_row_index=-1; //used for delete selection

	m_id_selection=-1;
	update_date(GTK_CALENDAR(calendar),label_date);
	set_event_marks_on_calendar(GTK_CALENDAR(calendar));
	display_events(m_start_year,m_start_month,m_start_day);

}










//----------------------------------------------------------------------
// listbox functions and callbks
//----------------------------------------------------------------------

static GtkWidget *create_widget (gpointer item, gpointer user_data)
{
  DisplayItem *obj = (DisplayItem *)item;
  GtkWidget *label;

  label = gtk_label_new ("");
  g_object_bind_property (obj, "label", label, "label", G_BINDING_SYNC_CREATE);

  return label;
}

//--------------------------------------------------------------------------------
static void add_separator (GtkListBoxRow *row, GtkListBoxRow *before, gpointer data)
{
  if (!before)
    return;

  gtk_list_box_row_set_header (row, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL));
}


static void callbk_row_activated (GtkListBox    *listbox,
								  GtkListBoxRow *row,
								  gpointer       user_data)
{
	m_row_index = gtk_list_box_row_get_index (row);
	DisplayItem *obj = g_list_model_get_item (G_LIST_MODEL (m_store), m_row_index);
	if(obj==NULL) return;
	gint id_value;
	gchar* label_value;
	g_object_get (obj, "id", &id_value, NULL);
	g_object_get (obj, "label", &label_value, NULL);
	m_id_selection=id_value;
	//g_print("m_id_selection = %d\n",m_id_selection);
}

//------------------------------------------------------------------------------------

static int compare_times (gconstpointer a, gconstpointer b, gpointer data)
{
  //Negative value if a < b; zero if a = b; positive value if a > b
  int starttime_a, starttime_b;
  g_object_get ((gpointer)a, "starttime", &starttime_a, NULL);
  g_object_get ((gpointer)b, "starttime", &starttime_b, NULL);

 // g_print("compare: starttime_a = %d starttime_b = %d\n", starttime_a, starttime_b);

  if (starttime_a>starttime_b) return 1;
    else if (starttime_a<starttime_b)return -1;
    return 0;

  //return starttime_a - starttime_b;
}



//-----------------------------------------------------------------
// Display events
//-----------------------------------------------------------------


static void display_events(int year, int month, int day) {

    gint evt_id;
    const gchar *summary_str;
    const gchar *location_str;
    const gchar *description_str;

    gint start_year;
    gint start_month;
    gint start_day;
    gint start_hour;
    gint start_min;

    gint end_year;
    gint end_month;
    gint end_day;
    gint end_hour;
    gint end_min;

    gint is_yearly;
    gint is_allday;
    gint is_priority;
    gint has_reminder;
    gint reminder_min;

    gint start_time_sort =0;

   //Display

   g_list_store_remove_all (m_store);//clear

   //g_print("There are %d values in the evt_arry\n",evt_arry->len);

  //loop through the events array adding events to listbox store

 for (int i=0; i<evt_arry->len; i++) {

   //g_print("event array loop\n");


   CalendarEvent *evt =g_array_index(evt_arry, CalendarEvent*, i);

   g_object_get (evt, "eventid", &evt_id, NULL);
   g_object_get (evt, "summary", &summary_str, NULL);
   g_object_get (evt, "location", &location_str, NULL);
   g_object_get (evt, "description", &description_str, NULL);
   g_object_get (evt, "startyear", &start_year, NULL);
   g_object_get (evt, "startmonth", &start_month, NULL);
   g_object_get (evt, "startday", &start_day, NULL);
   g_object_get (evt, "starthour", &start_hour, NULL);
   g_object_get (evt, "startmin", &start_min, NULL);
   g_object_get (evt, "endyear", &end_year, NULL);
   g_object_get (evt, "endmonth", &end_month, NULL);
   g_object_get (evt, "endday", &end_day, NULL);
   g_object_get (evt, "endhour", &end_hour, NULL);
   g_object_get (evt, "endmin", &end_min, NULL);
   g_object_get (evt, "isyearly", &is_yearly, NULL);
   g_object_get (evt, "isallday", &is_allday, NULL);
   g_object_get (evt, "ispriority", &is_priority, NULL);
   g_object_get (evt, "hasreminder", &has_reminder, NULL);
   g_object_get (evt, "remindermin", &reminder_min, NULL);

   //starthour_str = g_strdup_printf("%d", start_hour);
   //startmin_str = g_strdup_printf("%d", start_min);


   //g_print("event summary = %s\n", summary_str);
   //g_print("start hour = %s\n", starthour_str);
  // g_print("start min = %s\n", startmin_str);

   if ((year==start_year && month ==start_month && day==start_day)
	   || (is_yearly && month==start_month && day==start_day))
   {

	   gchar *display_str="";
	   gchar *time_str="";
	   gchar *starthour_str="";
	   gchar *startmin_str="";
	   gchar *endhour_str="";
	   gchar *endmin_str="";
	   gchar *ampm_str=" ";

	   if(m_12hour_format) {

		   if (start_hour >=13 && start_hour<=23) {
			   //start_hour= start_hour-12;
			   int shour=start_hour;
			   shour=shour-12;
			   ampm_str="pm ";
			   starthour_str = g_strdup_printf("%d", shour);

		   }
		   else {
			   ampm_str="am ";
			   starthour_str = g_strdup_printf("%d", start_hour);
		   }
	   } //12
	   else {
		   starthour_str = g_strdup_printf("%d", start_hour);
	   }//24

	   startmin_str = g_strdup_printf("%d", start_min);

	   if (start_min <10){
		   time_str = g_strconcat(time_str, starthour_str,":0", startmin_str,NULL);
	   } else
	   {
		   time_str = g_strconcat(time_str, starthour_str,":", startmin_str,NULL);
	   }

	   time_str = g_strconcat(time_str, ampm_str,NULL);



	   if(m_show_end_time){

		   if(m_12hour_format) {
			   ampm_str="";

			   if (end_hour >=13 && end_hour<=23) {
				   end_hour= end_hour-12;
				   ampm_str="pm ";
				   endhour_str = g_strdup_printf("%d", end_hour);

			   }
			   else {
				   ampm_str="am ";
				   endhour_str = g_strdup_printf("%d", end_hour);
			   }
		   } //12
		   else {
			   endhour_str = g_strdup_printf("%d", end_hour);
		   }//24

		   //endhour_str = g_strdup_printf("%d", end_hour);
		   endmin_str = g_strdup_printf("%d", end_min);

		   if (end_min <10){
			   time_str = g_strconcat(time_str,"to ",  endhour_str,":0", endmin_str,NULL);
		   } else {
			   time_str = g_strconcat(time_str,"to ", endhour_str,":", endmin_str,NULL);
		   }
		   time_str = g_strconcat(time_str, ampm_str,NULL);
	   }//show_end_time


	   if (is_allday) {
		   time_str="All day. ";
	   }
	   else {
		   time_str = g_strconcat(time_str,NULL);
	   }
	   if(m_show_location){
		   if(strlen(location_str) ==0)
		   {
			   display_str = g_strconcat(display_str,time_str,summary_str,". ", NULL);
		   }
		   else {
			   display_str = g_strconcat(display_str,time_str,summary_str, ". ",location_str, ".", NULL);
		   }
	   }
	   else {
		   display_str = g_strconcat(display_str,time_str,summary_str, ". ", NULL);
	   }


	   if(is_priority) {
		   display_str=g_strconcat(display_str, " High Priority.", NULL);
	   }

	   display_str=g_strconcat(display_str, "\n", NULL);



	   //Display day events

// 	   display_str = g_strconcat(display_str,starthour_str,":",startmin_str, " ", summary_str," ",location_str," ", NULL);

	   if(is_allday) {
		   start_hour=0;
		   start_min=0;
	   }
	   start_time_sort=start_hour*60*60 + 60*start_min;//seconds
	   //g_print("label = %s. start_time_sort = %d\n",display_str, start_time_sort);

	   DisplayItem *item=NULL;

	   item = g_object_new (display_item_get_type (),
						  "id",     evt_id,
						 "label",  display_str,
						 "starttime", start_time_sort,
						 NULL);



		g_list_store_insert_sorted(m_store, item, compare_times, NULL);

	   //g_list_store_append (m_store, item);
	   g_object_unref (item);

   }

  } //event array for loop


  //g_list_store_insert_sorted(m_store, obj, compare_items, NULL);
 // g_object_unref (obj);



}


//----------------------------------------------------------------
// Callback home (go to current date)
//-----------------------------------------------------------------
static void callbk_home(GSimpleAction * action, GVariant *parameter, gpointer user_data){
	
GtkWindow *window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (user_data),
															 GTK_TYPE_WINDOW));

	if( !GTK_IS_WINDOW(window)) {
		g_print("CRITICAL: callbk home: not a window\n");
		return;
	}


	GtkWidget *calendar =g_object_get_data(G_OBJECT(window), "window-calendar-key");
	GtkWidget *label_date =g_object_get_data(G_OBJECT(window), "window-label-date-key");


	GDateTime *date_time;
	date_time = g_date_time_new_now_local();   // get local datetime

	m_today_year = g_date_time_get_year (date_time);
	m_today_month=g_date_time_get_month(date_time);
	m_today_day =g_date_time_get_day_of_month(date_time);
	//g_print("Today is : %d-%d-%d \n", m_today_day, m_today_month,m_today_year);

	m_start_day=m_today_day;
	m_start_month=m_today_month;
	m_start_year=m_today_year;

	gchar* date_str="";
	gchar* weekday_str="";
	gint day_of_week = g_date_time_get_day_of_week (date_time);


	switch(day_of_week)
	{
		case G_DATE_MONDAY:
			weekday_str="Monday";
			break;
		case G_DATE_TUESDAY:
			weekday_str="Tuesday";
			break;
		case G_DATE_WEDNESDAY:
			weekday_str="Wednesday";
			break;
		case G_DATE_THURSDAY:
			weekday_str="Thursday";
			break;
		case G_DATE_FRIDAY:
			weekday_str="Friday";
			break;
		case G_DATE_SATURDAY:
			weekday_str="Saturday";
			break;
		case G_DATE_SUNDAY:
			weekday_str="Sunday";
			break;
		default:
			weekday_str="Unknown";
	}//switch

	//gchar *day_str = g_strdup_printf ("%02d", day);
	//gchar *year_str = g_strdup_printf ("%02d", year);

	gchar* day_str =  g_strdup_printf("%d",m_start_day);
	gchar *year_str = g_strdup_printf("%d",m_start_year );

	//gchar * date_str="";

	date_str =g_strconcat(date_str,weekday_str," ", day_str, " ", NULL);

	switch(m_start_month)
	{
		case G_DATE_JANUARY:
			date_str =g_strconcat(date_str,"January ",year_str, NULL);
			break;
		case G_DATE_FEBRUARY:
			date_str =g_strconcat(date_str,"February ",year_str, NULL);
			break;
		case G_DATE_MARCH:
			date_str =g_strconcat(date_str,"March ",year_str, NULL);
			break;
		case G_DATE_APRIL:
			date_str =g_strconcat(date_str,"April ",year_str, NULL);
			break;
		case G_DATE_MAY:
			date_str =g_strconcat(date_str,"May ",year_str, NULL);
			break;
		case G_DATE_JUNE:
			date_str =g_strconcat(date_str,"June ",year_str, NULL);
			break;
		case G_DATE_JULY:
			date_str =g_strconcat(date_str,"July ",year_str, NULL);
			break;
		case G_DATE_AUGUST:
			date_str =g_strconcat(date_str,"August ",year_str, NULL);
			break;
		case G_DATE_SEPTEMBER:
			date_str =g_strconcat(date_str,"September ",year_str, NULL);
			break;
		case G_DATE_OCTOBER:
			date_str =g_strconcat(date_str,"October ",year_str, NULL);
			break;
		case G_DATE_NOVEMBER:
			date_str =g_strconcat(date_str,"November ",year_str, NULL);
			break;
		case G_DATE_DECEMBER:
			date_str =g_strconcat(date_str,"December ",year_str, NULL);
			break;
		default:
			date_str =g_strconcat(date_str,"Unknown ",year_str, NULL);
	}



	if (m_holidays) {

		//append holiday text
		gchar * holiday_str = get_holiday_str(m_start_day);
		date_str =g_strconcat(date_str," ",holiday_str, NULL);
	}

	int event_num =get_number_of_day_events();
	if(event_num>0) {
		date_str =g_strconcat(date_str,"*", NULL);
	}


	gtk_label_set_text(GTK_LABEL(label_date), date_str);
	gtk_calendar_select_day (GTK_CALENDAR(calendar), date_time);
	set_event_marks_on_calendar(GTK_CALENDAR(calendar));

	display_events(m_start_year,m_start_month,m_start_day);




}
//----------------------------------------------------------------
// Callback quit
//-----------------------------------------------------------------
static void callbk_quit(GSimpleAction * action,
							G_GNUC_UNUSED GVariant      *parameter,
							              gpointer       user_data)
{
	//g_print("quit  called save data\n");
	save_csv_file();
	//g_print("free event g_array\n");
    g_array_free(evt_arry, FALSE);
	g_application_quit(G_APPLICATION(user_data));
	
}



//---------------------------------------------------------------------
// startup and shutdown
//---------------------------------------------------------------------
int file_exists(const char *file_name)
{
    FILE *file;
    file = fopen(file_name, "r");
    if (file){       
        fclose(file);
        return 1; //file exists return 1
    }
    return 0; //file does not exist
}

static void startup (GtkApplication *app)
{
	//g_print("startup  called\n");

	evt_arry =g_array_new(FALSE, FALSE, sizeof(CALENDAR_TYPE_EVENT)); //setup arraylist

	if(file_exists("events.csv"))
	{
		//g_print("events.csv exists-load it\n");
		load_csv_file();
	}

	//print_array(evt_arry); //debugging
}
//----------------------------------------------------------------
// Callback shutdown
//-----------------------------------------------------------------
void callbk_shutdown(GtkWindow *window, gint response_id,  gpointer  user_data){

	//g_print("shutdown called save data\n");
	save_csv_file();

	//g_print("free event g_array\n");
    g_array_free(evt_arry, FALSE);

}


//----------------------------------------------------------------------------

static void update_date(GtkCalendar *calendar, gpointer user_data){

	GtkWidget *label_date = (GtkWidget *) user_data;

	gchar *date_str ="";
	gint day, month, year;
	GDateTime* datetime = gtk_calendar_get_date (GTK_CALENDAR (calendar));
	year = g_date_time_get_year (datetime);
	month=g_date_time_get_month(datetime);
	day =g_date_time_get_day_of_month(datetime);

	m_start_day =day;
	m_start_month=month;
	m_start_year=year;
	m_id_selection=-1;
	m_row_index=-1;
	//g_print("Date: Day = %d Month =%d Year =%d\n", m_start_day, m_start_month,m_start_year);



	gchar* weekday_str="";
	gint day_of_week = g_date_time_get_day_of_week (datetime);

	switch(day_of_week)
	{
		case G_DATE_MONDAY:
			weekday_str="Monday";
			break;
		case G_DATE_TUESDAY:
			weekday_str="Tuesday";
			break;
		case G_DATE_WEDNESDAY:
			weekday_str="Wednesday";
			break;
		case G_DATE_THURSDAY:
			weekday_str="Thursday";
			break;
		case G_DATE_FRIDAY:
			weekday_str="Friday";
			break;
		case G_DATE_SATURDAY:
			weekday_str="Saturday";
			break;
		case G_DATE_SUNDAY:
			weekday_str="Sunday";
			break;
		default:
			weekday_str="Unknown";
	}//switch

	//gchar *day_str = g_strdup_printf ("%02d", day);
	//gchar *year_str = g_strdup_printf ("%02d", year);

	gchar* day_str =  g_strdup_printf("%d",m_start_day);
	gchar *year_str = g_strdup_printf("%d",m_start_year );

	//gchar * date_str="";

	date_str =g_strconcat(date_str,weekday_str," ", day_str, " ", NULL);

	switch(m_start_month)
	{
		case G_DATE_JANUARY:
			date_str =g_strconcat(date_str,"January ",year_str, NULL);
			break;
		case G_DATE_FEBRUARY:
			date_str =g_strconcat(date_str,"February ",year_str, NULL);
			break;
		case G_DATE_MARCH:
			date_str =g_strconcat(date_str,"March ",year_str, NULL);
			break;
		case G_DATE_APRIL:
			date_str =g_strconcat(date_str,"April ",year_str, NULL);
			break;
		case G_DATE_MAY:
			date_str =g_strconcat(date_str,"May ",year_str, NULL);
			break;
		case G_DATE_JUNE:
			date_str =g_strconcat(date_str,"June ",year_str, NULL);
			break;
		case G_DATE_JULY:
			date_str =g_strconcat(date_str,"July ",year_str, NULL);
			break;
		case G_DATE_AUGUST:
			date_str =g_strconcat(date_str,"August ",year_str, NULL);
			break;
		case G_DATE_SEPTEMBER:
			date_str =g_strconcat(date_str,"September ",year_str, NULL);
			break;
		case G_DATE_OCTOBER:
			date_str =g_strconcat(date_str,"October ",year_str, NULL);
			break;
		case G_DATE_NOVEMBER:
			date_str =g_strconcat(date_str,"November ",year_str, NULL);
			break;
		case G_DATE_DECEMBER:
			date_str =g_strconcat(date_str,"December ",year_str, NULL);
			break;
		default:
			date_str =g_strconcat(date_str,"Unknown ",year_str, NULL);
	}



	if (m_holidays) {
		//append holiday text
		gchar * holiday_str = get_holiday_str(m_start_day);
		date_str =g_strconcat(date_str," ",holiday_str," ", NULL);
	}

	int event_num =get_number_of_day_events();
	if(event_num>0) {
		date_str =g_strconcat(date_str,"*", NULL);
	}

	//g_date_time_unref (datetime);

	gtk_label_set_text(GTK_LABEL(label_date),date_str);

}


//----------------------------------------------------------------------
// Calendar callbks
//----------------------------------------------------------------------

static void callbk_calendar_next_month(GtkCalendar *calendar, gpointer user_data) {


    GtkWidget *label_date = (GtkWidget *) user_data;

	update_date(calendar, label_date);
	display_events(m_start_year,m_start_month,m_start_day);
	set_event_marks_on_calendar(GTK_CALENDAR(calendar));

	//if(m_holidays) mark_holidays_on_calendar(GTK_CALENDAR(calendar), m_start_month, m_start_year);

}

static void callbk_calendar_prev_month(GtkCalendar *calendar, gpointer user_data) {


    GtkWidget *label_date = (GtkWidget *) user_data;

	update_date(calendar, label_date);
	display_events(m_start_year,m_start_month,m_start_day);
	set_event_marks_on_calendar(GTK_CALENDAR(calendar));

	//if(m_holidays) mark_holidays_on_calendar(GTK_CALENDAR(calendar), m_start_month, m_start_year);


}

static void callbk_calendar_next_year(GtkCalendar *calendar, gpointer user_data) {

	GtkWidget *label_date = (GtkWidget *) user_data;

	update_date(calendar, label_date);
	display_events(m_start_year,m_start_month,m_start_day);
	set_event_marks_on_calendar(GTK_CALENDAR(calendar));

	//if(m_holidays) mark_holidays_on_calendar(GTK_CALENDAR(calendar), m_start_month, m_start_year);

}

static void callbk_calendar_prev_year(GtkCalendar *calendar, gpointer user_data) {

	GtkWidget *label_date = (GtkWidget *) user_data;

	update_date(calendar, label_date);
	display_events(m_start_year,m_start_month,m_start_day);
	set_event_marks_on_calendar(GTK_CALENDAR(calendar));

	//if(m_holidays) mark_holidays_on_calendar(GTK_CALENDAR(calendar), m_start_month, m_start_year);

}


static void callbk_calendar_day_selected(GtkCalendar *calendar, gpointer user_data)
{

	GtkWidget *label_date = (GtkWidget *) user_data;
	update_date(calendar, label_date); //updates m_start_year,m_month, m_day
	display_events(m_start_year,m_start_month,m_start_day);

}

//-------------------------------------------------------------------------
// save and load
//-------------------------------------------------------------------------

//----------------------------------------------------------------------
//  csv file storage functions
//----------------------------------------------------------------------

int break_fields(char *s, char** data, int n)
{
	//n = number of fields
	//assumes comma delimiter

	int fields=0;
	int i;
	char *start=s; //start at begiining of string
	char *end=s; //walking pointer

	for(i=0; i<n; i++)
	{
		while(*end !=',' && *end != '\0') {
			end++;
		}

		if(*end =='\0') {

			data[i]=(char*) malloc(strlen(start)+1);
			strcpy(data[i],start);
			fields++;
			break;
		}
		else if (*end ==',') {
			*end ='\0';
			data[i]=(char*) malloc(strlen(start)+1);
			strcpy(data[i],start);
			start=end+1;
			end=start;
			fields++;
		}
	}
	return fields;
}

void load_csv_file(){

	int count=0;
	int field_num =19; //fix for now?
	char *data[field_num]; // fields
	int i = 0; //counter
    int total_num_lines = 0; //total number of lines
    int ret;

	GFile *file;
    GFileInputStream *file_stream=NULL;
	GDataInputStream *input = NULL;

    file = g_file_new_for_path("events.csv");

	file_stream = g_file_read(file, NULL, NULL);
	if(!file_stream) {
		g_print("CRITICAL: error: unable to open database\n");
		return;
	}

	input = g_data_input_stream_new (G_INPUT_STREAM (file_stream));

	while (TRUE) {

		char *line;
		line = g_data_input_stream_read_line (input, NULL, NULL, NULL);
		if (line == NULL) break;

		//Event e;
		CalendarEvent *evt= g_object_new(CALENDAR_TYPE_EVENT,0);

        ret=break_fields(line,data,field_num);
        //g_print("break_fields return value %d\n",ret);

		for (int j=0; j<field_num;j++) {

			if (j==0) g_object_set (evt, "eventid", count, NULL);
			if (j==1) g_object_set (evt, "summary", g_strdup(data[j]), NULL);
			if (j==2) g_object_set (evt, "location", g_strdup(data[j]), NULL);
			if (j==3) g_object_set (evt, "description", g_strdup(data[j]), NULL);
			if (j==4) g_object_set (evt, "startyear", atoi(data[j]), NULL);

			if (j==5) g_object_set (evt, "startmonth", atoi(data[j]), NULL);
			if (j==6) g_object_set (evt, "startday", atoi(data[j]), NULL);
			if (j==7) g_object_set (evt, "starthour", atoi(data[j]), NULL);
			if (j==8) g_object_set (evt, "startmin", atoi(data[j]), NULL);
			if (j==9) g_object_set (evt, "endyear", atoi(data[j]), NULL); //to do
			if (j==10) g_object_set (evt, "endmonth",atoi(data[j]), NULL);
			if (j==11) g_object_set (evt, "endday", atoi(data[j]), NULL);
			if (j==12) g_object_set (evt, "endhour", atoi(data[j]), NULL);
			if (j==13) g_object_set (evt, "endmin", atoi(data[j]), NULL);
			if (j==14) g_object_set (evt, "isyearly", atoi(data[j]), NULL);
			if (j==15) g_object_set (evt, "isallday", atoi(data[j]), NULL);
			if (j==16) g_object_set (evt, "ispriority", atoi(data[j]), NULL);
			if (j==17) g_object_set (evt, "hasreminder", atoi(data[j]), NULL);
			if (j==18) g_object_set (evt, "remindermin", atoi(data[j]), NULL);

			free(data[j]);

		}

		g_array_append_val(evt_arry, evt);
		count=count+1;
		i++;
	}

	total_num_lines=i;
    //g_print("total_number_of_lines =%d\n",total_num_lines);
	g_object_unref (file_stream);
	g_object_unref (file);

}

void save_csv_file(){


	GFile *file;
	gchar *file_name ="events.csv";

	GFileOutputStream *file_stream;
	GDataOutputStream *data_stream;

	//g_print("Saving csv data with filename = %s\n",file_name);

	file = g_file_new_for_path (file_name);
	file_stream = g_file_replace (file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL);

	if (file_stream == NULL) {
		g_object_unref (file);
		g_print("CRITICAL error: unable to open and save events file\n");
		return;
	}

	data_stream = g_data_output_stream_new (G_OUTPUT_STREAM (file_stream));

	for (int i=0; i<evt_arry->len; i++) {

		gchar *line="";
		gint evt_id=0;
		gchar *summary_str="";
		gchar *location_str="";
		gchar *description_str="";

		gint start_year=0;
		gint start_month=0;
		gint start_day=0;
		gint start_hour=0;
		gint start_min=0;

		gint end_year=0;
		gint end_month=0;
		gint end_day=0;
		gint end_hour=0;
		gint end_min=0;

		gint is_yearly=0;
		gint is_allday=0;
		gint is_priority=0;
		gint has_reminder=0;
		gint reminder_min=0;


		//g_print("save csv evt_arry loop\n");

		CalendarEvent *evt =g_array_index(evt_arry, CalendarEvent*, i);

		g_object_get (evt, "eventid", &evt_id, NULL);
		g_object_get (evt, "summary", &summary_str, NULL);
		g_object_get (evt, "location", &location_str, NULL);
		g_object_get (evt, "description", &description_str, NULL);
		g_object_get (evt, "startyear", &start_year, NULL);
		g_object_get (evt, "startmonth", &start_month, NULL);
		g_object_get (evt, "startday", &start_day, NULL);
		g_object_get (evt, "starthour", &start_hour, NULL);
		g_object_get (evt, "startmin", &start_min, NULL);
		g_object_get (evt, "endyear", &end_year, NULL);
		g_object_get (evt, "endmonth", &end_month, NULL);
		g_object_get (evt, "endday", &end_day, NULL);
		g_object_get (evt, "endhour", &end_hour, NULL);
		g_object_get (evt, "endmin", &end_min, NULL);
		g_object_get (evt, "isyearly", &is_yearly, NULL);
		g_object_get (evt, "isallday", &is_allday, NULL);
		g_object_get (evt, "ispriority", &is_priority, NULL);
		g_object_get (evt, "hasreminder", &has_reminder, NULL);
		g_object_get (evt, "remindermin", &reminder_min, NULL);


		gchar *id_str = g_strdup_printf("%d", evt_id);
		gchar *start_year_str = g_strdup_printf("%d", start_year);
		gchar *start_month_str = g_strdup_printf("%d", start_month);
		gchar *start_day_str = g_strdup_printf("%d", start_day);

		gchar *start_hour_str = g_strdup_printf("%d", start_hour);
		gchar *start_min_str = g_strdup_printf("%d", start_min);

		gchar *end_year_str = g_strdup_printf("%d", end_year); //to do
		gchar *end_month_str = g_strdup_printf("%d", end_month);
		gchar *end_day_str = g_strdup_printf("%d", end_day);

		gchar *end_hour_str = g_strdup_printf("%d", end_hour);
		gchar *end_min_str = g_strdup_printf("%d", end_min);


		gchar *isyearly_str = g_strdup_printf("%d", is_yearly);
		gchar *isallday_str = g_strdup_printf("%d", is_allday);
		gchar *ispriority_str = g_strdup_printf("%d", is_priority);
		gchar *hasreminder_str = g_strdup_printf("%d", has_reminder);
		gchar *remindermin_str = g_strdup_printf("%d", reminder_min);

// 		g_print("e id = %s\n", id_str);
// 		g_print("e summary = %s\n", summary_str);
// 		g_print("e location = %s\n", location_str);
// 		g_print("e description = %s\n", description_str);
//
// 		g_print("start year = %s\n", start_year_str);
// 		g_print("start month = %s\n", start_month_str);
// 		g_print("start day = %s\n", start_day_str);
// 		g_print("start hour = %s\n", start_hour_str);
// 		g_print("start min = %s\n", start_min_str);
//
// 		g_print("end year = %s\n", end_year_str);
// 		g_print("end month = %s\n", end_month_str);
// 		g_print("end day = %s\n", end_day_str);
// 		g_print("end hour = %s\n", end_hour_str);
// 		g_print("end min = %s\n", end_min_str);
//
// 		g_print("isyearly = %s\n", isyearly_str);
// 		g_print("isallday = %s\n", isallday_str);
// 		g_print("ispriority = %s\n", ispriority_str);
// 		g_print("hasreminder = %s\n", hasreminder_str);
// 		g_print("reminder_min = %s\n", remindermin_str);


		line =g_strconcat(line,
					id_str,",",
					summary_str,",",
					location_str,",",
					description_str,",",
					start_year_str,",",
					start_month_str,",",
					start_day_str,",",
					start_hour_str,",",
					start_min_str,",",
					end_year_str,",",
					end_month_str,",",
					end_day_str,",",
					end_hour_str,",",
					end_min_str,",",
					isyearly_str,",",
					isallday_str,",",
					ispriority_str,",",
					hasreminder_str,",",
					remindermin_str,",",
					"\n",
					NULL);

		//g_print("%s",line); //debugging

		g_data_output_stream_put_string (data_stream, line, NULL, NULL)	;

	}

	g_object_unref (data_stream);
	g_object_unref (file_stream);
	g_object_unref (file);

}

//-----------------------------------------------------------------------------------
// Preferences
//----------------------------------------------------------------------------------

static void callbk_set_preferences(GtkButton *button, gpointer  user_data){

	GtkWidget *window = user_data;
	GtkWidget *calendar =g_object_get_data(G_OBJECT(window), "window-calendar-key");
	GtkWidget *label_date =g_object_get_data(G_OBJECT(window), "window-label-date-key");
	GtkWidget *dialog = g_object_get_data(G_OBJECT(button), "dialog-key");

	//calendar
	GtkWidget *check_button_hour_format= g_object_get_data(G_OBJECT(button), "check-button-hour-format-key");
	GtkWidget *check_button_show_end_time= g_object_get_data(G_OBJECT(button), "check-button-show-end-time-key");
	GtkWidget *check_button_show_location= g_object_get_data(G_OBJECT(button), "check-button-show-location-key");

	GtkWidget *check_button_holidays= g_object_get_data(G_OBJECT(button), "check-button-holidays-key");

	//talking
	GtkWidget *check_button_talk= g_object_get_data(G_OBJECT(button), "check-button-talk-key");
    GtkWidget *check_button_talk_startup= g_object_get_data(G_OBJECT(button), "check-button-talk-startup-key");
    GtkWidget *check_button_talk_event_number= g_object_get_data(G_OBJECT(button), "check-button-talk-event-number-key");
	GtkWidget *check_button_talk_times= g_object_get_data(G_OBJECT(button), "check-button-talk-times-key");
	GtkWidget *check_button_talk_location= g_object_get_data(G_OBJECT(button), "check-button-talk-location-key");
    GtkWidget *check_button_talk_priority= g_object_get_data(G_OBJECT(button), "check-button-talk-priority-key");

    GtkWidget *check_button_reset_all= g_object_get_data(G_OBJECT(button), "check-button-reset-all-key");


	m_12hour_format=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_hour_format));
	m_show_end_time=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_show_end_time));
	m_show_location=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_show_location));

	m_holidays=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_holidays));

	m_talk=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_talk));
	m_talk_at_startup=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_talk_startup));
	m_talk_event_number=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_talk_event_number));
	m_talk_time=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_talk_times));
	m_talk_location=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_talk_location));
	m_talk_priority=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_talk_priority));

	m_reset_preferences=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_reset_all));

	if(m_reset_preferences) {
	//reset everything
	m_12hour_format=1;
	m_show_end_time=0;
	m_show_location=1;
	m_holidays=0;

	m_talk = 1;
	m_talk_at_startup=1;
	m_talk_event_number=1;
	m_talk_time=1;
	m_talk_location=0;
	m_talk_priority=0;

    m_reset_preferences=0; //toggle

	}

	config_write();


	update_date(GTK_CALENDAR(calendar),label_date);
	display_events(m_start_year,m_start_month,m_start_day);
	gtk_window_destroy(GTK_WINDOW(dialog));

}

//---------------------------------------------------------------------
// callback preferences
//---------------------------------------------------------------------
static void callbk_preferences(GSimpleAction* action, GVariant *parameter,gpointer user_data)
{
	//g_print("Preferences callbk\n");

	GtkWidget *window =user_data;

	GtkWidget *dialog;
	GtkWidget *box;

	//Check buttons
	GtkWidget *check_button_talk;
	GtkWidget *check_button_talk_startup;
	GtkWidget *check_button_talk_event_number;
	GtkWidget *check_button_talk_times;
	GtkWidget *check_button_talk_location;
	GtkWidget *check_button_talk_priority;


	GtkWidget *check_button_hour_format;
	GtkWidget *check_button_show_end_time;
	GtkWidget *check_button_show_location;

	GtkWidget *check_button_holidays;

	GtkWidget *check_button_reset_all;

	GtkWidget *button_set;

	dialog =gtk_window_new(); //gtk_dialog_new_with_buttons to be deprecated gtk4.10

	gtk_window_set_title (GTK_WINDOW (dialog), "Preferences");
	gtk_window_set_default_size(GTK_WINDOW(dialog),350,100);

	box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);
	gtk_window_set_child (GTK_WINDOW (dialog), box);

	button_set = gtk_button_new_with_label ("Set Preferences");
	g_signal_connect (button_set, "clicked", G_CALLBACK (callbk_set_preferences), window);


	//calendar
	check_button_hour_format = gtk_check_button_new_with_label ("12 Hour Format");
	check_button_show_end_time= gtk_check_button_new_with_label ("Show End Time");
	check_button_show_location= gtk_check_button_new_with_label ("Show Location");

	check_button_holidays = gtk_check_button_new_with_label ("Public Holidays");

	//talk
	check_button_talk = gtk_check_button_new_with_label ("Talk");
	check_button_talk_startup = gtk_check_button_new_with_label ("Talk At Startup");
	check_button_talk_event_number = gtk_check_button_new_with_label ("Talk Event Number");
	check_button_talk_times = gtk_check_button_new_with_label ("Talk Time");
	check_button_talk_location = gtk_check_button_new_with_label ("Talk Location");
	check_button_talk_priority = gtk_check_button_new_with_label ("Talk Priority");

	check_button_reset_all = gtk_check_button_new_with_label ("Reset All");

	//calendar
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_hour_format),m_12hour_format);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_show_end_time), m_show_end_time);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_show_location), m_show_location);

	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_holidays),m_holidays);

	//talk
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk), m_talk);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_startup), m_talk_at_startup);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_event_number), m_talk_event_number);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_times), m_talk_time);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_location), m_talk_location);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_priority), m_talk_priority);

	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_reset_all), m_reset_preferences);

	//data setters
	g_object_set_data(G_OBJECT(button_set), "dialog-key",dialog);
	//calendar
	g_object_set_data(G_OBJECT(button_set), "check-button-hour-format-key",check_button_hour_format);
	g_object_set_data(G_OBJECT(button_set), "check-button-show-end-time-key",check_button_show_end_time);
	g_object_set_data(G_OBJECT(button_set), "check-button-show-location-key",check_button_show_location);

	g_object_set_data(G_OBJECT(button_set), "check-button-holidays-key",check_button_holidays);

	g_object_set_data(G_OBJECT(button_set), "check-button-talk-key",check_button_talk);
	g_object_set_data(G_OBJECT(button_set), "check-button-talk-startup-key",check_button_talk_startup);
	g_object_set_data(G_OBJECT(button_set), "check-button-talk-event-number-key",check_button_talk_event_number);
	g_object_set_data(G_OBJECT(button_set), "check-button-talk-times-key",check_button_talk_times);
	g_object_set_data(G_OBJECT(button_set), "check-button-talk-location-key",check_button_talk_location);
	g_object_set_data(G_OBJECT(button_set), "check-button-talk-priority-key",check_button_talk_priority);

	g_object_set_data(G_OBJECT(button_set), "check-button-reset-all-key",check_button_reset_all);


	gtk_box_append(GTK_BOX(box), check_button_hour_format);
	gtk_box_append(GTK_BOX(box), check_button_show_end_time);
	gtk_box_append(GTK_BOX(box), check_button_show_location);
	gtk_box_append(GTK_BOX(box), check_button_holidays);

	gtk_box_append(GTK_BOX(box), check_button_talk);
	gtk_box_append(GTK_BOX(box), check_button_talk_startup);
	gtk_box_append(GTK_BOX(box), check_button_talk_event_number);
	gtk_box_append(GTK_BOX(box), check_button_talk_times);
	gtk_box_append(GTK_BOX(box), check_button_talk_location);
	gtk_box_append(GTK_BOX(box), check_button_talk_priority);

	gtk_box_append(GTK_BOX(box), check_button_reset_all);

	gtk_box_append(GTK_BOX(box), button_set);

	gtk_window_present (GTK_WINDOW (dialog));


}


//----------------------------------------------------------------
// Calback information
//-----------------------------------------------------------------
static void callbk_info(GSimpleAction *action, GVariant *parameter,  gpointer user_data)
{

	GtkWidget *window =user_data;
	GtkWidget *dialog;
	GtkWidget *box;
	gint response;


	GtkWidget *label_record_info;
	GtkWidget *label_record_number;

	GtkWidget *label_font_info;

	GtkWidget *label_desktop_font;
	GtkWidget *label_gnome_text_scale;

	GtkWidget *label_work_dir;
	GtkWidget *label_working_dir;

	GtkWidget *label_speech_engine;
	GtkWidget *label_flite;

	GtkWidget *label_keyboard_shortcuts;
	GtkWidget *label_speak_shortcut;
	GtkWidget *label_home_shortcut;

	GSettings *settings;

	PangoAttrList *attrs;
	attrs = pango_attr_list_new ();
	pango_attr_list_insert (attrs, pango_attr_weight_new (PANGO_WEIGHT_BOLD));

    //replaced dialog with window as gtk_dialog_new_with_buttons to be deprecated gtk4.10
	dialog =gtk_window_new();

	gtk_window_set_default_size(GTK_WINDOW(dialog),380,100);
	gtk_window_set_title (GTK_WINDOW (dialog), "Information");

	box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);
	gtk_window_set_child (GTK_WINDOW (dialog), box);

	label_keyboard_shortcuts=gtk_label_new("Keyboard Shortcuts");
	 gtk_label_set_attributes (GTK_LABEL (label_keyboard_shortcuts), attrs);

	label_speak_shortcut=gtk_label_new("Speak: Spacebar");
	label_home_shortcut=gtk_label_new("Goto Today: Home Key");


	label_record_info=gtk_label_new("Storage");
	gtk_label_set_attributes (GTK_LABEL (label_record_info), attrs);

	char* record_num_str =" Number of records = ";
	char* n_str = g_strdup_printf("%d", get_total_number_of_events());
	record_num_str = g_strconcat(record_num_str, n_str,NULL);
	label_record_number =gtk_label_new(record_num_str);


	label_font_info=gtk_label_new("Font");
	gtk_label_set_attributes (GTK_LABEL (label_font_info), attrs);

	settings = g_settings_new ("org.gnome.desktop.interface");
	gchar* desktop_font_str = g_settings_get_string (settings, "font-name");

	char* desktop_str = "Desktop Font = ";
	desktop_str =g_strconcat(desktop_str, desktop_font_str,NULL);
	label_desktop_font=gtk_label_new(desktop_str);

	gdouble sf =g_settings_get_double (settings,"text-scaling-factor");
	//g_print("scale factor =%0.1lf\n",sf);
	char* gnome_text_scale_factor ="Gnome Text Scale Factor = ";
	char* font_scale_value_str = g_strdup_printf("%0.2lf", sf);
	gnome_text_scale_factor=g_strconcat(gnome_text_scale_factor, font_scale_value_str,NULL);
	label_gnome_text_scale=gtk_label_new(gnome_text_scale_factor);

	char *cur_dir;
	cur_dir = g_get_current_dir ();
	char* dir_str ="";
	dir_str = g_strconcat(dir_str, cur_dir,NULL);
	label_work_dir=gtk_label_new("Working Directory");
	gtk_label_set_attributes (GTK_LABEL (label_work_dir), attrs);
	label_working_dir=gtk_label_new(dir_str);
	//g_print("current directory = %s\n", cur_dir);


	//check for flite

	gchar* flite_str="";
	if(g_file_test(g_build_filename("/usr/bin/", "flite", NULL), G_FILE_TEST_IS_REGULAR))
	{
		flite_str = g_strconcat(flite_str, "Flite is installed",NULL);
	}
	else
	{
		flite_str = g_strconcat(flite_str, "Flite not installed. Install Flite for speech.",NULL);
	}

	label_speech_engine=gtk_label_new("Speech Synthesizer");
	gtk_label_set_attributes (GTK_LABEL (label_speech_engine), attrs);
	label_flite=gtk_label_new(flite_str);

	gtk_box_append(GTK_BOX(box),label_keyboard_shortcuts);
	gtk_box_append(GTK_BOX(box), label_speak_shortcut);
	gtk_box_append(GTK_BOX(box),label_home_shortcut);

	gtk_box_append(GTK_BOX(box), label_record_info);
	gtk_box_append(GTK_BOX(box), label_record_number);

	gtk_box_append(GTK_BOX(box),label_font_info);
	gtk_box_append(GTK_BOX(box),label_desktop_font);
	gtk_box_append(GTK_BOX(box),label_gnome_text_scale);

	gtk_box_append(GTK_BOX(box),label_speech_engine);
	gtk_box_append(GTK_BOX(box),label_flite);

	gtk_box_append(GTK_BOX(box),label_work_dir);
	gtk_box_append(GTK_BOX(box),label_working_dir);

	pango_attr_list_unref (attrs);
	gtk_window_present (GTK_WINDOW (dialog));


}


static void callbk_about(GSimpleAction * action, GVariant *parameter, gpointer user_data){


	GtkWidget *window = user_data;

	const gchar *authors[] = {"Alan Crispin", NULL};
	GtkWidget *about_dialog;
	about_dialog = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(about_dialog),GTK_WINDOW(window));
	gtk_widget_set_size_request(about_dialog, 200,200);
    gtk_window_set_modal(GTK_WINDOW(about_dialog),TRUE);
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "Tiki Calendar (Gtk4 version)");
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about_dialog), "Version 0.2.6");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog),"Copyright © 2023");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),"Talking calendar");
	gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG(about_dialog), GTK_LICENSE_GPL_3_0);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog),"https://github.com/crispinalan/");
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(about_dialog),"Tiki Calendar Website");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog), authors);
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about_dialog), "x-office-calendar");
	gtk_widget_set_visible (about_dialog, TRUE);


}

//---------------------------------------------------------------------
// create header
//---------------------------------------------------------------------

static void create_header (GtkWindow *window)
{
	GtkWidget *header;
	GtkWidget *button_new_event;
	GtkWidget *button_edit_event;
	GtkWidget *button_delete_selected;
	GtkWidget *menu_button; 

	header = gtk_header_bar_new ();	
	gtk_window_set_titlebar (GTK_WINDOW(window), header);
	
	button_new_event = gtk_button_new_with_label ("New Event");
	gtk_widget_set_tooltip_text(button_new_event, "New calendar event");
	g_signal_connect (button_new_event, "clicked", G_CALLBACK (callbk_new_event), window);

	button_edit_event = gtk_button_new_with_label ("Edit Event");
	gtk_widget_set_tooltip_text(button_edit_event, "Edit selected event");
	g_signal_connect (button_edit_event, "clicked", G_CALLBACK (callbk_edit_event), window);
	

	button_delete_selected = gtk_button_new_with_label ("Delete Event");
	gtk_widget_set_tooltip_text(button_delete_selected, "Delete selected event");
	g_signal_connect (button_delete_selected, "clicked", G_CALLBACK (callbk_delete_selected), window);

	//Packing
	gtk_header_bar_pack_start(GTK_HEADER_BAR (header),button_new_event);
	gtk_header_bar_pack_start(GTK_HEADER_BAR (header),button_edit_event);   
	gtk_header_bar_pack_start(GTK_HEADER_BAR (header), button_delete_selected);

	//Menu model
	GMenu *menu, *section; 	
	menu = g_menu_new ();  

	section = g_menu_new ();
	g_menu_append (section, "Preferences", "app.preferences");
	g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);
	
	section = g_menu_new ();
	g_menu_append (section, "Speak", "app.speak"); 	
	g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);

	section = g_menu_new ();
	g_menu_append (section, "Speak Month", "app.speak_month");
	g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);
	
	section = g_menu_new ();
	g_menu_append (section, "Information", "app.info"); //show app info
	g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);

	section = g_menu_new ();
	g_menu_append (section, "About", "app.about");
	g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);


	section = g_menu_new ();
	g_menu_append (section, "Quit", "app.quit");
	g_menu_append_section (menu, NULL, G_MENU_MODEL(section));
	g_object_unref (section);
	
	//Now hamburger style menu button
	menu_button = gtk_menu_button_new();
	gtk_widget_set_tooltip_text(menu_button, "Menu");
	gtk_menu_button_set_icon_name (GTK_MENU_BUTTON (menu_button),"open-menu-symbolic"); 		
	gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (menu_button), G_MENU_MODEL(menu));
	gtk_header_bar_pack_end(GTK_HEADER_BAR (header), menu_button);



	
}
    
//----------------------------------------------------------------------

static void activate (GtkApplication *app, gpointer  user_data)
{
	GtkWidget *window;
	GtkWidget *header;
	GtkWidget *calendar; //should this be a GtkCalendar and rather than cast with GTK_CALENDAR

	GtkWidget *box;

	GtkWidget *sw; //scrolled window
	GtkWidget* listbox;
	GtkWidget *label_date; //display date

	//GtkTextBuffer *text_buffer;
	gchar *today_str="";

	const gchar *speak_accels[2] = { "space", NULL };
	const gchar *speak_month_accels[2] = { "M", NULL };
	//const gchar *speak_month_accels[2] = { "<ctrl>M", NULL };
	const gchar *home_accels[2] = { "Home", NULL };

	// create a new window, and set its title
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), " ");
	gtk_window_set_default_size(GTK_WINDOW (window),600,400);
	g_signal_connect (window, "destroy", G_CALLBACK (callbk_shutdown), NULL);

	box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);
	gtk_window_set_child (GTK_WINDOW (window), box);

	//storage
	//evt_arry =g_array_new(FALSE, FALSE, sizeof(CALENDAR_TYPE_EVENT)); //setup arraylist
    m_store = g_list_store_new (display_item_get_type()); //setup display store

	label_date = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label_date), 0.5);

	PangoAttrList *attrs;
	attrs = pango_attr_list_new ();
	pango_attr_list_insert (attrs, pango_attr_weight_new (PANGO_WEIGHT_BOLD));
	gtk_label_set_attributes (GTK_LABEL (label_date), attrs);
	pango_attr_list_unref (attrs);

	//set scrolled window
	sw = gtk_scrolled_window_new ();
	gtk_widget_set_hexpand (GTK_WIDGET (sw), true);
	gtk_widget_set_vexpand (GTK_WIDGET (sw), true);


	listbox = gtk_list_box_new ();
	gtk_list_box_set_selection_mode (GTK_LIST_BOX (listbox), GTK_SELECTION_SINGLE);
	gtk_list_box_set_show_separators (GTK_LIST_BOX (listbox), TRUE);
	gtk_list_box_set_header_func (GTK_LIST_BOX (listbox), add_separator, NULL, NULL);
	gtk_list_box_bind_model (GTK_LIST_BOX (listbox), G_LIST_MODEL (m_store), create_widget, NULL, NULL);
	g_signal_connect (listbox, "row-activated", G_CALLBACK (callbk_row_activated),NULL);
	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (sw), listbox);

    m_row_index=-1;
    m_id_selection=-1;

	// calendar
	calendar = gtk_calendar_new(); //gtk calendar
	gtk_calendar_set_show_heading (GTK_CALENDAR(calendar),TRUE);
	gtk_calendar_set_show_day_names(GTK_CALENDAR(calendar),TRUE);
	g_signal_connect(GTK_CALENDAR(calendar), "day-selected", G_CALLBACK(callbk_calendar_day_selected), label_date);
	g_signal_connect(GTK_CALENDAR(calendar), "next-month", G_CALLBACK(callbk_calendar_next_month), label_date);
	g_signal_connect(GTK_CALENDAR(calendar), "prev-month", G_CALLBACK(callbk_calendar_prev_month), label_date);
	g_signal_connect(GTK_CALENDAR(calendar), "next-year", G_CALLBACK(callbk_calendar_next_year),label_date);
	g_signal_connect(GTK_CALENDAR(calendar), "prev-year", G_CALLBACK(callbk_calendar_prev_year), label_date);
	GDateTime* datetime = gtk_calendar_get_date (GTK_CALENDAR (calendar));
	m_today_year = g_date_time_get_year (datetime);
	m_today_month=g_date_time_get_month(datetime);
	m_today_day =g_date_time_get_day_of_month(datetime);
	//g_print("Today is : %d-%d-%d \n", m_today_day, m_today_month,m_today_year);

	//gtk_calendar_select_day (GTK_CALENDAR(calendar), datetime);

	m_start_day=m_today_day;
	m_start_month=m_today_month;
	m_start_year=m_today_year;
	update_date(GTK_CALENDAR(calendar), label_date);


	set_event_marks_on_calendar(GTK_CALENDAR(calendar));

	//testing
	//gchar * mark_str =get_marks_on_calendar(GTK_CALENDAR(calendar));
	//g_print("%s\n",mark_str);


	//actions
	GSimpleAction *speak_action;	
	speak_action=g_simple_action_new("speak",NULL); //app.speak
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(speak_action)); //make visible	
	g_signal_connect(speak_action, "activate",  G_CALLBACK(callbk_speak), window);

	GSimpleAction *speak_month_action;
	speak_month_action=g_simple_action_new("speak_month",NULL); //app.speak_month
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(speak_month_action)); //make visible
	g_signal_connect(speak_month_action, "activate",  G_CALLBACK(callbk_speak_month), window);
	
	GSimpleAction *about_action;
	about_action=g_simple_action_new("about",NULL); //app.about
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(about_action)); //make visible
	g_signal_connect(about_action, "activate",  G_CALLBACK(callbk_about), window);

	GSimpleAction *preferences_action;
	preferences_action=g_simple_action_new("preferences",NULL); //app.preferences
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(preferences_action)); //make visible
	g_signal_connect(preferences_action, "activate",  G_CALLBACK(callbk_preferences), window);

	GSimpleAction *home_action;	
	home_action=g_simple_action_new("home",NULL); //app.home
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(home_action)); //make visible	
	g_signal_connect(home_action, "activate",  G_CALLBACK(callbk_home), window);
	

	GSimpleAction *info_action;
	info_action=g_simple_action_new("info",NULL); //app.info
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(info_action)); //make visible
	g_signal_connect(info_action, "activate",  G_CALLBACK(callbk_info), window);

	GSimpleAction *quit_action;	
	quit_action=g_simple_action_new("quit",NULL); //app.quit
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(quit_action)); //make visible	
	g_signal_connect(quit_action, "activate",  G_CALLBACK(callbk_quit), app);

	// connect keyboard accelerators
	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
										  "app.speak", speak_accels);

	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
										  "app.speak_month", speak_month_accels);
	
	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
										  "app.home", home_accels);
	

	g_object_set_data(G_OBJECT(window), "window-calendar-key",calendar);
	g_object_set_data(G_OBJECT(window), "window-label-date-key",label_date);
	//g_object_set_data(G_OBJECT(calendar), "calendar-label-date-key",label_date);



	create_header(GTK_WINDOW(window));
	//update_store(m_year,m_month,m_day); //update and display store


	gtk_box_append(GTK_BOX(box), label_date);
	gtk_box_append(GTK_BOX(box), calendar);
	gtk_box_append(GTK_BOX(box), sw); //listbox inside sw

	gtk_window_present (GTK_WINDOW (window));    //use present not show with gtk4

	display_events(m_start_year,m_start_month,m_start_day);
	//speak_month_dates_with_events(window);
	if(m_talk && m_talk_at_startup) {
		speak_events(window);
		//speak_month_events(window);
	}
}

int main (int  argc, char **argv)
{

	config_initialize();
	GtkApplication *app;
	int status;

	app = gtk_application_new ("org.gtk.tikicalendar", G_APPLICATION_FLAGS_NONE);

	g_signal_connect_swapped(app, "startup", G_CALLBACK (startup),app);

	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);

	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
