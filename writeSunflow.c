/*
======================================================================
writeSunflow.c

Functions for exporting a Sunflow scene file.
====================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "objectdb.h"
#include "vecmat.h"
#include "com_math.h"
#include "com_transform.h"
#include "lay_transform.h"


/*
======================================================================
writeSunflowCamera()
====================================================================== */
int writeSunflowCamera( FILE *fp, LWItemInfo *iteminfo, LWCameraInfo *caminfo, LWSceneInfo *scninfo, LWItemID item, LWTime t )
{
	LWFMatrix4 M;
	double hfov, vfov, aspect;

	LWMAT_identity4( M );
	LWMAT_getItemTransform(iteminfo,item,t,M);
	caminfo->fovAngles(item, t, &hfov, &vfov);
	aspect = (double)(scninfo->frameWidth)/(double)(scninfo->frameHeight);
	
	fprintf( fp, "camera {\n");
	fprintf( fp, "\ttype pinhole\n");
	fprintf( fp, "\ttransform col %f %f %f %f %f %f %f %f %f %f -%f %f %f %f %f %f\n",// reverse Z direction
            M[0][0],M[0][1],M[0][2],M[0][3],
			M[1][0],M[1][1],M[1][2],M[1][3],
			M[2][0],M[2][1],M[2][2],M[2][3],
			M[3][0],M[3][1],M[3][2],M[3][3]);
	fprintf( fp, "\tfov    %f\n", DEGREES(hfov));
	fprintf( fp, "\taspect %f\n", aspect);
	//fprintf( fp, "\tfdist  320\n");
	//fprintf( fp, "\tlensr  9\n");
	fprintf( fp, "}\n\n" );

	return 1;
}

