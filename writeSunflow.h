//prototypes

int writeSunflowCamera( FILE *fp, LWItemInfo *iteminfo, LWCameraInfo *caminfo, LWSceneInfo *scninfo, LWItemID item, LWTime t );
int writeSunflowObject( ObjectDB *odb, FILE *fp, int c, LWItemInfo *iteminfo, LWItemID item, LWTime t );
int writeSunflowImage( FILE *fp, LWCameraInfo *caminfo, LWItemID camid);
//int writeSunflowLight(FILE *fp, LWItemInfo *iteminfo, LWLightInfo *lightinfo, LWItemID item, LWTime t, int type);
int writeSunflowLight(FILE *fp, LWItemInfo *iteminfo, LWLightInfo *lightinfo, LWItemID item, LWTime t);
int writeSunflowGroundPlane(FILE *fp);
int writeSunflowShaders(FILE *fp);