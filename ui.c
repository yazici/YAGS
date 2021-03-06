////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ui.c		Handle user interface (touch screen and jog wheel)
//
//////////////////////////////////////////////////////////////////////
#include <gtk/gtk.h>

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/stat.h>

GtkWidget *statusline;

GtkWidget *droX;
GtkWidget *jogX;
GtkWidget *droY;
GtkWidget *jogY;
GtkWidget *droZ;
GtkWidget *jogZ;
GtkWidget *droA;
GtkWidget *jogA;

GtkWidget *jog1;
GtkWidget *jogdot1;
GtkWidget *jogdot01;
GtkWidget *jogdot001;

GtkWidget *zeroX;
GtkWidget *zeroY;
GtkWidget *zeroZ;
GtkWidget *zeroA;

GtkWidget *goX;
GtkWidget *goY;
GtkWidget *goZ;
GtkWidget *goA;

GtkWidget *toggleCoor;

GtkWidget *commstat;
GtkWidget *lineh;
GtkWidget *gclineh;
GtkWidget *feedh;
GtkWidget *velocityh;

GtkWidget *planqh;
GtkWidget *measureh;
GtkWidget *feedOverrideh;

GtkWidget *Play;
GtkWidget *Pause;
GtkWidget *Stop;

GtkWidget *SpinCtrl;
GtkWidget *CoolCtrl;
GtkWidget *LasOnCtrl;

GtkWidget *Goto_Zero;
GtkWidget *Home_Cycle;
GtkWidget *Set_Zero;

GtkWidget *GCFile;

GtkWidget *NCFilter;

GtkWidget *EstTime;
GtkWidget *RemTime;

GtkWidget *Level;

GtkWidget *ZProbe;

struct encoder
{
    int pin_a;
    int pin_b;
    volatile long value;
    volatile int lastEncoded;
};

int PlayCGFile (void);
void sersendz(char *buf);
void getCurr(float *cX, float *cY, float *cZ, float *cA);


extern struct encoder *encoder;

extern void sersend (char *buf, int len);
extern void *send_gcode (void *ptr);

double 	dX = 0.0, dY = 0.0, dZ = 0.0, dA = 0.0;

void destroy (void)
{
     gtk_main_quit ();
}

int coor = 0;

void zeroX_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;
	char temp[80];
	float x, y, z, a;


	if(isIdle())
	{
    // get the current xyza values
		getCurr(&x, &y, &z, &a);

    // set the g10 offset values
    sprintf(temp, "g10 L2 P%d x%8.4f\n", coor + 1, x);
    sersendz (temp);		// set the zero values

    // reset g28 so zero is here TCSANOW
    sprintf(temp, "g28.3 x%8.4f\n", x);
    sersendz (temp);		// set the zero values
    sersendz ("{}\n");		// fetch an update

		dX = 0.0;
		gtk_label_set (GTK_LABEL (droX), "000.000");
	}
}

void zeroY_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;
	char temp[80];
	float x, y, z, a;


	if(isIdle())
	{
    // get the current xyza values
		getCurr(&x, &y, &z, &a);

    // set the g10 offset values
    sprintf(temp, "g10 L2 P%d y%8.4f\n", coor + 1, y);
    sersendz (temp);		// set the zero values

    // reset g28 so zero is here TCSANOW
    sprintf(temp, "g28.3 y%8.4f\n", y);
    sersendz (temp);		// set the zero values
    sersendz ("{}\n");		// fetch an update

		dY = 0.0;
		gtk_label_set (GTK_LABEL (droY), "000.000");
	}
}

void zeroZ_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;
	char temp[80];
	float x, y, z, a;


	if(isIdle())
	{
    // get the current xyza values
		getCurr(&x, &y, &z, &a);


    // set the g10 offset values
    sprintf(temp, "g10 L2 P%d z%8.4f\n", coor + 1, z);
    sersendz (temp);		// set the zero values

    // reset g28 so zero is here TCSANOW
    sprintf(temp, "g28.3 z%8.4f\n", z);
    sersendz (temp);		// set the zero values
    sersendz ("{}\n");		// fetch an update

		dZ = 0.0;
		gtk_label_set (GTK_LABEL (droZ), "000.000");
	}
}

