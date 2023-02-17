/*
 * main.c
 * 
/*
 * main.c
 *
 * Copyright 2023 Alan Crispin <crispinalan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2+ of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

/*
 * Use MAKEFILE to compile
 *
*/

#include <gtk/gtk.h>
#include <glib/gstdio.h>  //needed for g_mkdir
#include <math.h>  //compile with -lm
#include "wavcat.h"


#define CONFIG_DIRNAME "tikical-gtk4"
#define CONFIG_FILENAME "tikical-gtk-config-020"

static GMutex lock;

//declarations

static void update_calendar(GtkWindow *window);
static void update_header (GtkWindow *window);
static GtkWidget *create_widget (gpointer item, gpointer user_data);
static void update_store(int m_year,int m_month,int m_day);
static void update_marked_dates(int month, int year);
static void reset_marked_dates();
static void update_priority_dates(int month, int year);
static void reset_priority_dates();
int compare (const void * a, const void * b);
gboolean check_day_events_for_overlap();


gchar* get_css_string();
GDate* calculate_easter(gint year);
gboolean check_day_events_for_overlap();


//Event Dialogs
static void callbk_check_button_allday_toggled (GtkCheckButton *check_button, gpointer user_data);
static void callbk_add(GtkButton *button, gpointer  user_data);
static void callbk_new_event(GtkButton *button, gpointer  user_data);
static void callbk_update(GtkButton *button, gpointer  user_data);
static void callbk_edit_event(GtkButton *button, gpointer  user_data);

//Actions

static void speak_events();
static void callbk_speak(GSimpleAction* action, GVariant *parameter,gpointer user_data);
static void callbk_about(GSimpleAction* action, GVariant *parameter, gpointer user_data);
static void callbk_home(GSimpleAction* action, GVariant *parameter, gpointer user_data);
static void callbk_delete(GSimpleAction* action, GVariant *parameter,  gpointer user_data);
static void callbk_quit(GSimpleAction* action,G_GNUC_UNUSED GVariant *parameter, gpointer user_data);
static void callbk_delete_selected(GtkButton *button, gpointer  user_data);


void save_csv_file(); //need a new approach
void load_csv_file();
void reload_csv_file();
int file_exists(const char *file_name);

//config
static char * m_config_file = NULL;

static int m_holidays=0; //holidays to do
static int m_show_end_time=0; //show end_time
//static int m_startup_notification=0;


static int m_talk =1;
static int m_talk_at_startup =1;
static int m_talk_event_number=1;
static int m_talk_time=0;
static int m_talk_tags=0;
static int m_talk_overlap=0; //to do
static int m_talk_priority=0; //to do
static int m_talk_reset=0;

static int m_frame =0; // grid button frames


static int m_today_year=0;
static int m_today_month=0;
static int m_today_day=0;

static int m_row_index=-1; //selection index

//todo: write a Gobject calendar event class
static const char* m_summary ="title";
static const char* m_location ="";
static const char* m_description ="todo";
static int m_year=0;
static int m_month=0;
static int m_day=0;
static float m_start_time=0.0;
static float m_end_time=0.0;
static int m_priority=0;
static int m_is_yearly=0;
static int m_is_allday=0;

static GListStore *m_store;

static int m_id_selection=-1;

//todo: write a Gobject calendar event class
typedef struct {
	int id;	
	char summary[101];
	char location[101];
	char description[101];
	int year;
	int month;
	int day;
	float start_time;
	float end_time;	
	int priority;
	int is_yearly;
	int is_allday;
} Event;

int max_records=5000;
Event *event_store=NULL;

int m_store_size=0;
int marked_date[31]; //month days with events
int priority_date[31]; //month days with high priority
int num_marked_dates = 0;
int num_priority_dates = 0;

//---------------------------------------------------------------------
// map menu actions to callbacks

const GActionEntry app_actions[] = { 
  { "speak", callbk_speak },  
  { "home", callbk_home}   
};

//--------------------------------------------------------------------
// DisplayObject functions
//--------------------------------------------------------------------
enum
{
  
  PROP_LABEL = 1, //Prop label must start with 1
  PROP_ID,  
  PROP_STARTTIME, //for sorting
  LAST_PROPERTY //the size of the GParamSpec properties array
};

static GParamSpec *properties[LAST_PROPERTY] = { NULL, };

typedef struct
{
  GObject parent; //inheritance i.e. parent is a GObject 
  //fields
  char *label; //label to be displayed in listview
  int id; //id  
  int starttime;  //for sorting
  
} DisplayObject;


typedef struct
{
  GObjectClass parent_class;
} DisplayObjectClass;

//declaring a GType
static GType display_object_get_type (void);
//------------------------------------------------------------------
static void config_load_default()
{		
	m_talk=1;
	m_talk_at_startup=1;
	m_talk_event_number=1;	
	m_talk_time=1;
	m_talk_tags=1;
	m_talk_priority=0;	//todo
	m_talk_overlap=0; //todo
	m_holidays=0; //todo
	m_frame=0;
	m_show_end_time=0;
}

