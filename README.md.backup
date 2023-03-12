# Tiki Calendar

Tiki Calendar is a personal desktop calendar with some speech capability developed using C and [Gtk4](https://docs.gtk.org/gtk4/) for Linux desktops such as GNOME.


![](tikicalendar.png)

## Core Features

* built with Gtk4.6
* bespoke custom calendar which allows dates with events to be marked up
* event summary, location, start and end time can be entered and edited
* priority and is-yearly can be used
* csv file storage
* some speech capability
* binary for 64-bit Gtk4 distributions


## Deployment

### Prebuilt Binary

A 64 bit prebuilt binary is available and can be downloaded from [binary](https://github.com/crispinalan/tikicalendar/tree/main/binary) and can be used with Linux distributions that have Gtk4 in their repositories such as Fedora 35 onwards, Ubuntu 22.04 and Debian Bookworm (in testing) etc.

Assuming that the Gtk4 base libraries are installed the Tiki Calendar binary can be run from the terminal using:

```
./tikicalendar
```

or double click on the tikicalendar file. Make sure it has executable permissions. Right click on it, then select permissions and ensure "Allow executing file as program" is selected. Audio output requires that the alsa-utils package is installed (this is usually installed by default).

Use a menu editor such as [MenuLibre](https://github.com/bluesabre/menulibre) to create a launcher for Tiki Calendar which allows the working directory to be chosen.

The database called "events.csv" is located in the working directory. With Tiki Calendar you can use the  menu item information to show the current working directory where the events database should be located.

## Calendar Usage

### Adding New Event

* Select event date using the calendar
* Click the New button on the headerbar to invoke the "New Event" dialog
* Enter the event summary
* Enter the location
* Enter start and end times
* Events are sorted by start time when displayed
* Navigate through the year using the calendar to add events

![](tiki-v027-new-event.png)


### Editing Existing Event

* Select the event in the list view and click the Edit button on the headerbar to edit
* Change details as appropriate

![](tiki-v027-edit-event.png)

### Preferences

* Use the Preferences dialog in the hamburger menu to change  options

![](tiki-v027-preferences.png)

You can use 12 hour format and event end-times can be shown in the list view. If public holidays is selected then the date label shows the public holiday (UK only) which is also spoken.

Talk options can be changed.


## Talking

Tiki Calendar has some speech capability. Tiki (the talking) Calendar (version 0.27 onwards) uses espeak which is a free speech synthesizer available in most Linux distributions. You need to install espeak to enable talking.
 
With Ubuntu 22.04 you do this by using the terminal command below.

```
sudo apt install espeak
```

With Debian Bookworm you need become superuser (su) and so you install espeak using the commands below.

```
su
apt install espeak
```


With Fedora use the following commands

```
su
dnf install espeak
```

* Enable talking in the Preferences window
* Enable talk details to be spoken
* Click on a calendar date with events
* Press the spacebar to read out the event(s) for the selected date
* Enable "Talk At Startup" in the preferences window to read out date and event details when the calendar is started

The use of espeak replaces the previous date reader code which was based on concatenating pre-recorded wav files. Although I added some speech tag words this approach was limited in scope but had the advantage that no external library was needed.

Use the information menu item to check if espeak is installed on your system.


### Information

* Use the Information window to show the current working directory (in which the events.csv file should be stored) and other system information such as the system font being used. You can also check if espeak speech synthesizer is installed.

![](tiki-v027-information.png)


* Use the About dialog to display current version.


### Keyboard Shortcuts
```
Speak           Spacebar
Speak Marks     M
Today           Home Key
```

## Startup Applications

Add Tiki Calendar to your start-up programs to read out the date and any event details when the computer is switched on.

With the GNOME desktop use the GNOME "Tweak Tool" to add Tiki Calendar to your startup applications if required.

## Known Issues

* Calendar visual markers (now resolved v0.27)

~~The css context style and provider classes which were used to colour days with events on the previous button grid calendar are being depreciated in Gtk4.10 (see notes and comments below). Consequently, I have replaced bespoke button grid calendar with the [gk4 calendar widget](https://docs.gtk.org/gtk4/class.Calendar.html) which is styled using the Linux (GNOME) system css theme. This calendar uses gtk_calendar_mark_day() to place a visual marker on a particular day and gtk_calendar_unmark_day() to remove the marker. However, visual markers are not visible on the GNOME desktops (Debian, Fedora)  that I have used for testing Tiki Calendar.~~ 

Tiki Calendar v0.27 has re-introduced the bespoke custom grid calendar (replacing the Calendar shipped with Gtk4) which has now been rewritten as a gobject class using a grid of labels rather than buttons. The custom calendar uses the [gtk_widget_add_css_class](https://docs.gtk.org/gtk4/method.Widget.add_css_class.html) and Pango. All code using the functions gtk_widget_get_style_context, gtk_style_context_add_provider and gtk_color_chooser_get_rgba has been removed since these functions are being depreciated in Gtk4.10. This custom calendar allows dates with events to be marked with a "*". New features will be added as the project moves along.

I have kept the "Speak Month" feature which reads out month days with events. To use this either press the "M" key (M for month markers) or use the menu item called "Speak Month".

* The events.css storage file becomes corrupted

If you open the events.css events storage file with another application (or some text editors) hidden characters or new line characters may be added. If this happens then it will cause a segmentation fault when Tiki Calendar attempts to re-open it. To solve this problem either rename the corrupted events.css file and restart Tiki Calendar which will generated a new events.css file or remove hidden/unwanted characters from the original events.css file.

* Gtk4.10 depreciations

I believe that I have found and removed most of the functions that are to be depreciated in Gtk4.10 (see note below) which has resulted in a large number of changes to the code base. As a consequence some features have been temporarily removed.


## Build From Source

The C source code for the Tiki Calendar project is provided in the src directory.

You need the Gtk4 development libraries and the gcc compiler.

### Fedora

With Fedora you need to install the following packages.

```
sudo dnf install gtk4-devel
sudo dnf install gtk4-devel-docs
sudo dnf install glib-devel
sudo dnf install alsa-lib-devel
```

### Ubuntu and Debian Bookworm

With both Ubuntu 22.04 and Debian Bookworm (in testing) and you need to install the following packages

```
apt install build-essential
apt install libgtk-4-dev
apt install gtk-4-examples
apt install libasound2-dev
apt install flite
```
The packages:
```
apt install libglib2.0-dev
apt install alsa-utils
```
are needed but should be installed by default.

With Ubuntu 22.04 the base Gtk4 libraries should be installed by default. With other Ubuntu based distributions you may have to install these using the command below.

```
sudo apt install libgtk-4-1
```

Use the MAKEFILE to compile.

```
make
./tikicalendar
```
## Themes

The Tiki Calendar version 0.2 series is now themed using the Gtk application theme.  Some examples of how Tiki Calendar looks with  system themes are shown below.

### Adwaita Dark

![](tiki-v027-adwaita-dark.png)

### Ubuntu Yaru Magenta Dark

![](tiki-v027-ubuntu-yaru-magenta-dark.png)

### High Contrast

![](tiki-v027-high-contrast.png)

### User Themes

There is an article [here](https://www.linuxfordevices.com/tutorials/linux/change-gtk4-application-theme) about how to change the Gtk4 application theme in GNOME 40 onwards. There is more information about themes [here](https://www.omgubuntu.co.uk/2017/11/best-gtk-themes-for-ubuntu). Gtk4 application themes can be download from [GNOME-Look](https://www.gnome-look.org/browse/).

The [Flat Remix](https://github.com/daniruiz/flat-remix-gtk) and Tokyo night are popular themes.

As a side note, with Debian Bookworm, to get rid of any installed application themes delete the <ins>.themes</ins> hidden directory in the home folder and any files in the  <ins>.config/gtk-4.0</ins> directory and restart. This should reset everything.


## History

This is a hobby project under development to learn Gtk programming.

The first iteration of the Tiki Calendar project used Gtk3 but then migrated to the Gtk4 toolkit. The Gtk3 project was forked and so can be found elsewhere on github. This Gtk4 version of Tiki Calendar uses a csv file to store events (rather than sqlite). The events storage file is called "events.csv" and should be located in the current working directory.

One of the reasons why I started to investigate using Gtk was that I was concerned with the Qt [licensing changes](https://www.qt.io/blog/qt-offering-changes-2020) relating to LTS versions and how this would impact my [Talk Calendar](https://github.com/crispinalan/talkcalendar) project in the longer term. Qt is [dual-licensed](https://www.qt.io/licensing/) under commercial and open source licenses.

To avoid any confusion with my Qt5 calendar project I have called this Gtk4 version Tiki Calendar. Tiki is an acronym for Tightly Integrated Knowledge Infrastructure. The Tiki Calendar v0.2x series introduced speech capability using the Flite (and now espeak) speech synthesizer. Gtk4 uses one license the [GNU Lesser General Public License version 2.1](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html).


## Gtk4 Depreciations

I have been using Gtk4.6.6 for developing Tiki Calendar. To determine the version of Gtk4 running on a Linux system use the following terminal command.

```
dpkg -l | grep libgtk*
```

Gtk 4.10 has been released and can be downloaded [here](https://download.gnome.org/sources/gtk/). Gtk have announced on their [Gtk4 api website](https://docs.gtk.org/gtk4/) that the following classes

```
AppChooserButton, AppChooserDialog, AppChooserWidget,
CellArea,CellAreaBox, CellAreaContext, CellRenderer, CellRendererAccel, CellRendererCombo, CellRendererPixbuf, CellRendererProgress, CellRendererSpin, CellRendererSpinner, CellRendererText, CellRendererToggle,
ColorButton, ColorChooserDialog, ColorChooserWidget,
ComboBox, ComboBoxText,
Dialog,
EntryCompletion,
FileChooserDialog,FileChooserNative, FileChooserWidget,
IconView,
ListStore, LockButton,
MessageDialog,
Statusbar, StyleContext
TreeModelFilter,TreeModelSort, TreeSelection, TreeStore, TreeView, TreeViewColumn,
VolumeButton
```

are being deprecated in Gtk4.10 onwards.

It was the intention of Gtk developers to eventually replace GtkTreeView and GtkComboBox with [list widgets](https://blog.gtk.org/2020/06/08/more-on-lists-in-gtk-4/) and so I did not use these classes in the development of this calendar.

Functions such as

```
gtk_widget_get_style_context
gtk_style_context_add_provider
gtk_color_chooser_get_rgba
```
 are being depreciated in Gtk4.10. These were used in the Tiki Calendar v0.1.x series to colour the calendar dates with events using css. These changes are discussed [here](https://docs.gtk.org/gtk4/migrating-3to4.html). Style contexts will be removed in [Gtk5](https://docs.gtk.org/gtk4/method.Widget.get_style_context.html). I am in the process of re-writing the code base using  the [gtk_widget_add_css_class](https://docs.gtk.org/gtk4/method.Widget.add_css_class.html) and Pango.

The Gtk ListStore class is going to be depreciated in Gtk4.10. This means functions such as [gtk_list_store_new()](https://docs.gtk.org/gtk4/ctor.ListStore.new.html) used to create a new list store are labeled "deprecated: 4.10". I have used GListStore from [Gio](https://docs.gtk.org/gio/index.html) in this calendar project creating a new GListStore with [g_list_store_new](https://docs.gtk.org/gio/ctor.ListStore.new.html).

Removing MessageDialog means that the function [gtk_message_dialog_new](https://docs.gtk.org/gtk4/ctor.MessageDialog.new.html) is being depreciated which was widely used in the Tiki Calendar v0.1.x series. I am in the process of removing all of these.

The [gtk_dialog_new_with_buttons()](https://docs.gtk.org/gtk4/ctor.Dialog.new_with_buttons.html) function is being depreciated and so the only other way I can see to create a dialog is to use [gtk_window_new](https://docs.gtk.org/gtk4/ctor.Window.new.html). All of the dialogs are now being changed to use gtk_window_new in the Tiki Calendar 0.2.x series.

I have have been going through the calendar code line-by-line removing  class functions that are on the Gtk4.10 depreciation hit list. This has  has meant that some features have had to be temporarily removed.

GTK developers are planning the [Gtk5](https://www.phoronix.com/news/GTK5-Likely-After-GTK-4.12) toolkit discussing making it a Wayland only [release](https://www.phoronix.com/news/GTK5-Might-Drop-X11).

## My Gtk3 to Gtk4 Migration Notes

These (older) notes may be of help if your are migrating a C Gtk3 project to Gtk4.

Gtk4 uses [list widgets](https://docs.gtk.org/gtk4/migrating-3to4.html#consider-porting-to-the-new-list-widgets) such as GtkListBox and porting the Gtk3 version of this Calendar project has involved replacing the display of events with a GtkListBox. A significant effort had to be invested into this aspect of the porting. There is an article on scalable lists in gtk4 [here](https://blog.gtk.org/2020/06/07/scalable-lists-in-gtk-4/).

Gtk have said [publically](https://www.youtube.com/watch?v=qjF-VotgfeY&t=824s) that it is their intention to eventually replace GtkTreeView and GtkComboBox with [list widgets](https://blog.gtk.org/2020/06/08/more-on-lists-in-gtk-4/). The GtkListBox widget provides a vertical list and can be sorted (in this application events are sorted by start time and then displayed). The application work flow has had to be changed as headerbar buttons are now used to create a new event, edit and delete a selected event in the list. I have used buttons with text labels (New, Edit, Delete).

In Gtk4.0, the function

```
gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
```

has been depreciated and so has had to be removed from the code. See this [discussion](https://discourse.gnome.org/t/how-to-center-gtkwindows-in-gtk4/3112).

In Gtk4, the function

```
gtk_dialog_run()
```

has been depreciated. This has been less of an issue as callback functions have been used. See this [discussion](https://discourse.gnome.org/t/how-should-i-replace-a-gtk-dialog-run-in-gtk-4/3501).

I could not place a visual marker on a particular GtkCalendar day using the "gtk_calendar_mark_day()" function. The [GtkInspector](https://wiki.gnome.org/action/show/Projects/GTK/Inspector?action=show&redirect=Projects%2FGTK%2B%2FInspector) debugging tool does not reveal any obvious CSS style theme option that should to be used to do this. To do this I compiled an example calendar app and pressed Ctrl+Shift+I. At the time I was using Fedora 35/36 GNOME version. With no success, I have ended up writing a bespoke month calendar using the Gtk4 grid layout [manager](https://docs.gtk.org/gtk4/class.Grid.html) which arranges child widgets in rows and columns. I used the layout manager arrange 31 buttons in a grid to create a month calendar. Again a significant effort has had to be invested in this aspect of the porting. I thought that I would be able to use css styling to colour mark calendar day buttons having an event but the css context functions that I used are now being depreciated in Gtk4.10 (see above). I also experienced some other issues with using a grid of buttons with a single call back which I can only describe as shuddering. I have no idea what the cause of this was but I have now written my own gobject custom calendar which uses a grid of labels. 

The function "gtk_spin_button_set_text()" has gone in Gtk4. The documented approach for showing spin button [leading zeros](https://people.gnome.org/~ebassi/docs/_build/Gtk/4.0/signal.SpinButton.output.html) I could not get to work. The spin boxes for the start and end times now accept floating point values. I have also removed the priority combobox as comboboxes are on the Gtk4 depreciation hit list (see list widget discussion above) and replaced it with a single high priority check button. It now turns out that dialogs are being depreciated in Gtk4.10. See the Gtk4 depreciations discussion above.

The Gtk3 container API has been dropped in Gtk4. In Gtk4, widgets can have children.

Other depreciations include "gtk_application_set_app_menu()" as discussed [here](https://wiki.gnome.org/HowDoI/ApplicationMenu). The function "gtk_button_set_image()" has gone. In the context of menu development it can be replaced with "gtk_menu_button_set_icon_name()".

## Libadwaita

From what I can make out libadwaita is a GNOME specific version of Gtk4 which takes control of the look and feel of the user interface. It seems that you have to use a GNOME project source code template which replaces GTK_TYPE_APPLICATION with ADW_TYPE_APPLICATION. You have to use #include <libadwaita.h> and the build system has to be modified to find the libadwaita library. Otherwise, libadwaita uses the Gtk API but the application style is different with windows, for example, having GNOME rounded corners. I believe it also adds things like notifications and animations.

This is not a libadwaita project but a Gtk4 only project. The project focus is on improving the current code base, removing functions which are to be deprecated in Gtk4.10 and adding further calendar features.

## GNOME Testing

I have been using GNOME classic for testing Tiki Calendar. With Ubuntu you have to install the 
gnome-shell-extensions package to get the GNOME classic desktop using the commands below. 

```
sudo apt update
sudo apt install gnome-shell-extensions
```
You select GNOME classic on log-in which gives the good old traditional Ubuntu desktop experience which can be enhanced with extensions such as Arc menu.

With Debian the GNOME classic desktop is installed by default and is selected on log-in.


## Versioning

[SemVer](http://semver.org/) is used for versioning. The version number has the form 0.0.0 representing major, minor and bug fix changes.

## Author

* **Alan Crispin** [Github](https://github.com/crispinalan)


## License

The Gtk4.0 GUI toolkit is licensed using LGPLv2.1.  Consequently, Tiki Calendar has been licensed using the GNU General Public License.

## Acknowledgements

* [Gtk](https://www.gtk.org/)
* GTK is a free and open-source project maintained by GNOME and an active community of contributors. GTK is released under the terms of the [GNU Lesser General Public License version 2.1](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html).

* [Gtk4 API](https://docs.gtk.org/gtk4/index.html)

* [GObject API](https://docs.gtk.org/gobject/index.html)

* [Glib API](https://docs.gtk.org/glib/index.html)

* [Gio API](https://docs.gtk.org/gio/index.html)

* [eSpeak](https://espeak.sourceforge.net/) open source software speech synthesizer used by most Linux distributions. It is licensed under GPL v3.0.

* [Visual Studio Code](https://code.visualstudio.com/) open source development tool. MIT license.