void zeroA_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;
	char temp[80];
	float x, y, z, a;


	if(isIdle())
	{
    // get the current xyza values
		getCurr(&x, &y, &z, &a);


    // set the g10 offset values
    sprintf(temp, "g10 L2 P%d a%8.4f\n", coor + 1, a);
    sersendz (temp);		// set the zero values

    // reset g28 so zero is here TCSANOW
    sprintf(temp, "g28.3 a%8.4f\n", a);
    sersendz (temp);		// set the zero values
    sersendz ("{}\n");		// fetch an update

		dA = 0.0;
		gtk_label_set (GTK_LABEL (droA), "000.000");
	}
}

int feed_over_state = 0;	// set as default
void feedover_clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

//    if(!isIdle())
//  	{
  		if (feed_over_state)
  		{
  			feed_over_state = 0;
  			gtk_button_set_label(GTK_BUTTON(feedOverrideh), "FeedOver");
  		}
  		else
  		{
        feed_over_state = 1;
  			gtk_button_set_label(GTK_BUTTON(feedOverrideh), "(FeedOver)");
        gtk_button_set_label(GTK_BUTTON(jogX), "JogX");
    		gtk_button_set_label(GTK_BUTTON(jogY), "JogY");
    		gtk_button_set_label(GTK_BUTTON(jogZ), "JogZ");
    		gtk_button_set_label(GTK_BUTTON(jogA), "JogA");
  		}
//  	}
}

int isFO(void)
{
  return feed_over_state;
}

int 	jogWhat = -1;	// default, no jogging
int getJW(void)
{
  if(	!isIdle() && feed_over_state)
  {
    return 4; // means we are jogging feed override
  }
  else
  {
    return jogWhat;
  }
}

void jogX_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	if (jogWhat == 0)
	{
		gtk_button_set_label(GTK_BUTTON(jogX), "JogX");
		jogWhat = -1;
	}
	else
	{
		gtk_button_set_label(GTK_BUTTON(jogX), "(JogX)");
		gtk_button_set_label(GTK_BUTTON(jogY), "JogY");
		gtk_button_set_label(GTK_BUTTON(jogZ), "JogZ");
		gtk_button_set_label(GTK_BUTTON(jogA), "JogA");
		jogWhat = 0;
	}
}

void jogY_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	if (jogWhat == 1)
	{
		gtk_button_set_label(GTK_BUTTON(jogY), "JogY");
		jogWhat = -1;
	}
	else
	{
		gtk_button_set_label(GTK_BUTTON(jogX), "JogX");
		gtk_button_set_label(GTK_BUTTON(jogY), "(JogY)");
		gtk_button_set_label(GTK_BUTTON(jogZ), "JogZ");
		gtk_button_set_label(GTK_BUTTON(jogA), "JogA");
		jogWhat = 1;
	}
}

void jogZ_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	if (jogWhat == 2)
	{
		gtk_button_set_label(GTK_BUTTON(jogZ), "JogZ");
		jogWhat = -1;
	}
	else
	{
		gtk_button_set_label(GTK_BUTTON(jogX), "JogX");
		gtk_button_set_label(GTK_BUTTON(jogY), "JogY");
		gtk_button_set_label(GTK_BUTTON(jogZ), "(JogZ)");
		gtk_button_set_label(GTK_BUTTON(jogA), "JogA");
		jogWhat = 2;
	}
}

void jogA_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	if (jogWhat == 3)
	{
		gtk_button_set_label(GTK_BUTTON(jogA), "JogA");
		jogWhat = -1;
	}
	else
	{
		gtk_button_set_label(GTK_BUTTON(jogX), "JogX");
		gtk_button_set_label(GTK_BUTTON(jogY), "JogY");
		gtk_button_set_label(GTK_BUTTON(jogZ), "JogZ");
		gtk_button_set_label(GTK_BUTTON(jogA), "(JogA)");
		jogWhat = 3;
	}
}

double	jogMult = 0.01;
double getJM(void)
{
	return jogMult;
}