static void config_read()
{
	// Clean up previously loaded configuration values	
	m_talk=1;
	m_talk_at_startup=1;
	m_talk_event_number=1;	
	m_talk_time=1;
	m_talk_tags=1;
	m_talk_priority=0;	
	m_talk_overlap=0;
	m_holidays=0;
	m_frame=0;
	m_show_end_time=0;	

	// Load keys from keyfile
	GKeyFile * kf = g_key_file_new();
	g_key_file_load_from_file(kf, m_config_file, G_KEY_FILE_NONE, NULL);
	
	//talk
	m_talk = g_key_file_get_integer(kf, "calendar_settings", "talk", NULL);
	m_talk_at_startup=g_key_file_get_integer(kf, "calendar_settings", "talk_startup", NULL);
	m_talk_event_number=g_key_file_get_integer(kf, "calendar_settings", "talk_event_number", NULL);	
	m_talk_time=g_key_file_get_integer(kf, "calendar_settings", "talk_time", NULL);	
	m_talk_tags=g_key_file_get_integer(kf, "calendar_settings", "talk_tags", NULL);
	m_talk_priority=g_key_file_get_integer(kf, "calendar_settings", "talk_priority", NULL);	//placeholder
	m_talk_overlap = g_key_file_get_integer(kf, "calendar_settings", "talk_overlap", NULL);	//placeholder
	m_holidays = g_key_file_get_integer(kf, "calendar_settings", "holidays", NULL);	//to do
    m_frame = g_key_file_get_integer(kf, "calendar_settings", "frame", NULL);
	m_show_end_time = g_key_file_get_integer(kf, "calendar_settings", "show_end_time", NULL);
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
	g_key_file_set_integer(kf, "calendar_settings", "talk_tags", m_talk_tags);
	g_key_file_set_integer(kf, "calendar_settings", "talk_priority", m_talk_priority);		
	g_key_file_set_integer(kf, "calendar_settings", "talk_overlap", m_talk_overlap);
	g_key_file_set_integer(kf, "calendar_settings", "holidays", m_holidays); //to do
	g_key_file_set_integer(kf, "calendar_settings", "frame", m_frame);
	g_key_file_set_integer(kf, "calendar_settings", "show_end_time", m_show_end_time);	

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

//---------------------------------------------------------------------

G_DEFINE_TYPE (DisplayObject, display_object, G_TYPE_OBJECT)

static void display_object_init (DisplayObject *obj)
{
}

static void display_object_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec){
							
							
  DisplayObject *obj = (DisplayObject *)object;
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

static void display_object_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec){
							
  DisplayObject *obj = (DisplayObject *)object;  
 
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

static void display_object_finalize (GObject *obj)
{
  DisplayObject *object = (DisplayObject *)obj;

  g_free (object->label);

  G_OBJECT_CLASS (display_object_parent_class)->finalize (obj);
}

static void display_object_class_init (DisplayObjectClass *class)
{

	// GObjectClass is the class structure for the GObject type
	// G_OBJECT_CLASS casts the DisplayObjectClass called class into a GObjectClass structure

	GObjectClass *object_class = G_OBJECT_CLASS (class);

	//the object_class has virtual functions called get_property and set_property
	//so you override these to provide your own getter and setter implementations

	//override the get_property of the object_class
	object_class->get_property = display_object_get_property;
	//override the set property of the object_class
	object_class->set_property = display_object_set_property;
	object_class->finalize = display_object_finalize;

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


//---------------------------------------------------------------------
// create widget
//---------------------------------------------------------------------

static GtkWidget *create_widget (gpointer item, gpointer user_data)
{
  DisplayObject *obj = (DisplayObject *)item;
  GtkWidget *label;

  label = gtk_label_new ("");
  g_object_bind_property (obj, "label", label, "label", G_BINDING_SYNC_CREATE);

  return label;
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



//--------------------------------------------------------------------
// Compare
//---------------------------------------------------------------------

static int compare_items (gconstpointer a, gconstpointer b, gpointer data)
{
  int starttime_a, starttime_b;
  g_object_get ((gpointer)a, "starttime", &starttime_a, NULL);
  g_object_get ((gpointer)b, "starttime", &starttime_b, NULL);
  return starttime_a - starttime_b;
}

//--------------------------------------------------------------------
// calendar functions
//---------------------------------------------------------------------
static int first_day_of_month(int month, int year)
{
    if (month < 3) {
        month += 12;
        year--;
    }
    int century = year / 100;
    year = year % 100;
    return (((13 * (month + 1)) / 5) +
            (century / 4) + (5 * century) +
            year + (year / 4)) % 7;
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


static int get_number_of_events()
{
	//repeat events to do
	
	int event_count=0;
	Event e;  
	for (int i=0; i<m_store_size; i++)
	{  
	e=event_store[i];
	if(m_year==e.year && m_month ==e.month && m_day==e.day)
	{		
	event_count++;
	}//if
	//check for yearly events
	if(m_year!=e.year && m_month ==e.month && m_day==e.day && e.is_yearly)
	{
	event_count++;
	}

	}//for
  
 return event_count;
}

//--------------------------------------------------------------------
// new event
//---------------------------------------------------------------------

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


//-----------------------------------------------------------------
// Speak
//------------------------------------------------------------------

static void callbk_speak(GSimpleAction* action, GVariant *parameter,gpointer user_data){
		
	speak_events();	
	
}


//---------------------------------------------------------------------
// Speaking using local talk wav files
//---------------------------------------------------------------------


char* convert_date_to_weekday(int day, int month, int year) {

	char* weekday_str="";
	GDate* day_date;
	day_date = g_date_new_dmy(day, month, year);
	GDateWeekday weekday =g_date_get_weekday(day_date);

	switch(weekday)
	{
	case G_DATE_MONDAY:
	weekday_str="talk/day/monday.wav";;
	break;
	case G_DATE_TUESDAY:
	weekday_str="talk/day/tuesday.wav";;
	break;
	case G_DATE_WEDNESDAY:
	weekday_str="talk/day/wednesday.wav";
	break;
	case G_DATE_THURSDAY:
	weekday_str="talk/day/thursday.wav";
	break;
	case G_DATE_FRIDAY:
	weekday_str="talk/day/friday.wav";
	break;
	case G_DATE_SATURDAY:
	weekday_str="talk/day/saturday.wav";
	break;
	case G_DATE_SUNDAY:
	weekday_str="talk/day/sunday.wav";
	break;
	default:
	weekday_str="talk/f/fullstop.wav";
	}//switch

	return weekday_str;
}

char* convert_day_to_ordinal_string(int day) {

	char* day_str ="";

	switch (day) {
		case 1:
		day_str="talk/ordinal/first.wav";
		break;
		case 2:
		day_str="talk/ordinal/second.wav";
		break;
		case 3:
		day_str="talk/ordinal/third.wav";
		break;
		case 4:
		day_str="talk/ordinal/fourth.wav";
		break;
		case 5:
		day_str="talk/ordinal/fifth.wav";
		break;
		case 6:
		day_str="talk/ordinal/sixth.wav";
		break;
		case 7:
		day_str="talk/ordinal/seventh.wav";
		break;
		case 8:
		day_str="talk/ordinal/eighth.wav";
		break;
		case 9:
		day_str="talk/ordinal/ninth.wav";
		break;
		case 10:
		day_str="talk/ordinal/tenth.wav";
		break;
		case 11:
		day_str="talk/ordinal/eleventh.wav";
		break;
		case 12:
		day_str="talk/ordinal/twelfth.wav";
		break;
		case 13:
		day_str="talk/ordinal/thirteenth.wav";

		break;
		case 14:
		day_str="talk/ordinal/fourteenth.wav";
		break;
		case 15:
		day_str="talk/ordinal/fifteenth.wav";

		break;
		case 16:
		day_str="talk/ordinal/sixteenth.wav";

		break;
		case 17:
		day_str="talk/ordinal/seventeenth.wav";

		break;
		case 18:
		day_str="talk/ordinal/eighteenth.wav";

		break;
		case 19:
		day_str="talk/ordinal/nineteenth.wav";

		break;
		case 20:
		day_str="talk/ordinal/twentieth.wav";

		break;
		case 21:
		day_str="talk/ordinal/twentyfirst.wav";

		break;
		case 22:
		day_str="talk/ordinal/twentysecond.wav";
		break;
		case 23:
		day_str="talk/ordinal/twentythird.wav";
		break;
		case 24:
		day_str="talk/ordinal/twentyfourth.wav";

		break;
		case 25:
		day_str="talk/ordinal/twentyfifth.wav";

		break;
		case 26:
		day_str="talk/ordinal/twentysixth.wav";

		break;
		case 27:
		day_str="talk/ordinal/twentyseventh.wav";

		break;
		case 28:
		day_str="talk/ordinal/twentyeighth.wav";

		break;
		case 29:
		day_str="talk/ordinal/twentyninth.wav";

		break;
		case 30:
		day_str="talk/ordinal/thirtieth.wav";

		break;
		case 31:
		day_str="talk/ordinal/thirtyfirst.wav";

		break;
		default:
		//Unknown ordinal
		day_str="talk/f/fullstop.wav";
		break;
	  } //day switch


	return day_str;

}

char* convert_month_to_string(int month) {

	char* result ="";

	switch(month) {
	case 1:
		result = "talk/month/january.wav";
		break;
	case 2:
		result = "talk/month/february.wav";
		break;
	case 3:
		result= "talk/month/march.wav";
		break;
	case 4:
		result = "talk/month/april.wav";
		break;
	case 5:
		result ="talk/month/may.wav";
		break;
	case 6:
		result = "talk/month/june.wav";
		break;
	case 7:
		result ="talk/month/july.wav";
		break;
	case 8:
		result ="talk/month/august.wav";
		break;
	case 9:
		result= "talk/month/september.wav";
		break;
	case 10:
		result = "talk/month/october.wav";
		break;
	case 11:
		result = "talk/month/november.wav";
		break;
	case 12:
		result = "talk/month/december.wav";
		break;
	default:
		result = "talk/f/fullstop.wav";
	}

	return result;
}

char* convert_hour_to_cardinal_string(int hour){

	char* result ="";


     switch(hour)
     {
         case 1:
		 result ="talk/cardinal/1.wav";
		 break;
		 case 2:
		 result ="talk/cardinal/2.wav";
		 break;
		 case 3:
		 result = "talk/cardinal/3.wav";
		 break;
		 case 4:
		 result ="talk/cardinal/4.wav";
		 break;
		 case 5:
		 result ="talk/cardinal/5.wav";
		 break;
		 case 6:
		 result ="talk/cardinal/6.wav";
		 break;
		 case 7:
		 result ="talk/cardinal/7.wav";
		 break;
		 case 8:
		 result="talk/cardinal/8.wav";
		 break;
		 case 9:
		 result="talk/cardinal/9.wav";
		 break;
		 case 10:
		 result="talk/cardinal/10.wav";
		 break;
		 case 11:
		 result="talk/cardinal/11.wav";
		 break;
		 case 12:
		 result="talk/cardinal/12.wav";
		 break;
		 case 13:
		 result="talk/cardinal/13.wav";
		 break;
		 case 14:
		 result ="talk/cardinal/14.wav";
		 break;
		 case 15:
		 result ="talk/cardinal/15.wav";
		 break;
		 case 16:
		 result="talk/cardinal/16.wav";
		 break;
		 case 17:
		 result="talk/cardinal/17.wav";
		 break;
		 case 18:
		 result="talk/cardinal/18.wav";
		 break;
		 case 19:
		 result="talk/cardinal/19.wav";
		 break;
		 case 20:
		 result ="talk/cardinal/20.wav";
		 case 21:
		 result="talk/cardinal/21.wav";
		 break;
		 case 22:
		 result="talk/cardinal/22.wav";
		 break;
		 case 23:
		 result="talk/cardinal/23.wav";
		 break;
         default:
           g_print ("default: hour Value is: %i\n", hour);
	 }//switch start hour


	return result;

}

char* convert_min_to_cardinal_string(int min){

	char* result ="";

	switch(min)
     {
	    case 1:
	     //g_print("o wav");
	    result="talk/cardinal/1.wav";
		break;
		case 2:
		result="talk/cardinal/2.wav";
		break;
		case 3:
		result="talk/cardinal/3.wav";
		break;
		case 4:
		result="talk/cardinal/4.wav";
		break;
		case 5:
		result="talk/cardinal/5.wav";
		break;
		case 6:
		result="talk/cardinal/6.wav";
		break;
		case 7:
		result="talk/cardinal/7.wav";
		break;
		case 8:
		result="talk/cardinal/8.wav";
		break;
		case 9:
		result="talk/cardinal/9.wav";
		break;
		case 10:
		result ="talk/cardinal/10.wav";
		break;
		case 11:
	     result="talk/cardinal/11.wav";
		break;
		case 12:
		result="talk/cardinal/12.wav";
		break;
		case 13:
		result="talk/cardinal/13.wav";
		break;
		case 14:
		result="talk/cardinal/14.wav";
		break;
		case 15:
		result="talk/cardinal/15.wav";
		break;
		case 16:
		result="talk/cardinal/16.wav";
		break;
		case 17:
		result="talk/cardinal/17.wav";
		break;
		case 18:
		result="talk/cardinal/18.wav";
		break;
		case 19:
		result ="talk/cardinal/19.wav";
		break;

		case 20:
		result="talk/cardinal/20.wav";
		break;
		case 21:
		result="talk/cardinal/21.wav";
		break;
		case 22:
		result="talk/cardinal/22.wav";
		break;
		case 23:
		result="talk/cardinal/23.wav";
		break;
		case 24:
		result="talk/cardinal/24.wav";
		break;
		case 25:
		result="talk/cardinal/25.wav";
		break;
		case 26:
		result="talk/cardinal/26.wav";
		break;
		case 27:
		result="talk/cardinal/27.wav";
		break;
		case 28:
		result="talk/cardinal/28.wav";
		break;
		case 29:
		result="talk/cardinal/29.wav";
		break;

		case 30:
		result= "talk/cardinal/30.wav";
		break;
		case 31:
		result= "talk/cardinal/31.wav";
		break;
		case 32:
		result= "talk/cardinal/32.wav";
		break;
		case 33:
		result= "talk/cardinal/33.wav";
		break;
		case 34:
		result= "talk/cardinal/34.wav";
		break;
		case 35:
		result= "talk/cardinal/35.wav";
		break;
		case 36:
		result= "talk/cardinal/36.wav";
		break;
		case 37:
		result= "talk/cardinal/37.wav";
		break;
		case 38:
		result= "talk/cardinal/38.wav";
		break;
		case 39:
		result= "talk/cardinal/39.wav";
		break;

		case 40:
		result ="talk/cardinal/40.wav";
		break;
		case 41:
		result ="talk/cardinal/41.wav";
		break;
		case 42:
		result ="talk/cardinal/42.wav";
		break;
		case 43:
		result ="talk/cardinal/43.wav";
		break;
		case 44:
		result ="talk/cardinal/44.wav";
		break;
		case 45:
		result ="talk/cardinal/45.wav";
		break;
		case 46:
		result ="talk/cardinal/46.wav";
		break;
		case 47:
		result ="talk/cardinal/47.wav";
		break;
		case 48:
		result ="talk/cardinal/48.wav";
		break;
		case 49:
		result ="talk/cardinal/49.wav";
		break;

		case 50:
		result ="talk/cardinal/50.wav";
		break;
		case 51:
		result ="talk/cardinal/51.wav";
		break;
		case 52:
		result ="talk/cardinal/52.wav";
		break;
		case 53:
		result ="talk/cardinal/53.wav";
		break;
		case 54:
		result ="talk/cardinal/54.wav";
		break;
		case 55:
		result ="talk/cardinal/55.wav";
		break;
		case 56:
		result ="talk/cardinal/56.wav";
		break;
		case 57:
		result ="talk/cardinal/57.wav";
		break;
		case 58:
		result ="talk/cardinal/58.wav";
		break;
		case 59:
		result ="talk/cardinal/59.wav";
		break;

		default:
           g_print ("default: min value is: %i\n", min);
		 break;
     } //switch start min

	return result;

}

char* voice_dictionary(char* word) {

char* result="";

	if (g_strcmp0(word,"activity")==0) {
	result ="talk/a/activity.wav";
    }

	 if (g_strcmp0(word,"alert")==0) {
	 result="talk/a/alert.wav";
	 }

	 if (g_strcmp0(word,"anniversary")==0) {
	result="talk/a/anniversary.wav";
	 }

	 if (g_strcmp0(word,"appointment")==0) {
	result="talk/a/appointment.wav";
	 }

	 //b words
	 if (g_strcmp0(word,"bank")==0) {
	 result="talk/b/bank.wav";
	 }

	 if (g_strcmp0(word,"birthday")==0) {
	 result="talk/b/birthday.wav";
	 }

	 if (g_strcmp0(word,"boxing")==0) {
	 result="talk/b/boxing.wav";
	 }

	 //c words
	 if (g_strcmp0(word,"calendar")==0) {
	 result="talk/c/calendar.wav";
	 }

	 if (g_strcmp0(word,"christmas")==0) {
	 result="talk/c/christmas.wav";
	 }

	 //d words
	 if (g_strcmp0(word,"day")==0) {
	 result="talk/d/day.wav";
	 }

	 if (g_strcmp0(word,"dentist")==0) {
	 result="talk/d/dentist.wav";
	 }

	 if (g_strcmp0(word,"doctor")==0) {
	 result="talk/d/doctor.wav";
	 }

	 //e words
	 if (g_strcmp0(word,"easter")==0) {
	 result="talk/e/easter.wav";
	 }

	 if (g_strcmp0(word,"event")==0) {
	 result="talk/e/event.wav";
	 }

	 if (g_strcmp0(word,"events")==0) {
	 result="talk/e/events.wav";
	 }

	 //f words
	 if (g_strcmp0(word,"family")==0) {
	 result ="talk/f/family.wav";
	 }

	 if (g_strcmp0(word,"friends")==0) {
	 result="talk/f/friends.wav";
	 }

	 if (g_strcmp0(word,"funeral")==0) {
	 result="talk/f/funeral.wav";
	 }

	 //h words
	 if (g_strcmp0(word,"halloween")==0) {
	 result="talk/h/halloween.wav";
	 }

	 if (g_strcmp0(word,"holiday")==0) {
	 result="talk/h/holiday.wav";
	 }

	 if (g_strcmp0(word,"hospital")==0) {
	 result="talk/h/hospital.wav";
	 }

	 //i words
	 if (g_strcmp0(word,"important")==0) {
	 result="talk/i/important.wav";
	 }

	 if (g_strcmp0(word,"information")==0) {
	 result="talk/i/information.wav";
	 }

	 //m words
	 if (g_strcmp0(word,"meal")==0) {
	 result="talk/m/meal.wav";
	 }

	 if (g_strcmp0(word,"medical")==0) {
	 result="talk/m/medical.wav";
	 }

	 if (g_strcmp0(word,"meeting")==0) {
	 result="talk/m/meeting.wav";
	 }


	 //n words
	 if (g_strcmp0(word,"new")==0) {
	 result="talk/n/new.wav";
	 }

	 if (g_strcmp0(word,"note")==0) {
	 result="talk/n/note.wav";
	 }

	 //p words
	 if (g_strcmp0(word,"party")==0) {
	 result="talk/p/party.wav";
	 }

	 if (g_strcmp0(word,"payment")==0) {
	 result="talk/p/payment.wav";
	 }

	 if (g_strcmp0(word,"priority")==0) {
	 result="talk/p/priority.wav";
	 }

	  if (g_strcmp0(word,"public")==0) {
	 result="talk/p/public.wav";
	 }

	 //q words
	 //r words

	 if (g_strcmp0(word,"reminder")==0) {
	 result="talk/r/reminder.wav";
	 }


	 if (g_strcmp0(word,"restaurant")==0) {
	 result="talk/r/restaurant.wav";
	 }

	  if (g_strcmp0(word,"retirement")==0) {
	 result="talk/r/retirement.wav";
	 }

	 // s words

	 // t words
	 if (g_strcmp0(word,"talk")==0) {
	 result="talk/t/talk.wav";
	 }

	 if (g_strcmp0(word,"travel")==0) {
	 result="talk/t/travel.wav";
	 }


	 //v words
	 if (g_strcmp0(word,"valentine")==0) {
	 result="talk/v/valentine.wav";
	 }

	 if (g_strcmp0(word,"visit")==0) {
	 result="talk/v/visit.wav";
	 }

	 // w words

	  if (g_strcmp0(word,"walk")==0) {
	 result="talk/w/walk.wav";
	 }

	 if (g_strcmp0(word,"wedding")==0) {
	 result="talk/w/wedding.wav";
	 }

	  if (g_strcmp0(word,"work")==0) {
	 result="talk/w/work.wav";
	 }

	 //y words
	 if (g_strcmp0(word,"year")==0) {
	 result="talk/y/year.wav";
	 }

return result;
}



static gpointer thread_aplaywav(gpointer user_data)
{

    gchar *file_name =user_data;

   	char input[50];
	strcpy(input, file_name);
	//wavplay(input);
    gchar* aplay_str ="aplay";
    aplay_str=g_strconcat(aplay_str," ",input, NULL);
    system(aplay_str);

    g_mutex_unlock (&lock); //thread mutex unlock
    return NULL;
}


//---------------------------------------------------------------------
// speak events
//---------------------------------------------------------------------
static void speak_events() {

	 if(m_talk==0) return;

	 int32_t sample_rate=8000;
	 //gpointer pt_data;
	 //gchar* list_str;
	 int day_events_number=0;

	 //Check for talk directory

	 gchar *cur_dir;
	 cur_dir = g_get_current_dir ();

   //Check if dictionary directory exists
	gchar* talk_directory = g_build_filename(cur_dir, "talk", NULL);

	if (g_file_test(talk_directory, G_FILE_TEST_IS_DIR)==FALSE){
		return;
	}

	GList *wavlist=NULL;
	gchar *dow_str=""; //day of week
	gchar *day_str="";
	gchar *month_str="";

	dow_str=convert_date_to_weekday(m_day, m_month, m_year);

	if (g_file_test(g_build_filename (cur_dir, dow_str, NULL), G_FILE_TEST_IS_REGULAR)) {
			wavlist = g_list_append(wavlist, g_build_filename (cur_dir, dow_str, NULL));
	    }
	day_str=convert_day_to_ordinal_string(m_day);

	if (g_file_test(g_build_filename (cur_dir, day_str, NULL), G_FILE_TEST_IS_REGULAR)) {
			wavlist = g_list_append(wavlist, g_build_filename (cur_dir, day_str, NULL));
	  }

	month_str=convert_month_to_string(m_month);
	if (g_file_test(g_build_filename (cur_dir, month_str, NULL), G_FILE_TEST_IS_REGULAR)) {
			 wavlist = g_list_append(wavlist, g_build_filename (cur_dir, month_str, NULL));
	  }


	int event_number=get_number_of_events();
	//g_print("event number = %i\n",event_number);

	//-----------------------------------------------------------------
	// get and sort day events
	//-----------------------------------------------------------------

   Event day_events[event_number];
   Event e;
   //load day events
   int jj=0;
   for (int i=0; i<m_store_size; i++)
	{
		e=event_store[i];
		if(m_year==e.year && m_month ==e.month && m_day==e.day)
		{
			day_events[jj] =e;
			day_events_number=day_events_number+1;
			jj++;
		}//if
		if(m_year!=e.year && m_month ==e.month && m_day==e.day && e.is_yearly)
		{
			//yearly events
			day_events[jj] =e;
			day_events_number=day_events_number+1;
			jj++;
		}

	}//for

   //sort

   qsort (day_events, event_number, sizeof(Event), compare);


	//--------------------------------------------------------------

   if(m_talk_event_number) {

    if (event_number ==0) {
    if (g_file_test(g_build_filename (cur_dir,"talk/n/no.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
    wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/n/no.wav", NULL));
    }
    if (g_file_test(g_build_filename (cur_dir,"talk/e/events.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
    wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/e/events.wav", NULL));
    }
    } //if no events
    else if(event_number ==1){
		//g_print("one event being added to wavlist\n");
	if (g_file_test(g_build_filename (cur_dir,"talk/cardinal/1.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/cardinal/1.wav", NULL));
	}
	if (g_file_test(g_build_filename (cur_dir,"talk/e/event.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/e/event.wav", NULL));
	}
	} // if 1 event

	else if(event_number ==2){
	if (g_file_test(g_build_filename (cur_dir,"talk/cardinal/2.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/cardinal/2.wav", NULL));
	}
	if (g_file_test(g_build_filename (cur_dir,"talk/e/events.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/e/events.wav", NULL));
	}
	}
	else if(event_number ==3){
	if (g_file_test(g_build_filename (cur_dir,"talk/cardinal/3.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/cardinal/3.wav", NULL));
	}
	if (g_file_test(g_build_filename (cur_dir,"talk/e/events.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/e/events.wav", NULL));
	}
	}
	else if(event_number ==4){
	if (g_build_filename (cur_dir,"talk/cardinal/4.wav", NULL), G_FILE_TEST_IS_REGULAR) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/cardinal/4.wav", NULL));
	}
	if (g_file_test(g_build_filename (cur_dir,"talk/e/events.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/e/events.wav", NULL));
	}
	}
	else if(event_number ==5){
	if ( g_file_test(g_build_filename (cur_dir,"talk/cardinal/5.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/cardinal/5.wav", NULL));
	}
	if (g_file_test(g_build_filename (cur_dir,"talk/e/events.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/e/events.wav", NULL));
	}
	}
	else {
	if (g_file_test(g_build_filename (cur_dir,"talk/m/many.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/m/many.wav", NULL));
	}
	if (g_file_test(g_build_filename (cur_dir,"talk/e/events.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
	wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/e/events.wav", NULL));
	}
	}

   } //if m_talk_event_number

	//--------------------------------------------------------------------------------------

	gchar* str;
	gchar** words;
	//gchar* word;
	gint j=0;
	//GList *event_word_list=NULL;

	Event day_event;

   for(int i=0; i<day_events_number; i++)
   {
	   day_event =day_events[i];

	  // g_print("Get event time\n");

	  //check for all day event here

	   float start_time =day_event.start_time;
	   //g_print("start_time = %f\n",start_time);

	   float integral_part, fractional_part;
	   fractional_part = modff(day_event.start_time, &integral_part);
	   int start_hour =(int) integral_part; //start_hour
	   fractional_part=round(fractional_part *100);
	   int start_min=(int) (fractional_part); //start_min

	   //g_print("start hour =%i\n",start_hour);
	   //g_print("start min = %i\n",start_min);

	   char* start_hour_str="";
	   char* start_min_str="";

	 //Time ----------------------------------------------------------

	   if (m_talk_time) {
		   if(day_event.is_allday) {
			   //g_print("All day event\n");
			   if (g_file_test(g_build_filename (cur_dir,"talk/a/all.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
				   wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/a/all.wav", NULL));
			   }
			   if (g_file_test(g_build_filename (cur_dir,"talk/d/day.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
				   wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/d/day.wav", NULL));
			   }
			   if (g_file_test(g_build_filename (cur_dir,"talk/e/event.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
				   wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/e/event.wav", NULL));
			   }

		   }
		   else {


			   //AM -------------------------------------------------------------

			   if(start_hour >=1 && start_hour<=12) {

				   start_hour_str=convert_hour_to_cardinal_string(start_hour);
				   start_min_str=convert_min_to_cardinal_string(start_min);

				   if (g_file_test(g_build_filename (cur_dir, start_hour_str, NULL), G_FILE_TEST_IS_REGULAR)) {
					   wavlist = g_list_append(wavlist, g_build_filename (cur_dir, start_hour_str, NULL));
				   }

				   //if (g_file_test(g_build_filename (cur_dir, start_min_str, NULL), G_FILE_TEST_IS_REGULAR)) {
				   //wavlist = g_list_append(wavlist, g_build_filename (cur_dir, start_min_str, NULL));
				   //}


				   if(start_min >=1 && start_min <= 9) {
					   if (g_file_test(g_build_filename (cur_dir,"talk/o/o.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
						   wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/o/o.wav", NULL));}
						   if (g_file_test(g_build_filename (cur_dir, start_min_str, NULL), G_FILE_TEST_IS_REGULAR)) {
							   wavlist = g_list_append(wavlist, g_build_filename (cur_dir, start_min_str, NULL));
						   }
				   } else {
					   if (g_file_test(g_build_filename (cur_dir, start_min_str, NULL), G_FILE_TEST_IS_REGULAR)) {
						   wavlist = g_list_append(wavlist, g_build_filename (cur_dir, start_min_str, NULL));
					   }

				   }



				   if (g_file_test(g_build_filename (cur_dir,"talk/a/am.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
					   wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/a/am.wav", NULL));
				   }
			   }



			   //PM-------------------------------------------

			   if (start_hour >=13 && start_hour<=23) {


				   start_hour_str=convert_hour_to_cardinal_string(start_hour-12);
				   start_min_str=convert_min_to_cardinal_string(start_min);

				   if (g_file_test(g_build_filename (cur_dir, start_hour_str, NULL), G_FILE_TEST_IS_REGULAR)) {
					   wavlist = g_list_append(wavlist, g_build_filename (cur_dir, start_hour_str, NULL));
				   }

				   //if (g_file_test(g_build_filename (cur_dir, start_min_str, NULL), G_FILE_TEST_IS_REGULAR)) {
				   //wavlist = g_list_append(wavlist, g_build_filename (cur_dir, start_min_str, NULL));
				   //}


				   if(start_min >=1 && start_min <= 9) {
					   if (g_file_test(g_build_filename (cur_dir,"talk/o/o.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
						   wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/o/o.wav", NULL));}
						   if (g_file_test(g_build_filename (cur_dir, start_min_str, NULL), G_FILE_TEST_IS_REGULAR)) {
							   wavlist = g_list_append(wavlist, g_build_filename (cur_dir, start_min_str, NULL));
						   }
				   } else {
					   if (g_file_test(g_build_filename (cur_dir, start_min_str, NULL), G_FILE_TEST_IS_REGULAR)) {
						   wavlist = g_list_append(wavlist, g_build_filename (cur_dir, start_min_str, NULL));
					   }
				   }

				   if (g_file_test(g_build_filename (cur_dir,"talk/p/pm.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
					   wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/p/pm.wav", NULL));
				   }
			   }

		   } //else has AM/PM time


		}//if m_talk_time

	 //now read out event tag words

	 if(m_talk_tags) {

	 GList *event_word_list=NULL;
	 //str=day_event.type;
	 str=day_event.summary;
	  //g_print("summary = %s\n",str);
	  words = g_strsplit (str, " ", 0); //split on space
	j=0;
	while(words[j] != NULL)
	{
	//g_print("words = %s\n",words[j]);
	event_word_list = g_list_append(event_word_list, words[j]);
	j++;
	} //while loop words

 	//cycle through the event type word list
	gpointer pt_event_word_list;
	gchar* event_word_list_str;
	gchar* event_word_list_str_lower;

	//g_print("g_list length = %i\n",g_list_length(event_word_list));

	for(int i=0;i <g_list_length(event_word_list);i++)
	{
		pt_event_word_list=g_list_nth_data(event_word_list,i);
		event_word_list_str=(gchar *)pt_event_word_list;
		event_word_list_str_lower= g_ascii_strdown(event_word_list_str, -1);

		char* voice_str =voice_dictionary(event_word_list_str_lower);
		//g_print("voice_str = %s\n",voice_str);

		if (g_file_test(g_build_filename (cur_dir,voice_str, NULL), G_FILE_TEST_IS_REGULAR)) {
			wavlist = g_list_append(wavlist, g_build_filename (cur_dir,voice_str, NULL));
		}

     }//for_list_length

    g_list_free(event_word_list);

      } //m_tags


      if(m_talk_priority) {

		  if(day_event.priority)
		  {
			   if (g_file_test(g_build_filename (cur_dir,"talk/h/high.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
				   wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/h/high.wav", NULL));
			   }
			   if (g_file_test(g_build_filename (cur_dir,"talk/p/priority.wav", NULL), G_FILE_TEST_IS_REGULAR)) {
				   wavlist = g_list_append(wavlist, g_build_filename (cur_dir,"talk/p/priority.wav", NULL));
			   }
		  }
	  }

   } //for dayevents


	//--------------------------------------------------------------
	//merge and talk
	//--------------------------------------------------------------
	gpointer pt_data;
	gchar* list_str;

	char* merge_file ="/tmp/talkout.wav";
	int num_files = g_list_length(wavlist);
	char* file_names[g_list_length(wavlist)];

	for(int i=0;i<g_list_length(wavlist);i++) //iterate through GList wavlist
	{
	pt_data=g_list_nth_data(wavlist,i);
	list_str=(char *)pt_data;
	file_names[i] = list_str;	//populate char* array
	g_print("list_str =%s\n",list_str);
	//printf("main: locate file_names[%d] = '%s'\n",i,file_names[i]);
	}

	merge_wav_files2(merge_file, num_files, file_names,sample_rate);

	g_list_free(wavlist);
    g_free (cur_dir);

	//play audio in a thread
	GThread *thread_audio;
	gchar* wav_file ="/tmp/talkout.wav";
	g_mutex_lock (&lock);
    thread_audio = g_thread_new(NULL, thread_aplaywav, wav_file);
	g_thread_unref (thread_audio);

	//--------------------------------------------------------------
}


static void callbk_add(GtkButton *button, gpointer  user_data){

    //g_print("add button clicked");
	GtkWidget *window = user_data;

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

	int fd;
	Event event;
	event.id =m_store_size;
	strcpy(event.summary,m_summary);
	strcpy(event.location,m_location);
	strcpy(event.description,m_description);
	event.year=m_year;
	event.month=m_month;
	event.day=m_day;

	m_start_time =gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_button_start_time));
	m_end_time =gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_button_end_time));

	event.start_time=m_start_time;
	event.end_time=m_end_time;

	if(event.end_time<event.start_time) {
		event.end_time=event.start_time;
	}

	event.is_yearly=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_isyearly));
	event.is_allday=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_allday));
	event.priority=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_priority));

	event_store[m_store_size] =event;
	m_store_size=m_store_size+1;
	update_calendar(GTK_WINDOW(window));
	update_store(m_year,m_month,m_day);
	m_id_selection=-1;

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
  event_date = g_date_new_dmy(m_day,m_month,m_year);
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


static void callbk_update(GtkButton *button, gpointer  user_data){

	g_print("update event");

	GtkWidget *window = user_data;

	GtkWidget *dialog = g_object_get_data(G_OBJECT(button), "dialog-key");


	GtkEntryBuffer *buffer_summary;
	GtkWidget *entry_summary = g_object_get_data(G_OBJECT(button), "entry-summary-key");

	GtkEntryBuffer *buffer_location;
	GtkWidget *entry_location = g_object_get_data(G_OBJECT(button), "entry-location-key");

	GtkWidget *spin_button_start_time= g_object_get_data(G_OBJECT(button), "spin-start-time-key");
    GtkWidget *spin_button_end_time= g_object_get_data(G_OBJECT(button), "spin-end-time-key");

    GtkWidget *check_button_allday= g_object_get_data(G_OBJECT(button), "check-button-allday-key");
    GtkWidget  *check_button_isyearly= g_object_get_data(G_OBJECT(button), "check-button-isyearly-key");
    GtkWidget  *check_button_priority= g_object_get_data(G_OBJECT(button), "check-button-priority-key");



	//set data
	buffer_summary = gtk_entry_get_buffer (GTK_ENTRY(entry_summary));
	m_summary= gtk_entry_buffer_get_text (buffer_summary);
	m_summary =remove_semicolons(m_summary);

	buffer_location = gtk_entry_get_buffer (GTK_ENTRY(entry_location));
	m_location= gtk_entry_buffer_get_text (buffer_location);
	m_location =remove_semicolons(m_location);


	m_description="todo";

	//insert change into database
	Event event;
    for(int i=0; i<m_store_size; i++)
    {
	event=event_store[i];

	if(event.id==m_id_selection){

	strcpy(event.summary, m_summary);
	strcpy(event.location, m_location);
	strcpy(event.description,m_description);
	event.year=m_year;
	event.month=m_month;
	event.day=m_day;
	//float start_time, end_time;
	m_start_time =gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_button_start_time));
	m_end_time =gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin_button_end_time));

	event.start_time=m_start_time;
	event.end_time=m_end_time;

	if(event.end_time<event.start_time) {
		event.end_time=event.start_time;
	}
	//GTK_IS_CHECK_BUTTON
	event.is_yearly=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_isyearly));
	event.is_allday=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_allday));
	event.priority=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_priority));
	event_store[i]=event;
	break;
	} //if

    } //for

	update_calendar(GTK_WINDOW(window));
	update_store(m_year,m_month,m_day);
	m_row_index=-1;
	m_id_selection=-1;

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
	event_date = g_date_new_dmy(m_day,m_month,m_year);
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

	//find event in database    
	Event e;
	for(int i=0; i<m_store_size; i++)
	{
		e=event_store[i];
		if(e.id==m_id_selection){
			m_summary =e.summary;
			m_location =e.location;
			m_description =e.description;
			m_year=e.year;
			m_month=e.month;
			m_day=e.day;
			break;
		}
	}


	label_date =gtk_label_new(date_str);

	button_update = gtk_button_new_with_label ("Update");
    g_signal_connect (button_update, "clicked", G_CALLBACK (callbk_update), window);
	
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
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_start_time),e.start_time);
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
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_end_time),e.end_time);
	box_end_time=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);

	gtk_box_append (GTK_BOX(box_end_time),label_end_time);
	gtk_box_append (GTK_BOX(box_end_time),spin_button_end_time);
	gtk_box_append(GTK_BOX(box), box_end_time);

	g_object_set_data(G_OBJECT(button_update), "spin-end-time-key",spin_button_end_time);

	//check buttons
	check_button_allday = gtk_check_button_new_with_label ("Is All Day");
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_allday), e.is_allday);

	g_object_set_data(G_OBJECT(check_button_allday), "cb_allday_spin_start_time_key",spin_button_start_time);
	g_object_set_data(G_OBJECT(check_button_allday), "cb_allday_spin_end_time_key",spin_button_end_time);

	g_signal_connect_swapped (GTK_CHECK_BUTTON(check_button_allday), "toggled",
							  G_CALLBACK (callbk_check_button_allday_toggled), check_button_allday);

	if(e.is_allday) {
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

	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_isyearly), e.is_yearly);

	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_priority), e.priority);
	
	g_object_set_data(G_OBJECT(button_update), "check-button-allday-key",check_button_allday);
	g_object_set_data(G_OBJECT(button_update), "check-button-isyearly-key",check_button_isyearly);
	g_object_set_data(G_OBJECT(button_update), "check-button-priority-key",check_button_priority);

	//set_widget_font_size(dialog);
	//g_signal_connect (dialog, "response", G_CALLBACK (callbk_edit_event_response),window);

	gtk_window_present (GTK_WINDOW (dialog));

}