/*
======================================================================
writeSunflowLight()

LWLIGHT_DISTANT = 0
LWLIGHT_POINT	= 1
LWLIGHT_SPOT	= 2
LWLIGHT_LINEAR	= 3
LWLIGHT_AREA	= 4
====================================================================== */
int writeSunflowLight(FILE *fp, LWItemInfo *iteminfo, LWLightInfo *lightinfo, LWItemID item, LWTime t)
{
	LWDVector wpos;
	LWDVector rawcolor;
	LWFVector lightDir = {0.0f,0.0f,1.0f};
	LWFVector lightFinal;
	LWFVector areaVertices[4] = {{-0.8f,0.8f,0.0f},{0.8f,0.8f,0.0f},{0.8f,-0.8f,0.0f},{-0.8f,-0.8f,0.0f}};
	LWFMatrix4 M,S,MS;
	double radius, edge;
	float power, intensity;
	int i, samples, type;

	LWMAT_identity4( M );
	LWMAT_identity4( S );
	LWMAT_identity4( MS );
	type = lightinfo->type(item);
	samples = 4;
	
	switch(type) {
		case 0: //distant light
			LWMAT_getRotations(iteminfo, item, t, M);
			LWMAT_transformp( lightDir, M, lightFinal);
			fprintf( fp, "light {\n");
			fprintf( fp, "\ttype sunsky\n");
			fprintf( fp, "\tup 0 1 0\n");
			fprintf( fp, "\teast 1 0 0\n");
			fprintf( fp, "\tsundir %f %f %f\n", -1*(lightFinal[0]), -1*(lightFinal[1]), -1*(lightFinal[2]));
			fprintf( fp, "\tturbidity 2\n");
			fprintf( fp, "\tsamples 16\n");
			fprintf( fp, "}\n\n" );
			break;

		case 1: //point light
			LWMAT_getWorldPos(iteminfo, item, t, wpos);
			lightinfo->rawColor(item, t, rawcolor);
			power = lightinfo->intensity(item, t)*100;
			fprintf( fp, "light {\n");
			fprintf( fp, "\ttype point\n");
			fprintf( fp, "\tcolor %f %f %f\n", rawcolor[0], rawcolor[1], rawcolor[2]);
			fprintf( fp, "\tpower %f \n", power);
			fprintf( fp, "\tp %f %f %f\n", wpos[0], wpos[1], wpos[2]);
			fprintf( fp, "}\n\n" );
			break;

		case 2: //spotlight  ###Sunflow directional light is a cylinder, Lightwave spotlight is a cone
			LWMAT_getWorldPos(iteminfo, item, t, wpos);
			LWMAT_getRotations(iteminfo, item, t, M);
			LWMAT_getScales(iteminfo, item, t, S);
			LWMAT_transformp( lightDir, M, lightFinal);
			VADD(lightFinal,wpos);
			lightinfo->coneAngles( item, t, &radius, &edge );
			lightinfo->rawColor(item, t, rawcolor);
			intensity = lightinfo->intensity(item, t);
			fprintf( fp, "light {\n");
			fprintf( fp, "\ttype directional\n");
			fprintf( fp, "\tsource %f %f %f\n", wpos[0], wpos[1], wpos[2]);
			fprintf( fp, "\ttarget %f %f %f\n", lightFinal[0], lightFinal[1], lightFinal[2]);
			//fprintf( fp, "\tradius %f \n", DEGREES(radius));
			fprintf( fp, "\tradius %f \n", 10.0); //hardcode 10 until there is a Lightwave style cone light
			//fprintf( fp, "\temit { \"sRGB nonlinear\" %f %f %f }\n", rawcolor[0], rawcolor[1], rawcolor[2]);
			fprintf( fp, "\temit %f %f %f\n", rawcolor[0], rawcolor[1], rawcolor[2]);
			fprintf( fp, "\tintensity %f \n", intensity);
			fprintf( fp, "}\n\n" );
			break;

		//case 3: linear light not implemented
		//	break;

		case 4: //area light
			LWMAT_getWorldPos(iteminfo, item, t, wpos);
			LWMAT_getRotations(iteminfo, item, t, M);
			LWMAT_getScales(iteminfo, item, t, S);
			LWMAT_matmul4(M,S,MS);

			lightinfo->rawColor(item, t, rawcolor);
			intensity = lightinfo->intensity(item, t);
			fprintf( fp, "light {\n");
			fprintf( fp, "\ttype meshlight\n");
			fprintf( fp, "\tname \"area_light\"\n");
			fprintf( fp, "\temit %f %f %f \n", rawcolor[0], rawcolor[1], rawcolor[2]);
			fprintf( fp, "\tradiance %f \n", intensity);
			fprintf( fp, "\tsamples %d \n", samples);	
			fprintf( fp, "\tpoints %d\n", 4 );
			for ( i = 0; i < 4; i++ )
			{
				LWMAT_transformp( areaVertices[i], MS, lightFinal);
				VADD(lightFinal,wpos);
				
				fprintf( fp, "\t\t%g %g %g\n", lightFinal[0], lightFinal[1], lightFinal[2]);
			}
			fprintf( fp, "\ttriangles %d\n", 2 );
			fprintf( fp, "\t\t %d %d %d\n", 2, 1, 0 );
			fprintf( fp, "\t\t %d %d %d\n", 3, 2, 0 );
			fprintf( fp, "\t}\n\n" );
			break;
	}
	return 1;
}

/*
======================================================================
writeSunflowImage()
====================================================================== */
int writeSunflowImage(FILE *fp, LWCameraInfo *caminfo, LWItemID camid)
{
	int width, height;

	caminfo->resolution(camid, &width, &height);

	fprintf( fp, "image {\n");
	fprintf( fp, "\tresolution %d %d\n", width, height);
	fprintf( fp, "\taa 1 1\n");
	fprintf( fp, "\tsamples 4\n");
	fprintf( fp, "\tfilter gaussian\n");
	fprintf( fp, "}\n\n" );

	return 1;
}