void jog1_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;
    gtk_button_set_label(GTK_BUTTON(jog1), "(Jog 1.0)");
    gtk_button_set_label(GTK_BUTTON(jogdot1), "Jog 0.1");
    gtk_button_set_label(GTK_BUTTON(jogdot01), "Jog 0.01");
    gtk_button_set_label(GTK_BUTTON(jogdot001), "Jog 0.001");
    jogMult = 1.0;
}
void jogdot1_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;
    gtk_button_set_label(GTK_BUTTON(jog1), "Jog 1.0");
    gtk_button_set_label(GTK_BUTTON(jogdot1), "(Jog 0.1)");
    gtk_button_set_label(GTK_BUTTON(jogdot01), "Jog 0.01");
    gtk_button_set_label(GTK_BUTTON(jogdot001), "Jog 0.001");
    jogMult = 0.1;
}
void jogdot01_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;
    gtk_button_set_label(GTK_BUTTON(jog1), "Jog 1.0");
    gtk_button_set_label(GTK_BUTTON(jogdot1), "Jog 0.1");
    gtk_button_set_label(GTK_BUTTON(jogdot01), "(Jog 0.01)");
    gtk_button_set_label(GTK_BUTTON(jogdot001), "Jog 0.001");
    jogMult = 0.01;
}
void jogdot001_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;
    gtk_button_set_label(GTK_BUTTON(jog1), "Jog 1.0");
    gtk_button_set_label(GTK_BUTTON(jogdot1), "Jog 0.1");
    gtk_button_set_label(GTK_BUTTON(jogdot01), "Jog 0.01");
    gtk_button_set_label(GTK_BUTTON(jogdot001), "(Jog 0.001)");
    jogMult = 0.001;
}

void goX_Clicked(GtkButton * b, gpointer data)
{
  if(isIdle())
	{
    sersendz ("g0 x0\n");		// go to the zero setting
  }
}
void goY_Clicked(GtkButton * b, gpointer data)
{
  if(isIdle())
	{
    sersendz ("g0 y0\n");		// go to the zero setting
  }
}
void goZ_Clicked(GtkButton * b, gpointer data)
{
  if(isIdle())
	{
    sersendz ("g0 z0\n");		// go to the zero setting
  }
}
void goA_Clicked(GtkButton * b, gpointer data)
{
  if(isIdle())
	{
    sersendz ("g0 a0\n");		// go to the zero setting
  }
}

void togglecoor_Clicked(GtkButton * b, gpointer data)
{
  if(coor == 0)
  {
      coor++;
      gtk_button_set_label(GTK_BUTTON(toggleCoor), "G55 Coor");
      sersendz ("g55\n");
  }
  else if(coor == 1)
  {
    coor++;
    gtk_button_set_label(GTK_BUTTON(toggleCoor), "G56 Coor");
    sersendz ("g56\n");
  }
  else if(coor == 2)
  {
    coor++;
    gtk_button_set_label(GTK_BUTTON(toggleCoor), "G57 Coor");
    sersendz ("g57\n");
  }
  else if(coor == 3)
  {
    coor++;
    gtk_button_set_label(GTK_BUTTON(toggleCoor), "G58 Coor");
    sersendz ("g58\n");
  }
  else if(coor == 4)
  {
    coor++;
    gtk_button_set_label(GTK_BUTTON(toggleCoor), "G59 Coor");
    sersendz ("g59\n");
  }
  else if(coor == 5)
  {
    coor = 0;
    gtk_button_set_label(GTK_BUTTON(toggleCoor), "Mach Coor");
    sersendz ("g54\n");
  }
  sersendz ("{}\n");		// fetch an update

}
void enablePlay(int enF)
{
  gtk_widget_set_sensitive(Play, enF);
}
void enablePause(int enF)
{
  gtk_widget_set_sensitive(Pause, enF);
}
void enableStop(int enF)
{
  gtk_widget_set_sensitive(Stop, enF);
}

void Play_Clicked(GtkButton * b, gpointer data)
{

    (void)b;
    (void)data;

	Tr_Play();
}

void Pause_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	Tr_Pause();
}

void Stop_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	Tr_Stop();
}

// attempt to start a leveling sequence
void Level_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	if (isCommUp() && isIdle())
	{
		Tr_Level();
	}
}

void file_set(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	estCGFile ();
}

void Goto_Zero_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	if(isIdle())
	{
    sersendz ("g0 z0 x0 y0\n");		// go to the zero setting
	}
}

void Home_Cycle_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	if(isIdle())
	{
		sersend ("g28.2 z0 x0 y0\n", 15);		// home cycle
    sersendz("g10 L2 P1 x0 y0 z0 a0\n");
	}
}

void ZProbe_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;

	if(isIdle())
	{
		// zero z probe
		Tr_ZProbe();
	}
}

