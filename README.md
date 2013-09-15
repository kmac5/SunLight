SunLight - SunFlow Exporter for LightWave
=========================================

SunLight v0.1.2 Beta - 03/05/07
-Compiles for LightWave versions 8.5 and 9.0
-Camera and light positions are now exported
-Four of the five LightWave light types are eported to the nearest SunFlow
 equivalents:

	LightWave		SunFlow
	---------		-------
	Distant		->	Sun/Sky
	Point		->	Point
	Spot		->	Directional
	Area		->	Meshlight

	-The exported lights are not a 1:1 conversion:
	-LightWave spot lights are cones, SunFlow directional lights are columns
	-LightWave area lights do not render and emit light from both sides,
	 SunFlow meshlights render as geometry and emit light in the direction of
	 its surface normals.

Fixed Bugs:
-Z axis was reversed on the camera

Note: .sc files are saved to the project folder current at export time


SunLight v0.1.1 Beta - 2/18/07
Limitations of the current build:
-Single light source currently hard coded
-Does not currently export camera motions, only initial position and rotation
-All objects currently exported with simple default shader
-All objects must be frozen and tripled prior to export
-Name of exported .sc files not yet user definable
-hard coded xz ground plane

Fixed Bugs:
-Camera Heading rotation +/- reversed

Known Bugs:
-Occasional funky exported scene names
-Does not properly export deformations (like ODEfL)

Other:
-Image size and aspect ratio are calculated from camera resolution
-Camera fov is LightWave's horizontal fov