//---------------------------------------------------------------------
// callback delete selected
//---------------------------------------------------------------------
static void callbk_delete_selected(GtkButton *button, gpointer  user_data){
		
	if (m_row_index==-1) return;
	
	GtkWindow *window =user_data;
	
	//remove event from db  
	int j=0;
	Event e;
	for(int i=0; i<m_store_size; i++)
	{
	e=event_store[i];
	if(e.id==m_id_selection){
	continue;
	}
	event_store[j] = event_store[i];
	j++;
	}
	
	////Decrement db size
	m_store_size=m_store_size-1;
	
	g_list_store_remove (m_store, m_row_index); //remove selected
	update_calendar(GTK_WINDOW(window));
	update_store(m_year, m_month, m_day); 
	
}

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
	
	int field_num =12; //fix for now?	
	char *data[field_num]; // fields
	int i = 0; //counter	
    int total_num_lines = 0; //total number of lines
    int ret;
    
	GFile *file;
    GFileInputStream *file_stream=NULL;   
	GDataInputStream *input = NULL;
    
    file = g_file_new_for_path("eventsdb.csv");

	file_stream = g_file_read(file, NULL, NULL);	
	if(!file_stream) {
		g_print("error: unable to open database\n");
		return;
	}
	
	input = g_data_input_stream_new (G_INPUT_STREAM (file_stream));
		
	while (TRUE) {
		
		char *line;
		line = g_data_input_stream_read_line (input, NULL, NULL, NULL);		
		if (line == NULL) break;
		
		Event e;  
		      
        ret=break_fields(line,data,field_num);
        //g_print("break_fields return value %d\n",ret);
		
		for (int j=0; j<field_num;j++) {
			
			
			if (j==0) e.id =m_store_size;
			if (j==1) strcpy(e.summary,data[j]);
			if (j==2) strcpy(e.location,data[j]);
			if (j==3) strcpy(e.description,data[j]);
			if (j==4) e.year=atoi(data[j]);
			if (j==5) e.month=atoi(data[j]);
			if (j==6) e.day=atoi(data[j]);				          
			          
			if (j==7) e.start_time=atof(data[j]);
			if (j==8) e.end_time=atof(data[j]);          
			
			if (j==9) e.priority=atoi(data[j]);
			if (j==10) e.is_yearly=atoi(data[j]);
			if (j==11) e.is_allday=atoi(data[j]);
			
			//printf("data[%d] = %s\n",j, data[j]);
			free(data[j]);
			
		}
		
		event_store[m_store_size] =e;
		m_store_size=m_store_size+1;
		if(m_store_size>max_records)
		{
			g_print("Error: max number of records exceeded\n");
			exit(1);
		}
		i++;		
	}
	
	total_num_lines=i;
    //g_print("total_number_of_lines =%d\n",total_num_lines);	
	g_object_unref (file_stream);	
	g_object_unref (file);	
		
}