void Set_Zero_Clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;
	char temp[80];
	float x, y, z, a;


	if(isIdle())
	{
    // get the current xyza values
		getCurr(&x, &y, &z, &a);


    // set the g10 offset values
    sprintf(temp, "g10 L2 P%d x%8.4f y%8.4f z%8.4f a%8.4f\n", coor + 1, x, y, z, a);
    sersendz (temp);		// set the zero values

    // reset g28 so zero is here TCSANOW
    sprintf(temp, "g28.3 x%8.4f y%8.4f z%8.4f a%8.4f\n", x, y, z, a);
    sersendz (temp);		// set the zero values
    sersendz ("{}\n");		// fetch an update

		dA = 0.0;
		gtk_label_set (GTK_LABEL (droA), "000.000");
		dX = 0.0;
		gtk_label_set (GTK_LABEL (droX), "000.000");
		dY = 0.0;
		gtk_label_set (GTK_LABEL (droY), "000.000");
		dZ = 0.0;
		gtk_label_set (GTK_LABEL (droZ), "000.000");
	}
}

int meas_mode_mm = 1;	// set as default
void measure_clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;


	if(isIdle())
	{
		if (meas_mode_mm)
		{
			meas_mode_mm = 0;
			gtk_button_set_label(GTK_BUTTON(measureh), "mm/(inch)");
			sersendNT ("g20\n");	// set to inches
		}
		else
		{
			meas_mode_mm = 1;
			gtk_button_set_label(GTK_BUTTON(measureh), "(mm)/inch");
			sersendNT ("g21\n");	// set to mm
		}
	}
}

void measure_setinch(void)
{
	meas_mode_mm = 0;
	gtk_button_set_label(GTK_BUTTON(measureh), "mm/(inch)");
}
void measure_setmm(void)
{
	meas_mode_mm = 1;
	gtk_button_set_label(GTK_BUTTON(measureh), "(mm)/inch");
}

int spin_mode = 0;	// set as default
void spin_clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;


	if(isIdle() || isFreeh())
	{
		if (spin_mode)
		{
			spin_mode = 0;
			gtk_button_set_label(GTK_BUTTON(SpinCtrl), "Spindle On");
			sersendNT ("M5 S000\n");	// spindle off
		}
		else
		{
			spin_mode = 1;
			gtk_button_set_label(GTK_BUTTON(SpinCtrl), "Spindle Off");
			sersendNT ("M3 S1800\n");	// spindle on
		}
	}
}

int las_mode = 0;	// set as default
void LaserOn_clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;


	if(isIdle() || isFreeh())
	{
		if (las_mode)
		{
			las_mode = 0;
			gtk_button_set_label(GTK_BUTTON(LasOnCtrl), "Laser On");
			sersendNT ("M5 S1000\n");	// spindle off
		}
		else
		{
			las_mode = 1;
			gtk_button_set_label(GTK_BUTTON(LasOnCtrl), "Laser Off");
			sersendNT ("M3 S2000\n");	// spindle on
		}
	}
}

extern void initState(void);
int cool_mode = 0;	// set as default
void cool_clicked(GtkButton * b, gpointer data)
{
    (void)b;
    (void)data;


	if(isIdle() || isFreeh())
	{
		if (cool_mode)
		{
			cool_mode = 0;
			gtk_button_set_label(GTK_BUTTON(CoolCtrl), "Cooling On");
			sersendNT ("M9\n");	// cooling off
		}
		else
		{
			cool_mode = 1;
			gtk_button_set_label(GTK_BUTTON(CoolCtrl), "Cooling Off");
			sersendNT ("M7 M8\n");	// cooling on
		}
	}
}

void display_status (char *buf)
{
  gtk_label_set (GTK_LABEL (statusline), buf);
//  gtk_statusbar_push (GTK_STATUSBAR (statusline), 0, buf);
}

