/*
======================================================================
SunLight.c

Modified heavily from scenescan by Ernie Wright
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <lwhost.h>
#include <lwgeneric.h>
#include <lwdisplce.h>
#include <lwpanel.h>
#include "objectdb.h"
#include "vecmat.h"
#include "writeSunflow.h"
#include "image1.h"

#define PAN_WIDTH 322
#define PAN_HEIGHT 105
#define PAN_BORDER 10

typedef struct st_UserData {
	LWMessageFuncs *msg;
	LWPanelFuncs *panf;
	LWItemInfo *iteminfo;
	LWPanelID panel;
	LWObjectInfo *objinfo;
	LWSceneInfo *scninfo;
	LWInterfaceInfo *ui;
	LWCameraInfo *caminfo;
	LWLightInfo *lightinfo;
	LWControl *ctl[ 3 ];
	LWLayoutGeneric *local;
} UserData;

//Globals

//Prototypes
XCALL_( int )SunLightEvent( LWControl *ctrl, UserData *userData );

/*
======================================================================
DrawSunLightLogo()

Based on the tutorial by Nik Lever; also thanks for his tga->.h converter
Draws the plugin graphic.
====================================================================== */

void DrawSunLightLogo(LWPanelID panel, UserData *userData, DrMode dm)
{
    int x=0;
	int y=0;
	int i=0;
	int j=0;
	int index=0;
	int w=0;

	DrawFuncs *df;

	// Get panel width
    userData->panf->get(panel,PAN_W,&w);
	df=userData->panf->drawFuncs;
    index = 0;        

	/////////////////////////////////////////////////////////////////////
	//Depending on which app created the image used to generate image1.h,
	//we may need to flip the order in which we write y.
	/////////////////////////////////////////////////////////////////////
	//y = IMAGE1HEIGHT + PAN_BORDER;
	y = PAN_BORDER;
	for(j=0;j<IMAGE1HEIGHT;j++){
        x = (w-IMAGE1WIDTH)/2;
        for(i=0;i<IMAGE1WIDTH;i++){
            df->drawRGBPixel(panel,image1_data[index],
                 image1_data[index+1],image1_data[index+2],x++,y);       
			index+=3;
        }
		y++;
	}

	//Draw logo border lines
	df->drawLine(panel,COLOR_DK_GREY,PAN_BORDER,PAN_BORDER,PAN_WIDTH+PAN_BORDER,PAN_BORDER);
	df->drawLine(panel,COLOR_DK_GREY,PAN_BORDER,PAN_BORDER,PAN_BORDER,PAN_HEIGHT+PAN_BORDER);
	df->drawLine(panel,LWP_HILIGHT,PAN_WIDTH+PAN_BORDER,PAN_BORDER,PAN_WIDTH+PAN_BORDER,PAN_HEIGHT+PAN_BORDER);	
	df->drawLine(panel,LWP_HILIGHT,PAN_BORDER,PAN_HEIGHT+PAN_BORDER,PAN_WIDTH+PAN_BORDER,PAN_HEIGHT+PAN_BORDER);
}



/*
======================================================================
get_panel()

Create the main panel.
====================================================================== */

static LWPanelID get_panel( LWLayoutGeneric *local, UserData *userData )
{
   LWPanControlDesc desc;
   LWPanelID panel;
   LWValue ival = { LWT_INTEGER };
   //LWControl *ctl[ 3 ];
   int i, w, numControls;

   numControls = 1;

   if( !( panel = PAN_CREATE( userData->panf, "SunLight v0.1.2 BETA" )))
      return NULL;

   PAN_SETW( userData->panf, panel, PAN_WIDTH + 20);
  
   userData->ctl[ 0 ] = WBUTTON_CTL( userData->panf, panel, "Export Now!", PAN_WIDTH );
   MOVE_CON(userData->ctl[0],10,PAN_HEIGHT+10);
   CON_SETEVENT( userData->ctl[ 0 ], SunLightEvent, userData);

   //ctl[ 1 ] = FVEC_CTL( panf, panel, "Gravity");
   //SET_FVEC(ctl[ 1 ], gravity[0], gravity[1], gravity[2]);

   //ctl[ 2 ] = INT_CTL( panf, panel, "Step Size Multipier");
   //SET_INT(ctl[ 2 ], stepSizeMultiplier);


   // align controls
   for ( i = 1; i < numControls; i++ ) {
      w = CON_LW( userData->ctl[ i ] );
      ival.intv.value = 100 - w;
      userData->ctl[ i ]->set( userData->ctl[ i ], CTL_X, &ival );
   }

   return panel;
}