void save_csv_file(){

    GFile *file;
	gchar *file_name ="eventsdb.csv";

	GFileOutputStream *file_stream;
	GDataOutputStream *data_stream;
	
	//g_print("Saving csv data with filename = %s\n",file_name);
	
	file = g_file_new_for_path (file_name);
	file_stream = g_file_replace (file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL);
	
	if (file_stream == NULL) {
		g_object_unref (file);
		g_print("error: unable to open and save database file\n");
		return;
	}

	data_stream = g_data_output_stream_new (G_OUTPUT_STREAM (file_stream));
	
	
	for (int i=0; i<m_store_size; i++)
	{
	char *line="";  
	Event e;
	e=event_store[i];
	
	gchar *id_str = g_strdup_printf("%d", e.id); 
	gchar *year_str = g_strdup_printf("%d", e.year); 
	gchar *month_str = g_strdup_printf("%d", e.month);
	gchar *day_str = g_strdup_printf("%d", e.day); 
	
	gchar *starttime_str = g_strdup_printf("%0.2f", e.start_time); 
	gchar *endtime_str = g_strdup_printf("%0.2f", e.end_time);
	
	gchar *priority_str = g_strdup_printf("%d", e.priority); 
	gchar *isyearly_str = g_strdup_printf("%d", e.is_yearly); 
	gchar *isallday_str = g_strdup_printf("%d", e.is_allday); 
		
	
	line =g_strconcat(line,
	id_str,",",
	e.summary,",",
	e.location,",",
	e.description,",",
	year_str,",",
	month_str,",",
	day_str,",",
	starttime_str,",",
	endtime_str,",",	
	priority_str,",",
	isyearly_str,",",
	isallday_str,",",
	"\n", NULL);
	
	g_data_output_stream_put_string (data_stream, line, NULL, NULL)	;
	}
	
	g_object_unref (data_stream);
	g_object_unref (file_stream);
	g_object_unref (file);
	
}
 