void setupui (int argc, char **argv)
{
    GtkBuilder *builder;
    GtkWidget  *window;
    GError     *error = NULL;
    PangoFontDescription *dfl;
    PangoFontDescription *efl;
    PangoFontDescription *sfl;


    // create font specifiers
    dfl = pango_font_description_from_string ("Monospace");
    pango_font_description_set_size (dfl, 30*PANGO_SCALE);
    efl = pango_font_description_from_string ("Monospace");
    pango_font_description_set_size (efl, 17*PANGO_SCALE);
    sfl = pango_font_description_from_string ("Monospace");
    pango_font_description_set_size (sfl, 11*PANGO_SCALE);

    /* Init GTK+ */
    gdk_threads_init();
    gtk_init( &argc, &argv );

    /* Create new GtkBuilder object */
    builder = gtk_builder_new();

    /* Load UI from file. If error occurs, report it and quit application.
     * Replace "tut.glade" with your saved project. */
    if( ! gtk_builder_add_from_file( builder, "cnc-head.glade", &error ) )
    {
        g_warning( "%s", error->message );
        g_free( error );
        return;
    }

	// setup the leveler
	levInit();

    /* Get main window pointer from UI */
    window = GTK_WIDGET( gtk_builder_get_object( builder, "window1" ) );

  	gtk_signal_connect (GTK_OBJECT (window), "destroy",
						GTK_SIGNAL_FUNC (destroy), NULL);

  // get status line handle
    statusline = GTK_WIDGET( gtk_builder_get_object( builder, "statusLine" ) );
    display_status ("Status");

	// get dro label handles
    droX = GTK_WIDGET( gtk_builder_get_object( builder, "droX" ) );
    gtk_widget_modify_font(droX, dfl);
    jogX = GTK_WIDGET( gtk_builder_get_object( builder, "JogX" ) );
    droY = GTK_WIDGET( gtk_builder_get_object( builder, "droY" ) );
    gtk_widget_modify_font(droY, dfl);
    jogY = GTK_WIDGET( gtk_builder_get_object( builder, "JogY" ) );
    droZ = GTK_WIDGET( gtk_builder_get_object( builder, "droZ" ) );
    gtk_widget_modify_font(droZ, dfl);
    jogZ = GTK_WIDGET( gtk_builder_get_object( builder, "JogZ" ) );
    droA = GTK_WIDGET( gtk_builder_get_object( builder, "droA" ) );
    gtk_widget_modify_font(droA, dfl);
    jogA = GTK_WIDGET( gtk_builder_get_object( builder, "JogA" ) );

    // get jog size handles
    jog1 = GTK_WIDGET( gtk_builder_get_object( builder, "jog1" ) );
    jogdot1 = GTK_WIDGET( gtk_builder_get_object( builder, "jogdot1" ) );
    jogdot01 = GTK_WIDGET( gtk_builder_get_object( builder, "jogdot01" ) );
    jogdot001 = GTK_WIDGET( gtk_builder_get_object( builder, "jogdot001" ) );

    // get zero command handles
    zeroX = GTK_WIDGET( gtk_builder_get_object( builder, "ZeroX" ) );
    zeroY = GTK_WIDGET( gtk_builder_get_object( builder, "ZeroY" ) );
    zeroZ = GTK_WIDGET( gtk_builder_get_object( builder, "ZeroZ" ) );
    zeroA = GTK_WIDGET( gtk_builder_get_object( builder, "ZeroA" ) );

    // get zero command handles
    goX = GTK_WIDGET( gtk_builder_get_object( builder, "GoX" ) );
    goY = GTK_WIDGET( gtk_builder_get_object( builder, "GoY" ) );
    goZ = GTK_WIDGET( gtk_builder_get_object( builder, "GoZ" ) );
    goA = GTK_WIDGET( gtk_builder_get_object( builder, "GoA" ) );

    toggleCoor = GTK_WIDGET( gtk_builder_get_object( builder, "machcoor" ) );

    commstat = GTK_WIDGET( gtk_builder_get_object( builder, "commstat" ) );
    lineh 	= GTK_WIDGET( gtk_builder_get_object( builder, "lineh" ) );
  	gclineh 	= GTK_WIDGET( gtk_builder_get_object( builder, "gclineh" ) );
    feedh 	= GTK_WIDGET( gtk_builder_get_object( builder, "feedh" ) );
    velocityh = GTK_WIDGET( gtk_builder_get_object( builder, "velh" ) );
    gtk_widget_modify_font(lineh, efl);
    gtk_widget_modify_font(gclineh, efl);
    gtk_widget_modify_font(feedh, efl);
    gtk_widget_modify_font(velocityh, efl);
    gtk_widget_modify_font(statusline, sfl);

	planqh 		= GTK_WIDGET( gtk_builder_get_object( builder, "planq" ) );
  measureh 	= GTK_WIDGET( gtk_builder_get_object( builder, "measure" ) );
  feedOverrideh 	= GTK_WIDGET( gtk_builder_get_object( builder, "FeedOver" ) );

	CoolCtrl 	= GTK_WIDGET( gtk_builder_get_object( builder, "CoolingCtrl" ) );
	SpinCtrl 	= GTK_WIDGET( gtk_builder_get_object( builder, "SpinCtrl" ) );
	LasOnCtrl 	= GTK_WIDGET( gtk_builder_get_object( builder, "LaserOn" ) );

    Play	 = GTK_WIDGET( gtk_builder_get_object( builder, "Play" ) );
    Pause	 = GTK_WIDGET( gtk_builder_get_object( builder, "Pause" ) );
    Stop	 = GTK_WIDGET( gtk_builder_get_object( builder, "Stop" ) );

	Goto_Zero	 = GTK_WIDGET( gtk_builder_get_object( builder, "Goto_Zero" ) );
	Home_Cycle	 = GTK_WIDGET( gtk_builder_get_object( builder, "Home_Cycle" ) );
	Set_Zero	 = GTK_WIDGET( gtk_builder_get_object( builder, "Set_Zero" ) );

	GCFile	= GTK_WIDGET( gtk_builder_get_object( builder, "GCode_file" ) );
	NCFilter = GTK_WIDGET( gtk_builder_get_object( builder, "filefilter1" ) );
	gtk_file_filter_add_pattern(GTK_FILE_FILTER(NCFilter), "*.nc");

	EstTime	= GTK_WIDGET( gtk_builder_get_object( builder, "est_time" ) );
	RemTime	= GTK_WIDGET( gtk_builder_get_object( builder, "rem_time" ) );

	Level	= GTK_WIDGET( gtk_builder_get_object( builder, "level" ) );
	ZProbe	= GTK_WIDGET( gtk_builder_get_object( builder, "zprobe" ) );

    /* Connect signals */
    gtk_signal_connect (GTK_OBJECT(ZProbe), "clicked", GTK_SIGNAL_FUNC(ZProbe_Clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(jogX), "clicked", GTK_SIGNAL_FUNC(jogX_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(jogY), "clicked", GTK_SIGNAL_FUNC(jogY_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(jogZ), "clicked", GTK_SIGNAL_FUNC(jogZ_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(jogA), "clicked", GTK_SIGNAL_FUNC(jogA_Clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(zeroX), "clicked", GTK_SIGNAL_FUNC(zeroX_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(zeroY), "clicked", GTK_SIGNAL_FUNC(zeroY_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(zeroZ), "clicked", GTK_SIGNAL_FUNC(zeroZ_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(zeroA), "clicked", GTK_SIGNAL_FUNC(zeroA_Clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(goX), "clicked", GTK_SIGNAL_FUNC(goX_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(goY), "clicked", GTK_SIGNAL_FUNC(goY_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(goZ), "clicked", GTK_SIGNAL_FUNC(goZ_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(goA), "clicked", GTK_SIGNAL_FUNC(goA_Clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(toggleCoor), "clicked", GTK_SIGNAL_FUNC(togglecoor_Clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(jog1), "clicked", GTK_SIGNAL_FUNC(jog1_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(jogdot1), "clicked", GTK_SIGNAL_FUNC(jogdot1_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(jogdot01), "clicked", GTK_SIGNAL_FUNC(jogdot01_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(jogdot001), "clicked", GTK_SIGNAL_FUNC(jogdot001_Clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(Play), "clicked", GTK_SIGNAL_FUNC(Play_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(Pause), "clicked", GTK_SIGNAL_FUNC(Pause_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(Stop), "clicked", GTK_SIGNAL_FUNC(Stop_Clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(Goto_Zero), "clicked", GTK_SIGNAL_FUNC(Goto_Zero_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(Home_Cycle), "clicked", GTK_SIGNAL_FUNC(Home_Cycle_Clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(Set_Zero), "clicked", GTK_SIGNAL_FUNC(Set_Zero_Clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(measureh), "clicked", GTK_SIGNAL_FUNC(measure_clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(feedOverrideh), "clicked", GTK_SIGNAL_FUNC(feedover_clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(CoolCtrl), "clicked", GTK_SIGNAL_FUNC(cool_clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(SpinCtrl), "clicked", GTK_SIGNAL_FUNC(spin_clicked), NULL);
    gtk_signal_connect (GTK_OBJECT(LasOnCtrl), "clicked", GTK_SIGNAL_FUNC(LaserOn_clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(Level), "clicked", GTK_SIGNAL_FUNC(Level_Clicked), NULL);

    gtk_signal_connect (GTK_OBJECT(GCFile), "file-set", GTK_SIGNAL_FUNC(file_set), NULL);
    gtk_file_chooser_set_current_folder_uri ((GtkFileChooser *)GCFile, "file:///home/pi/Documents");

    /* Destroy builder, since we don't need it anymore */
    g_object_unref( G_OBJECT( builder ) );

    /* Show window. All other widgets are automatically shown by GtkBuilder */
    gtk_widget_show( window );

    initState();

}

int commStat = 0;
int isCommUp (void)
{
	return commStat;
}

void updatecommstat(int commOn)
{
	commStat = commOn;
	gdk_threads_enter();
	if (commOn)
	{
		gtk_label_set (GTK_LABEL (commstat), "(Connected)");
	}
	else
	{
		gtk_label_set (GTK_LABEL (commstat), "(Disconnected)");
	}

	gdk_threads_leave();
}

void updatePlanq (float pQ)
{
	char temp[100];
	float disp_prog;

	// reform the percent for display purposes
	disp_prog = (32.0 - pQ) * 3;

	gdk_threads_enter();
	gtk_progress_set_value (GTK_PROGRESS (planqh), disp_prog);
	gdk_threads_leave();
}

void updateGCLine (int GCL, int lock)
{
	char temp[100];

	if(lock) gdk_threads_enter();
	sprintf(temp, "%d", GCL);
	gtk_label_set (GTK_LABEL (gclineh), temp);
	if(lock) gdk_threads_leave();
}

void updateGCEstTime (int time, int lock)
{
	char temp[100];
	int hour, min, sec;

	if(lock) gdk_threads_enter();
	sec = time % 60; min = (time / 60) % 60; hour = time / 3600;
	sprintf(temp, "%02d:%02d:%02d", hour, min, sec);
	gtk_label_set (GTK_LABEL (EstTime), temp);
	if(lock) gdk_threads_leave();
}

void updateGCRemTime (int time, int lock)
{
	char temp[100];
	int hour, min, sec;

	if(lock) gdk_threads_enter();
	sec = time % 60; min = (time / 60) % 60; hour = time / 3600;
	sprintf(temp, "%02d:%02d:%02d", hour, min, sec);
	gtk_label_set (GTK_LABEL (RemTime), temp);
	if(lock) gdk_threads_leave();
}

void updateExLine (int GCL, int lock)
{
	char temp[100];

	if(lock) gdk_threads_enter();
	sprintf(temp, "%d", GCL);
	gtk_label_set (GTK_LABEL (lineh), temp);
	if(lock) gdk_threads_leave();
}

void updateGCLineT (char *text)
{
	gdk_threads_enter();
	gtk_label_set (GTK_LABEL (gclineh), text);
	gdk_threads_leave();
}

float feedFactor = 1.0;
float oldFeedFactor = 1.0;

void setFeedFactor (float ff)
{
  feedFactor = ff;
}

float getFeedFactor (void)
{
  return feedFactor;
}

void updateDro (float tX, float tY, float tZ, float tA, float tF, float tV, int bitmap)
{
	char temp[120];

	gdk_threads_enter();

	// only update the fields that have changed
	if (bitmap & 1)
	{
		sprintf(temp, "%07.3f", tX);
		gtk_label_set (GTK_LABEL (droX), temp);
	}
	if (bitmap & 2)
	{
		sprintf(temp, "%07.3f", tY);
		gtk_label_set (GTK_LABEL (droY), temp);
	}
	if (bitmap & 4)
	{
		sprintf(temp, "%07.3f", tZ);
		gtk_label_set (GTK_LABEL (droZ), temp);
	}
	if (bitmap & 8)
	{
		sprintf(temp, "%07.3f", tA);
		gtk_label_set (GTK_LABEL (droA), temp);
	}
	if ((bitmap & 16) || (feedFactor != oldFeedFactor))
	{
		sprintf (temp, "%5.0f/%3.1f", tF, feedFactor);
		gtk_label_set (GTK_LABEL (feedh), temp);
    oldFeedFactor = feedFactor;
	}
	if (bitmap & 32)
	{
		sprintf (temp, "%5.1f", tV);
		gtk_label_set (GTK_LABEL (velocityh), temp);
	}

	gdk_threads_leave();
}

void uiUpdate (void)
{
    static pthread_t	win_update;

	// create thread to sending of gcode files and leveling
	pthread_create (&win_update, NULL, send_gcode, NULL);
}