/*
======================================================================
SunLightGeneric()

The ODE activation function.
====================================================================== */

XCALL_( int )
SunLightGeneric( long version, GlobalFunc *global, LWLayoutGeneric *local, void *serverData )
{
	int ok;
	LWPanelID panel;
	UserData *userData;

	userData = (UserData*)malloc(sizeof(UserData));

	if ( version != LWLAYOUTGENERIC_VERSION )
		return AFUNC_BADVERSION;

	///////////////////////////////////////////////
	// get the message, panels and other functions
	///////////////////////////////////////////////
	userData->msg = global( LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT );
	userData->panf = global( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
	userData->objinfo = global( LWOBJECTINFO_GLOBAL, GFUSE_TRANSIENT );
	userData->iteminfo = global( LWITEMINFO_GLOBAL, GFUSE_TRANSIENT );
	userData->scninfo = global( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT );
	userData->ui = global( LWINTERFACEINFO_GLOBAL, GFUSE_TRANSIENT );
	userData->caminfo = global( LWCAMERAINFO_GLOBAL, GFUSE_TRANSIENT );
	userData->lightinfo = global( LWLIGHTINFO_GLOBAL, GFUSE_TRANSIENT );

	if ( !userData->msg ||
		 !userData->panf ||
		 !userData->objinfo ||
		 !userData->iteminfo ||
		 !userData->scninfo ||
		 !userData->ui ||
		 !userData->caminfo ||
		 !userData->lightinfo )
		return AFUNC_BADGLOBAL;

	// initialize the panels functions and create a panel
	userData->panf->globalFun = global;
	userData->local = local;

	panel = get_panel(local, userData);
	if ( !panel ) return AFUNC_BADGLOBAL;

	//Draw logo defined in image1.h
	PAN_SETDATA( userData->panf, panel, userData );
	PAN_SETDRAW( userData->panf, panel, DrawSunLightLogo );

	ok = userData->panf->open( panel, PANF_BLOCKING );

	if ( !ok ) return AFUNC_OK; // do nothing and quit

	//GET_FVEC( ctl[1], gravity[0], gravity[1], gravity[2] );
	//GET_INT ( ctl[2], stepSizeMultiplier );

	// free the panel
	PAN_KILL( userData->panf, panel );

	return AFUNC_OK;
}

/*
======================================================================
lwcommand()

Issue a LayoutGeneric command.  This just saves us from using a lot of
sprintf() calls in code that issues commands.  If the command doesn't
take any arguments, "fmt" is NULL.  Otherwise, it contains a printf
formatting string, and the arguments that follow it are printf
arguments.
====================================================================== */

static int lwcommand( LWLayoutGeneric *gen, const char *cmdname,
   const char *fmt, ... )
{
   static char cmd[ 256 ], arg[ 256 ];

   if ( fmt ) {
      va_list ap;
      va_start( ap, fmt );
      vsprintf( arg, fmt, ap );
      va_end( ap );
      sprintf( cmd, "%s %s", cmdname, arg );
      return gen->evaluate( gen->data, cmd );
   }
   else
      return gen->evaluate( gen->data, cmdname );
}


/*
======================================================================
SunLightEvent()

Activation function for the generic.  This creates and prints an
object database for every object in the scene.  It uses the GoToFrame
command to step through each frame and get the final point positions.
====================================================================== */

XCALL_( int )
SunLightEvent( LWControl *ctrl, UserData *userData )
{
	LWMeshInfo *mesh;
	LWItemID objid,camid,lightid;
	ObjectDB *odb;
	FILE *fp;
	int j, bbt, bbt0, result;
	char outFileName[128];
	const char *LWSName;
	char LWSBaseName[128];
	LWFrame cf, frameStart,frameEnd;
	LWTime t, timeStart, timeEnd;
	double fps;



	/* make sure there are objects in the scene */
	if ( userData->scninfo->numPoints <= 0 ) {
		userData->msg->error( "No objects in the scene.", NULL );
		return AFUNC_OK;
	}

	memset(LWSBaseName,'\0',128);
	LWSName = userData->scninfo->name;
	strncpy(LWSBaseName,LWSName,(strlen(LWSName)-4));

	frameStart = userData->scninfo->frameStart;
	frameEnd = userData->scninfo->frameEnd;
	fps = userData->scninfo->framesPerSecond;
	timeStart = frameStart * ( 1 / fps);
	timeEnd = frameEnd * ( 1 / fps);
	cf = frameStart;

	for ( t = timeStart; t <= timeEnd; t=t+(1/fps) ) {
   
		sprintf(outFileName,"%s%s%04d%s",LWSBaseName,".",cf,".sc");
		
		/* open the output file */
		fp = fopen( outFileName, "w" );
		if ( !fp ) {
			userData->msg->error( "Couldn't open output file", outFileName );
			return AFUNC_OK;
		}

		camid = userData->iteminfo->first( LWI_CAMERA, 0 );
		//lightid = userData->iteminfo->first( LWI_LIGHT, 0 );

		result = writeSunflowCamera(fp, userData->iteminfo, userData->caminfo, userData->scninfo, camid, t);
		result = writeSunflowImage(fp, userData->caminfo, camid);
		//result = writeSunflowLight(fp, userData->iteminfo, userData->lightinfo, lightid, t, 1);
		result = writeSunflowGroundPlane(fp);
		result = writeSunflowShaders(fp);


		lightid = userData->iteminfo->first( LWI_LIGHT, 0 );
		/* for each light */
		while ( lightid ) {			
			//int type;
			//type = userData->lightinfo->type(lightid);
			//result = writeSunflowLight(fp, userData->iteminfo, userData->lightinfo, lightid, t, type);
			result = writeSunflowLight(fp, userData->iteminfo, userData->lightinfo, lightid, t);
			lightid = userData->iteminfo->next( lightid );
		}

		//lwcommand( userData->local, "EditObjects", NULL );

		/* We need to make sure the Bounding Box Threshold is set high enough
			to force Layout to transform all of the points of every object at
			each frame.  If we need to set it here, we'll restore it later. */

		bbt0 = userData->ui->boxThreshold;

		objid = userData->iteminfo->first( LWI_OBJECT, 0 );
		
		bbt = 0;
		while ( objid ) {
			mesh = userData->objinfo->meshInfo( objid, 1 );
			if ( mesh ) {
				j = mesh->numPoints( mesh );
				if ( j > bbt ) bbt = j;
				if ( mesh->destroy )
					mesh->destroy( mesh );
			}
			objid = userData->iteminfo->next( objid );
		}
		++bbt;

		if ( bbt > bbt0 )
			lwcommand( userData->local, "BoundingBoxThreshold", "%d", bbt );

		lwcommand( userData->local, "GoToFrame", "%d", cf );
		lwcommand( userData->local, "RefreshNow", NULL );

		objid = userData->iteminfo->first( LWI_OBJECT, 0 );

		/* for each object */
		while ( objid ) {

			/* skip nulls */
			if ( 0 >= userData->objinfo->numPolygons( objid )) {
				objid = userData->iteminfo->next( objid );
				continue;
			}

			/* get object data */
			odb = getObjectDB( objid, userData->panf->globalFun );
			if ( !odb ) {
				userData->msg->error( "Couldn't alloc object database for",
				userData->iteminfo->name( objid ));
				return AFUNC_OK;
			}

			//write the object data to file
			result = writeSunflowObject(odb, fp, 1, userData->iteminfo, objid, t);// 0 for no deformations

			freeObjectDB( odb );
			objid = userData->iteminfo->next( objid );
		}
		fclose( fp );
		cf++; //increment the frame
	}

   /* restore the user's Bounding Box Threshold setting */
   if ( bbt > bbt0 )
      lwcommand( userData->local, "BoundingBoxThreshold", "%d", bbt0 );

   //test: render with sunflow - working
   //system("java -Xmx1G -jar \"c:\\Program Files\\Sunflow\\sunflow.jar\" -ipr \"c:\\Program Files\\Sunflow\\bendy.0005.sc\"");

   return AFUNC_OK;
}


/* stuff for the displacement handler */
typedef struct {
   GlobalFunc *global;
   LWItemID    objid;
   char        name[ 80 ];
   ObjectDB   *odb;
   int         index;
   LWFrame     frame;
} MyData;


/*
======================================================================
Create()

DisplacementHandler callback.  Allocate and initialize instance data.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, LWItemID item, LWError *err )
{
   MyData *dat;
   LWItemInfo *iteminfo;

   if ( dat = calloc( 1, sizeof( MyData ))) {
      dat->objid = item;
      dat->global = ( GlobalFunc * ) priv;
      if ( iteminfo = dat->global( LWITEMINFO_GLOBAL, GFUSE_TRANSIENT ))
         strcpy( dat->name, iteminfo->name( item ));
   }

   return dat;
}


/*
======================================================================
Destroy()

Handler callback.  Free the instance.
====================================================================== */

XCALL_( static void )
Destroy( MyData *dat )
{
   if( dat ) {
      if ( dat->odb ) freeObjectDB( dat->odb );
      free( dat );
   }
}


/*
======================================================================
Copy()

Handler callback.  Copy instance data.
====================================================================== */

XCALL_( static LWError )
Copy( MyData *to, MyData *from )
{
   to->objid = from->objid;
   return NULL;
}


/*
======================================================================
Describe()

Handler callback.  Write a short, human-readable string describing
the instance data.
====================================================================== */

XCALL_( static const char * )
Describe( MyData *dat )
{
   return "SunLight, the free SunFlow exporter";
}


/*
======================================================================
ChangeID()

Handler callback.  An item ID has changed.
====================================================================== */

XCALL_( void )
ChangeID( MyData *dat, const LWItemID *id )
{
   int i;

   for ( i = 0; id[ i ]; i += 2 )
      if ( id[ i ] == dat->objid ) {
         dat->objid = id[ i + 1 ];
         break;
      }
}


/*
======================================================================
Init()

Handler callback, called at the start of rendering.
====================================================================== */

XCALL_( static LWError )
Init( MyData *dat, int mode )
{
   return NULL;
}


/*
======================================================================
Cleanup()

Handler callback, called at the end of rendering.
====================================================================== */

XCALL_( static void )
Cleanup( MyData *dat )
{
   if ( dat->odb ) {
      freeObjectDB( dat->odb );
      dat->odb = NULL;
   }
   return;
}


/*
======================================================================
NewTime()

Handler callback, called at the start of each sampling pass.  The
world coordinate positions of the points haven't been calculated yet,
so we have to wait until Evaluate() to get those.  If we allocated an
ObjectDB in a previous Evaluate(), we free it here.
====================================================================== */

XCALL_( static LWError )
NewTime( MyData *dat, LWFrame fr, LWTime t )
{
   if ( dat->odb ) {
      freeObjectDB( dat->odb );
      dat->odb = NULL;
   }

   dat->odb = getObjectDB( dat->objid, dat->global );
   if ( dat->odb ) {
      freeObjectDB( dat->odb );
      dat->odb = NULL;
   }

   dat->index = 0;
   dat->frame = fr;
   return NULL;
}


/*
======================================================================
Flags()

Handler callback.
====================================================================== */

XCALL_( static int )
Flags( MyData *dat )
{
   return LWDMF_WORLD;
}


/*
======================================================================
Evaluate()

Handler callback.  Called for each point in the object.

If this is the first call since newTime() was called, we allocate an
ObjectDB.  Then we look up the point and store its current position.
====================================================================== */

XCALL_( static void )
Evaluate( MyData *dat, LWDisplacementAccess *da )
{
   int i;

   if ( dat->index == 0 ) {
      dat->odb = getObjectDB( dat->objid, dat->global );

      if ( dat->odb ) {
         FILE *fp;
         char buf[ 80 ];
         int i, len;

         sprintf( buf, "%s%03d.txt", dat->name, dat->frame );
         len = (int)strlen( buf );
         for ( i = 0; i < len; i++ )
            if ( buf[ i ] == ':' ) buf[ i ] = '_';
         if ( fp = fopen( buf, "w" )) {
            printObjectDB( dat->odb, fp, 0 );
            fclose( fp );
         }
      }
   }

   ++dat->index;

   if ( dat->odb ) {
      i = findVert( dat->odb, da->point );
      if ( i != -1 ) {
         dat->odb->pt[ i ].pos[ 1 ][ 0 ] = ( float ) da->source[ 0 ];
         dat->odb->pt[ i ].pos[ 1 ][ 1 ] = ( float ) da->source[ 1 ];
         dat->odb->pt[ i ].pos[ 1 ][ 2 ] = ( float ) da->source[ 2 ];
      }
   }
}


/*
======================================================================
ODBDisplace()

Handler activation function.
====================================================================== */

XCALL_( static int )
ODBDisplace( long version, GlobalFunc *global, LWDisplacementHandler *local,
   void *serverData)
{
   if ( version != LWDISPLACEMENT_VERSION ) return AFUNC_BADVERSION;

   local->inst->priv     = global;
   local->inst->create   = Create;
   local->inst->destroy  = Destroy;
   local->inst->copy     = Copy;
   local->inst->descln   = Describe;
   local->item->changeID = ChangeID;
   local->rend->init     = Init;
   local->rend->cleanup  = Cleanup;
   local->rend->newTime  = NewTime;
   local->evaluate       = Evaluate;
   local->flags          = Flags;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWLAYOUTGENERIC_CLASS, "SunLight", SunLightGeneric },
//   { LWDISPLACEMENT_HCLASS, "Object_Database", ODBDisplace },
   { NULL }
};