//----------------------------------------------------------------------

static void callbk_day_selected (GtkButton *button, gpointer user_data)
{
   
  GDate *date =user_data;  
  GtkWidget *window = g_object_get_data(G_OBJECT(button), "button-window-key");  
  int day =g_date_get_day(date);
  int month =g_date_get_month(date);  
  int year =g_date_get_year(date);
  m_day =day;
  m_month=month;
  m_year=year;  
  update_calendar(GTK_WINDOW(window)); 
  update_store(m_year,m_month,m_day);
}

static void callbk_next_year(GtkButton *button, gpointer user_data)
{

	GtkWindow *window = user_data;

	m_month=m_month;
	m_day=1;
	m_year =m_year+1;
	m_id_selection=-1;
	m_row_index=-1;
	update_calendar(GTK_WINDOW (window));
}

static void callbk_prev_year(GtkButton *button, gpointer user_data)
{

	GtkWindow *window = user_data;
	m_month=m_month;
	m_day =1;
	m_year=m_year-1;
	m_id_selection=-1;
	m_row_index=-1;
	update_calendar(GTK_WINDOW (window));

}


static void callbk_next_month(GtkButton *button, gpointer user_data)
{
	
	GtkWindow *window = user_data;	
	
	m_month=m_month+1;
	m_day=1;
	
	if (m_month >= 13) {
	m_day=1;
	m_month = 1;
	m_year =m_year+1;
	}
	m_id_selection=-1;
	m_row_index=-1;
	update_calendar(GTK_WINDOW (window));
}

static void callbk_prev_month(GtkButton *button, gpointer user_data)
{
	
	GtkWindow *window = user_data;	
	
	m_month=m_month-1;
	m_day =1;
	if (m_month < 1)
	{
	m_month = 12;
	m_year=m_year-1;
	m_day=1;
	}
	m_id_selection=-1;
	m_row_index=-1;
	update_calendar(GTK_WINDOW (window));
	
}
//---------------------------------------------------------------------
//list box methods
//---------------------------------------------------------------------
static void add_separator (GtkListBoxRow *row, GtkListBoxRow *before, gpointer data)
{
  if (!before)
    return;

  gtk_list_box_row_set_header (row, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL));
}
//---------------------------------------------------------------------
static void callbk_row_activated (GtkListBox    *listbox,
                                  GtkListBoxRow *row,
                                  gpointer       user_data){
   
  
  m_row_index = gtk_list_box_row_get_index (row);
 
  DisplayObject *obj = g_list_model_get_item (G_LIST_MODEL (m_store), m_row_index); 
   
   if(obj==NULL) return; 
   
   gint id_value;
   gchar* label_value;
  
   g_object_get (obj, "id", &id_value, NULL);
   g_object_get (obj, "label", &label_value, NULL);
 
  
  m_id_selection=id_value;
  
  
}
//---------------------------------------------------------------------
static void update_store(int year, int month, int day) {	
   
  g_list_store_remove_all (m_store);//clear
 
  Event e; 
  int start_time=0; 
  for (int i=0; i<m_store_size; i++)
  {
  e=event_store[i];
 
  if ((year==e.year && month ==e.month && day==e.day) || (e.is_yearly && month==e.month && day==e.day))
  {  
  
  DisplayObject *obj; 
  char *time_str="";
  char *summary_str="";
  char* starthour_str="";
  char *startmin_str="";
  char* endhour_str="";
  char *endmin_str="";
  
  float integral_part, fractional_part;
  fractional_part = modff(e.start_time, &integral_part);  
  int start_hour =(int) integral_part; //start_hour
  fractional_part=round(fractional_part *100);
  int start_min=(int) (fractional_part); //start_min
  
  starthour_str = g_strdup_printf("%d", start_hour); 
  startmin_str = g_strdup_printf("%d", start_min);
  
  if (start_min <10){       
  time_str = g_strconcat(time_str, starthour_str,":0", startmin_str," ",NULL);
  } else
  {
  time_str = g_strconcat(time_str, starthour_str,":", startmin_str," ",NULL);
  }
  
   
 // now end time (optional)

 if(m_show_end_time){
	 
  float integral_part_end, fractional_part_end;
  fractional_part_end = modff(e.end_time, &integral_part_end);  
  int end_hour =(int) integral_part_end; //end_hour
  fractional_part_end=round(fractional_part_end *100);
  int end_min=(int) (fractional_part_end); //start_min
  
  endhour_str = g_strdup_printf("%d", end_hour); 
  endmin_str = g_strdup_printf("%d", end_min); 
  
  if (end_min <10){       
     time_str = g_strconcat(time_str,"to ",  endhour_str,":0", endmin_str," ",NULL);
    } else {
		time_str = g_strconcat(time_str,"to ", endhour_str,":", endmin_str," ",NULL);
	} 
 }//show_end_time 
  
  
  if (e.is_allday) {
	  time_str="All day. ";
  }
  else {
	 time_str = g_strconcat(time_str,NULL);
  }
    
  char *display_str ="";
  summary_str=e.summary;
  if(strlen(e.location) ==0)
  {
	display_str = g_strconcat(display_str,time_str,summary_str,".\n", NULL);
  }
  else { 
   display_str = g_strconcat(display_str,time_str,summary_str, " ",e.location, ".", NULL);
  }
  
  if(e.priority) {	  
	  display_str=g_strconcat(display_str, " High Priority.", NULL);
  }
  
  display_str=g_strconcat(display_str, "\n", NULL);
    
  
  float integral_part_sort, fractional_part_sort;
  fractional_part_sort = modff(e.start_time, &integral_part_sort);  
  int start_hour_sort =(int) integral_part_sort; //sort start_hour 
  fractional_part_sort=round(fractional_part_sort *100);
  int start_min_sort=(int) (fractional_part_sort); //sort start_min 
  
  start_time =start_hour_sort*60*60+60*start_min_sort;
  
  //create a g_object for sorting
  obj = g_object_new (display_object_get_type (),
  //"id",     e.id,
  "id",     e.id,
  "label",  display_str, 
  "starttime", start_time,   
  NULL); 					
						
  
  g_list_store_insert_sorted(m_store, obj, compare_items, NULL); 
  g_object_unref (obj);
  } //if selected date 
  } //for
}


//----------------------------------------------------------------------
static void reset_marked_dates() {
	
  //reset marked dates
  int i=0;
  for (i=0;i<31;i++) marked_date[i] = FALSE;
  num_marked_dates = 0;
  
}

static void update_marked_dates(int month, int year) {
	
  //reset marked dates 
  num_marked_dates = 0;  
  Event e;  
  for (int i=0; i<m_store_size; i++)
  {
  e=event_store[i];
  if ((e.month==month && e.year ==year)  || (e.is_yearly && month==e.month))  
  {
	  marked_date[e.day-1]=TRUE; //zero index so 1=0
	  num_marked_dates= num_marked_dates+1;
  } //if
  } //for 
}

static void reset_priority_dates() {
	
  //reset marked dates
  int i=0;
  for (i=0;i<31;i++) priority_date[i] = FALSE;
  num_priority_dates = 0;
  
}

static void update_priority_dates(int month, int year) {
	
  //reset marked dates 
  num_priority_dates = 0;  
  Event e;  
  for (int i=0; i<m_store_size; i++)
  {
  e=event_store[i];
  if ((e.month==month && e.year ==year && e.priority==1)  || (e.is_yearly && month==e.month && e.priority==1))  
  {
	  priority_date[e.day-1]=TRUE; //zero index so 1=0
	  num_priority_dates= num_priority_dates+1;
  } //if
  } //for 
}


