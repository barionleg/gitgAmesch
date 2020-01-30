#ifndef MESHGLCOLORS_H
#define MESHGLCOLORS_H

#include <QtOpenGL/QtOpenGL>

class MeshGLColors {
	public:
		MeshGLColors();

		// Color configuration:
		enum eColorSettings{
			COLOR_MESH_SOLID,         //!< Solid Color.
			COLOR_MESH_BACKFACE,      //!< Color of the backfaces.
			COLOR_VERTEX_MONO,        //!< Color of the vertices, when shown in one color.
			COLOR_VERTEX_LOCAL_MIN,   //!< Color of vertices tagged as local minimum.
			COLOR_VERTEX_LOCAL_MAX,   //!< Color of vertices tagged as local minimum.
			//! \todo COLOR_VERTEX_LOCAL_MAX is also used for selected vertices -> seperate!
			COLOR_EDGE_MONO,          //!< Color of the edges, when shown in one color.
			//! \todo COLOR_EDGE_MONO is NOT used -> pass as uEdgeColor (existing!) to the shader!
			COLOR_POLYLINE_MONO,      //!< Color of the polylines, when shown in one color.
			//! \todo COLOR_POLYLINE_MONO is not used!
			COLOR_LABEL_NOT_ASSIGNED, //!< Monochrome color for unlabled areas.
			COLOR_LABEL_SOLID,        //!< Monochrome color for labels as alternative to color per label.
			COLOR_LABEL_BORDER,       //!< Monochrome color for border between labels (not between label and "no label").
			COLOR_LABEL_BACKGROUND,   //!< Monochrome color the areas labeled as background.

			COLOR_NPR_DIFFUSE1,       //!< Color of the 1st brightest diffuse NPR value
			COLOR_NPR_DIFFUSE2,       //!< Color of the 2nd brightest diffuse NPR value
			COLOR_NPR_DIFFUSE3,       //!< Color of the 3rd brightest diffuse NPR value
			COLOR_NPR_DIFFUSE4,       //!< Color of the 4th brightest diffuse NPR value
			COLOR_NPR_DIFFUSE5,       //!< Color of the 5th brightest diffuse NPR value
			COLOR_NPR_SPECULAR,       //!< Color of the specular NPR value
			COLOR_NPR_OUTLINE,        //!< Color of the NPR outlines
			COLOR_NPR_HATCHLINE,      //!< Color of the NPR hatchlines
			COLOR_SETTING_COUNT       //!< Number of colormaps for OpenGL display.
		};

		bool getColorSettings( eColorSettings rColorId, GLfloat* rVec4 );
		bool setColorSettings(eColorSettings rColorId, const GLfloat* iVec4 );
	protected:
		GLubyte mColorSetting[COLOR_SETTING_COUNT][4]; //!< Selected colors.
};

#endif // MESHGLCOLORS_H