/*
======================================================================
writeSunflowGroundPlane()
====================================================================== */
int writeSunflowGroundPlane(FILE *fp)
{
	fprintf( fp,	"shader {\n");
	fprintf( fp,	"\tname ground.shader\n");
	fprintf( fp,	"\ttype diffuse\n");
	fprintf( fp,	"\tdiff 0.5 0.5 0.5\n");
	fprintf( fp,	"\t}\n\n");

	fprintf( fp,	"object {\n");
	fprintf( fp,	"\tshader ground.shader\n");
	fprintf( fp,	"\ttype plane\n");
	fprintf( fp,	"\tp 0 0 0\n");
	fprintf( fp,	"\tn 0 1 0\n");
	fprintf( fp,	"\t}\n\n");

	return 1;
}

/*
======================================================================
writeSunflowShaders()
====================================================================== */
int writeSunflowShaders(FILE *fp)
{
	fprintf( fp,	"shader {\n");
	fprintf( fp,	"\tname default\n");
	fprintf( fp,	"\ttype diffuse\n");
	fprintf( fp,	"\tdiff { \"sRGB nonlinear\" 0.7 0.7 0.7 }\n");
	fprintf( fp,	"\t}\n\n");

	return 1;
}

/*
======================================================================
writeSunflowObject()

Export a Lightwave scene to a Sunflow scene file.

e.g.

object {
	shader lambert1
	transform col 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1
	type generic-mesh
	name "test"
	points 3
		0 0 0
		3 4 -2
		5 2 0
	triangles 1
		0 1 2
	normals none
	uvs none
	face_shaders
		0
}

The c argument is 0 for initial normals and point positions
and 1 for final.
====================================================================== */
int writeSunflowObject( ObjectDB *odb, FILE *fp, int c, LWItemInfo *iteminfo, LWItemID item, LWTime t )
{
	int i, j;
	LWFMatrix4 M;

	LWMAT_identity4( M );
	LWMAT_getItemTransform(iteminfo,item,t,M);

	fprintf( fp, "object {\n");
	fprintf( fp, "\tshader default\n");
	fprintf( fp, "\ttransform col %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", 
             M[0][0],M[0][1],M[0][2],M[0][3],
			 M[1][0],M[1][1],M[1][2],M[1][3],
			 M[2][0],M[2][1],M[2][2],M[2][3],
			 M[3][0],M[3][1],M[3][2],M[3][3]);
	fprintf( fp, "\ttype generic-mesh\n");
	fprintf( fp, "\tname \"%s\"\n",iteminfo->name(item));
   
	//Sunflow points
	fprintf( fp, "\tpoints %d\n", odb->npoints );
	for ( i = 0; i < odb->npoints; i++ )
	{
		fprintf( fp, "\t\t%g %g %g\n",
				odb->pt[ i ].pos[ c ][ 0 ],
				odb->pt[ i ].pos[ c ][ 1 ],
				odb->pt[ i ].pos[ c ][ 2 ]);
	}

	//Sunflow triangles
	fprintf( fp, "\ttriangles %d\n", odb->npolygons );
	for ( i = 0; i < odb->npolygons; i++ )
	{
		fprintf( fp, "\t\t");
		for ( j = 0; j < odb->pol[ i ].nverts; j++ )
		{
			fprintf( fp, "%d ", odb->pol[ i ].v[ j ].index);
		}
		fprintf( fp, "\n");
	}

	//There are three types of Sunflow normals; face, vertex and facevarying.  Only facevarying is implemented.
	fprintf( fp, "\tnormals facevarying\n" );
	for ( i = 0; i < odb->npolygons; i++ )
	{
		fprintf( fp, "\t\t");
		for ( j = 0; j < odb->pol[ i ].nverts; j++ )
		{
			fprintf( fp, "%g %g %g ",
			odb->pol[ i ].v[ j ].norm[ c ][ 0 ],
			odb->pol[ i ].v[ j ].norm[ c ][ 1 ],
			odb->pol[ i ].v[ j ].norm[ c ][ 2 ] );
		}
		fprintf( fp, "\n");
	}

	//Sunflow UVs
	fprintf( fp, "\tuvs none\n" );

	//Close the sunflow object
	fprintf( fp, "}\n\n" );

	return 1;
}