//--------------------------------------------------------------------
// About
//----------------------------------------------------------------------
static void callbk_about_close (GtkAboutDialog *dialog, gint response_id, gpointer user_data)
{
  // destroy the about dialog 
  gtk_window_destroy(GTK_WINDOW(dialog));
  
}
//-------------------------------------------------------------------
// About callbk
//-------------------------------------------------------------------
static void callbk_about(GSimpleAction * action, GVariant *parameter, gpointer user_data){
	
	
	GtkWindow *window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (user_data),
                                                 GTK_TYPE_WINDOW));
		
	const gchar *authors[] = {"Alan Crispin", NULL};		
	GtkWidget *about_dialog;
	about_dialog = gtk_about_dialog_new();	
	gtk_window_set_transient_for(GTK_WINDOW(about_dialog),GTK_WINDOW(window));
	gtk_widget_set_size_request(about_dialog, 200,200);
    gtk_window_set_modal(GTK_WINDOW(about_dialog),TRUE);	
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "Tiki Calendar (Gtk4 version)");
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about_dialog), "Version 0.2.0");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog),"Copyright © 2023");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),"Personal calendar"); 
	gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG(about_dialog), GTK_LICENSE_GPL_2_0);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog),"https://github.com/crispinalan/"); 
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(about_dialog),"Tiki Calendar Website");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog), authors);
		
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about_dialog), "x-office-calendar");
	
	//set_widget_font_size(about_dialog);
	gtk_widget_show(about_dialog);	
		
}

//-----------------------------------------------------------------------------------
// Preferences
//----------------------------------------------------------------------------------

static void callbk_set(GtkButton *button, gpointer  user_data){

	GtkWidget *window = user_data;
	GtkWidget *dialog = g_object_get_data(G_OBJECT(button), "dialog-key");

	//calendar
	GtkWidget *check_button_calendar_grid= g_object_get_data(G_OBJECT(button), "check-button-grid-key");
	GtkWidget *check_button_end_time= g_object_get_data(G_OBJECT(button), "check-button-end-time-key");


	//talking
	GtkWidget *check_button_talk= g_object_get_data(G_OBJECT(button), "check-button-talk-key");
    GtkWidget *check_button_talk_startup= g_object_get_data(G_OBJECT(button), "check-button-talk-startup-key");
    GtkWidget *check_button_talk_event_number= g_object_get_data(G_OBJECT(button), "check-button-talk-event-number-key");
    GtkWidget *check_button_talk_times= g_object_get_data(G_OBJECT(button), "check-button-talk-times-key");
    GtkWidget *check_button_talk_tags= g_object_get_data(G_OBJECT(button), "check-button-talk-tags-key");
    GtkWidget *check_button_talk_priority= g_object_get_data(G_OBJECT(button), "check-button-talk-priority-key");
    //GtkWidget *check_button_talk_overlap= g_object_get_data(G_OBJECT(button), "check-button-talk-overlap-key");
    GtkWidget *check_button_reset_all= g_object_get_data(G_OBJECT(button), "check-button-reset-all-key");


	m_show_end_time=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_end_time));
	m_frame=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_calendar_grid));


	m_talk=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_talk));
	m_talk_at_startup=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_talk_startup));
	m_talk_event_number=gtk_check_button_get_active (GTK_CHECK_BUTTON(check_button_talk_event_number));
	m_talk_time=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_talk_times));
	m_talk_tags=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_talk_tags));
	m_talk_priority=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_talk_priority));
	//m_talk_overlap=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_talk_overlap));

	m_talk_reset=gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button_reset_all));

	if(m_talk_reset) {
	//reset everything
	m_talk = 1;
	m_talk_at_startup=1;
	m_talk_event_number=1;
	m_talk_time=1;
	m_talk_tags=1;
	m_frame=0;
	m_show_end_time=0;
	m_talk_priority=0;
	m_talk_overlap=0;	//todo
    m_talk_reset=0; //toggle

	}

	config_write();
	//set_widget_font_size(dialog);
	update_calendar(GTK_WINDOW(window));
	update_store(m_year,m_month,m_day);
	update_header(GTK_WINDOW(window));
	gtk_window_destroy(GTK_WINDOW(dialog));
}

//---------------------------------------------------------------------
// callback preferences
//---------------------------------------------------------------------
static void callbk_preferences(GSimpleAction* action, GVariant *parameter,gpointer user_data)
{
	//g_print("Talk options\n");
	
	GtkWidget *window =user_data;
	
	GtkWidget *dialog; 
	GtkWidget *box; 
	gint response; 
	
	//Check buttons
	GtkWidget *check_button_talk;	
	GtkWidget *check_button_talk_startup;
	GtkWidget *check_button_talk_event_number;
	GtkWidget *check_button_talk_times;
	GtkWidget *check_button_talk_tags;

	GtkWidget *check_button_talk_priority;
	//GtkWidget *check_button_talk_overlap; //todo

	GtkWidget *check_button_calendar_grid;
	GtkWidget *check_button_end_time;
	
	GtkWidget *check_button_reset_all;

	GtkWidget *button_set;

	dialog =gtk_window_new(); //gtk_dialog_new_with_buttons to be deprecated gtk4.10
	
	gtk_window_set_title (GTK_WINDOW (dialog), "Preferences");
	gtk_window_set_default_size(GTK_WINDOW(dialog),350,100);  
	
	box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);  
	gtk_window_set_child (GTK_WINDOW (dialog), box);

	button_set = gtk_button_new_with_label ("Set Preferences");
	g_signal_connect (button_set, "clicked", G_CALLBACK (callbk_set), window);
	
	check_button_calendar_grid= gtk_check_button_new_with_label ("Calendar Grid");
	check_button_end_time= gtk_check_button_new_with_label ("Show End Time");

	check_button_talk = gtk_check_button_new_with_label ("Talk");
	check_button_talk_startup = gtk_check_button_new_with_label ("Talk At Startup");
	check_button_talk_event_number = gtk_check_button_new_with_label ("Talk Event Number");
	check_button_talk_times = gtk_check_button_new_with_label ("Talk Time");
	check_button_talk_tags = gtk_check_button_new_with_label ("Use Speech Tags");
	check_button_talk_priority = gtk_check_button_new_with_label ("Talk Priority");
	//check_button_talk_overlap = gtk_check_button_new_with_label ("Overlap Alert");
	check_button_reset_all = gtk_check_button_new_with_label ("Reset All");

	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_calendar_grid), m_frame);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_end_time), m_show_end_time);

	
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk), m_talk);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_startup), m_talk_at_startup);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_times), m_talk_time);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_tags), m_talk_tags);

	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_event_number), m_talk_event_number);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_priority), m_talk_priority);
	//gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_talk_overlap), m_talk_overlap);
	gtk_check_button_set_active (GTK_CHECK_BUTTON(check_button_reset_all), m_talk_reset);
	
	g_object_set_data(G_OBJECT(button_set), "dialog-key",dialog);

	g_object_set_data(G_OBJECT(button_set), "check-button-grid-key",check_button_calendar_grid);
	g_object_set_data(G_OBJECT(button_set), "check-button-end-time-key",check_button_end_time);

	g_object_set_data(G_OBJECT(button_set), "check-button-talk-key",check_button_talk);
	g_object_set_data(G_OBJECT(button_set), "check-button-talk-startup-key",check_button_talk_startup);
	g_object_set_data(G_OBJECT(button_set), "check-button-talk-times-key",check_button_talk_times);
	g_object_set_data(G_OBJECT(button_set), "check-button-talk-tags-key",check_button_talk_tags);
	g_object_set_data(G_OBJECT(button_set), "check-button-talk-event-number-key",check_button_talk_event_number);
	g_object_set_data(G_OBJECT(button_set), "check-button-talk-priority-key",check_button_talk_priority);
	//g_object_set_data(G_OBJECT(dialog), "check-button-talk-overlap-key",check_button_talk_overlap);
		
	g_object_set_data(G_OBJECT(button_set), "check-button-reset-all-key",check_button_reset_all);
	
	gtk_box_append(GTK_BOX(box), check_button_end_time);
	gtk_box_append(GTK_BOX(box), check_button_calendar_grid);

	gtk_box_append(GTK_BOX(box), check_button_talk);
	gtk_box_append(GTK_BOX(box), check_button_talk_startup);
	gtk_box_append(GTK_BOX(box), check_button_talk_event_number);
	gtk_box_append(GTK_BOX(box), check_button_talk_times);
	gtk_box_append(GTK_BOX(box), check_button_talk_tags);
	gtk_box_append(GTK_BOX(box), check_button_talk_priority);
	//gtk_box_append(GTK_BOX(box), check_button_talk_overlap); //todo
	gtk_box_append(GTK_BOX(box), check_button_reset_all);
	gtk_box_append(GTK_BOX(box), button_set);

	gtk_window_present (GTK_WINDOW (dialog)); 	
	
	
}


//-----------------------------------------------------------------
// Keyboard shortcuts
//-----------------------------------------------------------------

static void callbk_shortcuts(GSimpleAction * action, GVariant *parameter, gpointer user_data){

	GtkWidget *window =user_data;
	GtkWidget *dialog; 
	GtkWidget *box; 
	gint response; 
	
	//labels 	
	GtkWidget *label_speak_sc;
	GtkWidget *label_home_sc;
	
	dialog = gtk_dialog_new_with_buttons ("Information", GTK_WINDOW(window), 
	GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
	"Close", GTK_RESPONSE_CLOSE,
	NULL);                                         
	
	gtk_window_set_default_size(GTK_WINDOW(dialog),380,100);  
	
	box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);  
	gtk_window_set_child (GTK_WINDOW (dialog), box);
	
	label_speak_sc=gtk_label_new("Speak: Spacebar");  
	label_home_sc=gtk_label_new("Goto Today: Home Key");
		
	gtk_box_append(GTK_BOX(box), label_speak_sc);
	gtk_box_append(GTK_BOX(box),label_home_sc);
	
	gtk_window_present (GTK_WINDOW (dialog));
	g_signal_connect (dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);

}

//----------------------------------------------------------------
// Calback information
//-----------------------------------------------------------------
static void callbk_info(GSimpleAction *action, GVariant *parameter,  gpointer user_data){

	GtkWidget *window =user_data;
	GtkWidget *dialog; 
	GtkWidget *box; 
	gint response; 	
	//Check buttons
	GtkWidget *label_record_number;	
	
	GtkWidget *label_desktop_font;
	
	GtkWidget *label_gnome_text_scale;

	GtkWidget *label_work_dir;
	GtkWidget *label_working_dir;
		
	GSettings *settings;
                                               
   dialog = gtk_dialog_new_with_buttons ("Information", GTK_WINDOW(window), 
   GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
   "Close", GTK_RESPONSE_CLOSE,
   NULL);                                         
   
	gtk_window_set_default_size(GTK_WINDOW(dialog),380,100);  
	
	box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);  
	gtk_window_set_child (GTK_WINDOW (dialog), box);
	
	char* record_num_str =" Number of records (max 5000) = ";
	char* n_str = g_strdup_printf("%d", m_store_size);
	record_num_str = g_strconcat(record_num_str, n_str,NULL);   
	label_record_number =gtk_label_new(record_num_str); 

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
	label_working_dir=gtk_label_new(dir_str); 
	//g_print("current directory = %s\n", cur_dir);
	
	gtk_box_append(GTK_BOX(box), label_record_number);
	gtk_box_append(GTK_BOX(box),label_desktop_font);
	gtk_box_append(GTK_BOX(box),label_gnome_text_scale);
	gtk_box_append(GTK_BOX(box),label_work_dir);
	gtk_box_append(GTK_BOX(box),label_working_dir);
	
	//set_widget_font_size(dialog);
	gtk_window_present (GTK_WINDOW (dialog));
	g_signal_connect (dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
 	
}
//----------------------------------------------------------------
// Callback delete all response
//-----------------------------------------------------------------
static void callbk_delete_all_response(GtkDialog *dialog, gint response_id, gpointer  user_data)								 
{

	GtkWindow *window = user_data; //window data
    if(response_id==GTK_RESPONSE_OK)
	{    
    g_print("Danger: Deleting everything\n");
    
    Event e_empty;
    e_empty.id=0;    
    strcpy(e_empty.summary, "");
    strcpy(e_empty.location,"");
    strcpy(e_empty.description,"");
    e_empty.year=0;
    e_empty.month=0;
    e_empty.day=0;
    e_empty.start_time=0;
    e_empty.end_time=0;	
    e_empty.priority;
    e_empty.is_yearly=0;
    e_empty.is_allday=0;
    
    for(int i=0;  i<m_store_size; i++)
    {
		event_store[i]=e_empty;
	}
    
    reset_marked_dates(); 
    reset_priority_dates(); 
    update_calendar(GTK_WINDOW(window));
	update_store(m_year,m_month,m_day);
	m_store_size=0;
	m_id_selection=-1;
	m_row_index=-1; 	
    }
    gtk_window_destroy(GTK_WINDOW(dialog));	       

}

//----------------------------------------------------------------
// Callback home (go to current date)
//-----------------------------------------------------------------
static void callbk_home(GSimpleAction * action, GVariant *parameter, gpointer user_data){
	
		
   GtkWindow *window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (user_data),
                                                 GTK_TYPE_WINDOW));
     
   if( !GTK_IS_WINDOW(window)) {
	   g_print("callbk home: not a window\n");	   	
       return;
   }
   
  GDate *current_date; 
  current_date = g_date_new();
  g_date_set_time_t (current_date, time (NULL)); 
  
  m_day =g_date_get_day(current_date);
  m_month =g_date_get_month(current_date);  
  m_year =g_date_get_year(current_date); 
  g_date_free (current_date);
    
  //mark days with events
  reset_marked_dates();
  reset_priority_dates();
  update_marked_dates(m_month,m_year);
  update_priority_dates(m_month,m_year);
  update_calendar(GTK_WINDOW (window)); 
  update_store(m_year,m_month,m_day); 
   
}
//----------------------------------------------------------------
// Callback quit
//-----------------------------------------------------------------
static void callbk_quit(GSimpleAction * action,
							G_GNUC_UNUSED GVariant      *parameter,
							              gpointer       user_data)
{
	
	g_application_quit(G_APPLICATION(user_data));
	
}

//----------------------------------------------------------------
// compare two events
//-----------------------------------------------------------------
int compare (const void * a, const void * b)
{

  Event *eventA = (Event *)a;
  Event *eventB = (Event *)b;
  
  if (eventA->start_time>eventB->start_time) return 1;
    else if (eventA->start_time<eventB->start_time)return -1;
    return 0;
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
	
	//g_print("Allocating memory of size %d bytes\n",max_records*sizeof(Event));	
	event_store=malloc(max_records*sizeof(Event));
	
	if (event_store==NULL){
	g_print("memory allocation failed -not enough RAM\n");
	exit(1);
	}
	
	
	if(file_exists("eventsdb.csv"))
	{
		//g_print("eventsdb.csv exists-load it\n");
		load_csv_file();
	}
	
	
	
	 //---------------------------------------------------
  
		
}
//----------------------------------------------------------------
// Callback shutdown
//-----------------------------------------------------------------
void callbk_shutdown(GtkWindow *window, gint response_id,  gpointer  user_data){
	//g_print("shutdown function called\n");	
	save_csv_file();
	free(event_store);
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
	if (m_month==1 && day ==1) {  
	//new year	
	 return TRUE;
	}
	
	if (m_month==12 && day==25) {
	//christmas day
	return TRUE;	  
	} 
	
	if (m_month==12 && day==26) {
	//boxing day
	return TRUE;	  
	}
		
	if (m_month == 5) {
     //May complicated
     GDate *first_monday_may;
     first_monday_may = g_date_new_dmy(1, m_month, m_year);
             
     while (g_date_get_weekday(first_monday_may) != G_DATE_MONDAY)
       g_date_add_days(first_monday_may,1);
       
     int may_day = g_date_get_day(first_monday_may);  
       
     if( day==may_day) return TRUE;
     //else return FALSE;
     
     int days_in_may =g_date_get_days_in_month (m_month, m_year);
     int plus_days = 0;
     
     if (may_day + 28 <= days_in_may) {
       plus_days = 28;
     } else {
       plus_days = 21;
     }
     
     GDate *spring_bank =g_date_new_dmy (may_day, m_month, m_year);
     
     g_date_add_days(spring_bank,plus_days);
      
     int spring_bank_day = g_date_get_day(spring_bank);   
      
     if (g_date_valid_dmy (spring_bank_day,m_month,m_year) && day ==spring_bank_day) 
     return TRUE;       
	} //m_month==5 (may)
	
	GDate *easter_date =calculate_easter(m_year); 
	int easter_day = g_date_get_day(easter_date);
	int easter_month =g_date_get_month(easter_date);
	
	if(m_month==easter_month && day == easter_day)
	{
	//easter day
	return TRUE;
	}
	
	g_date_subtract_days(easter_date,2);
	int easter_friday = g_date_get_day(easter_date); 
	int easter_friday_month =g_date_get_month(easter_date); 
	
	if(m_month==easter_friday_month && day ==easter_friday)
	{
	//easter friday
	return TRUE;
	}
	
	g_date_add_days(easter_date,3);
	int easter_monday = g_date_get_day(easter_date); //easter monday
	int easter_monday_month =g_date_get_month(easter_date); 
    
	if(m_month==easter_monday_month && day ==easter_monday)
	{
	//easter monday
	return TRUE;
	}
	
	if (m_month == 8) {
      //August complicated
    GDate *first_monday_august;
     first_monday_august = g_date_new_dmy(1, m_month, m_year);
             
     while (g_date_get_weekday(first_monday_august) != G_DATE_MONDAY)
       g_date_add_days(first_monday_august,1);
       
     int august_day = g_date_get_day(first_monday_august);  
       
          
     int days_in_august =g_date_get_days_in_month (m_month, m_year);
     int plus_days = 0;
     
     if (august_day + 28 <= days_in_august) {
       plus_days = 28;
     } else {
       plus_days = 21;
     }
     
     GDate *august_bank =g_date_new_dmy (august_day, m_month, m_year);
     
     g_date_add_days(august_bank,plus_days);
      
     int august_bank_day = g_date_get_day(august_bank);   
      
     if (g_date_valid_dmy (august_bank_day,m_month,m_year) && day ==august_bank_day) 
     return TRUE;         
      
      
    } //m_month==8
		
	return FALSE;
}

char* get_holiday(int day) {
	
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
	if (m_month==1 && day ==1) { 
	return "New Year's Day";
	}
	
	if (m_month==12 && day==25) {
	//christmas day
	return "Christmas Day";	  
	} 
	
	if (m_month==12 && day==26) {
	//boxing day
	return "Boxing Day";	  
	}
	
	if (m_month == 5) {
     //May complicated
     GDate *first_monday_may;
     first_monday_may = g_date_new_dmy(1, m_month, m_year);
        
     
     while (g_date_get_weekday(first_monday_may) != G_DATE_MONDAY)
       g_date_add_days(first_monday_may,1);
       
     int may_day = g_date_get_day(first_monday_may);  
       
     if( day==may_day) return "May Bank Holiday";
     
     int days_in_may =g_date_get_days_in_month (m_month, m_year);
     
     int plus_days = 0;
     
     if (may_day + 28 <= days_in_may) {
       plus_days = 28;
     } else {
       plus_days = 21;
     }
     
     GDate *spring_bank =g_date_new_dmy (may_day, m_month, m_year);     
     g_date_add_days(spring_bank,plus_days);      
     int spring_bank_day = g_date_get_day(spring_bank);        
     if (g_date_valid_dmy (spring_bank_day,m_month,m_year) && day ==spring_bank_day) 
     return "Spring Bank Holiday";     
           
	} //m_month ==5 (May)
	
	GDate *easter_date =calculate_easter(m_year); 
	int easter_day = g_date_get_day(easter_date);
	int easter_month =g_date_get_month(easter_date);
	
	if(m_month==easter_month && day == easter_day)
	{
	//easter day
	return "Easter Day";
	}
	
	g_date_subtract_days(easter_date,2);
	int easter_friday = g_date_get_day(easter_date); 
	int easter_friday_month =g_date_get_month(easter_date); 
	
	if(m_month==easter_friday_month && day ==easter_friday)
	{
	//easter friday
	return "Easter Friday";
	}
	
	g_date_add_days(easter_date,3);
	int easter_monday = g_date_get_day(easter_date); //easter monday
	int easter_monday_month =g_date_get_month(easter_date); 
    
	if(m_month==easter_monday_month && day ==easter_monday)
	{
	//easter monday
	return "Easter Monday";
	}
	
	if (m_month == 8) {
      //August complicated
    GDate *first_monday_august;
     first_monday_august = g_date_new_dmy(1, m_month, m_year);
             
     while (g_date_get_weekday(first_monday_august) != G_DATE_MONDAY)
       g_date_add_days(first_monday_august,1);
       
     int august_day = g_date_get_day(first_monday_august);  
       
          
     int days_in_august =g_date_get_days_in_month (m_month, m_year);
     int plus_days = 0;
     
     if (august_day + 28 <= days_in_august) {
       plus_days = 28;
     } else {
       plus_days = 21;
     }
     
     GDate *august_bank =g_date_new_dmy (august_day, m_month, m_year);
     
     g_date_add_days(august_bank,plus_days);
      
     int august_bank_day = g_date_get_day(august_bank);   
      
     if (g_date_valid_dmy (august_bank_day,m_month,m_year) && day ==august_bank_day) 
     return "August Bank Holiday";   
     
    } //m_month==8
		
	return "";
}


//---------------------------------------------------------------------
// update ui
//---------------------------------------------------------------------
static void update_calendar(GtkWindow *window) {
	
 
  GtkWidget* label_date;
  GtkWidget *button;
  GtkWidget *grid;
  GtkWidget *button_next_month;
  GtkWidget *button_prev_month; 
  GtkWidget *button_next_year;
  GtkWidget *button_prev_year;
  GtkWidget *sw; //scrolled window
  GtkWidget* listbox;
  GtkWidget *label; //for days of week Mon, Tue etc..
  GtkCssProvider *cssProvider;
  
   //mark days with events
  reset_marked_dates();
  update_marked_dates(m_month,m_year);
  reset_priority_dates();
  update_priority_dates(m_month,m_year);
  
  grid =gtk_grid_new();
  gtk_window_set_child (GTK_WINDOW (window), grid);
  //gtk_grid_set_row_homogeneous (GTK_GRID (grid), TRUE);
  gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);
  
  //GListStore *store;		
  m_store = g_list_store_new (display_object_get_type ()); //class Gio.ListStore
  
  char* weekday_str="";
  GDate *day_date;
  day_date = g_date_new_dmy(m_day, m_month, m_year);
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
 
  gchar* day_str =	g_strdup_printf("%d",m_day);    
  gchar *year_str = g_strdup_printf("%d",m_year ); 
  
  gchar * day_month_year_str="";
  
  day_month_year_str =g_strconcat(day_month_year_str,weekday_str," ", day_str, " ", NULL);
  
  switch(m_month)
     {
         case G_DATE_JANUARY:                    
          day_month_year_str =g_strconcat(day_month_year_str,"January ",year_str, NULL);
          break;
         case G_DATE_FEBRUARY:          
           day_month_year_str =g_strconcat(day_month_year_str,"February ",year_str, NULL);
           break;
         case G_DATE_MARCH:
           day_month_year_str =g_strconcat(day_month_year_str,"March ",year_str, NULL);
           break;
          case G_DATE_APRIL:
           day_month_year_str =g_strconcat(day_month_year_str,"April ",year_str, NULL); 
           break;        
          case G_DATE_MAY:
           day_month_year_str =g_strconcat(day_month_year_str,"May ",year_str, NULL);
           break;         
          case G_DATE_JUNE:
          day_month_year_str =g_strconcat(day_month_year_str,"June ",year_str, NULL); 
           break;        
          case G_DATE_JULY:
           day_month_year_str =g_strconcat(day_month_year_str,"July ",year_str, NULL);
           break;         
          case G_DATE_AUGUST:
           day_month_year_str =g_strconcat(day_month_year_str,"August ",year_str, NULL);
           break;         
          case G_DATE_SEPTEMBER:
           day_month_year_str =g_strconcat(day_month_year_str,"September ",year_str, NULL); 
           break;                  
          case G_DATE_OCTOBER:
           day_month_year_str =g_strconcat(day_month_year_str,"October ",year_str, NULL);
           break;         
          case G_DATE_NOVEMBER:
           day_month_year_str =g_strconcat(day_month_year_str,"November ",year_str, NULL);
           break;         
          case G_DATE_DECEMBER:
           day_month_year_str =g_strconcat(day_month_year_str,"December ",year_str, NULL);
           break;         
         default:
           day_month_year_str =g_strconcat(day_month_year_str,"Unknown ",year_str, NULL);
    }
   
   if (m_holidays) {
	   
	   //append holiday text
	   gchar * holiday_str = get_holiday(m_day);
	   day_month_year_str =g_strconcat(day_month_year_str," ",holiday_str, NULL);
   }
   
   label_date = gtk_label_new("");
   gtk_label_set_xalign(GTK_LABEL(label_date), 0.5);
   gtk_label_set_text(GTK_LABEL(label_date),day_month_year_str);
   
   PangoAttrList *attrs;
   attrs = pango_attr_list_new ();
   pango_attr_list_insert (attrs, pango_attr_weight_new (PANGO_WEIGHT_BOLD));
   gtk_label_set_attributes (GTK_LABEL (label_date), attrs);
   pango_attr_list_unref (attrs);
   
   
  
  int first_day_month = first_day_of_month(m_month,m_year);  
  int week_start = 1; //Monday 
  int day=0;  
  int remainder = (first_day_of_month(m_month,m_year) - week_start + 7) % 7;   
  day = 1 - remainder;   
  int n_cols=7;
  int n_rows=8;  
  gchar* i_str;
  gchar* j_str;
  gchar* btn_str="";
  
  button_next_month=gtk_button_new_with_label (">");
  g_signal_connect (button_next_month, "clicked", G_CALLBACK (callbk_next_month),window);
  
  button_prev_month=gtk_button_new_with_label ("<");
  g_signal_connect (button_prev_month, "clicked", G_CALLBACK (callbk_prev_month),window);  

   button_next_year=gtk_button_new_with_label (">>");
   g_signal_connect (button_next_year, "clicked", G_CALLBACK (callbk_next_year),window);

   button_prev_year=gtk_button_new_with_label ("<<");
   g_signal_connect (button_prev_year, "clicked", G_CALLBACK (callbk_prev_year),window);


   //Grid is 7 days

   /*
	- -*------------------------------------
	0  | 1  | 2  | 3   | 4  | 5  | 6  |
	Mon Tue  Wed  Thu  Fri  Sat  Sun

	*/

  //void gtk_grid_attach (GtkGrid* grid, GtkWidget* child, int column, int row, int width, int height)
   
  gtk_grid_attach(GTK_GRID(grid),button_prev_year,0,0,1,1);
  gtk_grid_attach(GTK_GRID(grid),button_prev_month,1,0,1,1);

  gtk_grid_attach(GTK_GRID(grid),label_date,2,0,3,1);
  gtk_grid_attach(GTK_GRID(grid),button_next_month,5,0,1,1);
  gtk_grid_attach(GTK_GRID(grid),button_next_year,6,0,1,1);
    	
 
  label= gtk_label_new("Mon");
  //set_widget_font_size(label);
  gtk_grid_attach(GTK_GRID(grid),label,0,1,1,1);
  
  label= gtk_label_new("Tue");
  //set_widget_font_size(label);
  gtk_grid_attach(GTK_GRID(grid),label,1,1,1,1);
  
  label= gtk_label_new("Wed");
  //set_widget_font_size(label);
  gtk_grid_attach(GTK_GRID(grid),label,2,1,1,1);
  
  label= gtk_label_new("Thu");
  //set_widget_font_size(label);
  gtk_grid_attach(GTK_GRID(grid),label,3,1,1,1);
  
  label= gtk_label_new("Fri");
  //set_widget_font_size(label);
  gtk_grid_attach(GTK_GRID(grid),label,4,1,1,1);
  
  label= gtk_label_new("Sat");
  //set_widget_font_size(label);
  gtk_grid_attach(GTK_GRID(grid),label,5,1,1,1);
  
  label= gtk_label_new("Sun");
  //set_widget_font_size(label);
  gtk_grid_attach(GTK_GRID(grid),label,6,1,1,1);
  
  GDate *today_date; 
  today_date = g_date_new();
  g_date_set_time_t (today_date, time (NULL));
  int today_day= g_date_get_day(today_date);
  int today_month= g_date_get_month(today_date);
  int today_year= g_date_get_year(today_date);
  
   
  int days_in_month =g_date_get_days_in_month (m_month, m_year); 
 
  for (int row=2; row<n_rows; row++)
    {
        
        for(int col=0; col<n_cols; col++)
        {
                
        if (day > 0 && day <= days_in_month) {
        
        if(marked_date[day-1]) {

			if(day==today_day && m_month==today_month && m_year==today_year) {
				btn_str =g_strdup_printf ("%s%d%s%s","[", day,"]","*"); //%i
			}
			else {
			btn_str =g_strdup_printf ("%d%s", day,"*"); //%i
			}
		}
		else {
			if(day==today_day && m_month==today_month && m_year==today_year) {
				btn_str =g_strdup_printf ("%s%d%s","[", day,"]"); //%i
			}
			else {
			btn_str =g_strdup_printf ("%d", day); //%i
			}
		}

		//btn_str =g_strdup_printf ("%d", day); //%i
        
        button = gtk_button_new_with_label (btn_str); 
        
        gtk_button_set_has_frame (GTK_BUTTON(button),m_frame); //no frame
		
		gtk_widget_set_hexpand(button, TRUE); //horizontal expand
		gtk_widget_set_vexpand(button,TRUE); //vertical expand
		
		 GDate *current_date = g_date_new();
		 g_date_set_dmy (current_date, day, m_month, m_year);

		
		//set_widget_font_size(button);
		g_object_set_data(G_OBJECT(button), "button-window-key",window);         
        g_signal_connect (button, "clicked", G_CALLBACK (callbk_day_selected), current_date);
        gtk_grid_attach(GTK_GRID(grid),button,col,row,1,1);	
        btn_str="";         
         }                        
        day=day+1;        
        }
        
    }

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
  //col-rows (span all 7 days and a further 8 rows)
  gtk_grid_attach(GTK_GRID(grid),sw,0,n_rows+1,7,8);
  
  g_object_set_data(G_OBJECT(window), "window-listbox-key",listbox);

}

static void update_header (GtkWindow *window)
{
	GtkWidget *header;
	GtkWidget *button_new_event;
	GtkWidget *button_edit_event;
	GtkWidget *button_delete_selected;
	GtkWidget *menu_button; 

	header = gtk_header_bar_new ();	
	gtk_window_set_titlebar (GTK_WINDOW(window), header);
	
	button_new_event = gtk_button_new_with_label ("New Event");
	//context = gtk_widget_get_style_context (button_new_event);
	gtk_widget_set_tooltip_text(button_new_event, "New calendar event");
	g_signal_connect (button_new_event, "clicked", G_CALLBACK (callbk_new_event), window);	
	//gtk_button_set_has_frame (GTK_BUTTON(button_new_event),TRUE);
	//g_object_set_data(G_OBJECT(button_new_event), "button-window-key",window);
	
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
	g_menu_append (section, "Information", "app.info"); //show app info
	g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);

	section = g_menu_new ();
	g_menu_append (section, "Shortcuts", "app.shortcuts"); //show shortcuts
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

	const gchar *speak_accels[2] = { "space", NULL };
	const gchar *home_accels[2] = { "Home", NULL };

	// create a new window, and set its title
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), " ");
	gtk_window_set_default_size(GTK_WINDOW (window),700,450);
	g_signal_connect (window, "destroy", G_CALLBACK (callbk_shutdown), NULL);

	GDate *current_date;
	current_date = g_date_new();
	g_date_set_time_t (current_date, time (NULL));

	m_day =g_date_get_day(current_date);
	m_month =g_date_get_month(current_date);
	m_year =g_date_get_year(current_date);

	m_today_day =m_day;
	m_today_month=m_month;
	m_today_year =m_year;

	g_date_free (current_date);

	//mark days with events
	reset_marked_dates();
	update_marked_dates(m_month,m_year);
	reset_priority_dates();
	update_priority_dates(m_month,m_year);

	GSimpleAction *speak_action;	
	speak_action=g_simple_action_new("speak",NULL); //app.speak
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(speak_action)); //make visible	
	g_signal_connect(speak_action, "activate",  G_CALLBACK(callbk_speak), window);
	
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
	
	
	GSimpleAction *shortcuts_action;	
	shortcuts_action=g_simple_action_new("shortcuts",NULL); //app.shortcuts
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(shortcuts_action)); //make visible	
	g_signal_connect(shortcuts_action, "activate",  G_CALLBACK(callbk_shortcuts), window);
	
	GSimpleAction *quit_action;	
	quit_action=g_simple_action_new("quit",NULL); //app.quit
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(quit_action)); //make visible	
	g_signal_connect(quit_action, "activate",  G_CALLBACK(callbk_quit), app);

	// connect keyboard accelerators
	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
										  "app.speak", speak_accels);
	
	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
										  "app.home", home_accels);
	
	update_header(GTK_WINDOW(window));
	update_calendar(GTK_WINDOW (window));
	gtk_window_present (GTK_WINDOW (window));    //use present not show with gtk4
	update_store(m_year,m_month,m_day);

	if(m_talk && m_talk_at_startup) speak_events();
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
